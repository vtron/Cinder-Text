#include "Shaper.h"
#include "FontManager.h"

#include "cinder/Log.h"

#include "harfbuzz/hb-ft.h"

namespace txt
{
	namespace
	{
		const hb_tag_t KernTag = HB_TAG( 'k', 'e', 'r', 'n' ); // kerning operations
		const hb_tag_t LigaTag = HB_TAG( 'l', 'i', 'g', 'a' ); // standard ligature substitution
		const hb_tag_t CligTag = HB_TAG( 'c', 'l', 'i', 'g' ); // contextual ligature substitution

		static hb_feature_t LigatureOff = { LigaTag, 0, 0, std::numeric_limits<unsigned int>::max() };
		static hb_feature_t LigatureOn = { LigaTag, 1, 0, std::numeric_limits<unsigned int>::max() };
		static hb_feature_t KerningOff = { KernTag, 0, 0, std::numeric_limits<unsigned int>::max() };
		static hb_feature_t KerningOn = { KernTag, 1, 0, std::numeric_limits<unsigned int>::max() };
		static hb_feature_t CligOff = { CligTag, 0, 0, std::numeric_limits<unsigned int>::max() };
		static hb_feature_t CligOn = { CligTag, 1, 0, std::numeric_limits<unsigned int>::max() };
	}

	Shaper::Shaper( const Font& font )
	{
		FT_Face face = FontManager::get()->getSize( font )->face;
		mFont = hb_ft_font_create( face, NULL );

		mBuffer = hb_buffer_create();

		CI_ASSERT_MSG( hb_buffer_allocation_successful( mBuffer ), "Could not allocate Harfbuzz buffer." );
	}

	Shaper::~Shaper()
	{
		hb_buffer_destroy( mBuffer );
		hb_font_destroy( mFont );
	}

	void Shaper::addFeature( Feature feature )
	{
		switch( feature ) {
			case LIGATURES:
				mFeatures.push_back( LigatureOn );
				break;

			case KERNING:
				mFeatures.push_back( KerningOn );
				break;

			case CLIG:
				mFeatures.push_back( CligOn );
				break;
		}
	}

	void Shaper::removeFeature( Feature feature )
	{
		switch( feature ) {
			case LIGATURES:
				mFeatures.push_back( LigatureOff );
				break;

			case KERNING:
				mFeatures.push_back( KerningOff );
				break;

			case CLIG:
				mFeatures.push_back( CligOff );
				break;
		}
	}

	std::vector<Shaper::Glyph> Shaper::getShapedText( Text& text )
	{
		hb_buffer_reset( mBuffer );

		hb_buffer_set_direction( mBuffer, text.direction );
		hb_buffer_set_script( mBuffer, text.script );
		hb_buffer_set_language( mBuffer, hb_language_from_string( text.language.c_str(), text.language.size() ) );

		hb_buffer_add_utf8( mBuffer, text.c_data(), text.data.length(), 0, text.data.length() );

		hb_shape( mFont, mBuffer, mFeatures.empty() ? NULL : &mFeatures[0], mFeatures.size() );

		unsigned int glyph_count;
		hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos( mBuffer, &glyph_count );
		hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions( mBuffer, &glyph_count );

		std::vector<Glyph> glyphs;

		for( int i = 0; i < glyph_count; i++ ) {
			Glyph glyph;
			glyph.index = glyph_info[i].codepoint;
			glyph.cluster = glyph_info[i].cluster;

			glyph.offset = ci::vec2( glyph_pos[i].x_offset / 64.f, glyph_pos[i].y_advance / 64.f );
			glyph.advance = ci::vec2( glyph_pos[i].x_advance / 64.f, glyph_pos[i].y_advance / 64.f );

			glyphs.push_back( glyph );
		}

		return glyphs;
	}
}