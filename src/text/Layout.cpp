#include "text/Layout.h"

#include "harfbuzz/hb.h"

#include "cinder/app/App.h"

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
	{
	}

	void Layout::calculateLayout( const  Font& font, std::string text, ci::vec2 size )
	{
		Parser parser( font, text );
		std::deque<Parser::Substring> substrings = parser.getSubstrings();

		ci::vec2 pen( 0 );
		Line curLine;

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

			for( int j = 0; j < shapedGlyphs.size(); j++ ) {
				ci::vec2 pos = pen + shapedGlyphs[j].offset;

				Layout::Glyph glyph = { shapedGlyphs[j].index, pos };
				curWord.push_back( glyph );

				pen.x += shapedGlyphs[j].advance.x + mTracking;

				// Check for new line
				if( size.x != 0 && pen.x > size.x ) {
					pen.x = 0.f;
					pen.y += FontManager::get()->getSize( font )->metrics.height / 64.f + mLeading;

					if( !curRun.glyphs.empty() ) {
						curLine.runs.push_back( curRun );
						curRun.glyphs.clear();
					}

					mLines.push_back( curLine );
					curLine = Line();

					j -= curWord.size();
					curWord.clear();
				}
				else if( isWhitespace( font, shapedGlyphs[j].index ) ) {
					curRun.glyphs.insert( curRun.glyphs.begin(), curWord.begin(), curWord.end() );
					curWord.clear();
				}
			}

			curRun.glyphs.insert( curRun.glyphs.begin(), curWord.begin(), curWord.end() );
			curLine.runs.push_back( curRun );

			if( substrings[i].forceBreak ) {
				pen.x = 0.f;
				pen.y += FontManager::get()->getSize( font )->metrics.height / 64.f + mLeading;
			}
		}

		mLines.push_back( curLine );
	}
}