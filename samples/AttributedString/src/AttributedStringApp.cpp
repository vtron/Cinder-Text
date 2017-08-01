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

		std::string testText = "This is a test of a basic string to be drawn in a textbox.";
};

void AttributedStringApp::setup()
{
	mLayout.setSize( mTextBox.getSize() );
	mLayout.calculateLayout( testText );
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
	mRendererGl.drawLayout( mLayout );
}

CINDER_APP( AttributedStringApp, RendererGl )
