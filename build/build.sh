# Naming conventions
OSX_LIB_FOLDER_NAME=macosx


BUILD_DIR=$(pwd)/build

#export CC=llvm-gcc
#export CPP=llvm-g++

# Source Directories
SRC_DIR=$(pwd)/source
FREETYPE_SRC_DIR=$SRC_DIR/freetype-2.6.5
HARFBUZZ_SRC_DIR=$SRC_DIR/harfbuzz
FONTCONFIG_SRC_DIR=$SRC_DIR/fontconfig-2.12.1


# #------------------------------------------------------
# # FREETYPE
# FREETYPE_OUTPUT_DIR=$FREETYPE_SRC_DIR/output
# FREETYPE_LIB_PREFIX=freetype

# # Build the libs
# pushd ${FREETYPE_SRC_DIR}

# make clean
# ./configure --prefix=$BUILD_DIR --enable-static --disable-shared --disable-dependency-tracking
# make
# make install

# popd



#------------------------------------------------------
# FREETYPE
HARFBUZZ_OUTPUT_DIR=$HARFBUZZ_SRC_DIR/output
HARFBUZZ_LIB_PREFIX=harfbuzz

# Build the libs
pushd ${HARFBUZZ_SRC_DIR}

make clean
./configure --prefix=$BUILD_DIR --enable-static --disable-shared --disable-dependency-tracking --with-glib=no
make
make install

popd



# #------------------------------------------------------
# # FONT-CONFIG

# # Build the libs
# pushd $FONTCONFIG_SRC_DIR

# # export FREETYPE_CFLAGS=-I$BUILD_DIR/include/freetype2
# export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$BUILD_DIR/lib/pkgconfig/

# make clean
# ./configure --prefix=$BUILD_DIR --enable-static --disable-shared
# make
# make install

# popd





# Create directories if necessary
# FREETYPE_BUILD_DIR=$BUILD_DIR/freetype
# FREETYPE_LIBS_DIR=$FREETYPE_BUILD_DIR/lib
# FREETYPE_LIBS_OSX_DIR=$FREETYPE_LIBS_DIR/$OSX_LIB_FOLDER_NAME

# mkdir $FREETYPE_BUILD_DIR
# mkdir $FREETYPE_LIBS_DIR
# mkdir $FREETYPE_LIBS_OSX_DIR

# # Copy files to dirs
# cp -r $FREETYPE_OUTPUT_DIR/include $FREETYPE_BUILD_DIR

# FREETYPE_STATIC_LIB=lib$FREETYPE_LIB_PREFIX.a
# cp $FREETYPE_OUTPUT_DIR/lib/$FREETYPE_STATIC_LIB $FREETYPE_BUILD_DIR/lib/macosx/$FREETYPE_STATIC_LIB