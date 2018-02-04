#include "txt/TextLayout.h"

#include <string.h>

#include "harfbuzz/hb.h"

#include "cinder/app/App.h"
#include <cinder/Unicode.h>

#include "txt/FontManager.h"

#include "libunibreak\linebreak.h"
#include "libunibreak\wordbreak.h"

namespace txt
{
	bool isWhitespace( const Font& font, uint32_t codepoint )
	{
		std::string separatorSpaces;
		separatorSpaces += ' '; // SPACE
		separatorSpaces += '\u00A0'; // NO-BREAK SPACE
		separatorSpaces += '\u1680'; // OGHAM SPACE MARK
		separatorSpaces += '\u2000'; // EN QUAD
		separatorSpaces += '\u2001'; // EM QUAD
		separatorSpaces += '\u2002'; // EN SPACE
		separatorSpaces += '\u2003'; // EM SPACE
		separatorSpaces += '\u2004'; // THREE-PER-EM SPACE
		separatorSpaces += '\u2005'; // FOUR-PER-EM SPACE
		separatorSpaces += '\u2006'; // SIX-PER-EM SPACE
		separatorSpaces += '\u2007'; // FIGURE SPACE
		separatorSpaces += '\u2008'; // PUNCTUATION SPACE
		separatorSpaces += '\u2009'; // THIN SPACE
		separatorSpaces += '\u200A'; // HAIR SPACE
		separatorSpaces += '\u202F'; // NARROW NO-BREAK SPACE
		separatorSpaces += '\u205F'; // MEDIUM MATHEMATICAL SPACE
		separatorSpaces += '\u3000'; // IDEOGRAPHIC SPACE

		std::vector<uint32_t> separatorSpaceIndices = FontManager::get()->getGlyphIndices( font, separatorSpaces );

		std::vector<uint32_t>::iterator iter = std::find( separatorSpaceIndices.begin(), separatorSpaceIndices.end(), codepoint );

		if( iter == separatorSpaceIndices.end() ) {
			return false;
		}

		return true;

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
		, mAlignment( Alignment::LEFT )
		, mSize( GROW )
		, mLanguage( "en" )
		, mScript( HB_SCRIPT_LATIN )
		, mDirection( HB_DIRECTION_LTR )
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
		mCharPos = 0.f;
		mLinePos = 0.f;
		mCurLine = Line();
		mCurLineHeight = 0.f;
		mCurLineWidth = 0.f;
		mMaxLinesReached = false;
		mLayoutSize = mSize;
	}

	const ci::vec2 Layout::measure()
	{
		ci::vec2 size( mSize );

		if( size.x == txt::GROW ) {
			size.x = mCurLineWidth;
		}

		if( size.y == txt::GROW ) {
			size.y = mLinePos;
		}

		return size;
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
			unsigned long prevSubstringSize = -1;

			// Keep processing the substring until finished
			while( ( remainingSubstring.text.size() ) ) {
				// Don't bother continuing if we aren't going to display any more lines
				if( mMaxLinesReached ) {
					applyAlignment();
					return;
				}

				// Process the remaining characters of the substring
				addSubstringToCurLine( remainingSubstring );

				// Make sure we actually processed something
				if( remainingSubstring.text.size() == prevSubstringSize ) {
					// Return to avoid infinite loop when a string or character
					// will not fit our layout (too big)
					applyAlignment();
					return;
				}
				else {
					// Record the previous size
					prevSubstringSize = remainingSubstring.text.size();
				}
			}
		}

