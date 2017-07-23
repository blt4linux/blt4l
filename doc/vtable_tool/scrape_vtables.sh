#!/bin/bash

nm -S "$@" | grep '_ZT' | cut -d' ' -f2,4
