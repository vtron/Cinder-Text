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
	txt::FontRef font1 = txt::Font::create( getAssetPath( fontName ).string(), 12 );
	txt::FontRef font2 = txt::Font::create( getAssetPath( fontName ).string(), 12 );

	FT_Face face1 = txt::FontManager::get()->getFace( font1 );
	FT_Face face2 = txt::FontManager::get()->getFace( font2 );

	font1 = nullptr;
	font2 = nullptr;

	face1 = nullptr;
	face2 = nullptr;



	font1 = txt::Font::create( getAssetPath( fontName ).string(), 12 );
	face1 = txt::FontManager::get()->getFace( font1 );



	//face1 = nullptr;
	//face2 = nullptr;

	//face1 = txt::FontManager::get()->getFace( font );
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
