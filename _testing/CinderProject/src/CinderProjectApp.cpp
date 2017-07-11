#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "text/FontManager.h"
#include "text/TextRendererGl.h"

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
		txt::RendererGl mRendererGl;

		std::string fontName = "SourceSerifPro/SourceSerifPro-Regular.otf";
};

void CinderProjectApp::setup()
{
	mFont = std::make_shared<txt::Font>( getAssetPath( fontName ), 100 );

	txt::Font font1( getAssetPath( fontName ), 20 );
	txt::Font font2( getAssetPath( fontName ), 20 );

	FT_Face face1 = txt::FontManager::get()->getFace( font1 );
	FT_Face face2 = txt::FontManager::get()->getFace( font2 );

	//FT_UInt glyphIndex = txt::FontManager::get()->getGlyphIndex( ( FTC_FaceID )font1.faceId, 'M' );

	FT_Size size = txt::FontManager::get()->getSize( font1 );

	ci::vec2 maxGlyphSize = txt::FontManager::get()->getMaxGlyphSize( font1 );

	console() << "Max Glyph Width: " << maxGlyphSize / ci::vec2( 64.f ) << std::endl;

	// Harfbuzz testing

	std::string text = "Hello!";

	hb_buffer_t* buf;
	buf = hb_buffer_create();

	hb_buffer_add_utf8( buf, text.c_str(), strlen( text.c_str() ), 0, strlen( text.c_str() ) );

	const std::vector<uint32_t> codepoints = txt::FontManager::get()->getGlyphIndices( font1.faceId, text );
	//hb_buffer_add_codepoints( buf, ( hb_codepoint_t* )&codepoints, codepoints.size(), 0, codepoints.size() );

	hb_buffer_guess_segment_properties( buf );

	hb_font_t* font = txt::FontManager::get()->getHarfbuzzFont( font1 );

	hb_shape( font, buf, NULL, 0 );

	unsigned int glyph_count;
	hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos( buf, &glyph_count );
	hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions( buf, &glyph_count );

	console() << "Total characters: " << glyph_count << std::endl;




	// GL Renderer
	txt::RendererGl glRenderer;

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
	mRendererGl.drawString( *mFont, "The quick brown fox jumps over the lazy dog. 1234567890" );
}

CINDER_APP( CinderProjectApp, RendererGl )
