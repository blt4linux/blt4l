#!/bin/bash

PD2_APPID=218620

SCR_HOME=$(dirname $(readlink -f $0))
STEAM_BASE=$HOME/.steam/steam
# Set STEAM_LIBRARY to sane default *iff unset*
${STEAM_LIBRARY:=${STEAM_PREFIX:-$STEAM_BASE}}

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
# Check distribution ID
DISTRIB_ID=$(lsb_release -si)

# Checks for package $1 to be installed; if not adds it to
# MISSING_PKGS
# On unknown distros it will add $1 to the MISSING_PKGS
_test_pkg() {
    echo "Checking for $1"
    case $DISTRIB_ID in
        Arch)
            case $1 in
                libcurl4-openssl-dev)
                    PKGNAME=curl
                    ;;
                zlib1g-dev)
                    PKGNAME=zlib
                    ;;
                build-essential)
                    echo "Be sure to have base-devel installed!"
                    PKGNAME=glibc
                    ;;
                cmake)
                    PKGNAME=cmake
                    ;;
            esac
            if pacman -Qk $PKGNAME 2>&1 > /dev/null; then
                return
            fi
            ;;


        Debian|Ubuntu|LinuxMint)
            PKGNAME=$1
            if ! dpkg -s $PKGNAME 2>&1 > /dev/null; then
                return
            fi
            ;;
        *)
            PKGNAME=$1
            ;;
    esac

    echo "Not found."
    MISSING_PKGS="$MISSING_PKGS $PKGNAME"
}

# Installs all packages in $MISSING_PKGS
# On unknown distros this will just ask whether to proceed or not.
_install_packages() {
    if [ ! ${#MISSING_PKGS} -gt 1 ]; then
        return
    fi

    echo Needing these packages: $MISSING_PKGS
    case $DISTRIB_ID in
        Arch)
            sudo pacman -S $MISSING_PKGS --needed
            ;;
        Debian|Ubuntu|LinuxMint)
            sudo apt-get install $MISSING_PKGS
            ;;
        *)
            echo "Your distribution is unsupported but you can proceed if the packages are installed"
            printf "Proceed? [y/N]: "
            read yn
            if [ ! "$yn" == "y" ]; then
                exit
            fi
            ;;
    esac
}

case $DISTRIB_ID in
    Arch)
        ;;
    Debian|Ubuntu|LinuxMint)    # are these the correct DISTRIB_IDs?
        ;;
    *)
        echo "[WARNING] $DISTRIB_ID is unkown to installscript!"
        ;;
esac

_test_pkg libcurl4-openssl-dev
_test_pkg zlib1g-dev
_test_pkg cmake
_test_pkg build-essential

_install_packages

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
echo "  env LD_PRELOAD=\"\$LD_PRELOAD ./libblt_loader.so\" %command%"

# vim: set ts=4 softtabstop=0 sw=4 expandtab:
