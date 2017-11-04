#pragma once

#include <memory>
#include <string>

#include "harfbuzz/hb.h"
#include "cinder/Vector.h"

#include "txt/Font.h"

namespace txt
{
	class Shaper
	{
		public:
			enum Feature {
				LIGATURES,
				KERNING,
				CLIG
			};

			typedef struct {
				std::string data;
				std::string language;
				hb_script_t script;
				hb_direction_t direction;
				const char* c_data() { return data.c_str(); };
			} Text;

			typedef struct {
				uint32_t index;

				ci::vec2 offset;
				ci::vec2 advance;
				uint32_t cluster;
			} Glyph;

			Shaper( const Font& font );
			~Shaper();

			std::vector<Shaper::Glyph> getShapedText( Text& text );
			void addFeature( Feature feature );
			void removeFeature( Feature feature );

		private:
			// Harfbuzz
			hb_font_t* getHarfbuzzFont( Font& font ) { return mFont; };

			hb_font_t* mFont;
			hb_buffer_t* mBuffer;
			std::vector<hb_feature_t> mFeatures;
	};
}
