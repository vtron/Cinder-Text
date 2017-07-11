#include "text/Layout.h"

#include "harfbuzz/hb.h"

#include "cinder/app/App.h"

#include "text/FontManager.h"

namespace txt
{
	Layout::Layout( Font& font, std::string text )
	{
		hb_buffer_t* buf;
		buf = hb_buffer_create();
		hb_buffer_add_utf8( buf, text.c_str(), strlen( text.c_str() ), 0, strlen( text.c_str() ) );
		hb_buffer_guess_segment_properties( buf );

		hb_font_t* hbFont = txt::FontManager::get()->getHarfbuzzFont( font );

		hb_shape( hbFont, buf, NULL, 0 );

		unsigned int glyph_count;
		hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos( buf, &glyph_count );
		hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions( buf, &glyph_count );

		Run run( font );

		ci::vec2 pen( 0 );

		for( int i = 0; i < glyph_count; ++i ) {
			Glyph glyph;
			glyph.index = glyph_info[i].codepoint;
			glyph.pos = pen + ci::vec2( glyph_pos[i].x_offset / 64.f, glyph_pos[i].y_offset / 64.f );

			ci::vec2 advance( glyph_pos[i].x_advance / 64.f, glyph_pos[i].y_advance / 64.f );
			pen += advance;
			run.glyphs.push_back( glyph );
		}

		Line line;
		line.runs.push_back( run );

		mLines.push_back( line );
	}
}