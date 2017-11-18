@echo off

:: ---------------------------------------------------------------------------------------------
:: Init Vars

set PATH=%PATH%;C:\Program Files\7-Zip\

:: Global vars
SET VS_VERSION="Visual Studio 14 2015 Win64"
SET START_PATH=%cd%
SET TMP_DIR=%START_PATH%\tmp
SET TMP_BUILD_DIR=%TMP_DIR%\build
SET TMP_INCLUDE_DIR=%TMP_BUILD_DIR%\include
SET TMP_LIB_DIR=%TMP_BUILD_DIR%\lib

SET CMAKE_FLAGS=-DCMAKE_INSTALL_PREFIX:PATH=%TMP_BUILD_DIR%
SET CMAKE_FLAGS=%CMAKE_FLAGS% -DCMAKE_C_FLAGS_DEBUG="/MTd"
SET CMAKE_FLAGS=%CMAKE_FLAGS% -DCMAKE_C_FLAGS_RELEASE="/MT"
SET CMAKE_FLAGS=%CMAKE_FLAGS% -DCMAKE_CXX_FLAGS_DEBUG="/MTd"
SET CMAKE_FLAGS=%CMAKE_FLAGS% -DCMAKE_CXX_FLAGS_RELEASE="/MT"
SET CMAKE_FLAGS=%CMAKE_FLAGS% /NODEFAULTLIB:library

:: Create tmp dir
IF EXIST %TMP_DIR% (
	@RD /S /Q %TMP_DIR%
)

mkdir %TMP_DIR%
mkdir %TMP_LIB_DIR%
mkdir %TMP_INCLUDE_DIR%
cd %TMP_DIR%

:: ---------------------------------------------------------------------------------------------
:: Build libs
CALL :buildFreetype
CALL :buildHarfbuzz
CALL :buildFreetypeHarfbuzz
CALL :buildLibunibreak

:: ---------------------------------------------------------------------------------------------
:: Finish + clean up
CALL :print_message Finishing
cd %START_PATH%

robocopy %TMP_INCLUDE_DIR% ../include /E /MOVE
robocopy %TMP_LIB_DIR% ../lib /E /MOVE

@RD /S /Q %TMP_DIR%

exit

:: ---------------------------------------------------------------------------------------------
:: ---------------------------------------------------------------------------------------------
:: Library building functions

:: ---------------------------------------------------------------------------------------------
:: Freetype 2
:buildFreetype

SET FREETYPE_PKG=freetype-2.8.tar.bz2
SET FREETYPE_PKG_URL=http://download.savannah.gnu.org/releases/freetype/%FREETYPE_PKG%
SET FREETYPE_DIR_NAME=freetype
SET FREETYPE_DIR=%TMP_DIR%\%FREETYPE_DIR_NAME%
SET FREETYPE_BUILD_DIR=%FREETYPE_DIR%\build
SET FREETYPE_HEADER_DIR=%TMP_INCLUDE_DIR%\freetype2

:: Download
CALL :print_message Downloading Freetype 2

CALL :download_pkg %FREETYPE_PKG_URL%, %FREETYPE_DIR_NAME%, %FREETYPE_DIR%

:: Make
CALL :print_message Making Freetype 2

mkdir %FREETYPE_BUILD_DIR%
cd %FREETYPE_BUILD_DIR%

SET FREETYPE_CMAKE_CONFIG=%CMAKE_FLAGS%

cmake %FREETYPE_CMAKE_CONFIG% .. -G %VS_VERSION%

:: Build
CALL :print_message Building Freetype 2
msbuild INSTALL.vcxproj /p:PlatformToolset=v140 /p:Configuration="Release" /p:Platform="x64"

EXIT /B 0


:: ---------------------------------------------------------------------------------------------
::Harfbuzz with Freetype 2
:buildHarfbuzz

SET HARFBUZZ_PKG=harfbuzz-1.4.6.tar.bz2
SET HARFBUZZ_PKG_URL=https://www.freedesktop.org/software/harfbuzz/release/%HARFBUZZ_PKG%
SET HARFBUZZ_DIR_NAME=harfbuzz
SET HARFBUZZ_DIR=%TMP_DIR%\%HARFBUZZ_DIR_NAME%
SET HARFBUZZ_BUILD_DIR=%HARFBUZZ_DIR%\build
SET HARFBUZZ_INCLUDE_DIR=%TMP_INCLUDE_DIR%\%HARFBUZZ_DIR_NAME%

:: Download
CALL :print_message Downloading Harfbuzz
CALL :download_pkg %HARFBUZZ_PKG_URL%, %HARFBUZZ_DIR_NAME%, %HARFBUZZ_DIR%

