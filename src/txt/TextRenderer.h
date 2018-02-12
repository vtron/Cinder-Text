#pragma once

#include <string>
#include <cinder/Vector.h>

#include "txt/TextLayout.h"

namespace txt
{
	class Renderer
	{
		public:
			Renderer() { }

			virtual void draw( const std::string& text, const ci::vec2& size = ci::vec2( 0 ) ) = 0;
			virtual void draw( const std::string& text, const Font& font, const ci::vec2 size = ci::vec2( 0 ) ) = 0;
			virtual void draw( const Layout& layout ) = 0;

			virtual void preloadFont( const Font& font ) = 0;

		protected:
			virtual void drawGlyph( const Font& font, unsigned int glyphIndex ) = 0;
	};
}