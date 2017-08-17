#include "txt/AttributedString.h"

#include <string>
#include <sstream>
#include <vector>
#include <memory>

#include "rapidxml\rapidxml_print.hpp"

#include "cinder/app/App.h"

#include "txt/Font.h"
#include "txt/FontManager.h"

using namespace rapidxml;

namespace txt
{
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

	AttributedString::AttributedString()
		: AttributedString( "", DefaultFont() )
	{
	}

	AttributedString::AttributedString( const std::string& text, const Font& baseFont, const ci::Color& color )
	{
		// Find runs based on line breaks
		std::string textToParse = text;

		AttributeList attributes( baseFont.getFamily(), baseFont.getStyle(), baseFont.getSize(), color ) ;

		if( text.length() != 0 ) {
			std::vector<std::string> lineBreakStrings = split( text, '\n' );

			for( auto& str : lineBreakStrings ) {
				Substring substring( str, attributes, str == lineBreakStrings.back() ? false : true );
				mSubstrings.push_back( std::move( substring ) );
			}
		}
		else {
			mSubstrings.push_back( Substring( "", attributes ) );
		}
	}

	AttributedString::AttributedString( const RichText& richText, const Font& baseFont, const ci::Color& color )
	{
		RichTextParser parser( richText.richText, baseFont, color );
		mSubstrings = parser.getSubstrings();
	}

	// TODO: Make a list of these, saved here for now
	char lineBreak = '\u000A';

	void AttributedString::clear()
	{
		mSubstrings.clear();
	}

	void AttributedString::addText( std::string text )
	{
		// Copy current substring and start a new one
		mSubstrings.push_back( mSubstrings.back() );
		mSubstrings.back().text = text;
	}

	void AttributedString::addRichText( const RichText& richText )
	{
		// Store current substring attributes
		AttributeList curAttributes = mSubstrings.back().attributes;

		// Parse and push back rich text
		RichTextParser parser( richText.richText );
		mSubstrings.reserve( mSubstrings.size() + parser.getSubstrings().size() );
		mSubstrings.insert( mSubstrings.end(), parser.getSubstrings().begin(), parser.getSubstrings().end() );

		// Resume our current substring insertion with the current attributes (before Rich Text)
		mSubstrings.push_back( Substring( "", curAttributes ) );
	}

	void AttributedString::addAttribute( const Attribute& attribute )
	{
		switch( attribute.type ) {
			case LINE_BREAK:
				mSubstrings.back().forceBreak = true;
				break;

			case FONT_FAMILY: {
				std::string family = static_cast<const AttributeFontFamily&>( attribute ).family;
				mSubstrings.back().attributes.fontFamily = family;
			}
			break;

			case FONT_STYLE: {
				std::string style = static_cast<const AttributeFontStyle&>( attribute ).style;
				mSubstrings.back().attributes.fontStyle = style;
				break;
			}

			case FONT_SIZE: {
				int size = static_cast<const AttributeFontSize&>( attribute ).size;
				mSubstrings.back().attributes.fontSize = size;
				break;
			}

			case COLOR: {
				ci::Color color = static_cast<const AttributeColor&>( attribute ).color;
				mSubstrings.back().attributes.color = color;
				break;
			}

			case OPACITY: {
				float opacity = static_cast<const AttributeOpacity&>( attribute ).opacity;
				mSubstrings.back().attributes.opacity = opacity;
			}

			case LEADING: {
				const int leading = static_cast<const AttributeLeading&>( attribute ).leading;
				mSubstrings.back().attributes.leading = leading;
				break;
			}

			case KERNING: {
				const int kerning = static_cast<const AttributeKerning&>( attribute ).kerning;
				mSubstrings.back().attributes.kerning = kerning;
				break;
			}

		}
	}

	// Rich text parsing
	static const char* ATTR_BOLD( "b" );
	static const char* ATTR_ITALIC( "i" );
	static const char* ATTR_LINE_BREAK( "br" );
	static const char* ATTR_FONT_FAMILY( "font-family" );
	static const char* ATTR_FONT_STYLE( "font-style" );
	static const char* ATTR_FONT_SIZE( "font-size" );
	static const char* ATTR_COLOR( "color" );

	RichTextParser::RichTextParser( std::string richText, const Font& baseFont, const ci::Color& baseColor )
	{
		// Push first attribute
		mAttributesStack.push( AttributeList( baseFont.getFamily(), baseFont.getStyle(), baseFont.getSize(), baseColor ) );
		std::string wrappedText = "<txt>" + richText + "</txt>";
		xml_document<> doc;
		char* cstr = &wrappedText[0u];
		doc.parse<0>( cstr );
		xml_node<>* node = doc.first_node( "txt" );

		parseNode( node );
	}

	void RichTextParser::parseNode( rapidxml::xml_node<>* node )
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
				mSubstrings.push_back( AttributedString::Substring( node->value(), mAttributesStack.top() ) );
			}
			else if( strcmp( node->name(), ATTR_LINE_BREAK ) == 0 ) {
				mSubstrings.push_back( AttributedString::Substring( "", mAttributesStack.top(), true ) );
			}
		}

		mAttributesStack.pop();
	}

	void RichTextParser::pushNodeAttributes( rapidxml::xml_node<>* node )
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