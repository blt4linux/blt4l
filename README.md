BLT4L Readme
============

BLT4L is a part clean-room, part-line-by-line rewrite of BLT, the PAYDAY
2 Better Lua injecTor, for Linux. It is compatibile with all BLT mods,
presents the same API, and uses the same LUA base.

Credits
=======

* [Roman Hargrave](https://github.com/RomanHargrave) - Initial work, code, translations from BLT4WIN
* [Campbell Suter](https://github.com/ZNixian) - BLT2 updates, and maintenance
* [Leonard König](https://github.com/LeonardKoenig) - C++ cleanup, linker work
* [Ozymandias117](https://github.com/Ozymandias117) - Fixed subhook on 64-bit platforms
* [James Wilkinson](https://github.com/JamesWilko) - BLT LUA (and BLT)

Installing
==========

You can install BLT4L in a few different ways. 
The quickest way is to clone this repository, and run `install.sh`.
This will work best on Debian, or a Debian derivative, such as SteamOS, as
it can help you install missing dependecies.

If you're on Arch Linux (or a derivative like Manjaro), you can install the AUR package
[blt4l](https://aur.archlinux.org/packages/blt4l/). This package uses the new launcher script,
works with or without the Steam runtime, and will automatically install the base Lua if it's not present.

You can also find prebuilt copies under the [releases](https://github.com/blt4linux/blt4l/releases)
tab, or if you want to debug or modify BLT4L, you can build it manually.

Building & Manual Install
=========================

##### In order to build BLT4L, you will need:

* openssl 
* curl4-openssl (gnutls is NOT supported)
* zlib
* cmake
* a build tool chain (most distros have one preinstalled, or available in a build-essentials package)

##### Do the following:

```
$ git submodule init
$ git submodule update
$ mkdir build 
$ cd build
$ cmake .. 
$ make
```

You should find the hook in your build folder, named `libblt_loader.so`.
You will need to set `LD_PRELOAD` for the PAYDAY2 process to find the
loader.

Next, you will need to copy a LUA mod base to your PAYDAY 2 folder (or whatever working directory you intend to run PAYDAY 2 in).
There is a symlink to the BLT LUA mod base (from the BLT4WIN submodule) under `lua/mods`. 

If you set everyting up correctly, you should be up and running with the BLT mod API in PAYDAY 2.

##### Additional work for SELinux users (e.g. Fedora users):

Because our hook writes to executable sections of payday2, SELinux doesn't like us (and it shouldn't).
In order to add an SELinux exception for the BLT Hook, you must do the following:

Run Payday2 with libblt_loader.so (It will crash, and SELinux will log the error)
```
$ sudo ausearch -c 'payday2_release' --raw | audit2allow -M my-payday2-hook
$ sudo semodule -i my-payday-hook.pp
```

Lua & Developer Information
===========================

##### Errors

By default, any Lua error (outside of a pcall block) will immediately crash the game, and
generate a error.txt file (found in mods/logs) containing the backtrace.

In some situations, it might be useful to continue the game running despite the errors. In that
case, set the `BLT_CRASH` environment variable to `CONTINUE`. This can be done by
prepending `BLT_CRASH=CONTINUE` to the game launch arguments in Steam.

##### Lua API

BLT4L contains some additional Lua functions not present in windows PAYDAY.

The first set of these are in the `vm` table, and are functions copied from Lua 5.1
that are missing or have different behaviour in PAYDAY:

* `vm.dofile`
* `vm.loadfile`
* `vm.load`
* `vm.loadstring` (`loadstring` seems to be present in current versions, however)
* `vm.pcall`
* `vm.xpcall`

The Linux version of PAYDAY is also missing a couple of key Lua APIs that are commonly used
by mods.

The first thing here is the `SystemFS` API. Many mods rely on this to save and load files. While
BLT4L has added in some of these functions, it is very hard to determine the return types of some
functions. Mods should use the Lua `io` table wherever possible.

Many of the `DB` functions are missing - in particular, `DB:create_entry` is missing, which is
how mods usually load custom assets (models, textures, etc) into the game. Work is (slowly) being done
to reverse-engineer and reimplement this by @RomanHargrave and @ZNixian, however due to the difficulty of
this task this will probably take a long time. As a result of this, custom heist/weapon/mask mods will
not work (anything that goes into `mod_overrides` will still work, however).

If you find any other Lua functions that are missing in BLT4L, please open an issue.
