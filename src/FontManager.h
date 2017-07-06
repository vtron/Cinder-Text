#pragma once

#include <memory>
#include <unordered_map>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/ftcache.h>

#include "cinder/Filesystem.h"
namespace txt
{
	class FontManager;
	typedef std::shared_ptr<FontManager> FontManagerRef;

	struct Font {
		public:
			Font( ci::fs::path path, int size );

			const FTC_FaceID faceId;
			const int size;

			friend class FontManager;
	};

	class FontManager
	{
			friend class Font;
		public:
			static FontManagerRef get();

			FT_Face getFace( Font& font );

		protected:
			FontManager();

			void initFreetype();

			static FT_Error faceRequestor( FTC_FaceID face_id, FT_Library library, FT_Pointer req_data, FT_Face* aface );

			void checkForFTError( FT_Error error, std::string description );
			const char* getFTErrorMessage( FT_Error error );

			FTC_FaceID getFaceId( ci::fs::path path );
			FT_UInt getGlyphIndex( FTC_FaceID faceId, FT_UInt32 charCode, FT_Int mapIndex = -1 );
			FT_Glyph getGlyph( FTC_FaceID, FT_UInt glyphIndex );

			void createFaceId( std::string faceName );
			void removeFaceId( FTC_FaceID id );
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