#include "txt/FontManager.h"

#include <ShellScalingAPI.h>

#include "cinder/app/App.h"
#include "cinder/Log.h"
#include "cinder/gl/Context.h"

#include "harfbuzz/hb-ft.h"

#include "txt/SystemFonts.h"

namespace txt
{
	// Font Manager
	FontManagerRef FontManager::get()
	{
		static FontManagerRef ref = nullptr;

		if( ref == nullptr ) {
			ref = FontManagerRef( new FontManager() );
		}

		return ref;
	}

	FontManager::FontManager()
		: mNextFaceId( -1 )
	{
		initFreetype();
	}

	std::string FontManager::getFontFamily( const Font& font )
	{
		FaceFamilyAndStyle familyStyle( getFace( font ) );
		return familyStyle.family;
	}

	std::string FontManager::getFontStyle( const Font& font )
	{
		FaceFamilyAndStyle familyStyle( getFace( font ) );
		ci::app::console() << familyStyle.style << std::endl;
		return familyStyle.style;
	}

	void FontManager::loadFace( const ci::fs::path& path, const std::string& family, const std::string& style )
	{
		if( !mFaceIDsForPaths.count( path.string() ) ) {
			mNextFaceId++;

			uint32_t faceId = mNextFaceId;
			FTC_FaceID id = ( FTC_FaceID )faceId;

			// Store the path <---> id relationship
			mFaceIDsForPaths[path.string()] = id;
			mFacePathsForFaceID[id] = path.string();

			// Load the face family/style values
			FaceFamilyAndStyle familyStyleFromFace( getFace( ( uint32_t )id ) );
			FaceFamilyAndStyle familyStyleFromUser( family, style );

			// If user didn't provide a fonts family or style try to use the values in the face
			std::string f = family.empty() ? familyStyleFromFace.family : familyStyleFromUser.family;
			std::string s = style.empty() ? familyStyleFromFace.style : familyStyleFromUser.style;

			FaceFamilyAndStyle familyStyle( f, s );

			// Store the family/style <----------> id relationship
			registerFamilyStyleForFaceID( familyStyle, id );
		}
	}

	// --------------------------------------------------------
	// Freetype Functions

	FT_Face FontManager::getFace( const Font& font )
	{
		return getFace( font.mFaceId );
	}

	FT_Face FontManager::getFace( uint32_t faceId )
	{
		return getFace( ( FTC_FaceID )faceId );
	}

	FT_Face FontManager::getFace( FTC_FaceID faceId )
	{
		FT_Face face;
		FT_Error error;
		error = FTC_Manager_LookupFace( mFTCacheManager, ( FTC_FaceID )faceId, &face );

		std::stringstream errorMessage;
		errorMessage << "Could not lookup face " << faceId << ".";
		checkForFTError( error, errorMessage.str() );
		return face;
	}

	FT_Size FontManager::getSize( const Font& font )
	{
		FT_Size ftSize;
		FT_Error error;
		error = FTC_Manager_LookupSize( mFTCacheManager, getScaler( font ), &ftSize );

		std::stringstream errorMessage;
		errorMessage << "Could not lookup size for face " << font.mFaceId << " at size " << std::to_string( font.mSize ) << ".";
		checkForFTError( error, errorMessage.str() );

		return ftSize;
	}

	FT_UInt FontManager::getGlyphIndex( const Font& font, FT_UInt32 charCode, FT_Int mapIndex )
	{
		return FTC_CMapCache_Lookup( mFTCMapCache, ( FTC_FaceID )font.mFaceId, mapIndex, charCode );
	}

	std::vector<FT_UInt> FontManager::getGlyphIndices( const Font& font, std::string string )
	{
		std::vector<FT_UInt> indices;

		for( auto& character : string ) {
			indices.push_back( getGlyphIndex( font, character, 0 ) );
		}

		return indices;
	}

