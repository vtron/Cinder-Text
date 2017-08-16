#pragma once

#include "txt/Font.h"
#include "txt/TextLayout.h"
#include "txt/TextRenderer.h"

namespace txt
{
	class TextBox
	{
		public:
			TextBox( ci::vec2 size = ci::vec2( Layout::GROW, Layout::GROW ) );

			ci::vec2 getSize();
			TextBox& setSize( ci::vec2 size );

			TextBox& setFont( const Font& font );
			TextBox& setText( std::string text );
			TextBox& setAttrString( AttributedString attrString );

			TextBox& layoutIfNeeded();
			TextBox& doLayout();

			void draw();

		private:
			Font mFont;
			ci::vec2 mSize;

			AttributedString mAttrString;

			Layout mLayout;
			bool mNeedsLayout;

			std::shared_ptr<txt::Renderer> mRenderer;
	};
}