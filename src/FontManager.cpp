#include "FontManager.h"

#include "cinder/app/App.h"
#include "cinder/Log.h"

namespace txt
{
	// Font
	Font::Font( ci::fs::path path, int size )
		: size( size )
		, faceId( FontManager::get()->getFaceId( path ) )
	{}


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

	FT_Face FontManager::getFace( Font& font )
	{
		// Generate face ID and create scaler
		FTC_Scaler scaler = new FTC_ScalerRec_();
		scaler->face_id = font.faceId;
		scaler->pixel = 0;
		scaler->width = float( font.size ) * 64.f;
		scaler->height = float( font.size ) * 64.f;
		scaler->x_res = 72;
		scaler->y_res = 72;

		// Get FT Size
		FT_Size ftSize;
		FT_Error error;
		error = FTC_Manager_LookupSize( mFTCacheManager, scaler, &ftSize );

		std::stringstream errorMessage;
		errorMessage << "Could not lookup size for face " << font.faceId << " at size " << std::to_string( font.size ) << ".";
		checkForFTError( error, errorMessage.str() );

		return ftSize->face;
	}

	FT_UInt FontManager::getGlyphIndex( FTC_FaceID faceId, FT_UInt32 charCode, FT_Int mapIndex )
	{
		return FTC_CMapCache_Lookup( mFTCMapCache, faceId, mapIndex, charCode );
	}

	FT_Glyph FontManager::getGlyph( FTC_FaceID faceId, FT_UInt glyphIndex )
	{
		FTC_ImageType imageType = new FTC_ImageTypeRec_();
		imageType->face_id = faceId;

		FT_Glyph glyph;
		FT_Error error;
		error = FTC_ImageCache_Lookup( mFTCImageCache, imageType, glyphIndex, &glyph, NULL );

		std::stringstream errorMessage;
		errorMessage << "Could not get glyph " << glyphIndex << " for face " << faceId << ".";
		checkForFTError( error, errorMessage.str() );
	}

	FT_Error FontManager::faceRequestor( FTC_FaceID face_id, FT_Library library, FT_Pointer req_data, FT_Face* aface )
	{
		std::string fontName = FontManager::get()->mFaceNamesForID[face_id];

		FT_Error error;
		error = FT_New_Face( library,
		                     fontName.c_str(),
		                     0,
		                     aface );
		return error;
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

	FTC_FaceID FontManager::getFaceId( ci::fs::path path )
	{
		if( mFaceIDsForName.count( path.string() ) == 0 ) {
			createFaceId( path.string() );
		}

		return ( FTC_FaceID )mFaceIDsForName[path.string()];
	}

	void FontManager::createFaceId( std::string faceName )
	{
		mNextFaceId++;

		FTC_FaceID id = ( FTC_FaceID )mNextFaceId;

		mFaceIDsForName[faceName] = id;
		mFaceNamesForID[id] = faceName;

	}

	void FontManager::removeFaceId( FTC_FaceID id )
	{
		std::string name = mFaceNamesForID[id];
		mFaceIDsForName.erase( name );
		mFaceNamesForID.erase( id );

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