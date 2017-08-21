#include "txt/TextLayout.h"

#include "harfbuzz/hb.h"

#include "cinder/app/App.h"
#include <cinder/Unicode.h>

#include "txt/FontManager.h"
#include "txt/Shaper.h"

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
		, mMaxLinesReached( false )
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
		mMaxLinesReached = false;
	}

	ci::vec2 Layout::getSize()
	{
		ci::vec2 size( 0.f );

		if( mSize.x == GROW && !mLines.empty() ) {

			// Calculate width
			for( auto& line : mLines ) {
				float width = line.runs.back().glyphs.back().bbox.getLowerRight().x;

				if( width > size.x ) {
					size.x = width;
				}
			}
		}
		else {
			size.x = mSize.x;
		}

		if( mSize.y == GROW && !mLines.empty() ) {
			// Calculate Height
			for( auto& run : mLines.back().runs ) {
				for( auto& glyph : run.glyphs ) {
					if( glyph.bbox.y2 > size.y ) {
						size.y = glyph.bbox.y2;
					}
				}
			}
		}
		else {
			size.y = mSize.y;
		}

		return size;
	}

	void Layout::calculateLayout( std::string text, const Font& font, const ci::Color& color )
	{
		calculateLayout( AttributedString( text, font ) );
	}

	void Layout::calculateLayout( const AttributedString& attrString )
	{
		reset();

		std::vector<AttributedString::Substring> substrings = attrString.getSubstrings();

		for( int i = 0; i < substrings.size(); i++ ) {
			AttributedString::Substring remainingSubstring = substrings[i];

			while( remainingSubstring.text.size() || remainingSubstring.forceBreak ) {
				if( mMaxLinesReached ) {
					return;
				}

				addSubstringToCurLine( remainingSubstring );
			}
		}

		addCurLine();
	}

	void Layout::addSubstringToCurLine( AttributedString::Substring& substring )
	{
		// Create a font for this substring
		const Font runFont( substring.attributes.fontFamily, substring.attributes.fontStyle, substring.attributes.fontSize );

		// Get the line height + ascender for this run

		// Increase our current line height + ascender if this run is taller
		float lineHeight = FontManager::get()->getSize( runFont )->metrics.height / 64.f;
		float ascender = FontManager::get()->getSize( runFont )->metrics.ascender / 64.f;

		int prevLineHeight = mLineHeight;
		int prevAscender = mAscender;

		mLineHeight = std::max( lineHeight, mLineHeight );
		mLineLeading = mLeading + substring.attributes.leading;
		mAscender = std::max( ascender, mAscender );

		// Space betwen characters
		float kerning = mTracking + substring.attributes.kerning;

		// Check for height clipping
		// TODO: This needs to handle vertical layouts (clip width)
		if( mSize.y != GROW && mPen.y + mLineHeight > mSize.y ) {
			mMaxLinesReached = true;
			return;
		}

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
			"en",
			HB_SCRIPT_LATIN,
			HB_DIRECTION_LTR
		};

		std::vector<Shaper::Glyph> shapedGlyphs = shaper.getShapedText( shaperText );

		// Track current word for line breaks (need to remove and switch to unicode breaking)
		std::vector<Glyph> curWord;

		for( int i = 0; i < shapedGlyphs.size(); i++ ) {
			// Add the offset (generally 0 for latin) to the pen pos
			ci::vec2 pos = mPen + shapedGlyphs[i].offset;

			// Get the glyph metrics/position
			ci::vec2 advance = shapedGlyphs[i].advance;
			ci::Rectf glyphBBox( pos, pos + FontManager::get()->getGlyphSize( runFont, shapedGlyphs[i].index ) );

			FT_BitmapGlyph bitmapGlyph = FontManager::get()->getGlyphBitmap( runFont, shapedGlyphs[i].index );

			// Create a layout glyph and add to current word
			Layout::Glyph glyph = { shapedGlyphs[i].index, glyphBBox, bitmapGlyph->top };

			// Move the pen forward
			mPen.x += advance.x + kerning;

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

		// If the substring requests a line break push to next line
		if( substring.forceBreak ) {
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

		// Setup next line
		mCurLine = Line();

		mPen.x = 0.f;
		mPen.y += mLineHeight + mLineLeading;

		mLineWidth = 0;
		mLineHeight = 0.f;
		mAscender = 0.f;
	}
}