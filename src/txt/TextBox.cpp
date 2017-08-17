#include "txt/TextBox.h"
#include "txt/TextRendererGl.h"

namespace txt
{
	TextBox::TextBox()
		: TextBox( ci::vec2( Layout::GROW, Layout::GROW ) )
	{}

	TextBox::TextBox( ci::vec2 size )
		: mFont( DefaultFont() )
		, mSize( size )
		, mColor( ci::Color::white() )
		, mRenderer( std::make_shared<RendererGl>() )
		, mNeedsLayout( true )
		, mExternalAttributedString( false )
	{
		ci::app::console() << "Text Box Constructed!" << std::endl;
	}

	ci::vec2 TextBox::measure()
	{
		layoutIfNeeded();
		return mLayout.measure();
	}

	ci::vec2 TextBox::getSize()
	{
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
		mText = text;
		mExternalAttributedString = false;
		mNeedsLayout = true;
		return *this;
	}

	TextBox& TextBox::setColor( ci::ColorA color )
	{
		mColor = color;
		mNeedsLayout = true;
		return *this;
	}

	TextBox& TextBox::setAttrString( AttributedString attrString )
	{
		mAttrString = attrString;
		mExternalAttributedString = true;
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
		if( !mExternalAttributedString ) {
			mAttrString = AttributedString( mText, mFont, mColor );
		}

		mLayout.calculateLayout( mAttrString );
		mNeedsLayout = false;

		return *this;
	}

	void TextBox::draw()
	{
		layoutIfNeeded();
		mRenderer->draw( mLayout );
	}
}