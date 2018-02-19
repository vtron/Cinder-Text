#include "txt/TextBox.h"
#include "txt/TextRendererGl.h"

namespace txt
{
	TextBox::TextBox()
		: TextBox( ci::vec2( txt::GROW, txt::GROW ) )
	{}

	TextBox::TextBox( ci::vec2 size )
		: mFont( DefaultFont() )
		, mSize( size )
		, mColor( ci::Color::white() )
		, mRenderer( txt::RendererGl::instance() )
		, mNeedsLayout( true )
		, mExternalAttributedString( false )
	{
	}

	ci::ivec2 TextBox::getSize()
	{
		layoutIfNeeded();
		ci::vec2 size = mLayout.measure();
		return ci::ivec2( ceil( size.x ), ceil( size.y ) );
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


	TextBox& TextBox::setAlignment( Alignment alignment )
	{
		mLayout.setAlignment( alignment );
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

		mLayout.setSize( mSize );
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