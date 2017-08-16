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


	TextBox& TextBox::setAttrString( AttributedString attrString )
	{
		mAttrString = attrString;
		mNeedsLayout = true;
		return *this;
	}

	void TextBox::draw()
	{
		if( mNeedsLayout ) {
			//mAttrString << mFont;
			mLayout.calculateLayout( mAttrString );
			mNeedsLayout = false;
		}

		mRenderer->draw( mLayout );
	}
}