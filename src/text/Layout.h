#pragma once

#include <memory>
#include <vector>

#include <cinder/Vector.h>

#include "text/Font.h"
#include "text/FontManager.h"

namespace txt
{


	class Layout
	{
		public:
			typedef struct {
				uint32_t index;

				ci::vec2 pos;
			} Glyph;

			typedef struct {
				Font font;
				std::vector<Glyph> glyphs;
			} Run;

			struct Line {
				std::vector<Run> runs;
			};

			Layout() {};
			Layout( Font& font, std::string text, ci::vec2 size = ci::vec2( 0 ) );

			std::vector<Line>& getLines()
			{
				return mLines;
			};

		private:
			std::vector<Line> mLines;
	};
}