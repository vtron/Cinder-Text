#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/FileWatcher.h"
#include "cinder/Utilities.h"

#include "txt/FontManager.h"
#include "txt/TextRendererGl.h"
#include "txt/TextLayout.h"

#include "cinder/Unicode.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class CinderProjectApp : public App
{
	public:
		CinderProjectApp();

		void setup() override;
		void keyDown( KeyEvent event ) override;
		void update() override;
		void draw() override;

		void updateLayout();
		void textFileUpdated( const ci::WatchEvent& event );

		txt::Layout mLayout;
		txt::RendererGl mRendererGl;

		float mTracking = 0.f;
		float mLeading = 0.f;

		ci::Rectf mTextBox = ci::Rectf( 100.f, 100.f, 800.f, 800.f );

		std::string fontName = "SourceSerifPro/SourceSerifPro-Regular.otf";
		//"NotoArabic/NotoSansArabic-Regular.ttf";

		int mFontSize = 12;
		std::string mTestText;
		std::string testTextFilename = "english.txt";
};

CinderProjectApp::CinderProjectApp() {}

void CinderProjectApp::setup()
{
	setWindowSize( 1024.f, 768.f );

	ci::FileWatcher::instance().watch( ci::app::getAssetPath( testTextFilename ), std::bind( &CinderProjectApp::textFileUpdated, this, std::placeholders::_1 ) );


	GLint max_layers;
	glGetIntegerv( GL_MAX_ARRAY_TEXTURE_LAYERS, &max_layers );

	ci::app::console() << "texture array layers: " << max_layers << std::endl;
}

void CinderProjectApp::update()
{
}

void CinderProjectApp::draw()
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


void CinderProjectApp::updateLayout()
{
	txt::Font mFont( ci::app::loadAsset( fontName ), mFontSize );

	mLayout.setFont( mFont );
	mLayout.setSize( mTextBox.getSize() );
	mLayout.setTracking( mTracking );
	mLayout.setLeading( mLeading );
	mLayout.calculateLayout( mTestText );
}


void CinderProjectApp::textFileUpdated( const ci::WatchEvent& watchEvent )
{
	//std::ifstream file( watchEvent.getFile().c_str(), ios::binary );
	//std::stringstream buffer;

	//buffer << file.rdbuf();
	//std::string str = buffer.str();
	//std::cout << str;

	mTestText = ci::loadString( ci::loadFile( watchEvent.getFile() ) );
	updateLayout();
}

void CinderProjectApp::keyDown( KeyEvent event )
{
	if( event.getChar() == KeyEvent::KEY_1 ) {
		mLayout.setAlignment( txt::Alignment::LEFT );
	}

	else if( event.getCode() == KeyEvent::KEY_2 ) {
		mLayout.setAlignment( txt::Alignment::CENTER );
	}

	else if( event.getCode() == KeyEvent::KEY_3 ) {
		mLayout.setAlignment( txt::Alignment::RIGHT );
	}

	else if( event.getCode() == KeyEvent::KEY_UP ) {
		mLeading--;
	}

	else if( event.getCode() == KeyEvent::KEY_DOWN ) {
		mLeading++;
	}

	else if( event.getCode() == KeyEvent::KEY_LEFT ) {
		mTracking--;
	}

	else if( event.getCode() == KeyEvent::KEY_RIGHT ) {
		mTracking++;
	}

	else if( event.getChar() == '+' ) {
		mFontSize++;
	}

	else if( event.getChar() == '-' ) {
		if( mFontSize > 1 ) {
			mFontSize--;
		}
	}

	updateLayout();
}

CINDER_APP( CinderProjectApp, RendererGl, [&]( App::Settings* settings )
{
	settings->setHighDensityDisplayEnabled( true );
} )
