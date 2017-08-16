#include "txt/TextBox.h"
#include "txt/TextRendererGl.h"

namespace txt
{
	TextBox::TextBox( ci::vec2 size )
		: mFont( DefaultFont() )
		, mSize( size )
		, mRenderer( std::make_shared<RendererGl>() )
		, mNeedsLayout( true )
	{}

	ci::vec2 TextBox::getSize()
	{
		layoutIfNeeded();
		ci::app::console() << mLayout.getSize() << std::endl;
		return mLayout.getSize();
	}

	TextBox& TextBox::setSize( ci::vec2 size )
	{
		mSize = size;
		mNeedsLayout = true;
		return *this;
	}

	TextBox& TextBox::setFont( const Font& font )
	{
		mFont = font;
		mNeedsLayout = true;
		return *this;
	};

	TextBox& TextBox::setText( std::string text )
	{
		mAttrString = AttributedString( text, mFont );
		mNeedsLayout = true;
		return *this;
	}

	TextBox& TextBox::layoutIfNeeded()
	{
		if( mNeedsLayout ) {
			doLayout();
		}

		return *this;
	}

	TextBox& TextBox::doLayout()
	{
		mLayout.calculateLayout( mAttrString );
		mNeedsLayout = false;

		return *this;
	}


	TextBox& TextBox::setAttrString( AttributedString attrString )
	{
		mAttrString = attrString;
		mNeedsLayout = true;
		return *this;
	}

	void TextBox::draw()
	{
		layoutIfNeeded();

		mRenderer->draw( mLayout );
	}
}