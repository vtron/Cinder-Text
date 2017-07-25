#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "text\Parser.h"

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
	txt::Font regular = txt::Font( ci::app::getAssetPath( "SourceSerifPro/SourceSerifPro-Regular.otf" ), 10 );
	txt::Font italic = txt::Font( ci::app::getAssetPath( "SourceSerifPro/SourceSerifPro-Italic.otf" ), 10 );
	txt::Parser parser;
	parser.parseAttr( regular, "<span font-family=\"Source Serif Pro\" font-weight=\"Regular\" font-size=\"20\" color=\"#ff0000\">test1 <i>Hey!</i> <span color=\"#333\"> Dude!</span> test2</span>" );

	for( auto& substring : parser.getSubstrings() ) {
		ci::app::console() << substring << std::endl;
	}

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