:: Make
CALL :print_message Making Harfbuzz

mkdir %HARFBUZZ_BUILD_DIR%
cd %HARFBUZZ_BUILD_DIR%

:: Configure for Freetype
SET HARFBUZZ_CMAKE_CONFIG=%CMAKE_FLAGS%
SET HARFBUZZ_CMAKE_CONFIG=%HARFBUZZ_CMAKE_CONFIG% -DHB_HAVE_FREETYPE=ON
SET HARFBUZZ_CMAKE_CONFIG=%HARFBUZZ_CMAKE_CONFIG% -DFREETYPE2_HEADER_DIR=%FREETYPE_HEADER_DIR%
SET HARFBUZZ_CMAKE_CONFIG=%HARFBUZZ_CMAKE_CONFIG% -DFREETYPE2_ROOT_INCLUDE_DIR=%TMP_INCLUDE_DIR%
SET HARFBUZZ_CMAKE_CONFIG=%HARFBUZZ_CMAKE_CONFIG% -DFREETYPE2_LIBRARIES=%TMP_LIB_DIR%

echo Config: %HARFBUZZ_CMAKE_CONFIG%

cmake %HARFBUZZ_CMAKE_CONFIG% .. -G %VS_VERSION%

:: Build
CALL :print_message Building Harfbuzz
msbuild INSTALL.vcxproj /p:PlatformToolset=v140 /p:Configuration="Release" /p:Platform="x64"

EXIT /B 0

:: ---------------------------------------------------------------------------------------------
::Freetype 2 with Harfbuzz
:buildFreetypeHarfbuzz

cd %FREETYPE_BUILD_DIR%

:: Make
CALL :print_message Making Freetype with Harfbuzz
rm CMakeCache.txt

SET FREETYPEHB_CMAKE_CONFIG=%CMAKE_FLAGS%
SET FREETYPEHB_CMAKE_CONFIG=%FREETYPEHB_CMAKE_CONFIG% -DWITH_HarfBuzz=ON
SET FREETYPEHB_CMAKE_CONFIG=%FREETYPEHB_CMAKE_CONFIG% -DHARFBUZZ_INCLUDE_DIRS=%HARFBUZZ_INCLUDE_DIR%

cmake %FREETYPEHB_CMAKE_CONFIG% .. -G %VS_VERSION%

:: Build
CALL :print_message Building Freetype 2 with Harfbuzz
msbuild INSTALL.vcxproj /p:PlatformToolset=v140 /p:Configuration="Release" /p:Platform="x64"

EXIT /B 0

:: ---------------------------------------------------------------------------------------------
:: libunibreak
:: https://github.com/adah1972
:buildLibunibreak

SET UNIBREAK_PKG=libunibreak-4.0.tar.gz
SET UNIBREAK_PKG_URL=https://github.com/adah1972/libunibreak/releases/download/libunibreak_4_0/%UNIBREAK_PKG%
SET UNIBREAK_DIR_NAME=libunibreak
SET UNIBREAK_DIR=%TMP_DIR%\%UNIBREAK_DIR_NAME%
SET UNIBREAK_BUILD_DIR=%UNIBREAK_DIR%\src
SET UNIBREAK_HEADER_DIR=%TMP_INCLUDE_DIR%\libunibreak

:: Download
CALL :print_message Downloading libunibreak

CALL :download_pkg %UNIBREAK_PKG_URL%, %UNIBREAK_DIR_NAME%, %UNIBREAK_DIR%

:: Build (using nmake)
CALL :print_message Making libunibreak

mkdir %UNIBREAK_BUILD_DIR%
cd %UNIBREAK_BUILD_DIR%

nmake -f Makefile.msvc CFG="libunibreak - Win32 Release"
robocopy Release %TMP_LIB_DIR% unibreak.lib
robocopy .\ %TMP_INCLUDE_DIR%\%UNIBREAK_DIR_NAME% *.h

EXIT /B 0


:: ---------------------------------------------------------------------------------------------
:: ---------------------------------------------------------------------------------------------
:: Utility Functions

::Print out a status message
:print_message
echo.
echo -------------------------------------------
echo %*....
echo.
EXIT /B 0

::Download a file and clean up
:download_pkg URL, FOLDER_NAME, FOLDER_PATH
powershell -command "& { (New-Object Net.WebClient).DownloadFile('%~1 ', '%~2.tar.bz2') }"

7z x %~2.tar.bz2
7z x %~2.tar

powershell -command "mv %~2-* %~3"
powershell -command "rm %~2.*"
EXIT /B 0