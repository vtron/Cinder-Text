#pragma once

#include <memory>
#include <vector>

#include <cinder/Vector.h>
#include <cinder/Rect.h>

#include "txt/Font.h"
#include "txt/FontManager.h"
#include "txt/AttributedString.h"
#include "txt/TextUnits.h"
#include "txt/Shaper.h"

namespace txt
{
	typedef enum Alignment { LEFT, CENTER, RIGHT, JUSTIFIED } Alignment;
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
				int width;
				int getTotalGlyphs()
				{
					int totalGlyphs = 0;

					for( auto& run : runs ) {
						totalGlyphs += run.glyphs.size();
					}

					return totalGlyphs;
				}

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

			float getLineHeight() { return mLineHeight.getValue( getFont().getSize() ); }
			Layout& setLineHeight( const float& lineHeight ) { mLineHeight = txt::Unit( lineHeight ); return *this; };
			Layout& setLineHeight( const Unit& lineHeight ) { mLineHeight = lineHeight; return *this; };

			float getTracking() { return mTracking.getValue( getFont().getSize() ); }
			Layout& setTracking( float tracking ) { mTracking = txt::Unit( tracking ); return *this; };
			Layout& setTracking( const Unit& tracking ) { mTracking = tracking; return *this; };

			std::string getLanguage() { return mLanguage; }
			Layout& setLanguage( std::string language ) { mLanguage = language; return *this; }

			hb_script_t getScript() { return mScript; }
			Layout& setScript( hb_script_t script ) { mScript = script; return *this; }

			hb_direction_t getDirection() { return mDirection; }
			Layout& setDirection( hb_direction_t direction )
			{
				mDirection = direction;

				// If alignment isn't manually set, default to LEFT for LTR and RIGHT for RTL
				if( mUseDefaultAlignment ) {
					mAlignment = mDirection == HB_DIRECTION_LTR ? Alignment::LEFT : Alignment::RIGHT;
				}

				return *this;
			}

			const Alignment& getAlignment() { return mAlignment; }
			Layout& setAlignment( Alignment alignment )
			{
				mAlignment = alignment;
				mUseDefaultAlignment = false;
				return *this;
			};

		private:
			// Attributes
			Font mFont;
			ci::Color mColor;
			Alignment mAlignment;
			bool mUseDefaultAlignment;
			txt::Unit mLineHeight;
			txt::Unit mTracking;

			std::string mLanguage;
			hb_script_t mScript;
			hb_direction_t mDirection;

			ci::vec2 mSize;
			ci::vec2 mLayoutSize;

			// Layout calculation
			void resetLayout();

			// Line breaking
			struct BreakIndices {
				int textBreakIndex = -1;
				int glyphBreakIndex = -1;
				bool found = false;
			};
			BreakIndices getClosestBreakForShapedText( int startIndex, const std::vector<Shaper::Glyph>& shapedGlyphs, const std::vector<uint8_t> lineBreaks, const hb_direction_t& direction );

			ci::vec2 mCurDirection;
			float mCharPos, mLinePos;
			Line mCurLine;
			float mCurLineWidth = 0;
			float mCurLineHeight = 0;

			float getLineHeightForSubstring( const AttributedString::Substring& substring, const Font& runFont );

			void addSubstringToCurLine( AttributedString::Substring& substring );
			void addRunToCurLine( Run& run );
			void addCurLine();
			void applyAlignment();
			std::vector<Line> mLines;

			bool mMaxLinesReached = false;
	};
}