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
			typedef struct {
				uint32_t index;
				ci::Rectf bbox;
				unsigned int top;
			} Glyph;

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

			struct Line {
				std::vector<Run> runs;
			};

			Layout();

			void calculateLayout( std::string text, const Font& font = DefaultFont(), const ci::Color& color = ci::ColorA( 1.f, 1.f, 1.f, 1.f ) );
			void calculateLayout( const AttributedString& attrString );

			ci::vec2 getSize();

			void setSize( ci::vec2 size ) { mSize = size; }
			void setLeading( float leading ) { mLeading = leading; };
			void setTracking( float tracking ) { mTracking = tracking;};
			void setAlignment( Alignment alignment ) { mAlignment = alignment; };


			const std::vector<Line>& getLines() const { return mLines; };
			std::vector<Line>& getLines() { return mLines; };

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