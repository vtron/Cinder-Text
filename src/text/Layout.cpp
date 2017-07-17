#include "text/Layout.h"

#include "harfbuzz/hb.h"

#include "cinder/app/App.h"
#include <cinder/Unicode.h>

#include "text/FontManager.h"
#include "text/Parser.h"
#include "text/Shaper.h"

namespace txt
{
	bool isWhitespace( const Font& font, int codepoint )
	{
		FT_UInt spaceIndex = FontManager::get()->getGlyphIndex( font, ' ' );
		return codepoint == spaceIndex;
	}

	bool isNewline( Font& font, int codepoint )
	{
		FT_UInt newLineIndex = FontManager::get()->getGlyphIndex( font, '\u000A' );
		return newLineIndex == codepoint;
	}

	Layout::Layout()
		: mTracking( 0 )
		, mLeading( 0 )
		, mAlignment( Alignment::LEFT )
		, mSize( GROW )
	{
	}

	void Layout::reset()
	{
		mLines.clear();
		mPen = ci::vec2( 0.f );
		mCurLine = Line();
		mLineHeight = 0.f;
		mLineWidth = 0.f;
	}

	void Layout::calculateLayout( const  Font& font, std::string text )
	{
		reset();

		Parser parser( font, text );
		std::deque<Parser::Substring> substrings = parser.getSubstrings();

		for( int i = 0; i < substrings.size(); ++i ) {
			Shaper shaper( font );

			Shaper::Text shaperText = {
				substrings[i].text,
				"en",
				HB_SCRIPT_LATIN,
				HB_DIRECTION_LTR
			};

			std::vector<Shaper::Glyph> shapedGlyphs = shaper.getShapedText( shaperText );

			Run curRun( font );
			std::vector<Glyph> curWord;
			float lineHeight = FontManager::get()->getSize( font )->metrics.height / 64.f;

			if( lineHeight > mLineHeight ) {
				mLineHeight = lineHeight;
			}

			for( int j = 0; j < shapedGlyphs.size(); j++ ) {
				ci::vec2 pos = mPen + shapedGlyphs[j].offset;
				ci::vec2 advance = shapedGlyphs[j].advance;
				ci::Rectf glyphBBox( pos, pos + ci::vec2( advance.x, mPen.y ) );

				Layout::Glyph glyph = { shapedGlyphs[j].index, glyphBBox };
				curWord.push_back( glyph );

				mPen.x += advance.x + mTracking;

				// Check for new line
				if( mSize.x != GROW && mPen.x > mSize.x ) {

					if( !curRun.glyphs.empty() ) {
						addRunToCurLine( curRun );
						curRun.glyphs.clear();
					}

					addCurLine();


					if( mSize.y != GROW && mPen.y + mLineHeight > mSize.y ) {
						//Clip in Y Direction
						return;
					}

					j -= curWord.size();
					curWord.clear();
				}
				else if( isWhitespace( font, shapedGlyphs[j].index ) ) {
					curRun.glyphs.insert( curRun.glyphs.end(), curWord.begin(), curWord.end() );
					curWord.clear();
				}
			}

			curRun.glyphs.insert( curRun.glyphs.end(), curWord.begin(), curWord.end() );
			addRunToCurLine( curRun );
			curRun.glyphs.clear();

			if( substrings[i].forceBreak ) {
				addCurLine();
			}
		}

		addCurLine();
	}

	void Layout::addRunToCurLine( Run& run )
	{
		mCurLine.runs.push_back( run );
		mLineWidth = run.glyphs.back().bbox.x2;
	}
	void Layout::addCurLine( )
	{
		// Set the Y glyph position based on culmulative line-height
		for( auto& run : mCurLine.runs ) {
			for( auto& glyph : run.glyphs ) {
				glyph.bbox.offset( ci::vec2( 0.f, mLineHeight ) );
			}
		}

		// Align in frame (if necessary)
		if( mSize.x != GROW ) {
			switch( mAlignment ) {
				case LEFT:
					break;

				case CENTER:
				case RIGHT:
					float remainingWidth = mSize.x - mLineWidth;

					float xOffset = ( mAlignment == CENTER ) ? remainingWidth / 2.f : remainingWidth;

					for( auto& run : mCurLine.runs ) {
						for( auto& glyph : run.glyphs ) {
							glyph.bbox.offset( ci::vec2( xOffset, 0.f ) );
						}
					}

					break;
			}
		}

		// Add it to our lines
		mLines.push_back( mCurLine );

		// Reset
		mCurLine = Line();
		mLineWidth = 0;

		mPen.x = 0.f;
		mPen.y += mLineHeight + mLeading;
	}
}