	FT_Glyph FontManager::getGlyph( const Font& font, unsigned int glyphIndex )
	{
		//FT_Glyph glyph;
		FT_Glyph glyph;
		FT_Error error;
		error = FTC_ImageCache_LookupScaler( mFTCImageCache, getScaler( font ), NULL, glyphIndex, ( FT_Glyph* )&glyph, NULL );

		std::stringstream errorMessage;
		errorMessage << "Could not get glyph " << glyphIndex << " for face " << font.mFaceId << ".";
		checkForFTError( error, errorMessage.str() );

		return glyph;
	}

	FT_BitmapGlyph FontManager::getGlyphBitmap( const Font& font, unsigned int glyphIndex )
	{
		//FT_Glyph glyph;
		FT_BitmapGlyph glyph;
		FT_Error error;
		error = FTC_ImageCache_LookupScaler( mFTCImageCache, getScaler( font ), FT_LOAD_RENDER | FT_RENDER_MODE_NORMAL, glyphIndex, ( FT_Glyph* )&glyph, NULL );

		std::stringstream errorMessage;
		errorMessage << "Could not get glyph " << glyphIndex << " for face " << font.mFaceId << ".";
		checkForFTError( error, errorMessage.str() );

		return glyph;
	}

	ci::vec2 FontManager::getGlyphSize( const Font& font, unsigned int glyphIndex )
	{
		FT_BitmapGlyph glyph = getGlyphBitmap( font, glyphIndex );
		return ci::vec2( glyph->bitmap.width, glyph->bitmap.rows );
	}

	ci::vec2 FontManager::getMaxGlyphSize( const Font& font )
	{
		FT_Size size = getSize( font );
		FT_BBox bbox = size->face->bbox;

		bbox.xMin = FT_MulFix( bbox.xMin, size->metrics.x_scale );
		bbox.yMin = FT_MulFix( bbox.yMin, size->metrics.y_scale );
		bbox.xMax = FT_MulFix( bbox.xMax, size->metrics.x_scale );
		bbox.yMax = FT_MulFix( bbox.yMax, size->metrics.y_scale );

		return ci::vec2( bbox.xMax - bbox.xMin, bbox.yMax - bbox.yMin );
	}

	FTC_Scaler FontManager::getScaler( const  Font& font )
	{
		FTC_Scaler scaler = new FTC_ScalerRec_();
		scaler->face_id = ( FTC_FaceID )font.mFaceId;
		scaler->pixel = 0;
		scaler->width = float( font.mSize ) * 64.f;
		scaler->height = float( font.mSize ) * 64.f;

		//SetProcessDPIAware(); //true
		//HDC screen = GetDC( NULL );
		//double hPixelsPerInch = GetDeviceCaps( screen, LOGPIXELSX );
		//double vPixelsPerInch = GetDeviceCaps( screen, LOGPIXELSY );
		//ReleaseDC( NULL, screen );

		//scaler->x_res = hPixelsPerInch;
		//scaler->y_res = vPixelsPerInch;

		scaler->x_res = 72;
		scaler->y_res = 72;

		return scaler;
	}

	// This function gets called by the cache when a new face_id is requested
	FT_Error FontManager::faceRequestor( FTC_FaceID face_id, FT_Library library, FT_Pointer req_data, FT_Face* aface )
	{
		FontManagerRef fontManager = FontManager::get();

		FT_Error error;

		// Try to load the font from a file
		if( fontManager->mFacePathsForFaceID.count( face_id ) != 0 ) {
			ci::fs::path fontPath = fontManager->mFacePathsForFaceID[face_id];

			error = FT_New_Face( library,
			                     fontPath.string().c_str(),
			                     0,
			                     aface );

			std::stringstream errorMessage;
			errorMessage << "Could not load face for font file: " << fontPath;
			FontManager::checkForFTError( error, errorMessage.str() );
			return error;
		}

		// Otherwise try to load it as a system font
		else {
			FaceFamilyAndStyle familyStyle = fontManager->mFamilyAndStyleForFaceIDs[face_id];

			ci::BufferRef buffer = SystemFonts::get()->getFontBuffer( familyStyle.family, familyStyle.style );

			if( buffer != nullptr ) {
				error = FT_New_Memory_Face( library, reinterpret_cast<FT_Byte*>( buffer->getData() ), static_cast<FT_Long>( buffer->getSize() ), 0, aface );

				std::stringstream errorMessage;
				errorMessage << "Could not load system font with family-name: " << familyStyle.family << " and style: " << familyStyle.style;
				FontManager::checkForFTError( error, errorMessage.str() );
				return error;
			}
		}
	}

