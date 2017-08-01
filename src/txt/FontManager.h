#pragma once

#include <memory>
#include <unordered_map>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/ftcache.h>

#include "cinder/Filesystem.h"
#include "cinder/Vector.h"

#include "cinder/app/App.h"

#include "txt/Font.h"

namespace txt
{
	// Lookup tables for family + style to face id
	// Convenience struct to transform font family + style to lowercase
	// gives a better chance of matching
	struct FaceFamilyAndStyle {

		FaceFamilyAndStyle()
		{

		}

		FaceFamilyAndStyle( FT_Face face )
			: FaceFamilyAndStyle( face->family_name, face->style_name ) {}

		FaceFamilyAndStyle( std::string family, std::string style )
			: family( family )
			, style( style )
		{
			std::transform( this->family.begin(), this->family.end(), this->family.begin(), ::tolower );
			std::transform( this->style.begin(), this->style.end(), this->style.begin(), ::tolower );
		}

		FaceFamilyAndStyle( const FaceFamilyAndStyle& familyStyle ) : FaceFamilyAndStyle( familyStyle.family, familyStyle.style ) { }

		std::string family;
		std::string style;

		bool operator==( const FaceFamilyAndStyle& other ) const
		{
			ci::app::console() << "Family: " << this->family << " Other Family: " << other.family << std::endl;
			ci::app::console() << "Style: " << this->style << " Other Style: " << other.style << std::endl;
			bool match = ( this->family == other.family && this->style == other.style );
			return match;
		}

		FaceFamilyAndStyle& FaceFamilyAndStyle::operator=( const Font& other )
		{
			return *this;
		}
	};
}

// FaceFamilyStyle Hash Function
namespace std
{
	template <>
	struct hash<txt::FaceFamilyAndStyle> {
		std::size_t operator()( const txt::FaceFamilyAndStyle& k ) const
		{
			using std::size_t;
			using std::hash;
			using std::string;

			// Compute individual hash values for first,
			// second and third and combine them using XOR
			// and bit shifting:

			return ( ( hash < std::string>()( ( k.family ) )
			           ^ ( hash<std::string>()( k.style ) << 1 ) ) >> 1 );
		}
	};
}

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
			FT_Face getFace( FTC_FaceID faceId );
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

			uint32_t getFaceId( const ci::fs::path& path );
			uint32_t getFaceId( std::string family, std::string style );

			void loadFace( const FaceFamilyAndStyle& familyStyle );
			void removeFace( FTC_FaceID id );

		private:
			uint32_t mNextFaceId;

			// Lookup tables for FTC_FaceID caching
			std::unordered_map <std::string, FTC_FaceID > mFaceIDsForPaths;
			std::unordered_map < FTC_FaceID, std::string > mFacePathsForFaceID;

			void registerFamilyStyleForFaceID( const FaceFamilyAndStyle& familyStyle, FTC_FaceID id );
			std::unordered_map<FTC_FaceID, FaceFamilyAndStyle > mFamilyAndStyleForFaceIDs;
			std::unordered_map<FaceFamilyAndStyle,  FTC_FaceID> mFaceIDsForFamilyAndStyle;

			// Freetype libs + caches
			FT_Library mFTLibrary;
			FTC_Manager mFTCacheManager;
			FTC_CMapCache mFTCMapCache;
			FTC_ImageCache mFTCImageCache;
	};
}