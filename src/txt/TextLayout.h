#pragma once

#include <memory>
#include <vector>

#include <cinder/Vector.h>
#include <cinder/Rect.h>

#include "txt/Font.h"
#include "txt/FontManager.h"
#include "txt/AttributedString.h"

namespace txt
{
	class Layout
	{
		public:
			typedef struct {
				uint32_t index;
				ci::Rectf bbox;
			} Glyph;

			struct Run {
				Run( const Font& font, const ci::Color& color )
					: font( font )
					, color( color )
				{};

				const Font font;
				const ci::ColorA color;

				std::vector<Glyph> glyphs;
			};

			struct Line {
				std::vector<Run> runs;
			};

			typedef enum Alignment { LEFT, CENTER, RIGHT } Alignment;
			enum { GROW = 0 };

			Layout();

			void calculateLayout( std::string text, const Font& font = DefaultFont() );
			void calculateLayout( const AttributedString& attrString );

			const std::vector<Line>& getLines() const { return mLines; };

			void setSize( ci::vec2 size ) { mSize = size; }
			void setLeading( float leading ) { mLeading = leading; };
			void setTracking( float tracking ) { mTracking = tracking;};
			void setAlignment( Alignment alignment ) { mAlignment = alignment; };

		private:
			int mLeading;
			int mTracking;
			Alignment mAlignment;

			void reset();
			ci::vec2 mPen;
			Line mCurLine;
			float mLineWidth = 0;
			float mLineHeight = 0;
			float mLineLeading = 0;
			float mAscender = 0;

			ci::vec2 mSize;

			void addSubstringToCurLine( AttributedString::Substring& substring );
			void addRunToCurLine( Run& run );
			void addCurLine( );
			std::vector<Line> mLines;

			bool mMaxLinesReached = false;
	};
}