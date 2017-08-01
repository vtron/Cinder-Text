#include "text/Parser.h"

#include <string>
#include <sstream>
#include <vector>

#include "rapidxml\rapidxml_print.hpp"

#include "cinder/app/App.h"

#include "text/Font.h"
#include "text/FontManager.h"

using namespace rapidxml;

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

	void Parser::reset( const Font& baseFont )
	{
		// Clear previous parsing
		mSubstrings.clear();
		mAttributesStack = std::stack<AttributeList>();

		// Push default attributes
		mAttributesStack.push( AttributeList( FontManager::get()->getFontFamily( baseFont ), FontManager::get()->getFontStyle( baseFont ), baseFont.size ) );
	}

	void Parser::parse( const Font& baseFont, std::string text )
	{
		reset( baseFont );

		// Find runs based on line breaks
		std::string textToParse = text;

		std::vector<std::string> lineBreakStrings = split( text, '\n' );

		for( auto& str : lineBreakStrings ) {
			Substring substring( str, mAttributesStack.top(), str == lineBreakStrings.back() ? false : true );
			mSubstrings.push_back( std::move( substring ) );
		}
	}

	// Attributed string parsing
	void Parser::parseAttr( const Font& baseFont, std::string text )
	{
		// Clear substrings
		reset( baseFont );

		std::string wrappedText = "<txt>" + text + "</txt>";
		xml_document<> doc;
		char* cstr = &wrappedText[0u];
		doc.parse<0>( cstr );
		xml_node<>* node = doc.first_node( "txt" );

		parseNode( node );
	}

	static const char* ATTR_BOLD( "b" );
	static const char* ATTR_ITALIC( "i" );
	static const char* ATTR_LINE_BREAK( "br" );
	static const char* ATTR_FONT_FAMILY( "font-family" );
	static const char* ATTR_FONT_STYLE( "font-style" );
	static const char* ATTR_FONT_SIZE( "font-size" );
	static const char* ATTR_COLOR( "color" );


	void Parser::parseNode( rapidxml::xml_node<>* node )
	{
		bool lineBreak = strcmp( node->name(), ATTR_LINE_BREAK ) == 0;
		pushNodeAttributes( node );

		if( node->first_node() != 0 ) {
			for( xml_node<>* child = node->first_node(); child; child = child->next_sibling() ) {
				parseNode( child );
			}
		}

		else {
			if( node->value_size() != 0 ) {
				mSubstrings.push_back( Substring( node->value(), mAttributesStack.top() ) );
			}
		}

		mAttributesStack.pop();
	}

	void Parser::pushNodeAttributes( rapidxml::xml_node<>* node )
	{
		// Copy current attributes and push to stack
		mAttributesStack.push( mAttributesStack.top() );

		// Check for <b> or <i> tags
		if( strcmp( node->name(), ATTR_BOLD ) == 0 ) {
			mAttributesStack.top().fontStyle = "Bold";
		}

		else if( strcmp( node->name(), ATTR_ITALIC ) == 0 ) {
			mAttributesStack.top().fontStyle = "Italic";
		}

		// Parse out attributes
		for( xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute() ) {
			// Font-family
			if( strcmp( attr->name(), ATTR_FONT_FAMILY ) == 0 ) {
				mAttributesStack.top().fontFamily = attr->value();
			}

			// Font-style
			else if( strcmp( attr->name(), ATTR_FONT_STYLE ) == 0 ) {
				mAttributesStack.top().fontStyle = attr->value();
			}

			// Font-size
			else if( strcmp( attr->name(), ATTR_FONT_SIZE ) == 0 ) {
				try {
					int size = std::stoi( attr->value() );
					mAttributesStack.top().fontSize = size;
				}
				catch( std::exception const& e ) {
					//Log exception?
				}
			}

			// Color
			else if( strcmp( attr->name(), ATTR_COLOR ) == 0 ) {
				std::string color = attr->value();
				// Remove '#'
				color.erase( 0, 1 );

				// Check for three digit hex, double up (CSS feature, makes sense to support)
				if( color.length() == 3 ) {
					color += color;
				}

				// Create uint32_t hex value from string
				std::stringstream ss;
				ss << color << std::hex;

				uint32_t hexValue;
				ss >> hexValue;
				mAttributesStack.top().color = ci::Color::hex( hexValue );
			}
		}
	}
}