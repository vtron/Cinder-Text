#pragma once

#include "cinder/Filesystem.h"
#include "cinder/DataSource.h"

namespace txt
{
	struct Font {
		public:
			Font( ci::DataSourceRef dataSource, int size );
			Font( uint32_t faceId, int size );
			Font( std::string family, int size );
			Font( std::string family, std::string style, int size );
			Font( const Font& font ) : Font( font.mFaceId, font.mSize ) { }

			const uint32_t getFaceId() const { return mFaceId; }
			const unsigned int getSize() const { return mSize; }
			std::string getFamily() const;
			std::string getStyle() const;
			float getLineHeight() const;

			bool operator==( const Font& other ) const
			{
				return ( mFaceId == other.mFaceId
				         && mSize == other.mSize );
			}

			Font& Font::operator=( const Font& other )
			{
				mFaceId = other.mFaceId;
				mSize = other.mSize;
				return *this;
			}

			friend std::ostream& operator<< ( std::ostream& os, Font const& font )
			{
				os << "Font:" << std::endl;
				os << "Family: " << font.getFamily() << std::endl;
				os << "Style: " << font.getSize() << std::endl;
				os << "Size: " << font.getSize() << std::endl;
				return os;
			}

			friend class FontManager;

		private:
			uint32_t mFaceId;
			unsigned int mSize;
	};

	struct DefaultFont : public Font {
		public:
			DefaultFont();
	};
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
			return ( ( hash < uint32_t>()( ( uint32_t )k.getFaceId() )
			           ^ ( hash<unsigned int>()( k.getSize() ) << 1 ) ) >> 1 );
		}
	};

}