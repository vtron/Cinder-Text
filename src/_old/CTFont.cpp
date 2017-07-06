//
//  CTFont.cpp
//  FreetypeBasicLoading
//
//  Created by Stephen Varga on 9/23/16.
//
//
#include "cinder/app/App.h"

#include "cinder/CinderAssert.h"
#include "CTFont.hpp"
#include "harfbuzz/hb-ft.h"


namespace typography
{
	static FT_Library freetypeLibrary;

	void loadFreetype()
	{
		// Load Freetype
		FT_Error error = FT_Init_FreeType( &freetypeLibrary );

		if( error != FT_Err_Ok ) {
			ci::app::console() << "Could not init freetype due to error: " << error << std::endl;
			return;
		}
		else {
			ci::app::console() << "Freetype initialized!" << std::endl;
		}
	}

	FontRef Font::create( ci::DataSourceRef dataSource, float size )
	{
		FontRef ref( new Font( dataSource, size ) );
		return ref;
	}

	Font::Font( ci::DataSourceRef dataSource, float size )
		: mSize( size )
	{
		loadFreetype();

		// Load Face
		FT_Error error = FT_New_Face( freetypeLibrary, dataSource->getFilePath().string().c_str(), 0, &mFace );

		if( error ==  FT_Err_Unknown_File_Format ) {
			ci::app::console() << "Error loading font: Unknown Format for font at path: " << dataSource->getFilePath() << std::endl;
		}
		else if( error != FT_Err_Ok ) {
			ci::app::console() << "Could not load font " << dataSource->getFilePath() << std::endl;
			ci::app::console() << error << std::endl;
			return;
		}
		else {
			ci::app::console() << "Loaded font from: " << dataSource->getFilePath() << std::endl;
		}

		// Set the size
		error = FT_Set_Char_Size( mFace, 0, size * 64.0, 72, 0 );

		if( error != FT_Err_Ok ) {
			ci::app::console() << "Could not set the character size " << size << ", this may indicate that the font uses a fixed-size that was not specified." << std::endl;
		}

		// Get the glyph indices
		FT_ULong charcode;
		FT_UInt index;

		charcode = FT_Get_First_Char( mFace, &index );

		while( index != 0 ) {
			mGlyphIndices.push_back( index );
			charcode = FT_Get_Next_Char( mFace, charcode, &index );
		}

		// Create Harfbuzz font
		mHbFont = hb_ft_font_create( mFace, NULL );
	}

	Font::~Font()
	{
		hb_font_destroy( mHbFont );
		FT_Done_Face( mFace );
	}

	// Load a glyph slot
	// This should probably be an assert or something
	bool Font::loadGlyphAtIndex( FT_UInt index )
	{
		FT_Error error = FT_Load_Glyph( mFace, index, 0 );

		if( error != FT_Err_Ok ) {
			return false;
		}

		return true;
	}

	// Bitmap to Cinder Channel
	ci::ChannelRef Font::getCharacterBitmapChannel( char character )
	{
		FT_UInt glyphIndex = FT_Get_Char_Index( mFace, character );
		return getBitmapChannel( glyphIndex );
	}

	ci::ChannelRef Font::getBitmapChannel( FT_UInt glyphIndex )
	{
		if( loadGlyphAtIndex( glyphIndex ) ) {
			FT_Error error = FT_Render_Glyph( mFace->glyph, FT_RENDER_MODE_NORMAL );

			FT_GlyphSlot slot = mFace->glyph;
			ci::ChannelRef channel = ci::Channel::create( slot->bitmap.width, slot->bitmap.rows, slot->bitmap.width * sizeof( unsigned char ), 1, slot->bitmap.buffer );
			return channel;
		}

		return nullptr;
	}


	// Glyph Information
	FT_Glyph_Metrics Font::getMetrics( FT_UInt glyphIndex )
	{
		if( loadGlyphAtIndex( glyphIndex ) ) {
			return mFace->glyph->metrics;
		}
	}
}
