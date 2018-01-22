#include "Shaper.h"
#include "FontManager.h"

#include "cinder/Log.h"

#include "harfbuzz/hb-ft.h"

namespace txt
{
	// Create harfbuzz functions
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

	// Reverse Harfbuzz arrays, used for RTL text shaping
	template <typename t>
	void reverseHBArray( t* arr, int length )
	{
		int start = 0;
		int end = length - 1;
		t temp;

		while( start < end ) {
			temp = arr[start];
			arr[start] = arr[end];
			arr[end] = temp;
			start++;
			end--;
		}
	}

	std::vector<Shaper::Glyph> Shaper::getShapedText( Text& text )
	{
		hb_buffer_reset( mBuffer );

		hb_buffer_set_direction( mBuffer, text.direction );
		hb_buffer_set_script( mBuffer, text.script );
		hb_buffer_set_language( mBuffer, hb_language_from_string( text.language.c_str(), text.language.size() ) );

		hb_buffer_add_utf8( mBuffer, text.c_data(), text.data.length(), 0, text.data.length() );

		//hb_buffer_guess_segment_properties( mBuffer );

		hb_shape( mFont, mBuffer, mFeatures.empty() ? NULL : &mFeatures[0], mFeatures.size() );

		unsigned int glyph_count;
		hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos( mBuffer, &glyph_count );
		hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions( mBuffer, &glyph_count );


		// By default Harbuzz returns Glyphs in visual LTR order,
		// which is a PITA if you are line breaking
		// If we have RTL text, invert glyph parsing so that we get it in logical order
		// vs visual order
		reverseHBArray( glyph_info, glyph_count );
		reverseHBArray( glyph_pos, glyph_count );

		std::vector<Glyph> glyphs;

		for( int i = 0; i < glyph_count; i++ ) {
			Glyph glyph;
			glyph.index = glyph_info[i].codepoint;
			glyph.cluster = glyph_info[i].cluster;

			// Assign string values to clusters
			// for decomposing at a later time
			int clusterLength = 0;

			if( i != glyph_count - 1 ) {
				clusterLength = glyph_info[i + 1].cluster - glyph_info[i].cluster;
			}
			else {
				clusterLength = text.data.length() - glyph_info[i].cluster;
			}

			glyph.text = text.data.substr( glyph_info[i].cluster, clusterLength );

			for( int j = glyph.cluster; j < glyph.cluster + clusterLength; j++ ) {
				glyph.textIndices.push_back( j );
			}

			glyph.offset = ci::vec2( glyph_pos[i].x_offset / 64.f, glyph_pos[i].y_advance / 64.f );
			glyph.advance = ci::vec2( glyph_pos[i].x_advance / 64.f, glyph_pos[i].y_advance / 64.f );

			glyphs.push_back( glyph );
		}

		return glyphs;
	}
}