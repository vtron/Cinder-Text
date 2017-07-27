#include "text/Layout.h"

#include "harfbuzz/hb.h"

#include "cinder/app/App.h"
#include <cinder/Unicode.h>

#include "text/FontManager.h"
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
		mAscender = 0.f;
		mLineWidth = 0.f;
	}

	void Layout::calculateLayout( const Font& font, std::string text )
	{
		reset();

		Parser parser;
		parser.parseAttr( font, text );
		std::deque<Parser::Substring> substrings = parser.getSubstrings();

		for( int i = 0; i < substrings.size(); i++ ) {
			Parser::Substring remainingSubstring = substrings[i];

			while( remainingSubstring.text.size() ) {
				// Check for height clipping
				// TODO: This needs to handle vertical layouts (clip width)
				if( mSize.y != GROW && mPen.y + mLineHeight > mSize.y ) {
					//Clip in Y Direction
					return;
				}

				addSubstringToCurLine( remainingSubstring );
			}
		}

		addCurLine();
	}

	void Layout::addSubstringToCurLine( Parser::Substring& substring )
	{
		const Font runFont = FontManager::get()->getFont( substring.attributes.fontFamily, substring.attributes.fontStyle, substring.attributes.fontSize );
		const ci::Color runColor = substring.attributes.color;

		Shaper shaper( runFont );
		Shaper::Text shaperText = {
			substring.text,
			"en",
			HB_SCRIPT_LATIN,
			HB_DIRECTION_LTR
		};

		std::vector<Shaper::Glyph> shapedGlyphs = shaper.getShapedText( shaperText );

		// Get the line height + ascender for this run
		float lineHeight = FontManager::get()->getSize( runFont )->metrics.height / 64.f;
		float ascender = FontManager::get()->getSize( runFont )->metrics.ascender / 64.f;

		// Increase our current line height + ascender if this run is taller
		int prevLineHeight = mLineHeight;
		int prevAscender = mAscender;
		mLineHeight = std::max( lineHeight, mLineHeight );
		mAscender = std::max( ascender, mAscender );

		// Create a run to store our glyphs
		Run run( runFont, runColor );

		// Track current word for line breaks (need to remove and switch to unicode breaking)
		std::vector<Glyph> curWord;

		for( int i = 0; i < shapedGlyphs.size(); i++ ) {
			// Add the offset (generally 0 for latin) to the pen pos
			ci::vec2 pos = mPen + shapedGlyphs[i].offset;

			// Get the glyph metrics/position
			ci::vec2 advance = shapedGlyphs[i].advance;
			ci::Rectf glyphBBox( pos, pos + ci::vec2( advance.x, mPen.y ) );

			// Create a layout glyph and add to current word
			Layout::Glyph glyph = { shapedGlyphs[i].index, glyphBBox };

			// Move the pen forward
			mPen.x += advance.x + mTracking;

			// Check for a new line
			// TODO: Right to left + vertical
			if( mSize.x != GROW && mPen.x > mSize.x ) {
				// Add the current run to the line
				if( !run.glyphs.empty() ) {
					addRunToCurLine( run );
					run.glyphs.clear();
				}
				else {
					mLineHeight = prevLineHeight;
					mAscender = prevAscender;
				}

				addCurLine();

				// Clip the substrings text by what we've already added and return
				int remainingGlyphStart = i - ( int )curWord.size();
				substring.text = substring.text.substr( remainingGlyphStart, std::string::npos );
				return;
			}

			// Add glyph to current word
			curWord.push_back( glyph );

			// If we found whitespace push the current word into the run
			if( isWhitespace( run.font, shapedGlyphs[i].index ) ) {
				run.glyphs.insert( run.glyphs.end(), curWord.begin(), curWord.end() );
				curWord.clear();
			}
		}

		// Once we've gone through everything add the remaining word
		// (Only reached if we haven't filled a line)
		run.glyphs.insert( run.glyphs.end(), curWord.begin(), curWord.end() );
		addRunToCurLine( run );

		substring.text.clear();

		// If the substring requests a line break push to next line
		if( substring.forceBreak ) {
			addCurLine();
		}
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
				glyph.bbox.offset( ci::vec2( 0.f, mAscender ) );
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

		// Setup next line
		mCurLine = Line();

		mPen.x = 0.f;
		mPen.y += mLineHeight + mLeading;

		mLineWidth = 0;
		mLineHeight = 0.f;
		mAscender = 0.f;
	}
}