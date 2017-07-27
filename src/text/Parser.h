#pragma once

#include <string>
#include <stack>
#include <iostream>

#include "rapidxml\rapidxml.hpp"

#include "cinder/Color.h"

#include "text/Font.h"

namespace txt
{
	class Parser
	{
		public:
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

				ci::Color color;


				friend std::ostream& operator<< ( std::ostream& os, AttributeList const& attr )
				{
					os << "Font-Family: " << attr.fontFamily << std::endl;
					os << "Font-Style: " << attr.fontStyle << std::endl;
					os << "Font-Size: " << attr.fontSize << std::endl;
					os << "Color: " << attr.color << std::endl;
					return os;
				}

			};

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

			Parser() {};

			void parse( const Font& baseFont, std::string text );
			void parseAttr( const Font& baseFont, std::string text );

			std::deque<Substring>& getSubstrings() { return mSubstrings; };

		protected:
			void reset( const Font& baseFont );

			// Attributed Parsing

			void parseNode( rapidxml::xml_node<>* node );
			void pushNodeAttributes( rapidxml::xml_node<>* node );

			std::stack<AttributeList> mAttributesStack;
			std::deque<Substring> mSubstrings;
	};
}