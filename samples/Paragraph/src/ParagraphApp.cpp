#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/FileWatcher.h"
#include "cinder/Utilities.h"

#include "txt/FontManager.h"
#include "txt/TextRendererGl.h"
#include "txt/TextLayout.h"

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

		ci::Rectf mTextBox = ci::Rectf( 100.f, 100.f, 500.f, 500.f );

		std::string fontName = "SourceSerifPro/SourceSerifPro-Regular.otf";
		int mFontSize = 16;
		std::string mTestText = "Lorem ipsum dolor sit amet,\n\n\nconsectetur adipiscing elit. Duis consequat ullamcorper lectus eget dapibus. Aenean vel hendrerit nibh. Sed at lectus commodo, ornare velit sed, elementum nisi. Vestibulum imperdiet justo eget enim posuere facilisis. Sed ac lacus ac nibh vestibulum dignissim sit amet eget tellus. Etiam ultrices massa maximus lectus sodales, eget ornare enim malesuada. Morbi et tellus sodales, tempus est sit amet, accumsan erat. Quisque semper nec enim sed consequat. Ut nec velit id nibh elementum viverra.";
};

CinderProjectApp::CinderProjectApp() {}



void CinderProjectApp::setup()
{
	setWindowSize( 1024.f, 768.f );

	ci::FileWatcher::instance().watch( ci::app::getAssetPath( "text.txt" ), std::bind( &CinderProjectApp::textFileUpdated, this, std::placeholders::_1 ) );

	// Create base font
	txt::Font mFont( loadAsset( "SourceSansPro/SourceSansPro-Regular.otf" ), 12 );
	//mFont = txt::Font( "Arial", "Italic", 15 );

	// Layout text
	mLayout.setSize( mTextBox.getSize() );
	mLayout.calculateLayout( mTestText, mFont );

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

	txt::Font mFont( ci::app::loadAsset( fontName ), mFontSize );
	mLayout.setTracking( mTracking );
	mLayout.setLeading( mLeading );
	mLayout.calculateLayout( mTestText, mFont );
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
	mLayout.setTracking( mTracking );
	mLayout.setLeading( mLeading );
	mLayout.calculateLayout( mTestText, mFont );
}


void CinderProjectApp::textFileUpdated( const ci::WatchEvent& watchEvent )
{
	mTestText = ci::loadString( ci::loadFile( watchEvent.getFile() ) );
}

CINDER_APP( CinderProjectApp, RendererGl, [&]( App::Settings* settings )
{
	settings->setHighDensityDisplayEnabled( true );
} )
