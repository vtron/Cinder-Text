#pragma once

#include "txt/Layout.h"

namespace txt
{
	class TextBox
	{
		public:
			TextBox( ci::Rectf frame );

			void setText( std::string text );
			void setAttrText( AttributedString attText );

		private:
			AttributedString mAttrString;
			Layout mLayout;
	};
}