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
	}

	void Layout::resetLayout()
	{
		mLines.clear();
		mPen = ci::vec2( 0.f );
		mCurLine = Line();
		mLineHeight = 0.f;
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
			substring.attributes.language,
			substring.attributes.script,
			substring.attributes.direction
		};

		std::vector<Shaper::Glyph> shapedGlyphs = shaper.getShapedText( shaperText );

		// Calculate linebreaks
		std::vector<uint8_t> lineBreaks;
		ci::calcLinebreaksUtf8( substring.text.c_str(), &lineBreaks );

		// Track current word for line breaks (need to remove and switch to unicode breaking)
		std::vector<Glyph> curWord;

		for( int i = 0; i < shapedGlyphs.size(); i++ ) {
			ci::app::console() << "Text: " << shapedGlyphs[i].text << std::endl;

			// Add the offset (generally 0 for latin) to the pen pos
			ci::vec2 pos = mPen + shapedGlyphs[i].offset;

			// Get the glyph metrics/position
			ci::vec2 advance = shapedGlyphs[i].advance;

			FT_BitmapGlyph bitmapGlyph = FontManager::get()->getGlyphBitmap( runFont, shapedGlyphs[i].index );
			ci::vec2 glyphPos = pos + ci::vec2( bitmapGlyph->left, 0.f );
			ci::Rectf glyphBBox( glyphPos, glyphPos + ci::vec2( bitmapGlyph->bitmap.width, bitmapGlyph->bitmap.rows ) );

			// Create a layout glyph and add to current word
			Layout::Glyph glyph = { shapedGlyphs[i].index, glyphBBox, bitmapGlyph->top, shapedGlyphs[i].text };

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
				int remainingGlyphStart = shapedGlyphs[i].cluster + shapedGlyphs[i].text.length() - 1 - calculateShapedGlyphsLength( curWord );
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