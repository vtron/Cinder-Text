#pragma once

#include <unordered_map>

#include "cinder/gl/Texture.h"
#include "cinder/gl/Batch.h"

#include "txt/Layout.h"

namespace txt
{
	class RendererGl
	{
		public:
			RendererGl();

			void drawString( const  Font& font, std::string string, ci::vec2 frame = ci::vec2( 0.f ) );
			void drawAttrString( const Font& font, std::string string, ci::vec2 frame = ci::vec2( 0.f ) );
			void drawLayout( Layout& layout );

		private:
			void drawGlyph( const Font& font, unsigned int glyphIndex );

			ci::gl::TextureRef getGlyphTexture( const Font& font, unsigned int glyphIndex );

			void cacheGlyphAsTexture( const Font& font, uint32_t glyphIndex );
			std::unordered_map < Font, std::unordered_map<uint32_t, ci::gl::TextureRef > > mGlyphTextures;
	};
}