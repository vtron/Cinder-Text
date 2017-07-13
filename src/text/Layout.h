#pragma once

#include <memory>
#include <vector>

#include <cinder/Vector.h>

#include "text/Types.h"
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

			struct Run {
				Run( const Font& font )
					: font( font )
				{};

				const Font font;

				std::vector<Glyph> glyphs;

			};

			struct Line {
				std::vector<Run> runs;
			};

			Layout();

			void calculateLayout( const Font& font, std::string text, ci::vec2 size );

			std::vector<Line>& getLines()
			{
				return mLines;
			};

			void setLeading( float leading )
			{
				mLeading = leading;
			};
			void setTracking( float tracking )
			{
				mTracking = tracking;
			};

		private:
			float mLeading;
			float mTracking;
			std::vector<Line> mLines;
	};
}