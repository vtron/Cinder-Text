#pragma once

#include <memory>
#include <unordered_map>

#include <harfbuzz/hb.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/ftcache.h>

#include "cinder/Filesystem.h"
#include "cinder/Vector.h"

#include "text/Font.h"

namespace txt
{
	class FontManager;
	typedef std::shared_ptr<FontManager> FontManagerRef;

	class FontManager
	{
			friend struct Font;
		public:
			static FontManagerRef get();

			// Freetype
			uint32_t getGlyphIndex( uint32_t faceId, FT_UInt32 charCode, FT_Int mapIndex = -1 );
			std::vector<FT_UInt> getGlyphIndices( uint32_t faceId, std::string string );

			FT_Glyph getGlyph( Font& font, unsigned int glyphIndex );
			FT_BitmapGlyph getGlyphBitmap( Font& font, unsigned int glyphIndex );

			ci::vec2 getMaxGlyphSize( Font& font );

			FT_Face getFace( Font& font );
			FT_Size getSize( Font& font );
			FTC_Scaler getScaler( Font& font );

			// Harfbuzz
			hb_font_t* getHarfbuzzFont( Font& font );

		protected:
			FontManager();

			void initFreetype();

			static FT_Error faceRequestor( FTC_FaceID face_id, FT_Library library, FT_Pointer req_data, FT_Face* aface );

			static void checkForFTError( FT_Error error, std::string description );
			static const char* getFTErrorMessage( FT_Error error );

			uint32_t getFaceId( ci::fs::path path );
			void createFaceId( std::string faceName );
			void removeFaceId( FTC_FaceID id );

		private:
			uint32_t mNextFaceId;

			std::unordered_map<std::string, FTC_FaceID> mFaceIDsForName;
			std::unordered_map<FTC_FaceID, std::string> mFaceNamesForID;

			// Freetype libs + caches
			FT_Library mFTLibrary;
			FTC_Manager mFTCacheManager;
			FTC_CMapCache mFTCMapCache;
			FTC_ImageCache mFTCImageCache;
	};
}