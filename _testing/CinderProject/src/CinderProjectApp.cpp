#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "text/FontManager.h"
#include "text/TextRendererGl.h"
#include "text/Layout.h"

#include "harfbuzz/hb-ft.h"

#include <ShellScalingAPI.h>


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

		std::shared_ptr<txt::Font> mFont;
		txt::Layout mLayout;
		txt::RendererGl mRendererGl;

		std::string fontName = "SourceSerifPro/SourceSerifPro-Regular.otf";
		int fontSize = 16;
		//std::string testText = "hhhhhhhhhhhh \n\t";
		std::string testText = "Lorem ipsum dolor sit amet,\n consectetur adipiscing elit. Duis consequat ullamcorper lectus eget dapibus. Aenean vel hendrerit nibh. Sed at lectus commodo, ornare velit sed, elementum nisi. Vestibulum imperdiet justo eget enim posuere facilisis. Sed ac lacus ac nibh vestibulum dignissim sit amet eget tellus. Etiam ultrices massa maximus lectus sodales, eget ornare enim malesuada. Morbi et tellus sodales, tempus est sit amet, accumsan erat. Quisque semper nec enim sed consequat. Ut nec velit id nibh elementum viverra.";
};

void CinderProjectApp::setup()
{
	setWindowSize( 1024.f, 768.f );
	mFont = std::make_shared<txt::Font>( getAssetPath( fontName ), fontSize );

	mLayout.setLeading( 10 );
	mLayout.setTracking( 5 );
	mLayout.calculateLayout( *mFont, testText, ci::vec2( 400.f, 0.f ) );

	//txt::Font font1( getAssetPath( fontName ), 20 );
	//txt::Font font2( getAssetPath( fontName ), 20 );

	//FT_Face face1 = txt::FontManager::get()->getFace( font1 );
	//FT_Face face2 = txt::FontManager::get()->getFace( font2 );

	////FT_UInt glyphIndex = txt::FontManager::get()->getGlyphIndex( ( FTC_FaceID )font1.faceId, 'M' );

	//FT_Size size = txt::FontManager::get()->getSize( font1 );

	//ci::vec2 maxGlyphSize = txt::FontManager::get()->getMaxGlyphSize( font1 );

	//console() << "Max Glyph Width: " << maxGlyphSize / ci::vec2( 64.f ) << std::endl;

	//// Harfbuzz testing

	//std::string text = "Fi fi J.L. Hello!";

	//hb_buffer_t* buf;
	//buf = hb_buffer_create();

	//hb_buffer_add_codepoints( buf, &txt::FontManager::get()->getGlyphIndices( mFont->faceId, text )[0], text.length(), 0, text.length() );
	////hb_buffer_add_utf8( buf, text.c_str(), strlen( text.c_str() ), 0, strlen( text.c_str() ) );

	//hb_buffer_guess_segment_properties( buf );

	//FT_Library library;
	//FT_Init_FreeType( &library );

	//FT_Face face;
	//FT_New_Face( library, getAssetPath( fontName ).string().c_str(), 0, &face );
	//FT_Set_Char_Size( face, 36 * 64.f, 36.f * 64.f, 72, 72 );

	//hb_font_t* font = hb_ft_font_create( face, NULL );

	//hb_shape( font, buf, NULL, 0 );

	//unsigned int glyph_count;
	//hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos( buf, &glyph_count );
	//hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions( buf, &glyph_count );

	//for( int i = 0; i < glyph_count; i++ ) {
	//	console() << glyph_info[i].codepoint << std::endl;
	//	console() << glyph_pos[i].x_offset << std::endl;
	//}

	//console() << "Total characters: " << glyph_count << std::endl;

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
	ci::gl::color( 255.f, 255.f, 255.f );
	ci::gl::enableAlphaBlending();
	ci::gl::ScopedMatrices matrices;
	ci::gl::translate( 10.f, 125.f );
	//mRendererGl.drawString( *mFont, "The quick brown fox jumps over the lazy dog. 1234567890" );
	mRendererGl.drawLayout( mLayout );
}

CINDER_APP( CinderProjectApp, RendererGl )
