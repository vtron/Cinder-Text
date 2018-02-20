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
				uint32_t index;

				ci::vec2 offset;
				ci::vec2 advance;
				uint32_t cluster;
				std::string text;
				std::vector<int> textIndices;
			} Glyph;

			class Properties
			{
				public:
					Properties() : mIsDefault( true ) {}

					Properties( hb_script_t script, hb_direction_t direction, std::string language )
						: mScript( script )
						, mDirection( direction )
						, mLanguage( language )
						, mIsDefault( false )
					{}

					bool isDefault() const { return mIsDefault; }
					const hb_script_t& getScript() const { return mScript; }
					const hb_direction_t& getDirection() const { return mDirection; }
					const std::string& getLanguage() const { return mLanguage; }

				private:
					hb_script_t mScript;
					hb_direction_t mDirection;
					std::string mLanguage;

					bool mIsDefault;
			};

			Shaper( const Font& font );
			~Shaper();

			std::vector<Shaper::Glyph> getShapedText( std::string text );
			void addFeature( Feature feature );
			void removeFeature( Feature feature );

			// Properties
			static Properties getPropertiesForString( std::string string );
			void setProperties( const Properties& properties );


		private:
			// Harfbuzz
			hb_font_t* getHarfbuzzFont( Font& font ) { return mFont; };

			hb_font_t* mFont;
			hb_buffer_t* mBuffer;
			std::vector<hb_feature_t> mFeatures;

			// Manual Segment properties
			// If any are set, we switch over to using manual
			// vs auto
			Properties mProperties;
			bool mUseManualProperties;
	};
}
