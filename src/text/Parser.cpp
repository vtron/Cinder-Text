#include "text/Parser.h"

#include <string>
#include <sstream>
#include <vector>

#include "rapidxml\rapidxml_print.hpp"

#include "cinder/app/App.h"

#include "text/Font.h"
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
		parse( text );
	}

	void Parser::parse( std::string text )
	{
		// Find runs based on line breaks
		std::string textToParse = text;

		std::vector<std::string> lineBreakStrings = split( text, '\n' );

		for( auto& str : lineBreakStrings ) {
			Substring substring( str, mAttributeStack, str == lineBreakStrings.back() ? false : true );
			mSubstrings.push_back( std::move( substring ) );
		}
	}

	// -----------------------------------------------------
	// Attributed string parser
	ParserAttr::ParserAttr( const Font& baseFont, std::string text )
		: Parser( baseFont, text )
	{
		parse( text );
	}

	void ParserAttr::parse( std::string text )
	{
		using namespace rapidxml;

		std::string wrappedText = "<txt>" + text + "</txt>";
		xml_document<> doc;
		char* cstr = &wrappedText[0u];
		doc.parse<0>( cstr );
		ci::app::console() << "XML:" << std::endl;
		ci::app::console() << doc << std::endl;
		xml_node<>* node = doc.first_node( "txt" )->first_node( "span" );

		for( xml_node<>* child = node->first_node(); child; child = child->next_sibling() ) {
			// Check for italics
			if( child->name() == "i" ) {
				mAttributeStack.fontStyleStack.push( "Italic" );
			}

			// Check for bold
			else if( child->name() == "b" ) {
				mAttributeStack.fontStyleStack.push( "Regular" );
			}

			// Parse children
			ci::app::console() << "Run: " << std::endl;
			ci::app::console() << *child << std::endl;
			ci::app::console() << std::endl;


		}
	}

	void ParserAttr::pushNodeAttributes( rapidxml::xml_node<>* node )
	{

	}
}