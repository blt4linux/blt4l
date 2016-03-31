BLT4L Readme
============

BLT4L is an attempt to port the BLT mod loader for PayDay 2 to Linux.
It should be compatible with lua scripts meant for BLT so long as they use platform-agnostic design (e.g. file paths).

It is in early stages and does not yet work fully.

Building
========

BLT4L uses SubHook to perform runtime code modification in order to hook the statically linked LUA runtime in PayDay 2.
A SubHook submodule is present in the repository that has been checked out at a commit that is known to be working.
This needs to be built and installed before building BLT4L:

```
$ git submodule init
$ git submodule update
$ mkdir build && cd build
$ cmake ..
$ make
```

Notes
=====

I am a bit out of my element writing C++, as I prefer to use C; however, it is more convenient to use C++ when you're trying
to call and overwrite functions in a C++ application. For this reason you may notice a few C-like things in the C++ code.
Feel free to correct them and open a PR.

Additionally, I should point out that I am using the linker to access the `lua_*` functions. This is because (as of 2016-03-22)
the `payday2_release` image is not stripped (or at least not fully; as is sometimes the case with staticly linked images). For this
reason we can forego signature hunting and the like, as we have symbols and the linker, the ***ultimate*** signature system. I've
backed up a copy of the payday image should a stripped update come out and necessitate signatures.
