#include "txt/TextLayout.h"

#include <string.h>

#include "harfbuzz/hb.h"

#include "cinder/app/App.h"
#include <cinder/Unicode.h>

#include "txt/FontManager.h"
#include "txt/Shaper.h"

#include "libunibreak\linebreak.h"
#include "libunibreak\wordbreak.h"

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

	int calculateShapedGlyphsLength( const std::vector<Layout::Glyph>& glyphs )
	{
		int length = 0;

		for( const auto& glyph : glyphs ) {
			length += glyph.value.length();
		}

		return length;
	}

	Layout::Layout()
		: mFont( DefaultFont() )
		, mColor( ci::Color( 1.f, 1.f, 1.f ) )
		, mTracking( 0 )
		, mLeading( 0 )
		, mAlignment( Alignment::LEFT )
		, mSize( GROW )
		, mMaxLinesReached( false )
	{
		//const std::string testString( "testing test opportunity" );

		//const char* lang = "en_US";
		//const char* text = testString.c_str();
		//size_t len;
		//char* breaks;

		//len = strlen( testString.c_str() );
		//breaks = ( char* )malloc( len );

		//set_wordbreaks_utf8( ( const utf8_t* )text, len, lang, breaks );

		//ci::app::console() << std::endl;

		//for( int i = 0; i < len; i++ ) {
		//	ci::app::console() << "Can Break on " << text[i] << ": " << ( int )breaks[i] << std::endl;
		//}

	}

	void Layout::resetLayout()
	{
		mLines.clear();
		mPen = ci::vec2( 0.f );
		mCurLine = Line();
		mLineHeight = 0.f;
		mLineLeading = 0.f;
		mAscender = 0.f;
		mLineWidth = 0.f;
		mMaxLinesReached = false;
		mLayoutSize = mSize;
	}

	void Layout::calculateLayout( std::string text )
	{
		calculateLayout( AttributedString( text, mFont, mColor ) );
	}

	void Layout::calculateLayout( const AttributedString& attrString )
	{
		resetLayout();

		std::vector<AttributedString::Substring> substrings = attrString.getSubstrings();

		// Go through each substring
		for( int i = 0; i < substrings.size(); i++ ) {
			AttributedString::Substring remainingSubstring = substrings[i];

			// Keep track of the previous pass's size
			// if it remains the same we are stuck
			unsigned long prevSubstringSize = 0;

			// Keep processing the substring until finished
			while( ( remainingSubstring.text.size() ) || remainingSubstring.forceBreak ) {
				// Don't bother continuing if we aren't going to display any more lines
				if( mMaxLinesReached ) {
					return;
				}

				// Process the remaining characters of the substring
				addSubstringToCurLine( remainingSubstring );

				// Make sure we actually processed something
				if( remainingSubstring.text.size() == prevSubstringSize ) {
					// Return to avoid infinite loop when a string or character
					// will not fit our layout (too big)
					return;
				}
				else {
					// Record the previous size
					prevSubstringSize = remainingSubstring.text.size();
				}

			}
		}

		addCurLine();
	}

	// Process substring till we hit a linebreak or the end of the substring
	// then erase the characters we added
	// (it would be clearer to return the remainder vs modify the reference, but this should be faster)
	void Layout::addSubstringToCurLine( AttributedString::Substring& substring )
	{
		// Create a font for this substring
		const Font runFont( substring.attributes.fontFamily, substring.attributes.fontStyle, substring.attributes.fontSize );

		// Get the line height + ascender for this run
		float lineHeight	= FontManager::get()->getSize( runFont )->metrics.height / 64.f;
		float ascender		= FontManager::get()->getSize( runFont )->metrics.ascender / 64.f;

		// Increase our current line height + ascender if this run is taller
		int prevLineHeight	= mLineHeight;
		int prevAscender	= mAscender;

		mLineHeight		= std::max( lineHeight, mLineHeight );
		mLineLeading	= std::max( mLineLeading, mLeading + substring.attributes.leading );
		mAscender		= std::max( ascender, mAscender );

		// Check for height clipping
		// TODO: This needs to handle vertical layouts (clip width)
		if( mSize.y != GROW && mPen.y + mLineHeight > mSize.y ) {
			mMaxLinesReached = true;
			return;
		}

		// Space betwen characters
		float kerning = mTracking + substring.attributes.kerning;

		// Check for pure line break
		if( substring.forceBreak && substring.text == "" ) {
			addCurLine();
			substring.forceBreak = false;
			return;
		}

		// Create a run to store our glyphs
		Run run( runFont, substring.attributes.color, substring.attributes.opacity );

		// Shape the substring
		Shaper shaper( runFont );
		Shaper::Text shaperText = {
			substring.text,
			substring.attributes.language,
			substring.attributes.script,
			substring.attributes.direction
		};

		std::vector<Shaper::Glyph> shapedGlyphs = shaper.getShapedText( shaperText );

		// Calculate linebreaks
		std::vector<uint8_t> lineBreaks;
		ci::calcLinebreaksUtf8( substring.text.c_str(), &lineBreaks );

		for( int i = 0; i < shapedGlyphs.size(); i++ ) {
			// Add the offset (generally 0 for latin) to the pen pos
			ci::vec2 pos = mPen + shapedGlyphs[i].offset;

			// Get the glyph metrics/position
			ci::vec2 advance = shapedGlyphs[i].advance;

			FT_BitmapGlyph bitmapGlyph = FontManager::get()->getGlyphBitmap( runFont, shapedGlyphs[i].index );
			ci::vec2 glyphPos = pos + ci::vec2( bitmapGlyph->left, 0.f );
			ci::Rectf glyphBBox( glyphPos, glyphPos + ci::vec2( bitmapGlyph->bitmap.width, bitmapGlyph->bitmap.rows ) );

			// Move the pen forward, except with white space at the beginning of a line
			if( mPen.x != 0 || !isWhitespace( runFont, shapedGlyphs[i].index ) ) {
				mPen.x += advance.x + kerning;
			}

			// Check for a new line
			// TODO: Right to left + vertical
			if( mSize.x != GROW && mPen.x > mSize.x ) {
				// Go backwards through shaped glyphs to find the closest break
				int lineBreakIndex = 0;
				int shapedGlyphIndex = -1;

				bool lineBreakFound = false;

				for( int j = i; j >= 0; j-- ) {
					if( lineBreakFound ) { break; }

					// Unpack the glyph's cluster
					for( int k = shapedGlyphs[j].textIndices.size() - 1; k >= 0; k-- ) {
						// Look for allowed breaks
						if( lineBreaks[shapedGlyphs[j].textIndices[k]] == ci::UNICODE_ALLOW_BREAK ) {
							lineBreakIndex = shapedGlyphs[j].textIndices[k];
							shapedGlyphIndex = j;
							lineBreakFound = true;
							break;
						}
					}
				}

				// Clip the current run to the linebreak position
				// and add to the current line
				if( !run.glyphs.empty() && shapedGlyphIndex != -1 ) {
					run.glyphs.erase( run.glyphs.begin() + shapedGlyphIndex, run.glyphs.end() );
					addRunToCurLine( run );
					run.glyphs.clear();
				}
				else {
					mLineHeight = prevLineHeight;
					mAscender = prevAscender;
				}

				// Our line is complete, add it to our layout
				addCurLine();

				// Clip the substrings text by what we've already added and return
				substring.text = substring.text.substr( lineBreakIndex, std::string::npos );

				return;
			}

			// Create a layout glyph and add to run
			Layout::Glyph glyph = { shapedGlyphs[i].index, glyphBBox, bitmapGlyph->top, shapedGlyphs[i].text };
			run.glyphs.push_back( glyph );
		}

		// Once we've gone through everything add the remaining word
		// (Only reached if we haven't filled a line)
		//run.glyphs.insert( run.glyphs.end(), curWord.begin(), curWord.end() );
		addRunToCurLine( run );

		// If the substring requests a line break push to next line
		if( substring.forceBreak == true ) {
			addCurLine();
			substring.forceBreak = false;
		}

		substring.text.clear();
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
				glyph.bbox.offset( ci::vec2( 0.f, mAscender - glyph.top ) );
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

		// Update our layout size
		for( auto& run : mCurLine.runs ) {
			for( auto& glyph : run.glyphs ) {
				ci::vec2 bottomRight = glyph.bbox.getLowerRight();
				mLayoutSize.x = std::max( bottomRight.x, mLayoutSize.x );
				mLayoutSize.y = std::max( bottomRight.y, mLayoutSize.y );
			}
		}

		// Setup next line
		{
			mCurLine = Line();
		}

		mPen.x = 0.f;
		mPen.y += mLineHeight + mLineLeading;

		mLineWidth = 0;
		mLineHeight = 0.f;
		mAscender = 0.f;
	}
}