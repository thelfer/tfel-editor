#! /bin/bash

mkdir -p config

autopoint -f
# gettextize -f --intl

aclocal -I m4
libtoolize
automake --add-missing
autoconf
