#!/bin/bash

SCR_HOME=$(dirname $(readlink -f $0))
REPO_HOME=${REPO_HOME:-$(realpath $SCR_HOME/..)}
PKG_HOME=$REPO_HOME/packaging # in case SCR_HOME differs

# For source release, check that we are in the correct folder
if [ ! -f $REPO_HOME/CMakeLists.txt ]; then
    echo "Can't find CMakeLists.txt, is install.sh in the correct folder?"
fi

# Build release lib

BUILD_DIR=${BUILD_DIR:-$SCR_HOME/package_build}

if [ ! -d $BUILD_DIR ]; then
    mkdir $BUILD_DIR
fi

cd $BUILD_DIR

if ! cmake $REPO_HOME/CMakeLists.txt -B$BUILD_DIR -DCMAKE_BUILD_TYPE=Release
then
    _cmake_status=$?
    echo "cmake failed ($_cmake_status)"
    exit $_cmake_status
else
    make
fi

cd -

LIB_FILE=$BUILD_DIR/libblt_loader.so

if [ ! -f $LIB_FILE ]; then
    echo "could not find $LIB_FILE"
    exit 1
fi


_DIST_NAME=$(perl -n -e'/PRETTY_NAME="(.+)"$/; print $1 =~ s/[\s]/_/gr =~ s/\//-/gr;' < /etc/os-release)

_SUBSCRIPT="$(git describe --always --tag)_$_DIST_NAME"
STAGE_DIR_NAME=blt4l_$_SUBSCRIPT
STAGE_DIR=$BUILD_DIR/$STAGE_DIR_NAME

if [ ! -d $STAGE_DIR ]; then
    mkdir $STAGE_DIR
fi

# Copy artifacts
cp "$REPO_HOME/LICENSE"         "$STAGE_DIR/BLT4L_LICENSE"
cp "$SCR_HOME/README_BLT4L"     "$STAGE_DIR"
cp "$LIB_FILE"                  "$STAGE_DIR"
cp -r $(readlink -f "$REPO_HOME/lua/mods") "$STAGE_DIR"

# Copy package installer
cp "$PKG_HOME/dist_install.sh"  "$STAGE_DIR/install.sh"
rm -r "$STAGE_DIR/.installer" >/dev/null 2>&1
cp -r "$REPO_HOME/installer"    "$STAGE_DIR/.installer"

cd $BUILD_DIR
tar cfJ $BUILD_DIR/blt4l_$_SUBSCRIPT.tar.xz ./$STAGE_DIR_NAME/
