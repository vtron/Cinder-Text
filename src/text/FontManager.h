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

			Font getFont( std::string family, std::string style, int size );

			std::string getFontFamily( const Font& font );
			std::string getFontStyle( const Font& font );

			// Freetype
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

			void initFreetype();

			static FT_Error faceRequestor( FTC_FaceID face_id, FT_Library library, FT_Pointer req_data, FT_Face* aface );

			static void checkForFTError( FT_Error error, std::string description );
			static const char* getFTErrorMessage( FT_Error error );

			struct FaceNameAndStyle {
				FaceNameAndStyle( FT_Face face )
					: FaceNameAndStyle( face->family_name, face->style_name ) {}

				FaceNameAndStyle( std::string family, std::string style )
					: family( family )
					, style( style )
				{
					std::transform( family.begin(), family.end(), family.begin(), ::tolower );
					std::transform( style.begin(), style.end(), style.begin(), ::tolower );
				}

				std::string family;
				std::string style;
			};

			uint32_t getFaceId( ci::fs::path path );
			void loadFace( std::string faceName );
			void removeFace( FTC_FaceID id );

		private:
			uint32_t mNextFaceId;

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