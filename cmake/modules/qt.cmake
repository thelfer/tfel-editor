find_package(Qt5 5.3 COMPONENTS
  Core Widgets 
  Xml Svg
  Network
  PrintSupport
  WebKit
  WebKitWidgets
  REQUIRED)

macro(moc_source header_directory file)
  set(header_file "${header_directory}/${file}.hxx")
  set(moc_output  "moc_${file}.cxx")
  QT5_GENERATE_MOC(${header_file} ${moc_output})
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
  qt5_add_resources(${ARGN})
endmacro(qt_add_resources)

add_definitions("-DQT_DEPRECATED_WARNINGS")
