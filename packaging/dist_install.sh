#!/bin/bash
# Installer for prebuilt BLT hook

__color() {
    echo -ne "\e["$1"m"
}
_c_fail=$(__color 91)
_c_success=$(__color 92)
_c_note=$(__color 93)
_c_warn=$(__color 33)
_c_normal=$(__color 39)

SCR_HOME=$(dirname "$(readlink -f $0)")

_artifact=$SCR_HOME/libblt_loader.so 
_modbase=$SCR_HOME/mods 

_config_helper_py=$SCR_HOME/.installer/enable_blt_wrapper.py 

# Destination Setup

${STEAM_BASE:=${STEAM_BASE:-$HOME/.steam/steam}} 2>/dev/null 
${STEAM_LIBRARY:=${STEAM_PREFIX:-$STEAM_BASE}} 2>/dev/null

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
printf "Looking for PAYDAY 2: "
PD2_DATA=$(dirname "$(find $STEAM_LIBRARY -maxdepth 4 -type f -name payday2_release)")
if [ ! -d "$PD2_DATA" ]; then
    echo $_c_fail"Not found"$_c_note
    echo "  The PAYDAY 2 install location could not be determined."
    echo "  If you have a steam library located outside of $STEAM_BASE,"
    echo "  you should set \$STEAM_LIBRARY to point to it"
    printf $_c_normal
    _precheck_fail=1
else
    echo $_c_success$PD2_DATA$_c_normal
fi

### Look for localconfigs 
printf "Looking for Steam data: "
STEAM_USERCONFIG=$(find $STEAM_BASE/userdata -type f -name localconfig.vdf)
if [ -z "$STEAM_USERCONFIG" ]; then
    echo $_c_fail"No configurations found"$_c_note
    echo "  No steam configuration files (localconfig.vdf) were found in $STEAM_BASE/userdata."
    echo "  If your steam folder is in a different location, set \$STEAM_BASE."
    printf $_c_normal
    _precheck_fail=1
else
    echo $_c_success"${#STEAM_USERCONFIG[@]} found"$_c_normal
fi 

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

artifact_dest=$PD2_DATA/libblt_loader.so
modbase_dest=$PD2_DATA/mods

## Loader
printf "Installing loader: "
if cp "$_artifact" "$artifact_dest" 2>&1 >/dev/null; then
    echo $_c_success"OK"$_c_normal
else
    echo $_c_fail"Failed (cp status $?)"$_c_normal
    exit 2
fi

## Lua
printf "Installing lua base: "
if [ -d "$modbase_dest" ]; then
    echo $_c_success"Already Installed"$_c_normal
else
    if cp -r "$_modbase" "$modbase_dest" 2>&1 >/dev/null; then
        echo $_c_success"OK"$_c_normal
    else
        echo $_c_fail"Failed (cp status $?)"$_c_normal
        exit 2
    fi
fi

## Steam Config
__manual_config_notice() {
    printf $_c_note 
    echo "  Steam could not be automatically configured to launch PAYDAY 2 with BLT."
    echo "  In order for PAYDAY 2 to start with BLT installed, you will have to set"
    echo "  custom launch options for PAYDAY 2 in Steam."
    echo "  To do this, right-click on PAYDAY 2 and select 'Properties' and then 'Set Launch Options'"
    echo "  when prompted, enter the text on the next line and click 'OK':"
    echo "      env LD_PRELOAD=\"\$LD_PRELOAD ./libblt_loader.so\" %command%"
    echo "  if done correctly, PAYDAY 2 will now start with BLT."
    printf $_c_normal
}
printf "Configuring steam: "
if [ -f "$_config_helper_py" ]; then
    if which python2.7 >/dev/null 2>/dev/null; then
        echo $_c_success"Python Installed"$_c_normal
        for config in $STEAM_USERCONFIG; do 
            printf "... %s: " $config 
            if python2.7 $_config_helper_py $STEAM_USERCONFIG; then
                echo $_c_success"done"$_c_normal
            fi
        done
        printf $_c_note
        echo "  You may need to restart steam for these changes to take effect."
        printf $_c_normal
    else
        echo $_c_warn"Python Not Installed"$_c_normal
        __manual_config_notice
    fi
else
    echo $_c_error"Could not find config helper"$_c_normal
    __manual_config_notice
fi

echo $_c_success"BLT has been installed"$_c_normal
 
