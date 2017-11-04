#include "txt/SystemFonts.h"
#include "cinder/Font.h"

//#if defined( cinder_msw_desktop )
//#include <windows.h>
//#define max(a, b) (((a) > (b)) ? (a) : (b))
//#define min(a, b) (((a) < (b)) ? (a) : (b))
//#include <gdiplus.h>
//#undef min
//#undef max
//#include "cinder/msw/cindermsw.h"
//#include "cinder/msw/cindermswgdiplus.h"
//#pragma comment(lib, "gdiplus")
//#endif

#include "cinder/msw/cindermsw.h"
#include "cinder/msw/cindermswgdiplus.h"

#include "cinder/Unicode.h"
#include "cinder/app/App.h"

#include <unordered_map>
#include <strsafe.h>


namespace txt
{

	// Windows
#if defined( CINDER_MSW_DESKTOP )
	HDC mFontDC = nullptr;

	SystemFonts::SystemFonts()
	{
		mFontDC = ::CreateCompatibleDC( NULL );

		mDefaultFamily = "Arial";
		mDefaultStyle = "Regular";
		mDefaultSize = 12;
	}


	int CALLBACK EnumFacesExProc( ENUMLOGFONTEX* lpelfe, NEWTEXTMETRICEX* lpntme, int FontType, LPARAM lParam )
	{
		std::string familyName = ci::toUtf8( ( char16_t* )lpelfe->elfLogFont.lfFaceName );
		std::string style = ci::toUtf8( ( char16_t* )lpelfe->elfStyle );

		( *reinterpret_cast<std::map<std::string, std::vector<std::string>>*>( lParam ) )[familyName].push_back( style );

		return 1;
	}

	struct HFontRequest {
		LOGFONT logfont;
		std::string family;
		std::string style;
		bool fontFound = false;
	};

	HFontRequest mHFontRequest;

	int CALLBACK LoadHFontExProc( ENUMLOGFONTEX* lpelfe, NEWTEXTMETRICEX* lpntme, int FontType, LPARAM lParam )
	{
		HFontRequest* request = reinterpret_cast<HFontRequest*>( lParam );

		std::string familyName = ci::toUtf8( ( char16_t* )lpelfe->elfLogFont.lfFaceName );
		std::transform( familyName.begin(), familyName.end(), familyName.begin(), ::tolower );

		std::string style = ci::toUtf8( ( char16_t* )lpelfe->elfStyle );
		std::transform( style .begin(), style.end(), style.begin(), ::tolower );

		if( familyName == request->family && style == request->style ) {
			request->logfont = lpelfe->elfLogFont;
			request->fontFound = true;
		}

		return 1;
	}


	void SystemFonts::listFaces()
	{
		mFaces.clear();

		::LOGFONT lf;
		lf.lfCharSet = ANSI_CHARSET;
		lf.lfFaceName[0] = '\0';

		::EnumFontFamiliesEx( mFontDC, &lf, ( FONTENUMPROC )EnumFacesExProc, reinterpret_cast<LPARAM>( &mFaces ), 0 );

		for( auto& family : mFaces ) {
			ci::app::console() << family.first << std::endl;
			ci::app::console() << "---------------------" << std::endl;

			for( auto& style : family.second ) {
				ci::app::console() << style << std::endl;
			}

			ci::app::console() << std::endl;
		}
	}

	ci::BufferRef SystemFonts::getFontBuffer( std::string family, std::string style )
	{
		::LOGFONT lf;
		lf.lfCharSet = ANSI_CHARSET;
		lf.lfFaceName[0] = '\0';

		std::u16string faceName = ci::toUtf16( family );
		::StringCchCopy( lf.lfFaceName, LF_FACESIZE, ( LPCTSTR )faceName.c_str() );

		mHFontRequest.family = family;
		mHFontRequest.style = style;

		::EnumFontFamiliesEx( mFontDC, &lf, ( FONTENUMPROC )LoadHFontExProc, reinterpret_cast<LPARAM>( &mHFontRequest ), 0 );

		HFONT hFont;

		if( mHFontRequest.fontFound ) {
			hFont = ::CreateFontIndirectW( &mHFontRequest.logfont );

			if( hFont ) {
				::SelectObject( mFontDC, hFont );

				DWORD fontSize = ::GetFontData( mFontDC, 0, 0, NULL, 0 );

				void* fontBuffer = new BYTE[fontSize];
				DWORD length = ::GetFontData( mFontDC, 0, 0, fontBuffer, fontSize );


				return ci::Buffer::create( fontBuffer, ( unsigned int )length );
			}
			else {
				return nullptr;
			}
		}
		else {
			return nullptr;
		}
	}

#endif
}