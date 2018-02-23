#include "txt/gl/TextureRenderer.h"

#include "cinder/gl/gl.h"
#include "cinder/gl/scoped.h"
#include "cinder/GeomIo.h"
#include "cinder/ip/Fill.h"
#include "cinder/ip/Flip.h"

#include "txt/FontManager.h"

namespace txt
{
	namespace gl
	{
		// Shared font cache
		std::unordered_map<Font, TextureRenderer::FontCache> TextureRenderer::mFontCaches;

		// Shader
		const char* vertShader = R"V0G0N(
		#version 150

		uniform mat4	ciModelViewProjection;

		in vec4		ciPosition;
		in vec2		ciTexCoord0;
		in vec4		ciColor;

		out highp vec2 texCoord;
		out vec4 globalColor;

		void main( void )
		{
			texCoord = ciTexCoord0;
			globalColor = ciColor;
			gl_Position	= ciModelViewProjection * ciPosition;
		}
	)V0G0N";

		const char* fragShader = R"V0G0N(
		#version 150

		uniform sampler2DArray uTexArray;
		uniform uint uLayer;
		uniform vec2 uSubTexSize;

		in vec2 texCoord;
		in vec4 globalColor;

		uniform vec4 runColor;

		out vec4 color;

		void main( void )
		{ 
			vec3 coord = vec3(texCoord.x * uSubTexSize.x, texCoord.y * uSubTexSize.y, uLayer);
			vec4 texColor = texture( uTexArray, coord );

			color = vec4(1.0, 1.0, 1.0, texColor.r);
			color = color * globalColor;
			//color = vec4(1.0,0.0,0.0,1.0);
		}
	)V0G0N";

		TextureRenderer::TextureRenderer()
		{
			ci::gl::GlslProgRef shader = ci::gl::GlslProg::create( vertShader, fragShader );
			shader->uniform( "uTexArray", 0 );

			if( mBatch == nullptr ) {
				mBatch = ci::gl::Batch::create( ci::geom::Rect( ci::Rectf( 0.f, 0.f, 1.f, 1.f ) ), shader );
			}
		}

		void TextureRenderer::setLayout( const Layout& layout )
		{
			mLayout = layout;
			allocateFbo( std::max( mLayout.measure().x, mLayout.measure().y ) );
			renderToFbo();
		}

		ci::gl::TextureRef TextureRenderer::getTexture()
		{
			return mFbo->getColorTexture();
		}

		void TextureRenderer::allocateFbo( int size )
		{
			if( mFbo == nullptr || mFbo->getWidth() < size || mFbo->getHeight() < size ) {
				// Go up by pow2 until we get the new size
				int fboSize = 1;

				while( fboSize < size ) {
					fboSize *= 2;
				}

				// Allocate
				ci::gl::Fbo::Format fboFormat;
				fboFormat.setColorTextureFormat( ci::gl::Texture2d::Format().internalFormat( GL_RGBA32F ) );
				fboFormat.setSamples( 1 );

				mFbo = ci::gl::Fbo::create( fboSize, fboSize, fboFormat );
			}
		}

		void TextureRenderer::renderToFbo()
		{
			if( mFbo ) {
				// Set viewport
				ci::gl::ScopedViewport viewportScope( 0, 0, mFbo->getWidth(), mFbo->getHeight() );
				ci::gl::ScopedMatrices matricesScope;
				ci::gl::setMatricesWindow( mFbo->getSize(), true );

				// Draw text into FBO
				ci::gl::ScopedFramebuffer fboScoped( mFbo );
				ci::gl::clear( ci::ColorA( 0.0, 0.0, 0.0, 0.0 ) );

				ci::gl::ScopedBlendAlpha alpha;

				for( auto& line : mLayout.getLines() ) {
					for( auto& run : line.runs ) {
						ci::gl::color( ci::ColorA( run.color, run.opacity ) );

						for( auto& glyph : run.glyphs ) {
							// Make sure we have the glyph
							if( TextureRenderer::getCacheForFont( run.font ).glyphs.count( glyph.index ) != 0 ) {
								ci::gl::ScopedMatrices matrices;

								ci::gl::translate( ci::vec2( glyph.bbox.getUpperLeft() ) );
								ci::gl::scale( glyph.bbox.getSize().x, glyph.bbox.getSize().y );

								//ci::gl::ScopedBlendAlpha alphaBlend;
								mBatch->getGlslProg()->uniform( "uLayer", getCacheForFont( run.font ).glyphs[glyph.index].layer );

								ci::gl::Texture3dRef tex = getCacheForFont( run.font ).glyphs[glyph.index].texArray;

								//ci::vec2 subTexSize = glyph.bbox.getSize() / ci::vec2( tex->getWidth(), tex->getHeight() );
								mBatch->getGlslProg()->uniform( "uSubTexSize", getCacheForFont( run.font ).glyphs[glyph.index].subTexSize );

								ci::gl::ScopedTextureBind texBind( tex, 0 );
								mBatch->draw();
							}
							else {
								//ci::app::console() << "Could not find glyph for index: " << glyph.index << std::endl;
							}
						}
					}
				}
			}
		}

		//void TextureRenderer::draw( const std::string& string, const ci::vec2& frame )
		//{
		//	draw( string, DefaultFont(), frame );
		//}

		//void TextureRenderer::draw( const std::string& string, const Font& font, ci::vec2 frame )
		//{
		//	Layout layout;
		//	layout.setSize( frame );
		//	layout.setFont( font );
		//	layout.calculateLayout( string );
		//	draw( layout );
		//}

		void TextureRenderer::draw()
		{
			if( mFbo ) {
				ci::gl::ScopedBlendPremult blend;
				ci::gl::draw( mFbo->getColorTexture() );
			}
		}

		void TextureRenderer::loadFont( const Font& font )
		{
			if( !mFontCaches.count( font ) ) {
				TextureRenderer::cacheFont( font );
			}
		}

		void TextureRenderer::unloadFont( const Font& font )
		{
			TextureRenderer::uncacheFont( font );
		}

		TextureRenderer::FontCache& TextureRenderer::getCacheForFont( const Font& font )
		{
			if( !mFontCaches.count( font ) ) {
				TextureRenderer::cacheFont( font );
			}

			return mFontCaches[font];
		}

		// Cache glyphs to gl texture array(s)
		void TextureRenderer::cacheFont( const Font& font )
		{
			// Determine the max number of layers for texture arrays on platform
			GLint maxLayersPerArray;
			glGetIntegerv( GL_MAX_ARRAY_TEXTURE_LAYERS, &maxLayersPerArray );

			// Get the total number of glyphs
			std::vector<uint32_t> glyphIndices = txt::FontManager::get()->getGlyphIndices( font );
			unsigned int numGlyphs = glyphIndices.size();
			//numGlyphs = txt::FontManager::get()->getNumGlyphs( font );

			// Calculate max glyph size and pad out to 4 bytes
			ci::ivec2 maxGlyphSize = txt::FontManager::get()->getMaxGlyphSize( font );
			ci::ivec2 padding = ci::ivec2( 4 ) - ( maxGlyphSize % ci::ivec2( 4 ) );
			maxGlyphSize += padding;

			ci::gl::Texture3d::Format format;
			format.setTarget( GL_TEXTURE_2D_ARRAY );
			format.setInternalFormat( GL_RED );

			//format.setSwizzleMask( std::array<GLint, 4> {GL_ONE, GL_ONE, GL_ONE, GL_RED } );
			ci::gl::Texture3dRef curTexArray = nullptr;

			unsigned int curLayer = 0;
			unsigned int totalLayers = 0;

			// Go through each glyph and cache
			for( auto& glyphIndex : txt::FontManager::get()->getGlyphIndices( font ) ) {

				// Check to see if we need a new texture
				if( !curTexArray || curLayer >= maxLayersPerArray ) {
					unsigned int distFromTotal = numGlyphs - totalLayers;

					unsigned int numLayers = distFromTotal >= maxLayersPerArray ? maxLayersPerArray : distFromTotal;
					curTexArray = ci::gl::Texture3d::create( maxGlyphSize.x, maxGlyphSize.y, numLayers, format );

					curLayer = 0;
				}

				// Add the glyph to our cur tex array
				FT_BitmapGlyph glyph = txt::FontManager::get()->getGlyphBitmap( font, glyphIndex );
				ci::ivec2 glyphSize( glyph->bitmap.width, glyph->bitmap.rows );
				ci::ChannelRef channel = ci::Channel::create( glyphSize.x, glyphSize.y, glyphSize.x * sizeof( unsigned char ), sizeof( unsigned char ), glyph->bitmap.buffer );
				ci::ChannelRef flippedChannel = ci::Channel::create( glyphSize.x, glyphSize.y );
				ci::ip::flipVertical( *channel, flippedChannel.get() );
				ci::Channel8uRef expandedChannel = ci::Channel8u::create( maxGlyphSize.x, maxGlyphSize.y );
				ci::ip::fill( expandedChannel.get(), ( uint8_t )0 );
				expandedChannel->copyFrom( *flippedChannel, ci::Area( 0, 0, glyphSize.x, glyphSize.y ) );

				ci::Surface8u surface( *expandedChannel );
				curTexArray->update( surface, curLayer );

				mFontCaches[font].glyphs[glyphIndex].texArray = curTexArray;
				mFontCaches[font].glyphs[glyphIndex].layer = curLayer;
				mFontCaches[font].glyphs[glyphIndex].subTexSize = ci::vec2( glyphSize ) / ci::vec2( maxGlyphSize );

				curLayer++;
				totalLayers = totalLayers + 1;
			}
		}

		void TextureRenderer::uncacheFont( const Font& font )
		{
			if( !mFontCaches.count( font ) ) {
				std::unordered_map<Font, FontCache>::iterator it = mFontCaches.find( font );
				mFontCaches.erase( it );
			}
		}
	}
}