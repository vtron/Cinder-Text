#include "txt/TextRendererGl.h"

#include "cinder/gl/gl.h"
#include "cinder/gl/scoped.h"
#include "cinder/GeomIo.h"
#include "cinder/ip/Fill.h"

#include "txt/FontManager.h"

namespace txt
{
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

		out vec4 color;

		void main( void )
		{ 
			vec3 coord = vec3(texCoord.x * uSubTexSize.x, texCoord.y * uSubTexSize.y, uLayer);
			vec4 texColor = texture( uTexArray, coord );

			color = vec4(1.0, 1.0, 1.0, texColor.r);
			color = color * globalColor;
		}
	)V0G0N";
	RendererGl::RendererGl()
	{
		ci::gl::GlslProgRef shader = ci::gl::GlslProg::create( vertShader, fragShader );
		//ci::gl::GlslProgRef shader = ci::gl::GlslProg::create( ci::app::loadAsset( "shaders/shader.vert" ), ci::app::loadAsset( "shaders/shader.frag" ) );
		shader->uniform( "uTexArray", 0 );

		mBatch = ci::gl::Batch::create( ci::geom::Rect( ci::Rectf( 0.f, 0.f, 1.f, 1.f ) ), shader );
	}


	void RendererGl::draw( const std::string& string, const ci::vec2& frame )
	{
		draw( string, DefaultFont(), frame );
	}

	void RendererGl::draw( const std::string& string, const Font& font, ci::vec2 frame )
	{
		Layout layout;
		layout.setSize( frame );
		layout.setFont( font );
		layout.calculateLayout( string );
		draw( layout );
	}

	void RendererGl::draw( const Layout& layout )
	{
		ci::gl::ScopedBlendAlpha alpha;

		for( auto& line : layout.getLines() ) {
			for( auto& run : line.runs ) {
				ci::gl::color( ci::ColorA( run.color, run.opacity ) );

				for( auto& glyph : run.glyphs ) {
					// Make sure we have the glyph
					if( getFontCache( run.font ).glyphs.count( glyph.index ) != 0 ) {
						ci::gl::ScopedMatrices matrices;

						ci::gl::translate( ci::vec2( glyph.bbox.getLowerLeft() ) );
						ci::gl::scale( glyph.bbox.getSize().x, -glyph.bbox.getSize().y );

						ci::gl::ScopedBlendAlpha alphaBlend;
						mBatch->getGlslProg()->uniform( "uLayer", getFontCache( run.font ).glyphs[glyph.index].layer );

						ci::gl::Texture3dRef tex = getFontCache( run.font ).glyphs[glyph.index].texArray;

						//ci::vec2 subTexSize = glyph.bbox.getSize() / ci::vec2( tex->getWidth(), tex->getHeight() );
						mBatch->getGlslProg()->uniform( "uSubTexSize", getFontCache( run.font ).glyphs[glyph.index].subTexSize );

						ci::gl::ScopedTextureBind texBind( tex, 0 );
						mBatch->draw();
					}
					else {
						ci::app::console() << "Could not find glyph for index: " << glyph.index << std::endl;
					}
				}
			}
		}
	}

	RendererGl::FontCache& RendererGl::getFontCache( const Font& font )
	{
		if( !mFontCaches.count( font ) ) {
			cacheFont( font );
		}

		return mFontCaches[font];
	}

	// Cache glyphs to gl texture array(s)
	void RendererGl::cacheFont( const Font& font )
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
			ci::Channel8uRef expandedChannel = ci::Channel8u::create( maxGlyphSize.x, maxGlyphSize.y );
			ci::ip::fill( expandedChannel.get(), ( uint8_t )0 );
			expandedChannel->copyFrom( *channel, ci::Area( 0, 0, glyphSize.x, glyphSize.y ) );

			ci::Surface8u surface( *expandedChannel );
			curTexArray->update( surface, curLayer );

			//curTexArray->update( glyph->bitmap.buffer, GL_RED, GL_UNSIGNED_BYTE, 0, glyph->bitmap.width, glyph->bitmap.rows, 1, 0, 0, curLayer );

			mFontCaches[font].glyphs[glyphIndex].texArray = curTexArray;
			mFontCaches[font].glyphs[glyphIndex].layer = curLayer;
			mFontCaches[font].glyphs[glyphIndex].subTexSize = ci::vec2( glyphSize ) / ci::vec2( maxGlyphSize );

			curLayer++;
			totalLayers++;
		}
	}
}