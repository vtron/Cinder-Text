#pragma once

#include <string>
#include <stack>
#include <iostream>

#include "rapidxml\rapidxml.hpp"

#include "cinder/Color.h"

#include "txt/Font.h"

namespace txt
{
	// Attributed String
	enum AttributeType {
		LINE_BREAK,
		FONT_FAMILY,
		FONT_STYLE,
		FONT_SIZE,
		COLOR,
		RICH_TEXT
	};

	struct Attribute {
		Attribute( AttributeType type ) : type( type ) { };
		const AttributeType type;
	};

	struct AttributeLineBreak : public Attribute {
		AttributeLineBreak() : Attribute( AttributeType::LINE_BREAK ) {};
	};

	struct AttributeFontFamily : public Attribute {
		AttributeFontFamily( std::string family )
			: Attribute( AttributeType::FONT_FAMILY )
			, family( family ) {};

		const std::string family;
	};

	struct AttributeFontStyle : public Attribute {
		AttributeFontStyle( std::string fontStyle )
			: Attribute( AttributeType::FONT_STYLE )
			, style( fontStyle ) {};

		const std::string style;
	};

	struct AttributeFontSize : public Attribute {
		AttributeFontSize( int size )
			: Attribute( AttributeType::FONT_SIZE ),
			  size( size ) { };

		const int size;
	};

	struct AttributeColor : public Attribute {
		AttributeColor( ci::Color color ) : Attribute( AttributeType::COLOR ),
			color( color ) {};

		AttributeColor( ci::ColorA color ) : Attribute( AttributeType::COLOR ),
			color( color ) {};

		const ci::ColorA color;
	};

	struct RichText : public Attribute {
		RichText( std::string richText )
			: Attribute( AttributeType::RICH_TEXT )
			, richText( richText ) {};

		std::string richText;
	};

	struct AttributeList {
		AttributeList( std::string fontFamily, std::string fontStyle, int fontSize, ci::Color color = ci::Color::white() )
			: fontFamily( fontFamily )
			, fontStyle( fontStyle )
			, fontSize( fontSize )
			, color( color )
		{
		}

		std::string fontFamily;
		std::string fontStyle;
		int fontSize;

		ci::ColorA color;

		friend std::ostream& operator<< ( std::ostream& os, AttributeList const& attr )
		{
			os << "Font-Family: " << attr.fontFamily << std::endl;
			os << "Font-Style: " << attr.fontStyle << std::endl;
			os << "Font-Size: " << attr.fontSize << std::endl;
			os << "Color: " << attr.color << std::endl;
			return os;
		}

	};

	// Attributed String
	class AttributedString
	{
		public:
			struct Substring {
				Substring( std::string text, const AttributeList& attributes, bool forceBreak = false )
					: text( text )
					, attributes( attributes )
					, forceBreak( forceBreak )
				{
				}

				std::string text;
				AttributeList attributes;
				bool forceBreak;

				friend std::ostream& operator<< ( std::ostream& os, Substring const& s )
				{
					os << "Substring: " << std::endl;
					os << "Text: " << s.text << std::endl;
					os << "Attributes: " << std::endl;
					os << s.attributes << std::endl;
					return os;
				}
			};

			AttributedString();
			AttributedString( std::string string, const Font& baseFont = DefaultFont() );
			AttributedString( const RichText& richText );

			void addAttribute( const Attribute& attribute );
			void addText( std::string text );
			void addRichText( const RichText& richText );

			const std::vector<Substring>& getSubstrings() const { return mSubstrings; };
			void clear();

		private:
			std::vector<Substring> mSubstrings;
	};

	inline AttributedString& operator << ( AttributedString& attrStr, const Attribute& attr )
	{
		attrStr.addAttribute( attr );
		return attrStr;
	}


	inline AttributedString& operator << ( AttributedString& attrStr, const Font& font )
	{
		attrStr << AttributeFontFamily( font.family ) << AttributeFontSize( font.size ) << AttributeFontStyle( font.style );
		return attrStr;
	}

	inline AttributedString& operator << ( AttributedString& attrStr, const std::string& text )
	{
		attrStr.addText( text );
		return attrStr;
	}

	// Rich Text Parsing
	class RichTextParser
	{
		public:
			RichTextParser( std::string richText, const Font& baseFont = DefaultFont() );
			const std::vector<AttributedString::Substring>& getSubstrings() const { return mSubstrings; };

		protected:
			// Attributed Parsing
			void parseNode( rapidxml::xml_node<>* node );
			void pushNodeAttributes( rapidxml::xml_node<>* node );

			std::stack<AttributeList> mAttributesStack;
			std::vector<AttributedString::Substring> mSubstrings;
	};
}