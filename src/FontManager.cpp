#include "FontManager.h"

#include "cinder/app/App.h"
#include "cinder/Log.h"

namespace txt
{
	// Font
	FontRef Font::create( std::string name, int size )
	{
		return FontManager::get()->createFont( name, size );
	}

	Font::Font( FTC_FaceID id, int size )
		: mSize( size )
		, mFaceId( id )
	{}

	Font::~Font()
	{
		FontManager::get()->dereferenceFaceID( mFaceId );
	}


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
		loadFreetype();
		initFTCacheManager();
	}

	FontRef FontManager::createFont( std::string name, int size )
	{
		FTC_FaceID id = getFaceId( name );

		if( mFaceReferenceCountsForID.count( id ) == 0 ) {
			mFaceReferenceCountsForID[id] = 1;
		}
		else {
			mFaceReferenceCountsForID[id]++;
		}

		return FontRef( new Font( id, size ) );
	}

	void FontManager::dereferenceFaceID( FTC_FaceID id )
	{
		CI_ASSERT_MSG( mFaceReferenceCountsForID.count( id ) != 0, "Can not remove font by FaceID, it is not being tracked." );

		mFaceReferenceCountsForID[id]--;

		if( mFaceReferenceCountsForID[id] <= 0 ) {
			removeFaceId( id );
		}
	}

	FT_Face FontManager::getFace( FontRef font )
	{
		// Generate face ID and create scaler
		FTC_Scaler scaler = new FTC_ScalerRec_();
		scaler->face_id = font->getFaceId();
		scaler->pixel = 0;
		scaler->width = float( font->getSize() ) / 64.f;
		scaler->height = float( font->getSize() ) / 64.f;
		scaler->x_res = 72;
		scaler->y_res = 72;

		// Get FT Size
		FT_Size ftSize;
		FT_Error error;
		error = FTC_Manager_LookupSize( mFTCacheManager, scaler, &ftSize );

		std::stringstream errorMessage;
		errorMessage << "Could not lookup size for face " << font->getFaceId() << " at size " << std::to_string( font->getSize() ) << ".";
		checkForFTError( error, errorMessage.str() );

		return ftSize->face;
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
	void FontManager::loadFreetype()
	{
		FT_Error error;
		error = FT_Init_FreeType( &mFTLibrary );
		checkForFTError( error, "Could not initialize Freetype." );
	}

	void FontManager::initFTCacheManager()
	{
		FT_Error error;

		error = FTC_Manager_New( mFTLibrary, 0, 0, 0, &FontManager::faceRequestor, NULL, &mFTCacheManager );
		checkForFTError( error, "Could not initialize FTCacheManager" );
	}

	FTC_FaceID FontManager::getFaceId( std::string faceName )
	{
		if( mFaceIDsForName.count( faceName ) == 0 ) {
			createFaceId( faceName );
		}

		return ( FTC_FaceID )mFaceIDsForName[faceName];
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