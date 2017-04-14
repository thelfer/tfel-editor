option(enable-qt4 "build aster interface for mechanical behaviours" OFF)
option(enable-qt5 "build aster interface for mechanical behaviours" OFF)

if((NOT enable-qt4) AND (NOT enable-qt5))
  message(FATAL_ERROR "no qt version selected")
endif((NOT enable-qt4) AND (NOT enable-qt5))

if((enable-qt4) AND (enable-qt5))
  message(FATAL_ERROR "one and only one version "
    "of qt must be selected")
endif((enable-qt4) AND (enable-qt5))

if(enable-qt4)
  add_definitions("-DQEMACS_QT4")
  find_package(Qt4 4.6 COMPONENTS
    QtCore QtGui 
    QtXml QtSvg
    QtNetwork QtWebKit
    REQUIRED)
endif(enable-qt4)

if(enable-qt5)
  add_definitions("-DQEMACS_QT5")
  find_package(Qt5 5.3 COMPONENTS
    Core Widgets 
    Xml Svg
    Network
    PrintSupport
    WebKit
    WebKitWidgets
    REQUIRED)
endif(enable-qt5)

macro(moc_source header_directory file)
  set(header_file "${header_directory}/${file}.hxx")
  set(moc_output  "moc_${file}.cxx")
  if(enable-qt4)
    QT4_GENERATE_MOC(${header_file} ${moc_output})
  endif(enable-qt4)    
  if(enable-qt5)
    QT5_GENERATE_MOC(${header_file} ${moc_output})
  endif(enable-qt5)    
endmacro(moc_source)

macro(moc_sources flist header_directory)
  if(${ARGC} LESS 1)
    message(FATAL_ERROR "moc_sources : no source specified")
  endif(${ARGC} LESS 1)
  foreach(file ${ARGN})
    moc_source(${header_directory} ${file})
    set(${flist} "moc_${file}.cxx" ${${flist}})
  endforeach(file)
endmacro(moc_sources)

macro(qt_sources flist header_directory)
  if(${ARGC} LESS 1)
    message(FATAL_ERROR "qt_sources : no source specified")
  endif(${ARGC} LESS 1)
  moc_sources(${flist} ${header_directory} ${ARGN})
  foreach(file ${ARGN})
    set(${flist} "${file}.cxx" ${${flist}})
  endforeach(file)
endmacro(qt_sources)

macro(qt_add_resources)
  if(enable-qt4)
    qt4_add_resources(${ARGN})
  endif(enable-qt4)    
  if(enable-qt5)
    qt5_add_resources(${ARGN})
  endif(enable-qt5)    
endmacro(qt_add_resources)

add_definitions("-DQT_DEPRECATED_WARNINGS")