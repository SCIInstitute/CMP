# jc -- For more information building and packaging with CMake and Qt, see the
# following URL:  http://www.cmake.org/Wiki/BundleUtilitiesExample
# See also the following thread:
#  http://www.mail-archive.com/paraview@paraview.org/msg05234.html
# Additional information for deploying Qt applications with plugins
# (i.e the image reader/writers) can be found here: 
# http://doc.qt.nokia.com/qq/qq09-mac-deployment.html#plugins
#
# To properly build a package on the Mac, CMake version 2.8.3 or later really
# is required.

project( CMP )
cmake_minimum_required(VERSION 2.8.3)  

# Match everything when analyzing dependencies
INCLUDE_REGULAR_EXPRESSION("^.*$")
option (BUILD_SHARED_LIBS "Build shared libraries" OFF)
option (BUILD_TESTS "Build CMP test applications." OFF)
option (INSTALL_TEST_FILES "Include CMP test images and help files in the install." OFF)
option (INSTALL_SOURCE  "Install CMP source code." OFF)

find_package(OpenCV REQUIRED)

include(FindQwt.cmake)
if(QWT_FOUND)
  include_directories( ${QWT_INCLUDE_DIR})
  link_directories( ${QWT_LIBRARY} )
endif(QWT_FOUND)

# Include subdirectories in the cmp build
include_directories(${CMP_SOURCE_DIR}/code)

set( CMP_SRCS
     code/main.cpp
     code/cmp.cpp
     code/cmpPlots.cpp
     code/cmpNumerics.cpp
     code/cmpImage.cpp
     code/cmpWidgets.cpp
     code/cmpUtils.cpp
     code/cmpPreferences.cpp
     code/cmpColorPaletteWidget.cpp
     code/cmpQwtEllipsePlotItem.cpp
     code/cmpQwtDataPlot.cpp
     code/cmpQtPairPlotDialog.cpp
)

set( CMP_MOC_HDRS
     code/cmp.h
     code/cmpPreferences.h
     code/cmpColorPaletteWidget.h
     code/cmpWidgets.h
     code/cmpImage.h
     code/cmpQwtDataPlot.h
)


set( CMP_UIS
     code/cmp2.ui
     code/cmpPreferences.ui
     code/cmpAdvancedPlotDialog.ui
)

