#!/bin/bash

nm "$@" | grep '_ZT' | cut -d' ' -f3 
