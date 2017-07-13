#include "text/Parser.h"

#include <string>
#include <sstream>
#include <vector>

#include "text/FontManager.h"



namespace txt
{
	// TODO: Make a list of these, saved here for now
	char lineBreak = '\u000A';

	// Split on a character (from https://stackoverflow.com/a/27511119)
	std::vector<std::string> split( const std::string& s, char delim )
	{
		std::stringstream ss( s );
		std::string item;
		std::vector<std::string> elems;

		while( std::getline( ss, item, delim ) ) {
			elems.push_back( std::move( item ) );
		}

		return elems;
	}

	Parser::Parser( const Font& baseFont, std::string text )
		: mAttributeStack(	FontManager::get()->getFontFamily( baseFont ),
		                    FontManager::get()->getFontStyle( baseFont ),
		                    baseFont.size )

	{
		parseString( text );
	}

	void Parser::parseString( std::string text )
	{
		// Find runs based on line breaks
		std::string textToParse = text;

		std::vector<std::string> lineBreakStrings = split( text, '\n' );

		for( auto& str : lineBreakStrings ) {
			Substring substring( str, mAttributeStack, str == lineBreakStrings.back() ? false : true );
			mSubstrings.push_back( std::move( substring ) );
		}
	}
}