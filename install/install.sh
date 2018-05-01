# Naming conventions
OSX_LIB_FOLDER_NAME=macosx

# Build Paths
START_PATH=$(pwd)
TMP_DIR=${START_PATH}/tmp
TMP_SRC_DIR=${TMP_DIR}/source
TMP_BUILD_DIR=${TMP_DIR}/build
TMP_INCLUDE_DIR=${TMP_DIR}/include
TMP_LIB_DIR=${TMP_DIR}/lib


#export CC=llvm-gcc
#export CPP=llvm-g++

# Source Directories
mkdir -p ${TMP_SRC_DIR}


# ------------------------------------------------------
# FREETYPE
FREETYPE_PKG=freetype-2.9
FREETYPE_PKG_COMPRESSED=${FREETYPE_PKG}.tar.bz2
FREETYPE_PKG_URL=http://gnu.mirrors.pair.com/savannah/savannah/freetype/${FREETYPE_PKG_COMPRESSED}
FREETYPE_BUILD_DIR=${TMP_BUILD_DIR}/freetype

FREETYPE_INCLUDE_DIR=${FREETYPE_BUILD_DIR}/include/freetype2
FREETYPE_LIBS_DIR=${FREETYPE_BUILD_DIR}/lib

pushd ${TMP_SRC_DIR}

# # Download
# curl -o ${FREETYPE_PKG_COMPRESSED} ${FREETYPE_PKG_URL}
# tar -vxjf ${FREETYPE_PKG_COMPRESSED}
# chmod 777 ${FREETYPE_PKG}

# # Make
# pushd ${FREETYPE_PKG}

# make clean
# ./configure --prefix=${FREETYPE_BUILD_DIR} --enable-static --disable-shared --disable-dependency-tracking
# make
# make install

# popd #FREETYPE_PKG


# ------------------------------------------------------
# HARFBUZZ

HARFBUZZ_PKG=harfbuzz-1.4.6
HARFBUZZ_PKG_COMPRESSED=${HARFBUZZ_PKG}.tar.bz2
HARFBUZZ_PKG_URL=https://www.freedesktop.org/software/harfbuzz/release/${HARFBUZZ_PKG_COMPRESSED}
HARFBUZZ_BUILD_DIR=${TMP_BUILD_DIR}/harfbuzz

FREETYPE_CFLAGS="-I${FREETYPE_INCLUDE_DIR}"
FREETYPE_LIBS="-L${FREETYPE_LIBS_DIR} -lfreetype"

# Download
curl -o ${HARFBUZZ_PKG_COMPRESSED} ${HARFBUZZ_PKG_URL}
tar -vxjf ${HARFBUZZ_PKG_COMPRESSED}
chmod 777 ${HARFBUZZ_PKG}

# Make
pushd ${HARFBUZZ_PKG}

make clean
./configure --prefix=${HARFBUZZ_BUILD_DIR} --enable-static=yes --enable-shared=no --disable-dependency-tracking --with-glib=no --with-freetype=yes 
make
make install

popd # HARFBUZZ_PKG


popd # SRC_DIR

echo ${FREETYPE_LIBS}

# #------------------------------------------------------
# # HARFBUZZ
# HARFBUZZ_OUTPUT_DIR=$HARFBUZZ_SRC_DIR/output
# HARFBUZZ_LIB_PREFIX=harfbuzz

# # Build the libs
# pushd ${HARFBUZZ_SRC_DIR}

# make clean
# ./configure --prefix=$BUILD_DIR --enable-static --disable-shared --disable-dependency-tracking --with-glib=no
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