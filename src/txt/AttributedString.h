#pragma once

#include <iostream>
#include <string>

#include "cinder/Color.h"

namespace txt
{
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
		AttributeLineBreak() : Attribute( LINE_BREAK ) {};
	};

	struct AttributeFontFamily : public Attribute {
		AttributeFontFamily( std::string family )
			: Attribute( FONT_FAMILY )
			, family( family ) {};

		const std::string family;
	};

	struct AttributeFontStyle : public Attribute {
		AttributeFontStyle( std::string fontStyle )
			: Attribute( FONT_STYLE )
			, style( fontStyle ) {};

		const std::string style;
	};

	struct AttributeFontSize : public Attribute {
		AttributeFontSize( int size )
			: Attribute( FONT_SIZE ),
			  size( size ) { };

		const int size;
	};

	struct AttributeColor : public Attribute {
		AttributeColor( ci::Color color ) : Attribute( COLOR ),
			color( color ) {};

		AttributeColor( ci::ColorA color ) : Attribute( COLOR ),
			color( color ) {};

		const ci::ColorA color;
	};

	struct AttributeRichText : public Attribute {
		AttributeRichText( std::string richText )
			: Attribute( RICH_TEXT )
			, richText( richText ) {};

		std::string richText;
	};



	class AttributedString
	{
		public:
			AttributedString() {};

		private:

	};

	AttributedString& operator<<( AttributedString& attrStr, const Attribute& attr )
	{
		return attrStr;
	}
}