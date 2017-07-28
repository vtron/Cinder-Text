#pragma once

#include <memory>
#include <map>

#include "cinder/Buffer.h"

#include "text/Font.h"

namespace txt
{
	typedef std::shared_ptr<class SystemFonts> SystemFontsRef;

	class SystemFonts
	{
		public:
			static SystemFontsRef get()
			{
				static SystemFontsRef ref( new SystemFonts() );
				return ref;
			};

			ci::BufferRef getFontBuffer( std::string family, std::string style );

		private:
			SystemFonts();

			void loadFaces();
			std::map < std::string, std::vector < std::string>> mFaces;
	};
}