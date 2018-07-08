#pragma once

#include "txt/Font.h"
#include "txt/TextLayout.h"
#include "txt/TextRenderer.h"
#include "txt/gl/TextureRenderer.h"

namespace txt
{
	class TextBox
	{
		public:
			TextBox();
			TextBox( ci::vec2 size, RendererRef renderer = std::make_shared<txt::gl::TextureRenderer>() );

			ci::ivec2 getSize();
			TextBox& setSize( ci::vec2 size );

			const Font&	getFont() const	{ return mFont; }
			TextBox& setFont( const Font& font );

			const std::string&	getText() const	{ return mText; }
			TextBox& setText( std::string text );

			TextBox& setAttrString( AttributedString attrString );
			TextBox& setColor( ci::ColorA color );
			TextBox& setAlignment( Alignment alignment );

			TextBox& layoutIfNeeded();
			TextBox& doLayout();

			Layout& getLayout()				{ return mLayout; };
			const Layout& getLayout() const { return mLayout; };

			RendererRef getRenderer() const { return mRenderer; }
			void setRenderer( RendererRef renderer ) { mRenderer = renderer; }

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