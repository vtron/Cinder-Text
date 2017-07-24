#pragma once

#include <string>
#include <stack>
#include <iostream>

#include "rapidxml\rapidxml.hpp"

#include "cinder/Color.h"

#include "text/Types.h"

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
					, color()
				{
				}

				const std::string fontFamily;
				const std::string fontStyle;
				const int fontSize;

				ci::Color color;

			};

			struct Substring {
				Substring( std::string text, AttributeList attributes, bool forceBreak = false )
					:  text( text )
					, attributes( attributes )
					, forceBreak( forceBreak )
				{
				}

				const std::string text;
				const AttributeList attributes;
				const bool forceBreak;

				friend std::ostream& operator<< ( std::ostream& os, Substring const& t )
				{
					os << t.text;
					return os;
				}

			};

			Parser( const Font& baseFont, std::string text );

			std::deque<Substring>& getSubstrings() { return mSubstrings; };

		protected:
			virtual void parse( std::string text );

			AttributeStack mAttributeStack;

			std::deque<Substring> mSubstrings;
	};

	class ParserAttr : public Parser
	{
		public:
			ParserAttr( const Font& baseFont, std::string attrString );

		private:
			void parse( std::string text ) override;

			void pushNodeAttributes( rapidxml::xml_node<>* node );

	};
}