#pragma once

#include <memory>
#include <unordered_map>

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

			// Preload a face so that it can be referenced in rich text
			void loadFace( ci::fs::path path );

			// Get a font for a pre-loaded face or system font
			Font getFont( std::string family, std::string style, int size );

			// Get the font family or style for a previously loaded or system font
			std::string getFontFamily( const Font& font );
			std::string getFontStyle( const Font& font );

			// Freetype functions, used by renderers and shapers
			uint32_t getGlyphIndex( const Font& font, FT_UInt32 charCode, FT_Int mapIndex = 0 );
			std::vector<uint32_t> getGlyphIndices( const Font& font, std::string string );

			FT_Glyph getGlyph( const Font& font, unsigned int glyphIndex );
			FT_BitmapGlyph getGlyphBitmap( const Font& font, unsigned int glyphIndex );

			ci::vec2 getMaxGlyphSize( const Font& font );

			FT_Face getFace( const Font& font );
			FT_Face getFace( uint32_t faceId );
			FT_Size getSize( const Font& font );
			FTC_Scaler getScaler( const Font& font );

		protected:
			FontManager();

			// Load freetype libs
			void initFreetype();

			// Callback function used by FTCache, loads fonts when not present and requested
			static FT_Error faceRequestor( FTC_FaceID face_id, FT_Library library, FT_Pointer req_data, FT_Face* aface );

			// FT Error message wrapper
			static void checkForFTError( FT_Error error, std::string description );
			static const char* getFTErrorMessage( FT_Error error );

			uint32_t getFaceId( ci::fs::path path );
			void registerFace( std::string faceName );
			void removeFace( FTC_FaceID id );

		private:
			uint32_t mNextFaceId;

			// Lookup tables for FTC_FaceID caching
			std::unordered_map<std::string, FTC_FaceID> mFaceIDsForName;
			std::unordered_map<FTC_FaceID, std::string> mFaceNamesForID;
			std::unordered_map<std::string, std::unordered_map<std::string, FTC_FaceID> > mFaceIdsForFamilyAndStyle;

			// Freetype libs + caches
			FT_Library mFTLibrary;
			FTC_Manager mFTCacheManager;
			FTC_CMapCache mFTCMapCache;
			FTC_ImageCache mFTCImageCache;
	};
}