		addCurLine();
		applyAlignment();
	}

	float Layout::getLineHeightForSubstring( const AttributedString::Substring& substring, const Font& runFont )
	{
		float lineHeight;

		// Check for substring line height
		if( !substring.attributes.lineHeight.isDefault() ) {
			lineHeight = substring.attributes.lineHeight.getValue( runFont.getSize() );
		}
		// Default to our layout line height
		else {
			if( !mLineHeight.isDefault() ) {
				lineHeight = mLineHeight.getValue( runFont.getSize() );
			}
			else {
				// Otherwise use the default for the font
				lineHeight = runFont.getLineHeight();
			}
		}

		return lineHeight;
	}

	// Process substring till we hit a linebreak, go past the max line-length
	// or reach the end of the substring
	// If we don't reach the end, erase the characters we added from the substring
	// (it would be clearer to return the remainder vs modify the reference, but this should be faster)
	void Layout::addSubstringToCurLine( AttributedString::Substring& substring )
	{
		// Determine direction, script and language with overrides
		std::string language = substring.attributes.language == "" ? mLanguage : substring.attributes.language;
		hb_script_t script = substring.attributes.script == HB_SCRIPT_INVALID ? mScript : substring.attributes.script;
		hb_direction_t direction = substring.attributes.direction == HB_DIRECTION_INVALID ? mDirection : substring.attributes.direction;

		mCurDirection.x = direction == HB_DIRECTION_LTR ? 1 : -1;

		// Create a run for this substring
		const Font runFont( substring.attributes.fontFamily, substring.attributes.fontStyle, substring.attributes.fontSize );
		Run run( runFont, substring.attributes.color, substring.attributes.opacity );

		// Store the previous line height in case we need to abort and go to a new line
		int prevLineHeight = mCurLineHeight;

		// Get the line height for this run
		// and increase our current line height if this run is taller
		float lineHeight = getLineHeightForSubstring( substring, runFont );
		mCurLineHeight	= std::max( lineHeight, mCurLineHeight );

		// Character spacing
		float kerning = mTracking.getValue( runFont.getSize() ) + substring.attributes.kerning.getValue( runFont.getSize() );

		// Check for height clipping
		// TODO: This needs to handle vertical layouts (clip width)
		if( mSize.y != GROW && mLinePos + mCurLineHeight > mSize.y ) {
			mMaxLinesReached = true;
			return;
		}

		// Pad out the substring so we can ignore the last break
		// (needs improvement?)
		// then calculate linebreaks
		std::string paddedSubstring = substring.text + " ";
		std::vector<uint8_t> lineBreaks;
		ci::calcLinebreaksUtf8( paddedSubstring.c_str(), &lineBreaks );
		lineBreaks.pop_back();

		// Shape the substring
		Shaper shaper( runFont );
		Shaper::Text shaperText = {
			substring.text,
			language,
			script,
			direction
		};

		std::vector<Shaper::Glyph> shapedGlyphs = shaper.getShapedText( shaperText );

		for( int i = 0; i < shapedGlyphs.size(); i++ ) {
			// Get directional offset + advance
			ci::vec2 offset = shapedGlyphs[i].offset * mCurDirection;
			ci::vec2 advance = shapedGlyphs[i].advance * mCurDirection;

			// Add the offset (generally 0 for latin) to the pen pos
			ci::vec2 pos = ci::vec2( mCharPos, mLinePos ) + offset;

			// Get the glyph metrics/position
			FT_BitmapGlyph bitmapGlyph = FontManager::get()->getGlyphBitmap( runFont, shapedGlyphs[i].index );
			ci::vec2 glyphPos;
			ci::Rectf glyphBBox;

			if( direction == HB_DIRECTION_LTR ) {
				glyphPos = pos + ci::vec2( bitmapGlyph->left, 0.f );
				glyphBBox = ci::Rectf( glyphPos, glyphPos + ci::vec2( bitmapGlyph->bitmap.width, bitmapGlyph->bitmap.rows ) );
			}
			else {
				glyphPos = pos - ci::vec2( shapedGlyphs[i].advance.x - bitmapGlyph->left, 0.f );
				glyphBBox = ci::Rectf( glyphPos, glyphPos + ci::vec2( bitmapGlyph->bitmap.width, bitmapGlyph->bitmap.rows ) );
			}

			// Move the pen forward, except with white space at the beginning of a line
			if( mCharPos != 0 || !isWhitespace( runFont, shapedGlyphs[i].index ) ) {
				mCharPos += advance.x + kerning;
			}

			// Check for a new line
			// TODO: Right to left + vertical
			if( mSize.x != GROW && fabs( mCharPos ) > mSize.x ) {
				BreakIndices breaks = getClosestBreakForShapedText( i, shapedGlyphs, lineBreaks, direction );

				if( breaks.found ) {
					// Clip the current run to the linebreak position
					// and add to the current line
					if( !run.glyphs.empty() ) {
						run.glyphs.erase( run.glyphs.begin() + breaks.glyphBreakIndex, run.glyphs.end() );

						addRunToCurLine( run );
						run.glyphs.clear();
					}
					else {
						mCurLineHeight = prevLineHeight;
					}

					// Clip the substrings text by what we've already added and return
					substring.text = substring.text.substr( breaks.textBreakIndex, std::string::npos );
				}

				// Our line is complete, add it to our layout
				addCurLine();

				return;
			}

			// Create a layout glyph and add to run
			Layout::Glyph glyph = { shapedGlyphs[i].index, glyphBBox, bitmapGlyph->top, shapedGlyphs[i].text };
			run.glyphs.push_back( glyph );

			// Check for forced line breaks
			for( auto& index : shapedGlyphs[i].textIndices ) {
				if( lineBreaks[index] == ci::UNICODE_MUST_BREAK ) {
					// Add the current run then move to next line
					addRunToCurLine( run );
					addCurLine();

					// Clip the substring after the break
					int clipStart = shapedGlyphs[i].textIndices.back() + 1;

					if( clipStart < substring.text.size() ) {
						substring.text = substring.text.substr( clipStart, std::string::npos );
					}
					else {
						substring.text = "";
					}

					// Done
					return;
				}
			}
		}

		// Once we've gone through everything add the remaining word
		// (Only reached if we haven't filled a line)
		addRunToCurLine( run );

		substring.text.clear();
	}

	void Layout::addRunToCurLine( Run& run )
	{
		mCurLine.runs.push_back( run );

		if( !run.glyphs.empty() ) {
			mCurLineWidth = run.glyphs.back().bbox.x2;
		}
	}

	void Layout::addCurLine( )
	{
		// Set the Y glyph position based on culmulative line-height
		for( auto& run : mCurLine.runs ) {
			for( auto& glyph : run.glyphs ) {
				float xOffset = ( mDirection == HB_DIRECTION_RTL ? mSize.x : 0.0 );
				float yOffset = mCurLineHeight - glyph.top;
				glyph.bbox.offset( ci::vec2( xOffset, yOffset ) );
			}
		}

		mCurLine.width = mCurLineWidth;

		// Add it to our lines
		mLines.push_back( mCurLine );

		// Update our layout size
		mLayoutSize.x = std::max( mCurLineWidth, mLayoutSize.x );

		for( auto& run : mCurLine.runs ) {
			mLayoutSize.y = std::max( FontManager::get()->getMaxGlyphSize( run.font ).y, mLayoutSize.y );
		}

		// Setup next line
		{
			mCurLine = Line();
		}

		mCharPos = 0.f;
		mLinePos += mCurLineHeight;

		mCurLineHeight = mLineHeight.getValue( mFont.getSize() );
		mCurLineWidth = 0;
	}

	void Layout::applyAlignment()
	{
		if( mSize.x == GROW || mAlignment == LEFT ) {
			return;
		}

		// Align in frame (if necessary)
		for( int i = 0; i < mLines.size(); i++ ) {
			float remainingWidth = mSize.x - mLines[i].width;

			switch( mAlignment ) {
				case CENTER:
				case RIGHT: {
					float xOffset = ( mAlignment == CENTER ) ? remainingWidth / 2.f : remainingWidth;

					for( auto& run : mLines[i].runs ) {
						for( auto& glyph : run.glyphs ) {
							glyph.bbox.offset( ci::vec2( xOffset, 0.f ) );
						}
					}

					break;
				}

				// This whole thing points to a lot of optimization possibilities
				// in lines,runs and glyphs
				case JUSTIFIED: {
					if( i == mLines.size() - 1 || mLines[i + 1].getTotalGlyphs() == 1 ) {
						continue;
					}

					// A custom iterator would be awesome here
					std::vector<Glyph*> glyphRefs;
					std::vector<bool> glyphIsWhitespace;
					int totalWhitespaces = 0;

					for( auto& run : mLines[i].runs ) {
						// Get the char index for whitespace given this run's font
						FT_UInt spaceIndex = FontManager::get()->getGlyphIndex( run.font, ' ' );

						int index = 0;

						for( auto& glyph : run.glyphs ) {
							glyphRefs.push_back( &glyph );
							bool isWhitespace = index != 0 && glyph.index == spaceIndex;

							if( isWhitespace ) {
								totalWhitespaces++;
							}

							glyphIsWhitespace.push_back( isWhitespace );
							index++;
						}
					}

					float justificationPadding = remainingWidth / totalWhitespaces;

					for( int i = 0; i < glyphRefs.size(); i++ ) {
						if( glyphIsWhitespace[i] && i != 0 && i < glyphRefs.size() - 1 ) {
							for( int j = i + 1; j < glyphRefs.size(); j++ ) {
								glyphRefs[j]->bbox.offset( ci::vec2( justificationPadding, 0.f ) );
							}
						}
					}

					break;
				}
			}
		}
	}

	Layout::BreakIndices Layout::getClosestBreakForShapedText( int startIndex, const std::vector<Shaper::Glyph>& shapedGlyphs, const std::vector<uint8_t> lineBreaks, const hb_direction_t& direction )
	{
		Layout::BreakIndices indices;

		for( int i = startIndex; i >= 0; i-- ) {
			if( indices.found ) { break; }

			// Unpack the glyph's cluster
			for( int j = shapedGlyphs[i].textIndices.size() - 1; j >= 0; j-- ) {
				// Look for allowed breaks
				if( lineBreaks[shapedGlyphs[i].textIndices[j]] == ci::UNICODE_ALLOW_BREAK ) {
					indices.textBreakIndex = shapedGlyphs[i].textIndices[j];
					indices.glyphBreakIndex = i;
					indices.found = true;
					break;
				}
			}
		}

		return indices;
	}
}