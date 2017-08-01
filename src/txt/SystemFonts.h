#pragma once

#include <memory>
#include <map>

#include "cinder/Buffer.h"

#include "txt/Font.h"

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

			std::string getDefaultFamily() { return mDefaultFamily; };
			std::string getDefaultStyle() { return mDefaultStyle; };
			int getDefaultSize() { return mDefaultSize; };

		private:
			SystemFonts();

			void loadFaces();
			std::map < std::string, std::vector < std::string>> mFaces;

			std::string mDefaultFamily;
			std::string mDefaultStyle;
			int mDefaultSize;
	};
}