	// Freetype Initialization
	void FontManager::initFreetype()
	{
		// Init Freetype
		FT_Error error;
		error = FT_Init_FreeType( &mFTLibrary );
		checkForFTError( error, "Could not initialize Freetype." );

		// Create Cache Manager
		error = FTC_Manager_New( mFTLibrary, 0, 0, 0, &FontManager::faceRequestor, NULL, &mFTCacheManager );
		checkForFTError( error, "Could not initialize FTCacheManager" );

		// Create Char Map Cache
		error = FTC_CMapCache_New( mFTCacheManager, &mFTCMapCache );
		checkForFTError( error, "Could not initialize FTCMapCache" );

		// Create Image Cache (Glyph Images)
		error = FTC_ImageCache_New( mFTCacheManager, &mFTCImageCache );
		checkForFTError( error, "Could not initialize FTCImageCache" );
	}

	void FontManager::registerFamilyStyleForFaceID( const FaceFamilyAndStyle& familyStyle, FTC_FaceID id )
	{
		mFamilyAndStyleForFaceIDs[id] = familyStyle;
		mFaceIDsForFamilyAndStyle[familyStyle] = id;
	}

	uint32_t FontManager::getFaceId( const ci::fs::path& path )
	{
		if( mFaceIDsForPaths.count( path.string() ) == 0 ) {
			loadFace( path );
		}

		return ( uint32_t )mFaceIDsForPaths[path.string()];
	}

	uint32_t FontManager::getFaceId( std::string family, std::string style )
	{
		FaceFamilyAndStyle familyStyle( family, style );

		if( mFaceIDsForFamilyAndStyle.count( familyStyle ) == 0 ) {
			loadFace( familyStyle );
		}

		return ( uint32_t )mFaceIDsForFamilyAndStyle[familyStyle];
	}

	void FontManager::loadFace( const FaceFamilyAndStyle& familyStyle )
	{
		if( !mFaceIDsForFamilyAndStyle.count( familyStyle ) ) {
			mNextFaceId++;

			FTC_FaceID faceId = ( FTC_FaceID )mNextFaceId;

			registerFamilyStyleForFaceID( familyStyle, faceId );

			getFace( ( uint32_t )faceId );
		}
	}

	void FontManager::removeFace( FTC_FaceID id )
	{
		// Remove family/style cached id
		if( mFamilyAndStyleForFaceIDs.count( id ) != 0 ) {
			FaceFamilyAndStyle familyStyle = mFamilyAndStyleForFaceIDs[id];
			mFaceIDsForFamilyAndStyle.erase( familyStyle );
			mFamilyAndStyleForFaceIDs.erase( id );
		}

		// Remove path cached id
		if( mFacePathsForFaceID.count( id ) != 0 ) {
			ci::fs::path path = mFacePathsForFaceID[id];
			mFaceIDsForPaths.erase( path.string() );
			mFacePathsForFaceID.erase( id );
		}

		// Empty face from cache
		FTC_Manager_RemoveFaceID( mFTCacheManager, id );
	}

	// Error Checking
	void FontManager::checkForFTError( FT_Error error, std::string description )
	{
		if( error != FT_Err_Ok ) {
			std::stringstream ss;
			ss << "Freetype Error: Description: ";
			ss << description;
			ss << " Error Message:";
			ss << getFTErrorMessage( error );

			CI_LOG_E( ss.str() );
		}
	}

	const char* FontManager::getFTErrorMessage( FT_Error err )
	{
		std::string ftErrorMessage;
#undef __FTERRORS_H__
#define FT_ERRORDEF( e, v, s )  case e: return s;
#define FT_ERROR_START_LIST     switch (err) {
#define FT_ERROR_END_LIST       }
#include FT_ERRORS_H
		return "(Unknown error)";
	}
}