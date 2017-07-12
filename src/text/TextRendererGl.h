#pragma once

#include <unordered_map>

#include "cinder/gl/Texture.h"
#include "cinder/gl/Batch.h"

#include "text/Layout.h"

namespace txt
{
	class RendererGl
	{
		public:
			RendererGl();

			void drawString( Font& font, std::string string );
			void drawLayout( Layout& layout );

		private:
			void drawGlyph( Font& font, unsigned int glyphIndex );

			ci::gl::TextureRef getGlyphTexture( Font& font, unsigned int glyphIndex );

			void cacheGlyphAsTexture( Font& font, uint32_t glyphIndex );
			std::unordered_map < Font, std::unordered_map<uint32_t, ci::gl::TextureRef > > mGlyphTextures;
	};
}