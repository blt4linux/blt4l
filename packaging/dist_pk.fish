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

set scr_dir (dirname (status -f))
set scm     (git remote get-url origin)

info packaging!

function chr_bld -a chroot_name
    info building $chroot_name
  
    set chroot_home $HOME
    set pkbase      $chroot_home/blt4l/
    set pkdir       /var/chroots/$chroot_name/$pkbase/packaging

    function -S chdo
        info "($chroot_name) $argv"
        schroot --directory $pkbase -c $chroot_name -- $argv
    end

    # Remove repo
    if [ -d /var/chroots/$chroot_name/$pkbase ]
        chdo rm -rf $pkbase/.git
        chdo rm -r $pkbase
    end

    # Remove build dir
    if [ -d $pkdir/package_build ]
        rm -r $pkdir/package_build
    end
    
    info checking out blt4l

    if      schroot --directory $chroot_home -c $chroot_name -- git clone --recursive $scm $pkbase
        and chdo git checkout (chdo git tag | sort -V | tail -1)
    else 
        error "Failed to clone & checkout blt4linux"
    end
    
    # Run build
    if schroot --directory $pkbase/packaging -c $chroot_name ./package.sh
        # Move artifacts out of chroot
        mv $pkdir/*.tar.xz $scr_dir/ ^/dev/null
    else 
        error build in $chroot_name failed
    end
end

cd $scr_dir

chr_bld ubuntu_trusty
chr_bld ubuntu_precise

set -x CMAKE_CXX_COMPILER (which g++4.8)
set -x CMAKE_C_COMPILER   (which gcc-4.8)

./package.sh
