#include "text/Layout.h"

#include "harfbuzz/hb.h"

#include "cinder/app/App.h"

#include "text/FontManager.h"
#include "freetype/ft2build.h"
#include "Shaper.h"

namespace txt
{
	Layout::Layout( Font& font, std::string text, ci::vec2 size )
	{
		Shaper shaper( font );
		Shaper::Text shaperText = {
			text,
			"en",
			HB_SCRIPT_LATIN,
			HB_DIRECTION_LTR
		};

		//shaper.removeFeature( Shaper::Feature::KERNING );
		// shaper.removeFeature( Shaper::Feature::LIGATURES );
		std::vector<Shaper::Glyph> glyphs = shaper.getShapedText( shaperText );

		Run run = { font };

		ci::vec2 pen( 0 );

		for( auto& glyph : glyphs ) {
			ci::vec2 pos = pen + glyph.offset;

			Glyph layoutGlyph = { glyph.index, pos };

			run.glyphs.push_back( layoutGlyph );

			pen += glyph.advance;
		}

		Line line;
		line.runs.push_back( run );

		mLines.push_back( line );
	}
}