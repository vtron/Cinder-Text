#include "txt/Font.h"
#include "txt/FontManager.h"
#include "txt/SystemFonts.h"

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

	Font::Font( std::string family, std::string style, int size )
		: size( size )
		, faceId( FontManager::get()->getFaceId( family, style ) )
	{
	}

	DefaultFont::DefaultFont()
		: Font( SystemFonts::get()->getDefaultFamily(), SystemFonts::get()->getDefaultStyle(), SystemFonts::get()->getDefaultSize() )
	{}
}