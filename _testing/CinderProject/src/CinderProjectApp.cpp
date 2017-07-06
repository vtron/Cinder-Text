#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "FontManager.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class CinderProjectApp : public App
{
	public:
		void setup() override;
		void mouseDown( MouseEvent event ) override;
		void update() override;
		void draw() override;

		std::string fontName = "SourceSerifPro/SourceSerifPro-Regular.otf";
};

void CinderProjectApp::setup()
{
	txt::Font font1( getAssetPath( fontName ), 12 );
	txt::Font font2( getAssetPath( fontName ), 12 );

	FT_Face face1 = txt::FontManager::get()->getFace( font1 );
	FT_Face face2 = txt::FontManager::get()->getFace( font2 );
}

void CinderProjectApp::mouseDown( MouseEvent event )
{
}

void CinderProjectApp::update()
{
}

void CinderProjectApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
}

CINDER_APP( CinderProjectApp, RendererGl )
