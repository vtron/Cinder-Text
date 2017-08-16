#pragma once

#include <unordered_map>

#include "cinder/gl/Texture.h"
#include "cinder/gl/Batch.h"

#include "txt/TextLayout.h"
#include "txt/TextRenderer.h"

namespace txt
{
	class RendererGl
		: public txt::Renderer
	{
		public:
			RendererGl();

			virtual void draw( const std::string& text, const ci::vec2& size = ci::vec2( 0 ) );
			virtual void draw( const std::string& text, const Font& font, const ci::vec2 size = ci::vec2( 0 ) );
			virtual void draw( const Layout& layout );

		private:
			void drawGlyph( const Font& font, unsigned int glyphIndex );

			ci::gl::TextureRef getGlyphTexture( const Font& font, unsigned int glyphIndex );

			void cacheGlyphAsTexture( const Font& font, uint32_t glyphIndex );
			std::unordered_map < Font, std::unordered_map<uint32_t, ci::gl::TextureRef > > mGlyphTextures;
	};
}