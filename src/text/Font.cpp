#include "text/Font.h"
#include "text/FontManager.h"

namespace txt
{
	// Font
	Font::Font( ci::fs::path path, int size )
		: size( size )
		, faceId( FontManager::get()->getFaceId( path ) )
	{
	}

	Font::Font( uint32_t faceId, int size )
		: size( size )
		, faceId( faceId )
	{

	}
}