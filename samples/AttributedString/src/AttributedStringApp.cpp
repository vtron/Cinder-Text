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
};

void AttributedStringApp::setup()
{
	txt::AttributedString attr;
	attr << txt::AttributeColor( ci::Color( 1.0, 0.0, 0.f ) );
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
}

CINDER_APP( AttributedStringApp, RendererGl )
