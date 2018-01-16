#pragma once

#include <string>
#include <stack>
#include <iostream>

#include "harfbuzz/hb.h"
#include "rapidxml/rapidxml.hpp"

#include "cinder/Color.h"

#include "txt/Font.h"
#include "txt/TextUnits.h"

namespace txt
{
	// Attributed String
	enum AttributeType {
		LINE_BREAK,
		FONT,
		FONT_FAMILY,
		FONT_STYLE,
		FONT_SIZE,
		COLOR,
		OPACITY,
		LINE_HEIGHT,
		KERNING
	};

	struct Attribute {
		Attribute( const AttributeType& type ) : type( type ) { };
		const AttributeType type;
	};

	struct AttributeLineBreak : public Attribute {
		AttributeLineBreak() : Attribute( AttributeType::LINE_BREAK ) {};
	};

	struct AttributeFont : public Attribute {
		AttributeFont( const Font& font )
			: Attribute( AttributeType::FONT )
			, font( font ) {};
		const Font font;
	};

	struct AttributeFontFamily : public Attribute {
		AttributeFontFamily( const std::string& family )
			: Attribute( AttributeType::FONT_FAMILY )
			, family( family ) {};

		const std::string family;
	};

	struct AttributeFontStyle : public Attribute {
		AttributeFontStyle( const std::string& fontStyle )
			: Attribute( AttributeType::FONT_STYLE )
			, style( fontStyle ) {};

		const std::string style;
	};

	struct AttributeFontSize : public Attribute {
		AttributeFontSize( const int& size )
			: Attribute( AttributeType::FONT_SIZE ),
			  size( size ) { };

		const int size;
	};

	struct AttributeColor : public Attribute {
		AttributeColor( const ci::Color& color ) : Attribute( AttributeType::COLOR ),
			color( color ) {};

		const ci::Color color;
	};

	struct AttributeOpacity : public Attribute {
		AttributeOpacity( float opacity ) : Attribute( AttributeType::OPACITY ),
			opacity( opacity ) {};

		const float opacity;
	};

	struct AttributeLineHeight : public Attribute {
		AttributeLineHeight( const float& lineHeight ) : Attribute( AttributeType::LINE_HEIGHT ),
			lineHeight( Unit( lineHeight ) ) {};

		AttributeLineHeight( const Unit& lineHeight ) : Attribute( AttributeType::LINE_HEIGHT ),
			lineHeight( lineHeight ) {};

		const Unit lineHeight;
	};

	struct AttributeKerning : public Attribute {
		AttributeKerning( const float& kerning ) : Attribute( AttributeType::KERNING ),
			kerning( Unit( kerning ) ) {};

		AttributeKerning( const Unit& kerning ) : Attribute( AttributeType::KERNING ),
			kerning( kerning ) {};

		const Unit kerning;
	};

	struct RichText {
		RichText( const std::string& richText )
			: richText( richText ) {};

		std::string richText;
	};

	struct AttributeList {
		AttributeList( const Font& font, const ci::Color& color )
			: AttributeList( font.getFamily(), font.getStyle(), font.getSize(), color )
		{

		}

		AttributeList( const std::string& fontFamily, const std::string& fontStyle, const int& fontSize, const ci::Color& color )
			: fontFamily( fontFamily )
			, fontStyle( fontStyle )
			, fontSize( fontSize )
			, color( color )
			, opacity( 1.f )
			, kerning( 0 )
			, language( "" )
			, script( HB_SCRIPT_INVALID )
			, direction( HB_DIRECTION_INVALID )
		{
		}

		std::string fontFamily;
		std::string fontStyle;
		int fontSize;

		Unit lineHeight;
		Unit kerning;

		ci::Color color;
		float opacity = 1.f;

		// Shaper properties
		std::string language;
		hb_script_t script;
		hb_direction_t direction;

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
				{
				}

				std::string text;
				AttributeList attributes;

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
			AttributedString( const std::string& string, const Font& baseFont = DefaultFont(), const ci::Color& baseColor = ci::Color::white() );
			AttributedString( const RichText& richText, const Font& baseFont = DefaultFont(), const ci::Color& baseColor = ci::Color::white() );

			void addAttribute( const Attribute& attribute );
			void addText( std::string text );
			void addRichText( const RichText& richText );

			std::vector<Substring> getSubstrings() const { return mSubstrings; };
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
		attrStr << AttributeFontFamily( font.getFamily() ) << AttributeFontSize( font.getSize() ) << AttributeFontStyle( font.getStyle() );
		return attrStr;
	}

	inline AttributedString& operator << ( AttributedString& attrStr, const std::string& text )
	{
		attrStr.addText( text );
		return attrStr;
	}

	inline AttributedString& operator << ( AttributedString& attrStr, const RichText& richText )
	{
		attrStr.addRichText( richText );
		return attrStr;
	}

	// Rich Text Parsing
	class RichTextParser
	{
		public:
			RichTextParser( std::string richText, const AttributeList& baseAttributes );
			const std::vector<AttributedString::Substring>& getSubstrings() const { return mSubstrings; };

		protected:
			// Attributed Parsing
			void parseNode( rapidxml::xml_node<>* node );
			void pushNodeAttributes( rapidxml::xml_node<>* node );

			std::stack<AttributeList> mAttributesStack;
			std::vector<AttributedString::Substring> mSubstrings;
	};
}