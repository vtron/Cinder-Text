#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "text/FontManager.h"
#include "text/Layout.h"
#include "text/TextRendererGl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class RichTextApp : public App
{
	public:
		void setup() override;
		void mouseDown( MouseEvent event ) override;
		void update() override;
		void draw() override;

		std::string mAttrString = "<span font-family=\"Source Serif Pro\" font-style=\"Regular\" font-size=\"20\" color=\"#ff0000\"><span font-size=\"50\">Ligatures like \"fi tf\"</span> This is a test of mixing font attributes like <i>italics</i>, <span color=\"#0000FF\">color</span> and <b> Bold!</b> </span><span font-family=\"Source Serif Pro\"> Here is some white serif text at <span font-size=\"30\">different</span><span font-size=\"10\"> sizes</span></span>";
		txt::Layout mLayout;

		ci::Rectf mTextBox;

		txt::RendererGl mRendererGl;

		// Base font, need to remove this, shouldn't be required
		std::shared_ptr<txt::Font> mBaseFont;

};

void RichTextApp::setup()
{
	setWindowSize( 1024.f, 768.f );

	mTextBox = ci::Rectf( 50, 50, 450, 450 );

	mBaseFont = std::make_shared<txt::Font>( getAssetPath( ( "SourceSansPro/SourceSansPro-Regular.otf" ) ), 12 );

	// Load font faces to use with rich text
	txt::FontManager::get()->loadFace( getAssetPath( "SourceSerifPro/SourceSerifPro-Regular.otf" ) );
	txt::FontManager::get()->loadFace( getAssetPath( "SourceSansPro/SourceSansPro-Regular.otf" ) );
	txt::FontManager::get()->loadFace( getAssetPath( "SourceSansPro/SourceSansPro-It.otf" ) );
	txt::FontManager::get()->loadFace( getAssetPath( "SourceSansPro/SourceSansPro-Bold.otf" ) );

	// Layout text
	mLayout.setSize( mTextBox.getSize() );
	mLayout.calculateLayout( *mBaseFont, mAttrString );
}

void RichTextApp::mouseDown( MouseEvent event )
{
}

void RichTextApp::update()
{
}

void RichTextApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
	ci::gl::enableAlphaBlending();
	ci::gl::ScopedMatrices matrices;
	ci::gl::translate( mTextBox.getUpperLeft() );

	ci::gl::color( 0.25, 0.25, 0.25 );
	ci::gl::drawStrokedRect( ci::Rectf( ci::vec2( 0.f ), mTextBox.getSize() ) );

	ci::gl::color( 1, 1, 1 );
	mRendererGl.drawLayout( mLayout );

}

CINDER_APP( RichTextApp, RendererGl )
