#pragma once

#include "txt/Font.h"
#include "txt/TextLayout.h"
#include "txt/TextRenderer.h"

namespace txt
{
	class TextBox
	{
		public:
			TextBox();
			TextBox( ci::vec2 size );

			ci::vec2 measure();

			ci::vec2 getSize();
			TextBox& setSize( ci::vec2 size );

			TextBox& setFont( const Font& font );
			TextBox& setText( std::string text );
			TextBox& setAttrString( AttributedString attrString );
			TextBox& setColor( ci::ColorA color );
			TextBox& setAlignment( Alignment alignment );

			TextBox& layoutIfNeeded();
			TextBox& doLayout();

			Layout& getLayout() { return mLayout; };

			void draw();

		private:
			Font mFont;
			ci::vec2 mSize;
			ci::Color mColor;

			std::string mText;
			AttributedString mAttrString;
			bool mExternalAttributedString;

			Layout mLayout;
			bool mNeedsLayout;

			std::shared_ptr<txt::Renderer> mRenderer;
	};
}