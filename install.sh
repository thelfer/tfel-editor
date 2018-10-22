#! /usr/bin/env bash

set -e
# PREFIX=$HOME
PREFIX=/home/tom/
PYTHON_VERSION=3.6

export TFELSRC=$PREFIX/codes/tfel/master/src
export TFELPLOTSRC=$PREFIX/codes/tfel-plot/master/src
export TFELEDITORSRC=$PREFIX/codes/tfel-editor/master/src
export TFELBASEHOME=$PREFIX/codes/tfel/master
export TFELPLOTBASEHOME=$PREFIX/codes/tfel-plot/master
export TFELEDITORBASEHOME=$PREFIX/codes/tfel-editor/master

mkdir -p $TFELSRC
mkdir -p $TFELPLOTSRC
mkdir -p $TFELEDITORSRC

pushd $TFELSRC
if [ -d tfel ]
then
	pushd tfel
	git pull
	popd 
else
	git clone https://github.com/thelfer/tfel.git
fi
popd

pushd $TFELPLOTSRC
if [ -d tfel-plot ]
then
	pushd tfel-plot
	git pull
	popd 
else
	git clone https://github.com/thelfer/tfel-plot.git
fi
popd

pushd $TFELEDITORSRC
if [ -d tfel-editor ]
then
	pushd tfel-editor
	git pull
	popd 
else
	git clone https://github.com/thelfer/tfel-editor.git
fi
popd

tfel_rev=$(cd $TFELSRC/tfel;git rev-parse HEAD)
tfel_plot_rev=$(cd $TFELPLOTSRC/tfel-plot;git rev-parse HEAD)
tfel_editor_rev=$(cd $TFELEDITORSRC/tfel-editor;git rev-parse HEAD)

export TFELHOME=$TFELBASEHOME/install-${tfel_rev}
export TFELPLOTHOME=$TFELPLOTBASEHOME/install-${tfel_plot_rev}
export TFELEDITORHOME=$TFELEDITORBASEHOME/install-${tfel_editor_rev}

pushd $TFELBASEHOME
tfel_previous=$(ls -1dtr install-* | tail -1)
tfel_previous_rev=${tfel_previous#install-}
popd

echo "tfel current  revision : $tfel_rev" 
echo "tfel previous revision : $tfel_previous_rev" 
if [ x"$tfel_previous_rev" != x"$tfel_rev" ];
then
  pushd $TFELSRC
  mkdir -p build
  pushd build
  cmake ../tfel -DCMAKE_BUILD_TYPE=Release -Dlocal-castem-header=ON -Denable-aster=ON -Denable-abaqus=ON -Denable-calculix=ON -Denable-ansys=ON -Denable-europlexus=ON -Denable-python=ON -Denable-python-bindings=ON -DPython_ADDITIONAL_VERSIONS=$PYTHON_VERSION -DCMAKE_INSTALL_PREFIX=$TFELHOME
  make
  make install
  popd
  popd
  
  cat >> ${TFELHOME}/env.sh <<EOF
  export TFELHOME=${TFELHOME}
  export PATH=${TFELHOME}/bin:\$PATH
  export LD_LIBRARY_PATH=${TFELHOME}/lib:\$LD_LIBRARY_PATH
  export PYTHONPATH=${TFELHOME}/lib/python${PYTHON_VERSION}/site-packages/:\$PYTHONPATH
  export PYTHONPATH=${TFELHOME}/lib/python${PYTHON_VERSION}m/site-packages/:\$PYTHONPATH
EOF
  
  pushd $TFELBASEHOME
  count=$(ls -1d install-* | wc -l)
  if (( $count > 4 )) ; then 
    find . -maxdepth 1 -ctime +60 -name "install-*" -exec rm -rf {} \;
  fi
  last=$(ls -1dtr install-* | tail -1)
  ln -nsf "${last}" install
fi

source ${TFELHOME}/env.sh

pushd $TFELPLOTBASEHOME
tfel_plot_previous=$(ls -1dtr install-* | tail -1)
tfel_plot_previous_rev=${tfel_plot_previous#install-}
popd

if [ x"$tfel_previous_rev" != x"$tfel_rev" ] || [ x"$tfel_plot_previous_rev" != x"$tfel_plot_rev" ];
then
  pushd $TFELPLOTSRC
  mkdir -p build
  pushd build
  cmake ../tfel-plot -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$TFELPLOTHOME
  make
  make install
  popd
  popd
  
  cat >> ${TFELPLOTHOME}/env.sh <<EOF
  export TFELPLOTHOME=${TFELPLOTHOME}
  source ${TFELHOME}/env.sh
  export PATH=${TFELPLOTHOME}/bin:\$PATH
  export LD_LIBRARY_PATH=${TFELPLOTHOME}/lib:\$LD_LIBRARY_PATH
EOF
  
  pushd $TFELPLOTBASEHOME
  count=$(ls -1d install-* | wc -l)
  if (( $count > 4 )) ; then 
    find . -maxdepth 1 -ctime +60 -name "install-*" -exec rm -rf {} \;
  fi
  last=$(ls -1dtr install-* | tail -1)
  ln -nsf "${last}" install
fi

source ${TFELPLOTHOME}/env.sh

pushd $TFELEDITORBASEHOME
tfel_editor_previous=$(ls -1dtr install-* | tail -1)
tfel_editor_previous_rev=${tfel_editor_previous#install-}
popd

if [ x"$tfel_previous_rev" != x"$tfel_rev" ] || [ x"$tfel_plot_previous_rev" != x"$tfel_plot_rev" ] || [ x"$tfel_editor_previous_rev" != x"$tfel_editor_rev" ];
then
  pushd $TFELEDITORSRC
  mkdir -p build
  pushd build
  cmake ../tfel-editor -DCMAKE_BUILD_TYPE=Release -Denable-tfel-plot=ON -DCMAKE_INSTALL_PREFIX=$TFELEDITORHOME
  make
  make install
  popd
  popd
  
  cat >> ${TFELEDITORHOME}/env.sh <<EOF
  export TFELEDITORHOME=${TFELEDITORHOME}
  source ${TFELPLOTHOME}/env.sh
  export PATH=${TFELEDITORHOME}/bin:\$PATH
  export LD_LIBRARY_PATH=${TFELEDITORHOME}/lib:\$LD_LIBRARY_PATH
  export LD_LIBRARY_PATH=${TFELEDITORHOME}/share/tfel-editor/packages:\$LD_LIBRARY_PATH
EOF
  
  pushd $TFELEDITORBASEHOME
  count=$(ls -1d install-* | wc -l)
  if (( $count > 4 )) ; then 
    find . -maxdepth 1 -ctime +60 -name "install-*" -exec rm -rf {} \;
  fi
  last=$(ls -1dtr install-* | tail -1)
  ln -nsf "${last}" install
fi

