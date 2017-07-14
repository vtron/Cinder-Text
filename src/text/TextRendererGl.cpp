#include "text/TextRendererGl.h"

#include "cinder/gl/gl.h"
#include "cinder/gl/scoped.h"
#include "cinder/GeomIo.h"

#include "text/FontManager.h"

namespace txt
{
	RendererGl::RendererGl()
	{
	}

	void RendererGl::drawString( const  Font& font, std::string string, ci::vec2 frame )
	{
		Layout layout;
		layout.setSize( frame );
		layout.calculateLayout( font, string );
		drawLayout( layout );
	}

	void RendererGl::drawLayout( Layout& layout )
	{
		for( auto& line : layout.getLines() ) {
			for( auto& run : line.runs ) {
				for( auto& glyph : run.glyphs ) {
					ci::gl::ScopedMatrices matrices;

					FT_BitmapGlyph ftGlyph = txt::FontManager::get()->getGlyphBitmap( run.font, glyph.index );

					ci::gl::translate( glyph.bbox.getUpperLeft() + ci::vec2( ftGlyph->left, -ftGlyph->top ) );
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
		ci::ChannelRef channel = ci::Channel::create( glyph->bitmap.width, glyph->bitmap.rows, glyph->bitmap.width * sizeof( unsigned char ), 1, glyph->bitmap.buffer );

		ci::gl::Texture::Format format;
		format.setSwizzleMask( std::array<GLint, 4> {GL_ONE, GL_ONE, GL_ONE, GL_RED } );
		mGlyphTextures[font][glyphIndex] = ci::gl::Texture::create( *channel, format );


		//ci::gl::Texture3d::Format format;
		//format.setTarget( GL_TEXTURE_2D_ARRAY );
		//ci::gl::Texture3dRef fontTex = ci::gl::Texture3d::create( 1, 1, ( GLint )numGlyphs, format );
	}
}