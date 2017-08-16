#include "txt/TextRendererGl.h"

#include "cinder/gl/gl.h"
#include "cinder/gl/scoped.h"
#include "cinder/GeomIo.h"

#include "txt/FontManager.h"

namespace txt
{
	RendererGl::RendererGl()
	{
	}

	void RendererGl::draw( const std::string& string, const ci::vec2& frame )
	{
		draw( string, DefaultFont(), frame );
	}

	void RendererGl::draw( const std::string& string, const Font& font, ci::vec2 frame )
	{
		Layout layout;
		layout.setSize( frame );
		layout.calculateLayout( string, font );
		draw( layout );
	}

	void RendererGl::draw( const Layout& layout )
	{
		ci::gl::ScopedBlendAlpha alpha;
		for( auto& line : layout.getLines() ) {
			for( auto& run : line.runs ) {

				ci::gl::color( run.color );

				for( auto& glyph : run.glyphs ) {
					ci::gl::ScopedMatrices matrices;

					FT_BitmapGlyph ftGlyph = txt::FontManager::get()->getGlyphBitmap( run.font, glyph.index );

					ci::gl::translate( ci::ivec2(glyph.bbox.getUpperLeft()) + ci::ivec2( ftGlyph->left, -ftGlyph->top ) );
					drawGlyph( run.font, glyph.index );
				}
			}
		}
	}

	void RendererGl::drawGlyph( const Font& font, uint32_t glyphIndex )
	{
		ci::gl::draw( getGlyphTexture( font, glyphIndex ) );
	}

	//ci::gl::Texture3dRef RendererGl::getTexture3dForFont( Font& font )
	//{
	//	uint32_t faceId = ( uint32_t )font.faceId;

	//	if( mFontTextures.count( faceId ) == 0 || mFontTextures[faceId].count( font.size ) ) {
	//		cacheFont( font );
	//	}

	//	return mFontTextures[faceId][font.size];
	//}

	ci::gl::TextureRef RendererGl::getGlyphTexture( const  Font& font, unsigned int glyphIndex )
	{
		// Check to see if we have the font
		if( mGlyphTextures.count( font ) == 0 || mGlyphTextures[font].count( glyphIndex ) ) {
			cacheGlyphAsTexture( font, glyphIndex );
		}

		return mGlyphTextures[font][glyphIndex];
	}

	void RendererGl::cacheGlyphAsTexture( const  Font& font, uint32_t glyphIndex )
	{
		FT_BitmapGlyph glyph = txt::FontManager::get()->getGlyphBitmap( font, glyphIndex );
		ci::ChannelRef channel = ci::Channel::create( glyph->bitmap.width, glyph->bitmap.rows, glyph->bitmap.width * sizeof( unsigned char ), sizeof(unsigned char), glyph->bitmap.buffer );

		ci::gl::Texture::Format format;
		format.setSwizzleMask( std::array<GLint, 4> {GL_ONE, GL_ONE, GL_ONE, GL_RED } );
		mGlyphTextures[font][glyphIndex] = ci::gl::Texture::create( *channel, format );


		//ci::gl::Texture3d::Format format;
		//format.setTarget( GL_TEXTURE_2D_ARRAY );
		//ci::gl::Texture3dRef fontTex = ci::gl::Texture3d::create( 1, 1, ( GLint )numGlyphs, format );
	}
}