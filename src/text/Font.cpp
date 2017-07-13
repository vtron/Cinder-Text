#include "text/Font.h"
#include "text/FontManager.h"

namespace txt
{
	// Font
	Font::Font( ci::fs::path path, int size )
		: path( path )
		, size( size )
		, faceId( FontManager::get()->getFaceId( path ) )
	{}
}