#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "txt/TextBox.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class TextboxApp : public App
{
	public:
		void setup() override;
		void mouseDown( MouseEvent event ) override;
		void update() override;
		void draw() override;

		txt::TextBox mTextBox;
};

void TextboxApp::setup()
{
	txt::Font font( loadAsset( "SourceSerifPro/SourceSerifPro-Regular.otf" ), 20 );
	mTextBox.setSize(ci::vec2(100))
		.setText("Testing the text box")
		.setFont( txt::Font( "Arial", 15 ) )
		.setFont( font );
}

void TextboxApp::mouseDown( MouseEvent event )
{
}

void TextboxApp::update()
{
}

void TextboxApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
	ci::gl::ScopedMatrices matrices;
	ci::gl::translate( 100, 100 );
	mTextBox.draw();
}

CINDER_APP( TextboxApp, RendererGl )
