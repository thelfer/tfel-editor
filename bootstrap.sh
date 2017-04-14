#! /bin/bash

if [ -d src/hunspell-1.3.2 ];
then
    pushd src/hunspell-1.3.2
    ./bootstrap.sh
    popd
fi

mkdir -p config
aclocal -I m4
libtoolize
automake --add-missing
autoconf
