#pragma once

#include "cinder/Filesystem.h"

namespace txt
{
	typedef struct Font {
		public:
			Font( ci::fs::path path, int size );

			const uint32_t faceId;
			const unsigned int size;

			bool operator==( const Font& other ) const
			{
				return ( faceId == other.faceId
				         && size == other.size );
			}

			friend class FontManager;
	} Font;
}

// Hash Function
namespace std
{
	template <>
	struct hash<txt::Font> {
		std::size_t operator()( const txt::Font& k ) const
		{
			using std::size_t;
			using std::hash;
			using std::string;

			// Compute individual hash values for first,
			// second and third and combine them using XOR
			// and bit shifting:

			return ( ( hash < uint32_t>()( ( uint32_t )k.faceId )
			           ^ ( hash<unsigned int>()( k.size ) << 1 ) ) >> 1 );
		}
	};

}