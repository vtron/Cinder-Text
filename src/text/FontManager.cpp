#include "text/FontManager.h"

#include <ShellScalingAPI.h>

#include "cinder/app/App.h"
#include "cinder/Log.h"
#include "cinder/gl/Context.h"

#include "harfbuzz/hb-ft.h"

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

	// --------------------------------------------------------
	// Freetype Functions

	FT_Face FontManager::getFace( Font& font )
	{
		FT_Face face;
		FT_Error error;
		error = FTC_Manager_LookupFace( mFTCacheManager, ( FTC_FaceID )font.faceId, &face );

		std::stringstream errorMessage;
		errorMessage << "Could not lookup face " << font.faceId << ".";
		checkForFTError( error, errorMessage.str() );
		return face;
	}

	FT_Size FontManager::getSize( Font& font )
	{
		FT_Size ftSize;
		FT_Error error;
		error = FTC_Manager_LookupSize( mFTCacheManager, getScaler( font ), &ftSize );

		std::stringstream errorMessage;
		errorMessage << "Could not lookup size for face " << font.faceId << " at size " << std::to_string( font.size ) << ".";
		checkForFTError( error, errorMessage.str() );

		return ftSize;
	}

	FT_UInt FontManager::getGlyphIndex( uint32_t faceId, FT_UInt32 charCode, FT_Int mapIndex )
	{
		return FTC_CMapCache_Lookup( mFTCMapCache, ( FTC_FaceID )faceId, mapIndex, charCode );
	}

	std::vector<FT_UInt> FontManager::getGlyphIndices( uint32_t faceId, std::string string )
	{
		std::vector<FT_UInt> indices;

		for( auto& character : string ) {
			indices.push_back( getGlyphIndex( faceId, character, 0 ) );
		}

		return indices;
	}

	FT_Glyph FontManager::getGlyph( Font& font, unsigned int glyphIndex )
	{
		//FT_Glyph glyph;
		FT_Glyph glyph;
		FT_Error error;
		error = FTC_ImageCache_LookupScaler( mFTCImageCache, getScaler( font ), NULL, glyphIndex, ( FT_Glyph* )&glyph, NULL );

		std::stringstream errorMessage;
		errorMessage << "Could not get glyph " << glyphIndex << " for face " << font.faceId << ".";
		checkForFTError( error, errorMessage.str() );

		return glyph;
	}

	FT_BitmapGlyph FontManager::getGlyphBitmap( Font& font, unsigned int glyphIndex )
	{
		//FT_Glyph glyph;
		FT_BitmapGlyph glyph;
		FT_Error error;
		error = FTC_ImageCache_LookupScaler( mFTCImageCache, getScaler( font ), FT_LOAD_RENDER | FT_RENDER_MODE_NORMAL, glyphIndex, ( FT_Glyph* )&glyph, NULL );

		std::stringstream errorMessage;
		errorMessage << "Could not get glyph " << glyphIndex << " for face " << font.faceId << ".";
		checkForFTError( error, errorMessage.str() );

		return glyph;
	}

	ci::vec2 FontManager::getMaxGlyphSize( Font& font )
	{
		FT_Size size = getSize( font );
		FT_BBox bbox = size->face->bbox;

		bbox.xMin = FT_MulFix( bbox.xMin, size->metrics.x_scale );
		bbox.yMin = FT_MulFix( bbox.yMin, size->metrics.y_scale );
		bbox.xMax = FT_MulFix( bbox.xMax, size->metrics.x_scale );
		bbox.yMax = FT_MulFix( bbox.yMax, size->metrics.y_scale );

		return ci::vec2( bbox.xMax - bbox.xMin, bbox.yMax - bbox.yMin );
	}

	FTC_Scaler FontManager::getScaler( Font& font )
	{
		FTC_Scaler scaler = new FTC_ScalerRec_();
		scaler->face_id = ( FTC_FaceID )font.faceId;
		scaler->pixel = 0;
		scaler->width = float( font.size ) * 64.f;
		scaler->height = float( font.size ) * 64.f;

		SetProcessDPIAware(); //true
		HDC screen = GetDC( NULL );
		double hPixelsPerInch = GetDeviceCaps( screen, LOGPIXELSX );
		double vPixelsPerInch = GetDeviceCaps( screen, LOGPIXELSY );
		ReleaseDC( NULL, screen );

		//scaler->x_res = hPixelsPerInch;
		//scaler->y_res = vPixelsPerInch;

		scaler->x_res = 72;
		scaler->y_res = 72;

		return scaler;
	}

	FT_Error FontManager::faceRequestor( FTC_FaceID face_id, FT_Library library, FT_Pointer req_data, FT_Face* aface )
	{
		FontManagerRef fontManager = FontManager::get();
		std::string fontName = FontManager::get()->mFaceNamesForID[face_id];

		FT_Error error;
		error = FT_New_Face( library,
		                     fontName.c_str(),
		                     0,
		                     aface );

		std::stringstream errorMessage;
		errorMessage << "Could not load face for font name: " << fontName;
		FontManager::checkForFTError( error, errorMessage.str() );
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

	uint32_t FontManager::getFaceId( ci::fs::path path )
	{
		if( mFaceIDsForName.count( path.string() ) == 0 ) {
			createFaceId( path.string() );
		}

		return ( uint32_t )mFaceIDsForName[path.string()];
	}

	void FontManager::createFaceId( std::string faceName )
	{
		mNextFaceId++;

		int faceId = mNextFaceId;
		FTC_FaceID id = ( FTC_FaceID )faceId;

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