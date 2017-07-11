#pragma once

#include <memory>
#include <vector>

#include <cinder/Vector.h>

#include "text/Font.h"

namespace txt
{
	struct Glyph {
		unsigned int index;
		ci::vec2 pos;
	};

	struct Run {
		Run( Font& font )
			: font( font )
		{};

		Font font;
		std::vector<Glyph> glyphs;
	};

	struct Line {
		std::vector<Run> runs;
	};

	class Layout
	{
		public:
			Layout( Font& font, std::string text );

			std::vector<Line>& getLines()
			{
				return mLines;
			};

		private:
			std::vector<Line> mLines;
	};
}