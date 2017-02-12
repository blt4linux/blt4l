#!/bin/bash
# Installer for prebuilt BLT hook

SCR_HOME=$(dirname "$(readlink -f $0)")
__INSTALLER_LIBDIR=.installer
source $SCR_HOME/$__INSTALLER_LIBDIR/framework.sh

_artifact=$SCR_HOME/libblt_loader.so 
_modbase=$SCR_HOME/mods 

## Destination Precheck

# Show a warning if build artifacts can't be found 
_git_trap() {
    if which git 2>&1 >/dev/null; then
        if git rev-parse --git-dir 2>&1 >/dev/null; then
            printf $_c_warn
            echo "  You appear to be running this from a git repository."
            echo "  If you wish to compile BLT and install it, run the install script"
            echo "  in the base of the repository."
            printf $_c_normal
        fi
    fi
}

_precheck_fail=0

### Look for PAYDAY 2 installation dir 
PD2_DATA="$(__locate_payday2_dir)" || _precheck_fail=1

### Look for localconfigs 
STEAM_USERCONFIG="$(__locate_userconfig)" || _precheck_fail=1

## Look for artifacts 

### Check loader
printf "Looking for loader: "
if [ ! -f $_artifact ]; then 
    echo $_c_fail"Not Found"$_c_normal
    _git_trap
    _precheck_fail=1
else
    echo $_c_success$_artifact$_c_normal
fi 

### Check lua 
printf "Looking for lua: "
if [ ! -d $_modbase ]; then
    echo $_c_fail"Not Found"$_c_normal
    _git_trap
    _precheck_fail=1
else 
    echo $_c_success$_modbase$_c_normal
fi

## Exit if either failed
if [ $_precheck_fail -gt 0 ]; then
    echo $_c_fail"Preinstall checks failed"
    exit 1
fi

# Install 
artifact_dest="$PD2_DATA/libblt_loader.so"
modbase_dest="$PD2_DATA/mods"

install_artifact "loader" "$_artifact" "$artifact_dest" 
install_artifact "lua base" "$_modbase" "$modbase_dest" noupdate

## Steam Config
__modify_userconfig $STEAM_USERCONFIG

echo $_c_success"BLT has been installed"$_c_normal
 
