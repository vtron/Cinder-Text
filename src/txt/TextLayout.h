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
	typedef enum Alignment { LEFT, CENTER, RIGHT } Alignment;
	enum { GROW = 0 };

	class Layout
	{
		public:
			// A single character
			typedef struct {
				uint32_t index;
				ci::Rectf bbox;
				unsigned int top;
				std::string value;
			} Glyph;

			// A group of characters with the same attributes
			struct Run {
				Run( const Font& font, const ci::Color& color, const float& opacity )
					: font( font )
					, color( color )
					, opacity( opacity )
				{};

				Font font;
				ci::Color color;
				float opacity;

				std::vector<Glyph> glyphs;
			};

			// A line of runs fit within the layout
			struct Line {
				std::vector<Run> runs;
			};

			Layout();

			// Layout Calculation
			void calculateLayout( std::string text );
			void calculateLayout( const AttributedString& attrString );

			// Lines
			const std::vector<Line>& getLines() const { return mLines; };
			std::vector<Line>& getLines() { return mLines; };

			// Layout Attributes
			const Font& getFont() { return mFont; }
			Layout& setFont( const Font& font ) { mFont = font; return *this; }

			const ci::Color& getColor() { return mColor; }
			Layout& setColor( const ci::Color& color ) { mColor = color; }

			const ci::vec2& getSize() { return mSize; }
			Layout& setSize( ci::vec2 size ) { mSize = size; return *this; }

			const ci::vec2 measure();

			float getLeading() { return mLeading; }
			Layout& setLeading( float leading ) { mLeading = leading; return *this; };

			float getTracking() { return mTracking; }
			Layout& setTracking( float tracking ) { mTracking = tracking; return *this; };

			const Alignment& getAlignment() { return mAlignment; }
			Layout& setAlignment( Alignment alignment ) { mAlignment = alignment; return *this; };

		private:
			// Attributes
			Font mFont;
			ci::Color mColor;
			float mLeading;
			float mTracking;
			Alignment mAlignment;

			ci::vec2 mSize;
			ci::vec2 mLayoutSize;

			// Layout calculation
			void resetLayout();

			float mCharPos, mLinePos;
			Line mCurLine;
			float mLineWidth = 0;
			float mLineHeight = 0;
			float mLineLeading = 0;

			void addSubstringToCurLine( AttributedString::Substring& substring );
			void addRunToCurLine( Run& run );
			void addCurLine( );
			std::vector<Line> mLines;

			bool mMaxLinesReached = false;
	};
}