#!/usr/bin/env fish
# Attention, github readers,
# This is my (Roman Hargrave) custom mass-packaging automation script
# It will probably not work for you, but it may.
# This is only here as an example of how to automate builds accross distros.
#
# You will need:
#
# - fish
# - schroot
# - my logging library, and fish-require, which are somewhere on my GH profile 

require logging

set scr_dir (realpath (dirname (status -f)))
set scm     'git://github.com/blt4linux/blt4l.git'

set -q git_name; or set git_name master

info packaging!

function chr_bld -a chroot_name
    info building $chroot_name


    set chroot_data $scr_dir/chroot_data/$chroot_name/
    set pkbase      $chroot_data/blt
    set pkbin       $chroot_data/bin

    test -d $chroot_data;  or mkdir -p $chroot_data
    test -d $pkbin;        or mkdir -p $pkbin

    info "chroot data in $chroot_data"
    info "repo base in $pkbase"
    info "repo bin in $pkbin"

    function chgit -S
        info "($chroot_name) $argv"
        schroot --directory $pkbase -c $chroot_name -- $argv
    end

    # Remove repo
    if [ -d $pkbase ]
        rm -rf $pkbase/.git
        rm -r $pkbase
    end

    # Remove build dir
    if [ -d $pkbin ]
        rm -r $pkbin
    end

    info checking out blt4l
    begin
        if schroot --directory $chroot_data -c $chroot_name -- git clone --recursive $scm $pkbase
            if chgit git checkout $git_name
                # Run build
                if schroot --directory $pkbase/packaging -c $chroot_name -- env BUILD_DIR="$pkbin" ./package.sh
                    # Move artifacts out of chroot
                    mv $pkbin/*.tar.xz $scr_dir/ ^/dev/null
                else 
                    error build in $chroot_name failed
                end
            else 
                error "Failed to checkout blt4linux"
            end
        else 
            error "Failed to clone blt4linux"
        end
    end
    
end

cd $scr_dir

chr_bld ubuntu_precise # steamrt
chr_bld fc26

set -x CMAKE_CXX_COMPILER (which g++4.8)
set -x CMAKE_C_COMPILER   (which gcc-4.8)

./package.sh
cp ./package_build/*.xz .
