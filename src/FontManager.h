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

	class Font;
	typedef std::shared_ptr<Font> FontRef;

	class Font
	{
		public:
			static FontRef create( std::string name, int size );

			~Font();

			const FTC_FaceID getFaceId() { return mFaceId; }
			const int getSize() { return mSize; }

		private:
			friend class FontManager;
			Font( FTC_FaceID id, int size );

			const FTC_FaceID mFaceId;
			const int mSize;
	};

	class FontManager
	{
			friend class Font;
		public:
			static FontManagerRef get();

			FT_Face getFace( FontRef font );

		protected:
			FontManager();

			FontRef createFont( std::string name, int size );
			void dereferenceFaceID( FTC_FaceID id );

			void loadFreetype();
			void initFTCacheManager();

			static FT_Error faceRequestor( FTC_FaceID face_id, FT_Library library, FT_Pointer req_data, FT_Face* aface );

			void checkForFTError( FT_Error error, std::string description );
			const char* getFTErrorMessage( FT_Error error );

			FTC_FaceID getFaceId( std::string faceName );
			void createFaceId( std::string faceName );
			void removeFaceId( FTC_FaceID id );
			uint32_t mNextFaceId;

			std::unordered_map<std::string, FTC_FaceID> mFaceIDsForName;
			std::unordered_map<FTC_FaceID, std::string> mFaceNamesForID;
			std::unordered_map < FTC_FaceID, uint32_t> mFaceReferenceCountsForID;

			// Freetype libs + caches
			FT_Library mFTLibrary;
			FTC_Manager mFTCacheManager;
	};
}