# BASh Library
#   Shared features for the BLT installers, source and binary

SCR_HOME=${SCR_HOME:-$(dirname "$(readlink -f $0)") }

# Set up path to the library directory 
__INSTALLER_LIBDIR="${__INSTALLER_LIBDIR:-installer}"
__INSTALLER_DATA=$SCR_HOME/$__INSTALLER_LIBDIR/

# Colors
__color() {
    echo -ne "\e["$1"m"
}
_c_fail=$(__color 91)
_c_success=$(__color 92)
_c_note=$(__color 93)
_c_warn=$(__color 33)
_c_normal=$(__color 39)

logf() {
    printf "$@" >&2
}

log() {
    echo "$@" >&2
}

###############################################################################
# Misc
###############################################################################

check_file() {
    logf "Looking for "$(realpath --relative-to=. "$1")": "
    if [ -e "$1" ]; then
        log $_c_success"Found"$_c_normal
        return 0
    else
        log $_c_fail"Missing"$_c_normal
        return 1
    fi
}


###############################################################################
# Steam
###############################################################################

__payday2_bin_name="payday2_release"

# base dir
${STEAM_BASE:=${STEAM_BASE:-$HOME/.steam/steam}} 2>/dev/null
# a folder with a steamapps folder beneath it
${STEAM_LIBRARY:=$STEAM_BASE} 2>/dev/null

__locate_userconfig() {
    logf "Looking for Steam configs: "
    __steam_configs=$(find $STEAM_BASE/userdata -type f -name localconfig.vdf)
    if [ -z "$__steam_configs" ]; then
        log $_c_fail"No configurations found"$_c_normal
        logf $_c_note 
        log "  No steam configuration files (localconfig.vdf) were found in $STEAM_BASE/userdata."
        log "  If your steam folder is in a different location, set \$STEAM_BASE."
        logf $_c_normal
        return 1
    else 
        log $_c_success"${#__steam_configs[@]} found"$_c_normal
        echo $__steam_configs
        return 0
    fi
}

__locate_payday2_dir() {
    logf "Looking for PAYDAY 2: "
    __pd2_bin="$(find "$STEAM_LIBRARY" -maxdepth 4 -type f -name "$__payday2_bin_name")"
    if [ -e "$__pd2_bin" ]; then
        log $_c_success$__pd2_bin$_c_normal
        echo $(dirname "$__pd2_bin")
        return 0
    else
        log $_c_fail"Not Found"$_c_normal 
        logf $_c_note 
        log "  The PAYDAY 2 install location could not be determined."
        log "  If you have a steam library located outside of $STEAM_BASE,"
        log "  you should set \$STEAM_LIBRARY to point to it"
        logf $_c_normal 
        return 1
    fi
}

###############################################################################
# Steam "installer"
###############################################################################

# Path to python script 
__userconfig_script=$__INSTALLER_DATA/enable_blt_wrapper.py

# Note for users without python
__manual_config_notice() {
    logf $_c_note 
    log "  Steam could not be automatically configured to launch PAYDAY 2 with BLT."
    log "  In order for PAYDAY 2 to start with BLT installed, you will have to set"
    log "  custom launch options for PAYDAY 2 in Steam."
    log "  To do this, right-click on PAYDAY 2 and select 'Properties' and then 'Set Launch Options'"
    log "  when prompted, enter the text on the next line and click 'OK':"
    log "      env LD_PRELOAD=\"\$LD_PRELOAD ./libblt_loader.so\" %command%"
    log "  if done correctly, PAYDAY 3 will now start with BLT."
    logf $_c_normal
}

# Install function, called 
__modify_userconfig() {
    _userconfig=$@
    _python_bin=python2.7 

    logf "Configuring steam: "
    if [ -f "$__userconfig_script" ]; then
        if which $_python_bin >/dev/null 2>/dev/null; then
            log $_c_success"..."$_c_normal
            for config in $_userconfig; do 
                printf "... %s: " $config >&2
                if $_python_bin $__userconfig_script $STEAM_USERCONFIG >/tmp/blt_pyvdf.log 2>&1; then
                    log $_c_success"OK"$_c_normal
                else
                    log $_c_fail"Failed"$_c_normal
                    logf $_c_fail
                    cat /tmp/blt_pyvdf.log 1>&2
                    logf $_c_normal
                fi
                rm /tmp/blt_pyvdf.log >/dev/null 2>&1
            done
            log "... "$_c_note"You may need to restart steam for these changes to take effect."$_c_normal
        else
            log $_c_warn"$_python_bin not found"$_c_normal
            __manual_config_notice
        fi
    else
        log $_c_fail"Helper Missing"$_c_normal
        __manual_config_notice
    fi
}

###############################################################################
# Installer
###############################################################################

install_artifact() {
    _description=$1
    _source=$2
    _dest=$3
    _flag=x$4 #hack because i don't want to wast time with getopt

    check_file "$_source" || return 1

    logf "Installing $_description: "
    if [ -e "$_dest" -a \( "$_flag" = "xnoupdate" \) ]; then
        log $_c_note"Already Installed"$_c_normal
        return 0
    else
        if cp -rTL "$_source" "$_dest" >/dev/null 2>&1; then
            log $_c_success"OK"$_c_normal
            return 0
        else
            log $_c_fail"Failed (cp: $?)"$_c_normal
            return 1
        fi
    fi
}

###############################################################################
# System
###############################################################################

DISTRIB_ID="$(lsb_release -si)"

dist_check_package() {
    _found=1
    logf "Checking for $1: "

    case $DISTRIB_ID in
        Arch|ManjaroLinux)
            pacman -Qk $1 >/dev/null 2>&1; _found=$?
            ;;
        Debian|Ubuntu|LinuxMint)
            dpkg -s $1 >/dev/null 2>&1; _found=$?
            ;;
        *)
            ;;
    esac

    case $_found in
        0)
            log $_c_success"Installed"$_c_normal
            return 0
            ;;
        *)
            log $_c_fail"Not Installed"$_c_normal
            return $_found
            ;;
    esac
}

dist_install_packages() {
    log $_c_note"Requesting root to install these packages: $@"$_c_normal
    case $DISTRIB_ID in 
        Arch|ManjaroLinux)
            sudo pacman -S $@ --needed
            ;;
        Debian|Ubuntu|LinuxMint)
            sudo apt-get install $@
            # can we use apt instead?
            ;;
        *)
            log $_c_fail"I don't know how to install packages on $DISTRIB_ID"$_c_normal
            return 1
            ;;
    esac
}

