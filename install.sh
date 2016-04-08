#!/bin/bash

PD2_APPID=218620

SCR_HOME=$(dirname $(readlink -f $0))
STEAM_BASE=$HOME/.steam/steam
STEAM_LIBRARY=${STEAM_PREFIX:-$STEAM_BASE}

# For source release, check that we are in the correct folder
if [ ! -f $SCR_HOME/CMakeLists.txt ]; then
    echo "Can't find CMakeLists.txt, is install.sh in the correct folder?"
fi

# Update submodules if git repo
if [ -d $SCR_HOME/.git ]; then
    git submodule init
    git submodule update
fi

# Dependency check
MISSING_PKGS=""

_test_lib() {
    #ld hack. should work on steamos/debian/ubuntu. sacrifices best compatibility for shorter solution that works for >80% of users
    # manual install is easy, too, and anyone that has a wierd ld should be able to do this themself
    ld -l$1 2> /dev/null
}

_test_pkg() {
    dpkg -s $@ 2>&1 > /dev/null
}

if ! _test_lib curl; then 
    MISSING_PKGS="$MISSING_PKGS libcurl4-openssl-dev"
fi

if ! _test_lib z; then
    MISSING_PKGS="$MISSING_PKGS zlib1g-dev"
fi

if ! which cmake 2>&1 > /dev/null; then
    MISSING_PKGS="$MISSING_PKGS cmake"
fi

if which dpkg 2>&1 > /dev/null; then
    if ! _test_pkg build-essential; then
        MISSING_PKGS="$MISSING_PKGS build-essential"
    fi
fi

if [ ${#MISSING_PKGS} -gt 1 ]; then
    if which apt-get 2>&1 > /dev/null; then
        echo Enter your password to install these packages: $MISSING_PKGS
        sudo apt-get install $MISSING_PKGS
    else
        echo The following packages must be installed: $MISSING_PKGS
        echo They may have a different name on your distribution
    fi
fi

# Build release lib

BUILD_DIR=$SCR_HOME/installer_build

if [ ! -d $BUILD_DIR ]; then
    mkdir $BUILD_DIR
fi

cd $BUILD_DIR

if ! cmake $SCR_HOME/CMakeLists.txt -B$BUILD_DIR -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-march=native -mtune=native" -DCMAKE_C_FLAGS="-march=native -mtune=native"
then
    _cmake_status=$?
    echo "cmake failed ($_cmake_status)"
    exit $_cmake_status
else
    make
fi

LIB_FILE=$BUILD_DIR/libblt_loader.so

if [ ! -f $LIB_FILE ]; then
    echo "could not find $LIB_FILE"
    exit 1
fi

# Find PAYDAY 2 

PD2_DATA="$STEAM_LIBRARY/steamapps/common/PAYDAY 2"
LIB_INSTALLED=$PD2_DATA/libblt_loader.so

if [ ! -d "$PD2_DATA" ]; then
    echo "Could not find PD2 at $PD2_DATA"
    echo "Either you need to install PAYDAY 2, or set the environment variable STEAM_LIBRARY to the location of the steam library it is installed in"
    exit 1
fi

cp "$LIB_FILE" "$LIB_INSTALLED"

# install mods folder

if [ ! -d "$PD2_DATA/mods" ]; then
    cp -r $(readlink -f "$SCR_HOME/lua/mods") "$PD2_DATA"
fi

# I may eventually automate this. For now I'll ask users to paste this in to their launch options
echo "Set the following line as your custom launch options for PAYDAY 2:"
echo "  env LD_PRELOAD=\"$LIB_INSTALLED\" %command%"
