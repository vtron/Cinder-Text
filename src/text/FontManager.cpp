#include "text/FontManager.h"

#include <ShellScalingAPI.h>

#include "cinder/app/App.h"
#include "cinder/Log.h"
#include "cinder/gl/Context.h"

#include "harfbuzz/hb-ft.h"

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

	Font FontManager::getFont( std::string family, std::string style, int size )
	{
		FaceNameAndStyle fns( family, style );

		if( mFaceIdsForFamilyAndStyle.count( fns.family ) == 0 ) {
			CI_LOG_F( "Font family not registered: " + family );
		}

		else if( mFaceIdsForFamilyAndStyle[fns.family].count( fns.style ) == 0 ) {
			CI_LOG_F( "Style not registered: " + style + " for Font Family: " + family );
		}

		return Font( ( uint32_t ) mFaceIdsForFamilyAndStyle[fns.family][fns.style], size );
	}

	std::string FontManager::getFontFamily( const Font& font )
	{
		FT_Face face = getFace( font );
		return face->family_name;
	}

	std::string FontManager::getFontStyle( const Font& font )
	{
		FT_Face face = getFace( font );
		return face->style_name;
	}

	// --------------------------------------------------------
	// Freetype Functions

	FT_Face FontManager::getFace( const Font& font )
	{
		return getFace( font.faceId );
	}

	FT_Face FontManager::getFace( uint32_t faceId )
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
		errorMessage << "Could not lookup size for face " << font.faceId << " at size " << std::to_string( font.size ) << ".";
		checkForFTError( error, errorMessage.str() );

		return ftSize;
	}

	FT_UInt FontManager::getGlyphIndex( const Font& font, FT_UInt32 charCode, FT_Int mapIndex )
	{
		return FTC_CMapCache_Lookup( mFTCMapCache, ( FTC_FaceID )font.faceId, mapIndex, charCode );
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
		errorMessage << "Could not get glyph " << glyphIndex << " for face " << font.faceId << ".";
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
		errorMessage << "Could not get glyph " << glyphIndex << " for face " << font.faceId << ".";
		checkForFTError( error, errorMessage.str() );

		return glyph;
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
		scaler->face_id = ( FTC_FaceID )font.faceId;
		scaler->pixel = 0;
		scaler->width = float( font.size ) * 64.f;
		scaler->height = float( font.size ) * 64.f;

		//SetProcessDPIAware(); //true
		//HDC screen = GetDC( NULL );
		//double hPixelsPerInch = GetDeviceCaps( screen, LOGPIXELSX );
		//double vPixelsPerInch = GetDeviceCaps( screen, LOGPIXELSY );
		//ReleaseDC( NULL, screen );

		//scaler->x_res = hPixelsPerInch;
		//scaler->y_res = vPixelsPerInch;

		scaler->x_res = 96;
		scaler->y_res = 96;

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
			loadFace( path.string() );
		}

		return ( uint32_t )mFaceIDsForName[path.string()];
	}

	void FontManager::loadFace( std::string faceName )
	{
		mNextFaceId++;

		uint32_t faceId = mNextFaceId;
		FTC_FaceID id = ( FTC_FaceID )faceId;

		mFaceIDsForName[faceName] = id;
		mFaceNamesForID[id] = faceName;

		FaceNameAndStyle fns( getFace( ( uint32_t )id ) );
		mFaceIdsForFamilyAndStyle[fns.family][fns.style] = id;

		FT_Face face = getFace( faceId );
		ci::app::console() << face->family_name << std::endl;
		ci::app::console() << face->style_name << std::endl;
	}

	void FontManager::removeFace( FTC_FaceID id )
	{
		std::string name = mFaceNamesForID[id];
		mFaceIDsForName.erase( name );
		mFaceNamesForID.erase( id );

		FaceNameAndStyle fns( getFace( ( uint32_t ) id ) );

		mFaceIdsForFamilyAndStyle[fns.family].erase( fns.style );

		if( mFaceIdsForFamilyAndStyle[fns.family].size() == 0 ) {
			mFaceIdsForFamilyAndStyle.erase( fns.family );
		}

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