#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "txt/Layout.h"
#include "txt/TextRendererGl.h"
#include "txt/AttributedString.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class AttributedStringApp : public App
{
	public:
		void setup() override;
		void mouseDown( MouseEvent event ) override;
		void update() override;
		void draw() override;

		txt::Layout mLayout;
		txt::RendererGl mRendererGl;

		ci::Rectf mTextBox = ci::Rectf( 100.f, 100.f, 500.f, 500.f );

		std::string testText = "This is a test of a basic string to be drawn in a textbox, testing 12345. This is a test of a basic string to be drawn in a textbox. This is a test of a basic string to be drawn in a textbox. This is a test of a basic string to be drawn in a textbox.";
};

void AttributedStringApp::setup()
{
	txt::AttributedString attrStr;
	attrStr << "It's" << txt::AttributeColor( ci::ColorA( 1.0f, 0.f, 1.f ) ) << txt::AttributeFontFamily( "Helvetica" );
	attrStr << " Wednesday" << txt::AttributeColor( ci::ColorA( 1.f, 1.f, 1.f ) )  << txt::AttributeFontStyle( "Italic" );
	attrStr << " my" << txt::AttributeFontStyle( "Regular" ) << txt::AttributeLineBreak();
	attrStr << "Dudes!" << txt::AttributeFontSize( 50.f ) << txt::AttributeFontStyle( "Bold" );
	attrStr << txt::RichText( "<br/><span font-fammily=\"Helvetica\" font-style=\"Italic\" color=\"#ff00ff\">Rich Text</span>" );

	mLayout.setSize( mTextBox.getSize() );
	mLayout.calculateLayout( attrStr );
}

void AttributedStringApp::mouseDown( MouseEvent event )
{
}

void AttributedStringApp::update()
{
}

void AttributedStringApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
	ci::gl::enableAlphaBlending();
	ci::gl::ScopedMatrices matrices;
	ci::gl::translate( mTextBox.getUpperLeft() );

	ci::gl::color( 0.25, 0.25, 0.25 );
	ci::gl::drawStrokedRect( ci::Rectf( ci::vec2( 0.f ), mTextBox.getSize() ) );

	ci::gl::color( 1, 1, 1 );
	mRendererGl.draw( mLayout );
}

CINDER_APP( AttributedStringApp, RendererGl )
