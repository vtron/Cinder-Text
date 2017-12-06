#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/FileWatcher.h"
#include "cinder/Utilities.h"

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

		void textFileUpdated( const ci::WatchEvent& event );
		std::string testTextFilename = "text/english.txt";

		txt::TextBox mTextBox;
};

void TextboxApp::setup()
{
	txt::Font font( loadAsset( "fonts/SourceSerifPro/SourceSerifPro-Regular.otf" ), 12 );
	mTextBox.setSize( ci::vec2( 400, txt::GROW ) )
	.setFont( txt::Font( "Arial", 11 ) )
	.setFont( font );

	ci::FileWatcher::instance().watch( ci::app::getAssetPath( testTextFilename ), std::bind( &TextboxApp::textFileUpdated, this, std::placeholders::_1 ) );
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

void TextboxApp::textFileUpdated( const ci::WatchEvent& watchEvent )
{
	mTextBox.setText( unescape( ci::loadString( ci::loadFile( watchEvent.getFile() ) ) ) );
	mTextBox.doLayout();
	ci::app::console() << "Height: " << mTextBox.getSize().y << std::endl;
}



CINDER_APP( TextboxApp, RendererGl, [&]( App::Settings* settings )
{
	settings->setHighDensityDisplayEnabled( true );
} )
