#pragma once

#include <string>
#include <stack>

#include "cinder/Color.h"
#include "text/Types.h"

namespace txt
{
	class Parser
	{
		public:
			struct AttributeStack {
				AttributeStack( std::string baseFamily, std::string baseStyle, int baseSize )
				{
					fontFamilyStack.push( baseFamily );
					fontStyleStack.push( baseStyle );
					fontSizeStack.push( baseSize );
				}

				std::stack<std::string> fontFamilyStack;
				std::stack<std::string> fontStyleStack;
				std::stack<int> fontSizeStack;
			};

			struct Substring {
				Substring( std::string text, AttributeStack stack, bool forceBreak = false )
					:  text( text )
					, fontFamily( stack.fontFamilyStack.top() )
					, fontStyle( stack.fontStyleStack.top() )
					, fontSize( stack.fontSizeStack.top() )
					, color( ci::Color::white() )
					, forceBreak( forceBreak )
				{
				}

				const std::string text;

				const std::string fontFamily;
				const std::string fontStyle;
				const int fontSize;
				const ci::Color color;
				const bool forceBreak;
			};

			Parser( const Font& baseFont, std::string text );

			std::deque<Substring>& getSubstrings() { return mSubstrings; };

		private:
			void parseString( std::string text );

			AttributeStack mAttributeStack;

			std::deque<Substring> mSubstrings;
	};
}