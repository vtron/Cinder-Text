#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/FileWatcher.h"
#include "cinder/Utilities.h"

#include "txt/FontManager.h"
#include "txt/TextRendererGl.h"
#include "txt/TextLayout.h"

#include "cinder/Unicode.h"

#include <string>
#include <iostream>

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

		std::shared_ptr<txt::Font> mFont;

		txt::Layout mLayout;
		txt::RendererGl mRendererGl;

		int mFontSize = 14.f;
		float mTracking = 0.f;

		float mLineHeight = 1.2;

		ci::vec2 mTextBoxPos = ci::vec2( 200.f, 200.f );
		ci::vec2 mTextBoxSize = ci::vec2( 600.f, 600.f );
		//ci::Rectf mTextBox = ci::Rectf( 100.f, 100.f, 800.f, 800.f );

		//std::string fontName = "fonts/NotoSerif/NotoSerif-Regular.ttf";
		//std::string fontName = "fonts/NotoArabic/NotoSansArabic-Regular.ttf";
		//std::string fontName = "fonts/NotoChinese/NotoSansCJKsc-Regular.otf";
		std::string fontName = "fonts/SourceSerifPro/SourceSerifPro-Regular.otf";

		std::string mTestText;
		std::string testTextFilename = "text/english.txt";
};

CinderProjectApp::CinderProjectApp() {}

void CinderProjectApp::setup()
{
	setWindowSize( 1024.f, 768.f );

	mFont = std::make_shared<txt::Font>( ci::app::loadAsset( fontName ), mFontSize );
	//mLineHeight = mFont->getLineHeight();

	ci::FileWatcher::instance().watch( ci::app::getAssetPath( testTextFilename ), std::bind( &CinderProjectApp::textFileUpdated, this, std::placeholders::_1 ) );
}

void CinderProjectApp::update()
{
}

void CinderProjectApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
	ci::gl::enableAlphaBlending();
	ci::gl::ScopedMatrices matrices;
	ci::gl::translate( mTextBoxPos );

	ci::gl::color( 0.25, 0.25, 0.25 );
	ci::gl::drawStrokedRect( ci::Rectf( ci::vec2( 0.f ), mLayout.measure() ) );

	ci::gl::color( 0.f, 1, 1 );
	mRendererGl.draw( mLayout );
}


void CinderProjectApp::updateLayout()
{
	mFont = std::make_shared<txt::Font>( ci::app::loadAsset( fontName ), mFontSize );

	mLayout.setFont( *mFont );
	mLayout.setSize( mTextBoxSize );
	mLayout.setTracking( mTracking );
	//mLayout.setLineHeight( mLineHeight );
	mLayout.setLineHeight( txt::Unit( mLineHeight, txt::EM ) );
	mLayout.calculateLayout( mTestText );



	ci::app::console() << "Default line-height: " << mFont->getLineHeight() << std::endl;
	ci::app::console() << "Line-height: " << mLayout.getLineHeight() << std::endl;
}

std::string unescape( const std::string& s )
{
	std::string res;
	std::string::const_iterator it = s.begin();

	while( it != s.end() ) {
		char c = *it++;

		if( c == '\\' && it != s.end() ) {
			switch( *it++ ) {
				case '\\':
					c = '\\';
					break;

				case 'n':
					c = '\n';
					break;

				case 't':
					c = '\t';
					break;

				// all other escapes
				default:
					// invalid escape sequence - skip it. alternatively you can copy it as is, throw an exception...
					continue;
			}
		}

		res += c;
	}

	return res;
}

void CinderProjectApp::textFileUpdated( const ci::WatchEvent& watchEvent )
{
	mTestText = unescape( ci::loadString( ci::loadFile( watchEvent.getFile() ) ) );
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
		mLineHeight--;
	}

	else if( event.getCode() == KeyEvent::KEY_DOWN ) {
		mLineHeight++;
	}

	else if( event.getCode() == KeyEvent::KEY_LEFT ) {
		mTracking -= 0.5;
	}

	else if( event.getCode() == KeyEvent::KEY_RIGHT ) {
		mTracking += 0.5;
	}

	else if( event.getChar() == '+' ) {
		mFontSize += 5;
	}

	else if( event.getChar() == '-' ) {
		if( mFontSize > 1 ) {
			mFontSize -= 5;
		}
	}

	updateLayout();
}

CINDER_APP( CinderProjectApp, RendererGl, [&]( App::Settings* settings )
{
	settings->setHighDensityDisplayEnabled( true );
} )
