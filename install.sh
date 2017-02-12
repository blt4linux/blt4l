#!/bin/bash

SCR_HOME=$(dirname "$(readlink -f $0)")
source $SCR_HOME/installer/framework.sh

STEAM_BASE=${STEAM_BASE:-$HOME/.steam/steam}
# Set STEAM_LIBRARY to sane default *if unset*
STEAM_LIBRARY=${STEAM_LIBRARY:-$STEAM_BASE}

###############################################################################
# Pre-Install
# - Check Source Tree
# - Check System Packages
# - Check Steam
###############################################################################

## Update submodules if git repo

if git rev-parse --git-dir >/dev/null 2>&1; then
    logf "Updating submodules: "
    if git submodule update --init --recursive >/dev/null 2>&1; then
        log $_c_success"OK"$_c_normal
    else
        log $_c_fail"Failed (git: $?)"$_c_normal
        exit
    fi
fi

## Build check 

check_file "$SCR_HOME/CMakeLists.txt" || exit

## System deps check 

MISSING_PKGS=()
WANTED_PKGS=()
case $DISTRIB_ID in 
    Arch|ManjaroLinux)
        WANTED_PKGS=(curl zlib glibc base-devel cmake)
        ;;
    Debian|Ubuntu|LinuxMint)
        WANTED_PKGS=(libcurl4-openssl-dev zlib1g-dev cmake build-essential)
        ;;
    *)
        log $_c_note"Note: No package list for $DISTRIB_ID"$_c_normal
        ;;
esac

for pk in ${WANTED_PKGS[@]}; do
    if ! dist_check_package $pk; then
        MISSING_PKGS+=$pk
    fi
done

if [ ${#MISSING_PKGS} -gt 1 ]; then 
    logf $_c_note"You are missing packages. Do you want to install them now? [Y/n]: "
    read yn
    if [ x$yn != "xn" ]; then
        dist_install_packages ${MISSING_PKGS[@]}
    else
        log $_c_warn"Continuing without installing packages"$_c_normal
    fi
fi

## Game/Steam

PD2_DATA="$(__locate_payday2_dir)" || exit $?
STEAM_USERCONFIG="$(__locate_userconfig)" || exit $?

###############################################################################
# Build
###############################################################################

_CMAKE_CFLAGS="-march=native -mtune=native"
_CMAKE_CXXFLAGS=$_CMAKE_CFLAGS
_CMAKE_COMMAND="cmake '$SCR_HOME/CMakeLists.txt' -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS='$_CMAKE_CXXFLAGS' -DCMAKE_C_FLAGS='$_CMAKE_CFLAGS'"

echo $_CMAKE_COMMAND

_CMAKE_LOGFILE=installer_cmake.log
_MAKE_LOGFILE=installer_make.log

BUILD_DIR=$SCR_HOME/installer_build

check_file $BUILD_DIR || mkdir $BUILD_DIR

cd $BUILD_DIR

logf "Running CMake: "

if ! eval $_CMAKE_COMMAND -B$BUILD_DIR >$PWD/$_CMAKE_LOGFILE 2>&1
then
    log $_c_fail"Failed"$_c_normal
    log $_c_note"Please see the CMake log at "$(realpath "$PWD/$_CMAKE_LOGFILE")$_c_normal
    exit $_cmake_status
else
    log $_c_success"OK"$_c_normal
    logf $_c_normal"Building BLT: "$_c_normal
    if ! make >$PWD/$_MAKE_LOGFILE 2>&1; then
        log $_c_fail"Failed"
        log $_c_note"Please see the Make log at "$(realpath "$PWD/$_MAKE_LOGFILE")$_c_normal
        exit $_make_status
    else
        log $_c_success"OK"$_c_normal
    fi
fi

install_artifact "loader" "$BUILD_DIR/libblt_loader.so" "$PD2_DATA/libblt_loader.so" || exit 1
install_artifact "lua base" "$SCR_HOME/lua/mods" "$PD2_DATA/mods" noupdate || exit 1
__modify_userconfig $STEAM_USERCONFIG

# vim: set ts=4 softtabstop=0 sw=4 expandtab:
