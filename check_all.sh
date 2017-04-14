#! /usr/bin/env bash

do_test(){
    qt=$1
    mkdir $qt
    pushd $qt
    mkdir autotools
    pushd autotools
    mkdir install-autotools
    if test "x$fcheck" == "xno" ;
    then
	mkdir install-autotools-debug
    fi
    mkdir build-autotools
    pushd build-autotools
    $src/configure --enable-$qt --prefix=$build/build-check/$qt/autotools/install-autotools 
    $make_exec
    $make_exec check
    $make_exec distcheck
    $make_exec install
    popd # from build-autotools
    if test "x$fcheck" == "xno" ;
    then
	mkdir build-autotools-debug
	pushd build-autotools-debug
	$src/configure --enable-debug --enable-$qt --prefix=$build/build-check/$qt/autotools/install-autotools-debug 
	$make_exec
	$make_exec check
	$make_exec distcheck
	$make_exec install
	popd # from build-autotools-debug
    fi
    popd # from autotools
    mkdir cmake
    pushd cmake
    tar -xvjf $build/build-check/$qt/autotools/build-autotools/qemacs-$pkg_name.tar.bz2
    mkdir install-cmake
    mkdir build-cmake
    if test "x$fcheck" == "xno" ;
    then
	mkdir install-cmake-release
	mkdir build-cmake-release
	mkdir install-cmake-debug
	mkdir build-cmake-debug
    fi
    pushd build-cmake
    cmake ../qemacs-$pkg_name/ -Denable-$qt=ON -DCMAKE_INSTALL_PREFIX=$build/build-check/$qt/cmake/install-cmake
    $make_exec 
    # if test "x$pbuild" == "xyes" ;
    # then
    # 	make check ARGS="-j $nbproc"
    # else
    # 	$make_exec check 
    # fi
    $make_exec install
    popd #from build-cmake
    if test "x$fcheck" == "xno" ;
    then
	pushd build-cmake-release
	cmake ../qemacs-$pkg_name/ -DCMAKE_BUILD_TYPE=Release -Denable-$qt=ON -DCMAKE_INSTALL_PREFIX=$build/build-check/$qt/cmake/install-cmake-release
	$make_exec
	# if [ test "x$pbuild" == "xyes" ];
	# then
	#     make check ARGS="-j $nbproc"
	# else
	#     $make_exec check 
	# fi
	$make_exec install
	popd #from build-cmake-release
	pushd build-cmake-debug
	cmake ../qemacs-$pkg_name/ -DCMAKE_BUILD_TYPE=Debug -Denable-$qt=ON -Denable-$qt=ON -DCMAKE_INSTALL_PREFIX=$build/build-check/$qt/cmake/install-cmake-debug
	$make_exec
	# if [ test "x$pbuild" == "xyes" ];
	# then
	#     make check ARGS="-j $nbproc"
	# else
	#     $make_exec check 
	# fi
	$make_exec install
	popd #from build-cmake-debug
    fi
    popd
    popd
}

# Exit if any error detected
set -e
# parallel build
pbuild=no
# fast check
fcheck=no
# cross compilation using mingw
while getopts ":w:fj:" opt;
do
  case $opt in
    f) fcheck=yes
      ;;
    j) pbuild=yes;
       nbproc="$OPTARG";
      ;;
    \?)
      echo "$0 : invalid option '$OPTARG'" >&2
      echo "bailing out." >&2
      exit 1
      ;;
    :)
      echo "$0 : option '-$OPTARG' requires an argument." >&2
      echo "bailing out." >&2
      exit 1
      ;;
  esac
done

make_exec=make
if test "x$pbuild" == "xyes" ;
then
    make_exec="$make_exec -j $nbproc"
fi

# remove previous temporary
if [ -d build-check ];
then
    chmod +w -R build-check
    rm -fr build-check
fi

# source directory
src=$(dirname $(realpath $0))
# current directory
build=$(pwd)

# get the package name
pkg_name=$(cat $src/configure.ac|grep AC_INIT|awk 'BEGIN{FS=","} {print $2}')

mkdir build-check
pushd build-check

for qt in qt4 qt5
do
    do_test $qt 
done