set( CMP_RCS
    code/cmp.qrc
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_definitions( -Wall )

# by default only QtCore and QtGui modules are enabled
# other modules must be enabled like this:
set( QT_USE_QT3SUPPORT TRUE )   
set( QT_USE_QTXML TRUE )

#--------------------------------------------------------------------------------
# This sets the windows build that will need the special winmain@16 call. Qt provides
#  this for us in the qtmain.lib file. Using this cmake code will ensure we have it
#  linked into our build. Not needed on Unix/OS X/Linux which is why we have the 
#  IF(WIN32) conditional.
if(WIN32)
  set (QT_USE_QTMAIN TRUE)
endif(WIN32)

# this command finds Qt4 libraries and sets all required variables
# note that it's Qt4, not QT4 or qt4
find_package( Qt4 REQUIRED )

# add some useful macros and variables
# (QT_USE_FILE is a variable defined by FIND_PACKAGE( Qt4 ) that contains a path to CMake script)
include( ${QT_USE_FILE} )

# this command will generate rules that will run rcc on all files from CMP_RCS
# in result CMP_RC_SRCS variable will contain paths to files produced by rcc
QT4_ADD_RESOURCES( CMP_RC_SRCS ${CMP_RCS} )

# this will run uic on .ui files:
QT4_WRAP_UI( CMP_UI_HDRS ${CMP_UIS} )

# and finally this will run moc:
QT4_WRAP_CPP( CMP_MOC_SRCS ${CMP_MOC_HDRS} )

# we need this to be able to include headers produced by uic in our code
# (CMAKE_BINARY_DIR holds a path to the build directory)
INCLUDE_DIRECTORIES( ${CMAKE_BINARY_DIR} )
#INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} )

# optionally build the test files
#IF ( BUILD_TESTS )
#   ADD_SUBDIRECTORY(test)
#endif(BUILD_TESTS)

# For Apple set the icns file containing icons
if(APPLE)
  # set how it shows up in the Info.plist file
  set(MACOSX_BUNDLE_ICON_FILE CMP.icns) 
  # set where in the bundle to put the icns file
  set_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/CMP.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  # include the icns file in the target
  set(CMP_SRCS ${CMP_SRCS} ${CMAKE_CURRENT_SOURCE_DIR}/CMP.icns)
endif(APPLE)

# here we instruct CMake to build CMP executable from all of the source files
add_executable( CMP MACOSX_BUNDLE WIN32 
                ${CMP_SRCS} 
                ${CMP_MOC_SRCS} 
                ${CMP_RC_SRCS} 
                ${CMP_UI_HDRS} )

target_link_libraries( CMP ${QT_LIBRARIES} ${OpenCV_LIBS} qwt)

# INSTALLATION AND PACKAGING
include(InstallRequiredSystemLibraries)

set(plugin_dest_dir bin)
set(qtconf_dest_dir bin)
set(APPS "\${CMAKE_INSTALL_PREFIX}/bin/CMP")
if(APPLE)
  set(plugin_dest_dir CMP.app/Contents/MacOS)
  set(qtconf_dest_dir CMP.app/Contents/Resources)
  set(contents_dest_dir CMP.app/Contents)
  set(APPS "\${CMAKE_INSTALL_PREFIX}/CMP.app")
endif(APPLE)
if(WIN32)
  set(APPS "\${CMAKE_INSTALL_PREFIX}/bin/CMP.exe")
endif(WIN32)



#--------------------------------------------------------------------------------
# Install the CMP application, on Apple, the bundle is at the root of the
# install tree, and on other platforms it'll go into the bin directory.
install(TARGETS CMP 
    BUNDLE DESTINATION . COMPONENT Runtime
    RUNTIME DESTINATION bin COMPONENT Runtime
    )

#--------------------------------------------------------------------------------
# Install needed Qt plugins by copying directories from the qt installation
# One can cull what gets copied by using 'REGEX "..." EXCLUDE'
install(DIRECTORY "${QT_PLUGINS_DIR}/imageformats" DESTINATION ${plugin_dest_dir}/plugins COMPONENT Runtime)

#--------------------------------------------------------------------------------
# Install needed Qt .nib files and others
if(APPLE)
  install(DIRECTORY "${QT_QTGUI_LIBRARY_RELEASE}/Versions/Current/Resources" DESTINATION ${contents_dest_dir} COMPONENT Runtime)
endif(APPLE)

#--------------------------------------------------------------------------------
# install a qt.conf file
# this inserts some cmake code into the install script to write the file
install(CODE "
   file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${qtconf_dest_dir}/qt.conf\" \"\")
    " COMPONENT Runtime)


#--------------------------------------------------------------------------------
# Use BundleUtilities to get all other dependencies for the application to work.
# It takes a bundle or executable along with possible plugins and inspects it
# for dependencies.  If they are not system dependencies, they are copied.

# directories to look for dependencies
set(DIRS ${QT_LIBRARY_DIRS} ${OpenCV_LIB_DIR} ${QWT_LIBRARY} ${QT_PLUGINS_DIR})

# Now the work of copying dependencies into the bundle/package
# The quotes are escaped and variables to use at install time have their $ escaped
# An alternative is the do a configure_file() on a script and use install(SCRIPT  ...).
# Note that the image plugins depend on QtSvg and QtXml, and it got those copied
# over.  

# fixup_bundle will run the appropriate install_name_tool
# commands on each of the dependencies of the main executable and the
# plugin libraries.  
#
# install_name_tool is used to point any non-system
# dynamically-loaded libraries to the appropriate libraries in the
# distribution
#
# fixup_bundle also grabs the appropriate parts of the qt framework
# (by following dependencies) and installs them in the distribution
# 
install(CODE "
    file(GLOB_RECURSE QTPLUGINS
      \"\${CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}/plugins/*${CMAKE_SHARED_LIBRARY_SUFFIX}\")
    include(BundleUtilities)
    fixup_bundle(\"${APPS}\" \"\${QTPLUGINS}\" \"${DIRS}\")
    " COMPONENT Runtime)

# To Create a package, one can run "cpack -G DragNDrop CPackConfig.cmake" on Mac OS X
# where CPackConfig.cmake is created by including CPack
# And then there's ways to customize this as well
set(CPACK_BINARY_DRAGNDROP ON)

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "CMP Research Sandbox")
set(CPACK_PACKAGE_VENDOR "University of Utah")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/PackageDescription.txt")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/Copyright.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "2")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "CMP ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

include(CPack)
