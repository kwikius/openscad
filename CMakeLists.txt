#
# CMake build system for OpenSCAD
#
# Configuration variables
#   -DHEADLESS=<ON|OFF>
#   -DNULLGL=<ON|OFF>
#   -DSNAPSHOT=<ON|OFF>
#   -DEXPERIMENTAL=<ON|OFF>
#

cmake_minimum_required(VERSION 3.13)

if(POLICY CMP0071) # Let AUTOMOC and AUTOUIC process GENERATED files.
  cmake_policy(SET CMP0071 NEW)
endif()
if(POLICY CMP0072) # FindOpenGL prefers GLVND by default when available.
  cmake_policy(SET CMP0072 NEW)
endif()
if(POLICY CMP0079) # `target_link_libraries()` allows use with targets in other directories.
  cmake_policy(SET CMP0079 NEW)
endif()
if(POLICY CMP0082) # Install rules from add_subdirectory() calls are interleaved with those in caller.
  cmake_policy(SET CMP0082 NEW)
endif()

set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum OS X deployment version")

include("cmake/Modules/openscad_version.cmake")
project(openscad LANGUAGES C CXX)

option(INFO "Display build configuration info at end of cmake config" ON)
option(ENABLE_TESTS "Run testsuite after building." ON)
option(EXPERIMENTAL "Enable Experimental Features" OFF)
option(SNAPSHOT "Create dev snapshot, uses nightly icons" OFF)
option(HEADLESS "Build without GUI frontend" OFF)
option(ENABLE_EGL "Enable EGL instead of GLX support if available" OFF)
option(WASM "Build WebAssembly, (implies NULLGL=ON) " OFF)
option(NULLGL "Build without OpenGL, (implies HEADLESS=ON) " OFF)
option(IDPREFIX "Prefix CSG nodes with index # (debugging purposes only, will break node cache)" OFF)
option(PROFILE "Enable compiling with profiling / test coverage instrumentation" OFF)
option(MXECROSS "Enable setup for MXE cross platform build" OFF)
option(OFFLINE_DOCS "Download Documentation for offline usage" OFF)
option(USE_MIMALLOC "Use mimalloc as malloc replacement." ON)
option(USE_CCACHE "Use ccache to speed up compilation." ON)
option(ENABLE_CAIRO "Enable support for cairo vector graphics library." ON)
option(ENABLE_SPNAV "Enable support for libspnav input driver." ON)
option(ENABLE_HIDAPI "Enable support for HIDAPI input driver." ON)
option(ALLOW_BUNDLED_HIDAPI "Allow usage of bundled HIDAPI library (Windows only)." OFF)
include(CMakeDependentOption)
cmake_dependent_option(APPLE_UNIX "Build OpenSCAD in Unix mode in MacOS X instead of an Apple Bundle" OFF "APPLE" OFF)
cmake_dependent_option(ENABLE_QTDBUS "Enable DBus input driver for Qt5." ON "NOT HEADLESS" OFF)
cmake_dependent_option(ENABLE_GAMEPAD "Enable Qt5Gamepad input driver." ON "NOT HEADLESS" OFF)

# GNUInstallDirs must be done before BuildParameters
include(GNUInstallDirs)

# Use clang-tidy if run with -DCLANG_TIDY=1
find_program(CLANG_TIDY_EXE NAMES clang-tidy)
set(CLANG_TIDY ${CLANG_TIDY} CACHE BOOL "Enable clang-tidy")
if(CLANG_TIDY AND CLANG_TIDY_EXE)
  include(RegexUtils)
  escape_string_as_regex(regex "${CMAKE_SOURCE_DIR}/src")
  # From cmake docs: https://cmake.org/cmake/help/latest/variable/CMAKE_LANG_CLANG_TIDY.html :
  #   "[CMAKE_CXX_CLANG_TIDY] is used to initialize the property on each target as it is created"
  # So this is done **before** the add_executable command.
  set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-header-filter=${regex}")
endif()

set(openscadEXECUTABLE OpenSCAD)
set(openscadSTATIC_LIBS openscadCGAL openscadGUI)
set(openscadALLTARGETS ${openscadEXECUTABLE} ${openscadSTATIC_LIBS})

add_executable(${openscadEXECUTABLE})

add_subdirectory(submodules)
# static libraries
add_subdirectory(src/geometry/cgal)
add_subdirectory(src/gui)

target_link_libraries(${openscadEXECUTABLE} PUBLIC
    ${openscadSTATIC_LIBS}
)

foreach(scadTarget ${openscadALLTARGETS})
   # Strict c++17, e.g. no GNU extensions
   set_property(TARGET ${scadTarget} PROPERTY CXX_STANDARD 17)
   set_property(TARGET ${scadTarget} PROPERTY CXX_EXTENSIONS OFF)
   set_property(TARGET ${scadTarget} PROPERTY CXX_STANDARD_REQUIRED ON)
   # Output compilation database (compile_commands.json), so we can e.g. run clang-tidy or other tools separately
   set_property(TARGET ${scadTarget} PROPERTY EXPORT_COMPILE_COMMANDS ON)
   set_property(TARGET ${scadTarget} PROPERTY LINKER_LANGUAGE CXX)
   # Verbose link step
   if(CMAKE_CXX_COMPILER_ID MATCHES "AppleClang|Clang|GNU")
     set_property(TARGET ${scadTarget} PROPERTY LINK_FLAGS -v)
   endif()
endforeach(scadTarget)

set(SUFFIX "" CACHE STRING "Installation suffix for binary (e.g. 'nightly')")
set(STACKSIZE 8388608 CACHE STRING "Stack size (default is 8MB)")

if(PROFILE)
  SET(GCC_COVERAGE_COMPILE_FLAGS "-g -O0 --coverage -fprofile-arcs -ftest-coverage -fprofile-dir=.gcov")
  SET(GCC_COVERAGE_LINK_FLAGS    "--coverage")
  # TODO use more specific commands "target_compile_options" etc. to avoid potentially affecting submodules?
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")
endif()

if(WASM)
  set(NULLGL ON CACHE BOOL "" FORCE)
  set(ENV{PKG_CONFIG_PATH} "/emsdk/upstream/emscripten/cache/sysroot/lib/pkgconfig")
  target_compile_definitions(openscadCGAL PRIVATE CGAL_DISABLE_ROUNDING_MATH_CHECK)
endif()

if(NULLGL)
  set(HEADLESS ON CACHE BOOL "" FORCE)
endif()
if(EXPERIMENTAL)
  target_compile_definitions(${openscadEXECUTABLE} PRIVATE ENABLE_EXPERIMENTAL)
endif()
if(IDPREFIX)
  target_compile_definitions(${openscadEXECUTABLE} PRIVATE IDPREFIX)
endif()

if (NOT "${SUFFIX}" STREQUAL "")
  set(SUFFIX_WITH_DASH "-${SUFFIX}")
  target_compile_definitions(${openscadEXECUTABLE} PRIVATE OPENSCAD_SUFFIX="${SUFFIX_WITH_DASH}")
endif()

if (MXECROSS OR WIN32)
  add_subdirectory(winconsole)
  foreach(library ${openscadEXECUTABLE} openscadGUI)
    set_property(TARGET ${library} PROPERTY WIN32_EXECUTABLE ON)
  endforeach(library)
endif()

set(OPENSCAD_LIB_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/objects")
file(MAKE_DIRECTORY ${OPENSCAD_LIB_OUTPUT_DIR})

# Default to Release build
#if(NOT CMAKE_BUILD_TYPE)
#  message(STATUS "CMAKE_BUILD_TYPE not specified.  Defaulting to 'Release'")
#  message(STATUS "Usage: cmake -DCMAKE_BUILD_TYPE=[Debug|Release|RelWithDebInfo|MinSizeRel] .")
#  set(CMAKE_BUILD_TYPE Release)
#else()
#  message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
#endif()
foreach(library ${openscadALLTARGETS})
   target_compile_options(${library} PRIVATE "$<$<CONFIG:DEBUG>:-DDEBUG>")
   target_compile_definitions(${library} PRIVATE _REENTRANT UNICODE _UNICODE)
endforeach(library)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frounding-math")
  if (WIN32) # gcc bug spams warnings, See issue #2771
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-attributes")
  endif()
endif()

find_package(PkgConfig)

if (MXECROSS)
  foreach(library ${openscadEXECUTABLE} openscadGUI)
     target_compile_definitions(${library} PRIVATE GLEW_STATIC LIBXML_STATIC STATIC_QT_SVG_PLUGIN)
  endforeach(library)
  target_link_libraries(${openscadEXECUTABLE} PRIVATE bcrypt)
endif()

macro(find_graphics)
  # NULLGL - Allow us to build without OpenGL(TM). run 'cmake .. -DNULLGL=1'
  # Most tests will fail, but it can be used for testing/experiments
  if(NULLGL)
    foreach(library ${openscadEXECUTABLE} openscadGUI)
       target_compile_definitions(${library} PRIVATE NULLGL)
    endforeach(library)
  else()
    find_package(OpenCSG REQUIRED QUIET)
    message(STATUS "OpenCSG: ${OPENCSG_VERSION_STRING}")
    foreach(library ${openscadEXECUTABLE} openscadGUI)
       target_link_libraries(${library} PRIVATE ${OPENCSG_LIBRARY})
       target_compile_definitions(${library} PRIVATE ENABLE_OPENCSG)
    endforeach(library)
    if(MSVC)
      find_package(GLEW CONFIG REQUIRED QUIET)
      find_path(OPENCSG_INCLUDE_DIRS opencsg/opencsg.h)
      message(STATUS "GLEW: Using target GLEW::GLEW")
      foreach(library ${openscadEXECUTABLE} openscadGUI)
          target_include_directories(${library} SYSTEM PRIVATE "${OPENCSG_INCLUDE_DIRS}/opencsg")
          target_link_libraries(${library} PRIVATE GLEW::GLEW)
      endforeach(library)
    else()
      find_package(GLEW REQUIRED)
      find_path(OPENCSG_INCLUDE_DIRS opencsg.h)
      message(STATUS "GLEW_INCLUDE_DIR: ${GLEW_INCLUDE_DIR}")
      message(STATUS "GLEW_LIBRARY: ${GLEW_LIBRARY}")
      foreach(library ${openscadEXECUTABLE} openscadGUI)
         target_include_directories(${library} SYSTEM PRIVATE ${GLEW_INCLUDE_DIR} ${OPENCSG_INCLUDE_DIRS})
         target_link_libraries(${library} PRIVATE ${GLEW_LIBRARY})
      endforeach(library)
    endif()

    find_package(OpenGL REQUIRED QUIET)
    foreach(library ${openscadEXECUTABLE} openscadGUI)
       target_link_libraries(${library} PRIVATE ${OPENGL_LIBRARIES})
    endforeach(library)
    message(STATUS "OpenGL: ${OPENGL_LIBRARIES}")
  endif()
endmacro(find_graphics)

if (MSVC)
  # Flex lexer options
  set(WINCOMPAT "--wincompat --nounistd")

  target_compile_definitions(${openscadEXECUTABLE} PRIVATE _USE_MATH_DEFINES)

  find_package(Eigen3 CONFIG REQUIRED)
  target_link_libraries(${openscadEXECUTABLE} PRIVATE Eigen3::Eigen)

  set(Boost_USE_STATIC_LIBS TRUE)
  find_package(Boost 1.56 REQUIRED COMPONENTS filesystem system regex program_options)
  message(STATUS "Boost: ${Boost_VERSION}")
  target_include_directories(${openscadEXECUTABLE} SYSTEM PRIVATE ${Boost_INCLUDE_DIRS})
  target_link_libraries(${openscadEXECUTABLE} PRIVATE ${Boost_LIBRARIES})

  find_package(harfbuzz CONFIG REQUIRED)
  find_path(HARFBUZZ_INCLUDE_DIRS harfbuzz)
  target_include_directories(${openscadEXECUTABLE} SYSTEM PRIVATE ${HARFBUZZ_INCLUDE_DIRS}/harfbuzz)
  target_link_libraries(${openscadEXECUTABLE} PRIVATE harfbuzz::harfbuzz)

  find_package(unofficial-fontconfig CONFIG REQUIRED)
  target_link_libraries(${openscadEXECUTABLE} PRIVATE unofficial::fontconfig::fontconfig)

  find_package(unofficial-glib CONFIG REQUIRED)
  foreach(library ${openscadALLTARGETS})
     target_include_directories(${library} SYSTEM PRIVATE ${GLIB2_INCLUDE_DIRS})
     target_link_libraries(${library} PRIVATE unofficial::glib::gio unofficial::glib::glib unofficial::glib::gmodule unofficial::glib::gobject)
  endforeach(library)
  find_package(double-conversion CONFIG REQUIRED)
  target_link_libraries(${openscadEXECUTABLE} PRIVATE double-conversion::double-conversion)

  find_library(GETTEXT_LIBRARY libintl)
  target_link_libraries(${openscadEXECUTABLE} PRIVATE ${GETTEXT_LIBRARY})

  # call before setting local CMAKE_MODULE_PATH so we use VCPKG version of FindGLEW
  find_graphics()

  # needed for Qt5QScintilla, maybe others
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
else()
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
  if(NOT "$ENV{OPENSCAD_LIBRARIES}" STREQUAL "")
    list(APPEND CMAKE_PREFIX_PATH "$ENV{OPENSCAD_LIBRARIES}")
  endif()

  find_package(Eigen3 REQUIRED)
  find_package(Boost 1.56 REQUIRED COMPONENTS filesystem system regex program_options)
  message(STATUS "Boost: ${Boost_VERSION}")
  find_package(HarfBuzz 0.9.19 REQUIRED QUIET)
  message(STATUS "Harfbuzz: ${HARFBUZZ_VERSION}")
  find_package(FontConfig 2.8.0 REQUIRED QUIET)
  message(STATUS "Fontconfig: ${FONTCONFIG_VERSION}")
  find_package(GLIB2 2.26 REQUIRED QUIET)
  message(STATUS "Glib: ${GLIB2_VERSION}")
  find_package(DoubleConversion REQUIRED QUIET)

foreach(library ${openscadALLTARGETS})
   target_include_directories(${library} SYSTEM PRIVATE ${EIGEN3_INCLUDE_DIR})
   target_compile_definitions(${library} PRIVATE EIGEN_DONT_ALIGN)
   target_include_directories(${library} SYSTEM PRIVATE ${Boost_INCLUDE_DIRS})
   target_link_libraries(${library} PRIVATE ${Boost_LIBRARIES})
   target_include_directories(${library} SYSTEM PRIVATE ${HARFBUZZ_INCLUDE_DIRS})
   target_link_libraries(${library} PRIVATE ${HARFBUZZ_LIBRARIES})
   target_link_libraries(${library} PRIVATE ${FONTCONFIG_LIBRARIES})
   target_include_directories(${library} SYSTEM PRIVATE ${GLIB2_INCLUDE_DIRS})
   target_link_libraries(${library} PRIVATE ${GLIB2_LIBRARIES})
   target_link_libraries(${library} PRIVATE ${DoubleConversion_LIBRARIES})
endforeach(library)
  find_graphics()
endif()

if (("${Boost_VERSION}" VERSION_GREATER "1.72") AND ("${Boost_VERSION}" VERSION_LESS "1.76"))
  # Avoid warning messages from boost which are also caused by boost's code.
  #   https://github.com/boostorg/property_tree/issues/51
foreach(library ${openscadALLTARGETS})
  target_compile_definitions(${library} PUBLIC BOOST_BIND_GLOBAL_PLACEHOLDERS)
endforeach(library)
endif()

# Note: Saving CMAKE_MODULE_PATH as CGAL will overwrite it.
# Reconsider this after CGAL 5.4: https://github.com/CGAL/cgal/pull/6029
set(ORIGINAL_CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})
set(CGAL_DO_NOT_WARN_ABOUT_CMAKE_BUILD_TYPE TRUE)
# Some older versions do not match with CGAL 5.0 REQUIRED, so we check after.
find_package(CGAL REQUIRED COMPONENTS Core)
if (${CGAL_MAJOR_VERSION} LESS 5)
  message(FATAL_ERROR "CGAL: ${CGAL_MAJOR_VERSION}.${CGAL_MINOR_VERSION} less than required minimum version 5.0")
endif()
message(STATUS "CGAL: ${CGAL_MAJOR_VERSION}.${CGAL_MINOR_VERSION}")
foreach(library ${openscadALLTARGETS})
   target_compile_definitions(${library} PRIVATE ENABLE_CGAL)
# The macro `CGAL_DEBUG` allows to force CGAL assertions, even if `NDEBUG` is defined,
# Enabling CGAL assertions is necessary for us to catch them before they cause a crash
# on bad geometry input.
   target_compile_definitions(${library} PRIVATE CGAL_DEBUG)
   target_compile_definitions(${library} PRIVATE CGAL_USE_GMPXX)
   if(TARGET CGAL::CGAL)
     target_link_libraries(${library}  PRIVATE CGAL::CGAL CGAL::CGAL_Core)
     message(STATUS "CGAL: Using target CGAL::CGAL CGAL::CGAL_Core")
   else()
     target_link_libraries(${library}  PRIVATE ${CGAL_LIBRARY} ${GMP_LIBRARIES} ${MPFR_LIBRARIES})
   endif()
endforeach(library)
# revert any changes to module path from CGAL_Macros.cmake; see note above
# Commented out code should work in CGAL>= 5.4
#if (CGAL_MODULE_PATH_IS_SET)
#  set(CMAKE_MODULE_PATH ${ORIGINAL_CMAKE_MODULE_PATH})
#endif()
set(CMAKE_MODULE_PATH ${ORIGINAL_CMAKE_MODULE_PATH})

find_package(LibZip REQUIRED QUIET)
message(STATUS "libzip: ${LIBZIP_VERSION}")
target_include_directories(${openscadEXECUTABLE} SYSTEM PRIVATE ${LIBZIP_INCLUDE_DIR_ZIP} ${LIBZIP_INCLUDE_DIR_ZIPCONF})
target_link_libraries(${openscadEXECUTABLE} PRIVATE ${LIBZIP_LIBRARY})
target_compile_definitions(${openscadEXECUTABLE} PRIVATE ENABLE_LIBZIP)

find_package(Freetype 2.4.9 REQUIRED QUIET)
message(STATUS "Freetype: ${FREETYPE_VERSION_STRING}")
find_package(LibXml2 2.9 REQUIRED QUIET)
message(STATUS "LibXml2: ${LIBXML2_VERSION_STRING}")
foreach(library ${openscadALLTARGETS})
   target_include_directories(${library} SYSTEM PRIVATE ${FREETYPE_INCLUDE_DIRS})
   target_link_libraries(${library} PRIVATE ${FREETYPE_LIBRARIES})
   target_include_directories(${library} SYSTEM PRIVATE ${LIBXML2_INCLUDE_DIR})
   target_link_libraries(${library} PRIVATE ${LIBXML2_LIBRARIES})
endforeach(library)

if(ENABLE_HIDAPI)
  find_package(HidAPI 0.10 QUIET)
  if(HIDAPI_FOUND)
    set(INPUT_DRIVER_HIDAPI_SOURCES src/gui/input/HidApiInputDriver.cc)
    target_include_directories(${openscadEXECUTABLE} SYSTEM PRIVATE ${HIDAPI_INCLUDE_DIR})
    target_link_libraries(${openscadEXECUTABLE} PRIVATE ${HIDAPI_LIBRARY})
    target_compile_definitions(${openscadEXECUTABLE} PRIVATE ENABLE_HIDAPI)
    message(STATUS "HidAPI: ${HIDAPI_VERSION_STRING}")
  elseif(ALLOW_BUNDLED_HIDAPI)
    set(HIDAPI_SRC_DIR "src/ext/hidapi")
    file(STRINGS "${HIDAPI_SRC_DIR}/VERSION" HIDAPI_VERSION_STRING)
    target_include_directories(${openscadEXECUTABLE} SYSTEM PRIVATE "${HIDAPI_SRC_DIR}")
    set(INPUT_DRIVER_HIDAPI_SOURCES "${HIDAPI_SRC_DIR}/hid.c" src/gui/input/HidApiInputDriver.cc)
    target_compile_definitions(${openscadEXECUTABLE} PRIVATE ENABLE_HIDAPI)
    target_link_libraries(${openscadEXECUTABLE} PRIVATE setupapi hid)
    message(STATUS "HidAPI: ${HIDAPI_VERSION_STRING} (bundled)")
  else()
    message(STATUS "HIDAPI: disabled")
  endif()
else()
  message(STATUS "HIDAPI: disabled per user request")
endif()

if(ENABLE_SPNAV)
  find_package(SpNav QUIET)
  if(SPNAV_FOUND)
    message(STATUS "SpNav: found")
    set(INPUT_DRIVER_SPNAV_SOURCES src/gui/input/SpaceNavInputDriver.cc)
    target_include_directories(${openscadEXECUTABLE} SYSTEM PRIVATE ${SPNAV_INCLUDE_DIR})
    target_link_libraries(${openscadEXECUTABLE} PRIVATE ${SPNAV_LIBRARY})
    target_compile_definitions(${openscadEXECUTABLE} PRIVATE ENABLE_SPNAV)
  else()
    message(STATUS "SpNav: disabled")
  endif()
endif()

if(ENABLE_CAIRO)
  find_package(Cairo 1.14 QUIET)
  if (CAIRO_VERSION)
    message(STATUS "Cairo: ${CAIRO_VERSION}")
    foreach(library ${openscadEXECUTABLE} openscadGUI)
       target_include_directories(${library} SYSTEM PRIVATE ${CAIRO_INCLUDE_DIRS})
       target_link_libraries(${library} PRIVATE ${CAIRO_LIBRARIES})
       target_compile_definitions(${library} PRIVATE ENABLE_CAIRO)
    endforeach(library)
  else()
    message(STATUS "Cairo: disabled")
  endif()
else()
  message(STATUS "Cairo: disabled per user request")
endif()

find_package(FLEX REQUIRED QUIET)
message(STATUS "Flex: ${FLEX_VERSION}")

find_package(BISON REQUIRED QUIET)
message(STATUS "Bison: ${BISON_VERSION}")

if(NOT MSVC)
  find_package(Lib3MF QUIET)
  if (LIB3MF_FOUND)
    message(STATUS "lib3mf: ${LIB3MF_VERSION}")
    target_compile_definitions(${openscadEXECUTABLE} PRIVATE ${LIB3MF_DEFINITIONS})
    target_include_directories(${openscadEXECUTABLE} SYSTEM PRIVATE ${LIB3MF_INCLUDE_DIRS})
    target_link_libraries(${openscadEXECUTABLE} PRIVATE ${LIB3MF_LIBRARIES})
  else()
    message(STATUS "lib3mf: disabled")
  endif()
endif()

# Automatically add the current source and build directories to the include path.
set(CMAKE_INCLUDE_CURRENT_DIR ON) # (does not propagate down to subdirectories)

target_include_directories(openscadCGAL PRIVATE
  "src"
  "src/core"
  "src/core/customizer"
  "src/geometry"
  "src/geometry/cgal"
  "src/platform"
  "src/io"
  "src/utils"
  "src/glview"
  "src/glview/cgal"
)

target_include_directories(openscadGUI PRIVATE
  "src"
  "src/core"
  "src/core/customizer"
  "src/ext/json"
  "src/ext/lexertl/include"
  "src/ext/libtess2/Include"
  "src/geometry"
  "src/geometry/cgal"
  "src/glview"
  "src/glview/preview"
  "src/glview/cgal"
  "src/gui"
  "src/io"
  "src/platform"
  "src/utils"
)


target_include_directories(${openscadEXECUTABLE} PRIVATE
  "src"
  "src/core"
  "src/core/customizer"
  "src/ext/json"
  "src/ext/lexertl/include"
  "src/ext/libtess2/Include"
  "src/geometry"
  "src/geometry/cgal"
  "src/glview"
  "src/glview/preview"
  "src/glview/cgal"
  "src/gui"
  "src/io"
  "src/platform"
  "src/utils"
)

FLEX_TARGET(openscad_lexer src/core/lexer.l ${OPENSCAD_LIB_OUTPUT_DIR}/lexer.cxx DEFINES_FILE ${OPENSCAD_LIB_OUTPUT_DIR}/lexer.hxx COMPILE_FLAGS ${WINCOMPAT})
BISON_TARGET(openscad_parser src/core/parser.y ${OPENSCAD_LIB_OUTPUT_DIR}/parser.cxx DEFINES_FILE ${OPENSCAD_LIB_OUTPUT_DIR}/parser.hxx COMPILE_FLAGS "-d -p parser")
ADD_FLEX_BISON_DEPENDENCY(openscad_lexer openscad_parser)

FLEX_TARGET(comment_lexer src/core/customizer/comment_lexer.l ${OPENSCAD_LIB_OUTPUT_DIR}/comment_lexer.cxx DEFINES_FILE ${OPENSCAD_LIB_OUTPUT_DIR}/comment_lexer.hxx COMPILE_FLAGS ${WINCOMPAT})
BISON_TARGET(comment_parser src/core/customizer/comment_parser.y ${OPENSCAD_LIB_OUTPUT_DIR}/comment_parser.cxx DEFINES_FILE ${OPENSCAD_LIB_OUTPUT_DIR}/comment_parser.hxx COMPILE_FLAGS "-d -p comment_parser")
ADD_FLEX_BISON_DEPENDENCY(comment_lexer comment_parser)

if(NOT HEADLESS)
  if (APPLE AND EXISTS /usr/local/opt/qt)
    list(APPEND CMAKE_PREFIX_PATH "/usr/local/opt/qt")
  endif()
  if (APPLE AND EXISTS /usr/local/opt/qt@5)
    list(APPEND CMAKE_PREFIX_PATH "/usr/local/opt/qt@5")
  endif()
  if (APPLE AND EXISTS /opt/homebrew/opt/qt@5)
    list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/opt/qt@5")
  endif()

  find_package(Qt5 5.4 COMPONENTS Core Widgets Multimedia OpenGL Concurrent Network Svg REQUIRED QUIET)
  message(STATUS "Qt5: ${Qt5_VERSION}")

foreach(library ${openscadEXECUTABLE} )
  if("${CMAKE_VERSION}" VERSION_GREATER_EQUAL "3.14")
    set_property(TARGET ${library} PROPERTY AUTOGEN_ORIGIN_DEPENDS OFF)
  endif()
  set_property(TARGET ${library} PROPERTY AUTOMOC ON)
  set_property(TARGET ${library} PROPERTY AUTOUIC ON)
  set_property(TARGET ${library} PROPERTY AUTORCC ON)
  set_property(TARGET ${library} PROPERTY AUTOUIC_OPTIONS --tr q_)
  set_property(TARGET ${library} PROPERTY AUTORCC_OPTIONS --threshold 10 --compress 9)

  if (Qt5_POSITION_INDEPENDENT_CODE)
    set_property(TARGET ${library} PROPERTY POSITION_INDEPENDENT_CODE ON)
  endif()

  if (("${Qt5_VERSION}" VERSION_GREATER_EQUAL "5.4"))
    target_compile_definitions(${library} PRIVATE USE_QOPENGLWIDGET)
  endif()
endforeach(library)
  find_package(Qt5QScintilla 2.8.0 REQUIRED QUIET)
  message(STATUS "QScintilla: ${QT5QSCINTILLA_VERSION_STRING}")

  if(ENABLE_QTDBUS)
    find_package(Qt5DBus QUIET)
    if (Qt5DBus_FOUND)
      message(STATUS "DBus input driver enabled")
foreach(library ${openscadEXECUTABLE} openscadGUI)
      target_compile_definitions(${library} PRIVATE ENABLE_DBUS)
endforeach(library)
      set(INPUT_DRIVER_DBUS_SOURCES src/gui/input/DBusInputDriver.cc)
      qt5_add_dbus_interface(INPUT_DRIVER_DBUS_SOURCES org.openscad.OpenSCAD.xml openscad_interface)
      qt5_add_dbus_adaptor(INPUT_DRIVER_DBUS_SOURCES org.openscad.OpenSCAD.xml gui/input/DBusInputDriver.h DBusInputDriver openscad_adaptor)
    else()
      message(STATUS "DBus input driver disabled as the QtDBus module could not be found.")
    endif()
  else()
    message(STATUS "DBus input driver disabled per user request.")
  endif()

  if(ENABLE_GAMEPAD)
    find_package(Qt5Gamepad QUIET)
    if (Qt5Gamepad_FOUND)
      message(STATUS "Qt5Gamepad input driver enabled")
      set(GUI_SOURCES ${GUI_SOURCES} src/gui/input/QGamepadInputDriver.cc)
      target_compile_definitions(${openscadEXECUTABLE} PRIVATE ENABLE_QGAMEPAD)
    else()
      message(STATUS "Qt5Gamepad input driver disabled as the Qt5Gamepad module could not be found.")
    endif()
  else()
    message(STATUS "Qt5Gamepad input driver disabled per user request.")
  endif()

endif()

# Setup ccache (if available) to speed up recompiles. It's especially useful
# when switching back and forth between branches where large numbers of files
# would otherwise need to be re-compiled each time.
if(USE_CCACHE)
  find_program(CCACHE_PATH ccache)
  if (CCACHE_PATH)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ${CCACHE_PATH})
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ${CCACHE_PATH})
  endif()
endif()

target_compile_definitions(${openscadEXECUTABLE} PRIVATE OPENSCAD_VERSION=${OPENSCAD_VERSION} OPENSCAD_SHORTVERSION=${OPENSCAD_SHORTVERSION} OPENSCAD_YEAR=${OPENSCAD_YEAR} OPENSCAD_MONTH=${OPENSCAD_MONTH})
if (DEFINED OPENSCAD_DAY)
  target_compile_definitions(${openscadEXECUTABLE} PRIVATE OPENSCAD_DAY=${OPENSCAD_DAY})
endif()
if(DEFINED OPENSCAD_COMMIT)
  target_compile_definitions(${openscadEXECUTABLE} PRIVATE OPENSCAD_COMMIT=${OPENSCAD_COMMIT})
endif()

#
# Platform specific settings
#

# Stack size 8MB; github issue 116
target_compile_definitions(${openscadEXECUTABLE} PRIVATE "STACKSIZE=${STACKSIZE}") # used as default in src/platform/PlatformUtils.h

if(NULLGL)
  set(OFFSCREEN_METHOD "NULLGL")
  message(STATUS "Offscreen OpenGL Context - using NULLGL")
else()
  SET(OFFSCREEN_METHOD "<undefined>")
endif()
if(APPLE)
  foreach(library ${openscadEXECUTABLE} openscadGUI)
     target_compile_definitions(${library} PRIVATE OPENSCAD_OS="Mac OS X")
  endforeach(library)
  message(STATUS "Offscreen OpenGL Context - using Apple CGL")
  set(PLATFORM_SOURCES src/platform/PlatformUtils-mac.mm src/io/imageutils-macosx.cc src/platform/CocoaUtils.mm)
  if(NOT HEADLESS)
    set(PLATFORM_SOURCES ${PLATFORM_SOURCES} src/gui/AppleEvents.cc)
  endif()
  if(NOT NULLGL)
    target_compile_definitions(${openscadEXECUTABLE} PRIVATE GL_SILENCE_DEPRECATION)
    set(OFFSCREEN_METHOD "Apple CGL")
    set(PLATFORM_SOURCES ${PLATFORM_SOURCES} src/glview/OffscreenContextCGL.mm)
  endif()
  find_library(COCOA_LIBRARY Cocoa)
  target_link_libraries(${openscadEXECUTABLE} PRIVATE ${COCOA_LIBRARY})
elseif(UNIX)
  foreach(library ${openscadEXECUTABLE} openscadGUI)
     target_compile_definitions(${openscadEXECUTABLE} PRIVATE OPENSCAD_OS="Unix")
  endforeach(library)
  set(PLATFORM_SOURCES src/io/imageutils-lodepng.cc src/platform/PlatformUtils-posix.cc)
  if(NOT NULLGL)
    if(ENABLE_EGL AND OpenGL_EGL_FOUND)
      set(OFFSCREEN_METHOD "Unix EGL")
      message(STATUS "Offscreen OpenGL Context - using Unix EGL")
      set(PLATFORM_SOURCES ${PLATFORM_SOURCES} src/glview/OffscreenContextEGL.cc)
       foreach(library ${openscadEXECUTABLE} openscadGUI)
            target_compile_definitions(${library} PRIVATE GLEW_EGL)
            target_link_libraries(${library} PRIVATE OpenGL::EGL)
       endforeach(library)
    else()
      set(OFFSCREEN_METHOD "Unix GLX on X11")
      message(STATUS "Offscreen OpenGL Context - using Unix GLX on X11")
      set(PLATFORM_SOURCES ${PLATFORM_SOURCES} src/glview/OffscreenContextGLX.cc)
      find_library(X11_LIBRARY X11)
      foreach(library ${openscadEXECUTABLE} openscadGUI)
         target_link_libraries(${library} PRIVATE ${X11_LIBRARY})
      endforeach(library)
    endif()
  endif()
elseif(WIN32)
  foreach(library ${openscadEXECUTABLE} openscadGUI)
     target_compile_definitions(${library} PRIVATE NOGDI)
     target_compile_definitions(${library} PRIVATE OPENSCAD_OS="Windows")
  endforeach(library)
  message(STATUS "Offscreen OpenGL Context - using Microsoft WGL")
  set(PLATFORM_SOURCES src/io/imageutils-lodepng.cc src/platform/PlatformUtils-win.cc)
  if(NOT NULLGL)
    set(OFFSCREEN_METHOD "Windows WGL")
    message(STATUS "Offscreen OpenGL Context - using Microsoft WGL")
    set(PLATFORM_SOURCES ${PLATFORM_SOURCES} src/glview/OffscreenContextWGL.cc)
  endif()
endif()

# NOTE: To keep HEADLESS builds working, do NOT add Qt-dependent sources here,
#       see GUI_SOURCES list below for that.
set(CORE_SOURCES
  src/Feature.cc
  src/FontCache.cc
  src/handle_dep.cc
  src/LibraryInfo.cc
  src/RenderStatistic.cc
  src/version.cc
  src/core/Arguments.cc
  src/core/Builtins.cc
  src/core/BuiltinContext.cc
  src/core/CgalAdvNode.cc
  src/core/Children.cc
  src/core/ColorNode.cc
  src/core/Context.cc
  src/core/ContextFrame.cc
  src/core/ContextMemoryManager.cc
  src/core/control.cc
  src/core/CSGNode.cc
  src/core/CsgOpNode.cc
  src/core/CSGTreeEvaluator.cc
  src/core/customizer/Annotation.cc
  src/core/customizer/CommentParser.cc
  src/core/EvaluationSession.cc
  src/core/Expression.cc
  src/core/ValueWrapper.cc
  src/core/builtin_functions.cc
  src/core/function.cc
  src/core/ImportNode.cc
  src/core/LinearExtrudeNode.cc
  src/core/LocalScope.cc
  src/core/ScopeContext.cc
  src/core/module.cc
  src/core/node.cc
  src/core/NodeDumper.cc
  src/core/OffsetNode.cc
  src/core/Parameters.cc
  src/core/parsersettings.cc
  src/core/primitives.cc
  src/core/progress.cc
  src/core/ProjectionNode.cc
  src/core/RenderNode.cc
  src/core/RoofNode.cc
  src/core/DrawingCallback.cc
  src/core/RotateExtrudeNode.cc
  src/core/SurfaceNode.cc
  src/core/TextNode.cc
  src/core/TransformNode.cc
  src/core/Value.cc
  src/core/Assignment.cc
  src/core/AST.cc
  src/core/FreetypeRenderer.cc
  src/core/GroupModule.cc
  src/core/ModuleLiteral.cc
  src/core/ModuleReference.cc
  src/core/ModuleInstantiation.cc
  src/core/MemberLookup.cc
  src/core/NodeVisitor.cc
  src/core/SourceFile.cc
  src/core/SourceFileCache.cc
  src/core/StatCache.cc
  src/core/UserModule.cc
  src/core/Tree.cc
  src/core/customizer/ParameterObject.cc
  src/core/customizer/ParameterSet.cc
  src/ext/lodepng/lodepng.cpp
  src/ext/polyclipping/clipper.cpp
  src/ext/libtess2/Source/bucketalloc.c
  src/ext/libtess2/Source/dict.c
  src/ext/libtess2/Source/geom.c
  src/ext/libtess2/Source/mesh.c
  src/ext/libtess2/Source/priorityq.c
  src/ext/libtess2/Source/sweep.c
  src/ext/libtess2/Source/tess.c
  src/geometry/ClipperUtils.cc
  src/geometry/Geometry.cc
  src/geometry/GeometryCache.cc
  src/geometry/GeometryUtils.cc
  src/geometry/IndexedMesh.cc
  src/geometry/Polygon2d.cc
  src/geometry/linalg.cc
  src/geometry/PolySet.cc
  src/geometry/PolySetUtils.cc
  src/geometry/roof_ss.cc
  src/geometry/roof_vd.cc
  src/glview/RenderSettings.cc
  src/glview/Camera.cc
  src/glview/ColorMap.cc
  src/glview/preview/CSGTreeNormalizer.cc
  src/io/DxfData.cc
  src/io/dxfdim.cc
  src/io/export.cc
  src/io/export_3mf.cc
  src/io/export_amf.cc
  src/io/export_dxf.cc
  src/io/export_obj.cc
  src/io/export_off.cc
  src/io/export_wrl.cc
  src/io/export_pdf.cc
  src/io/export_stl.cc
  src/io/export_svg.cc
  src/io/export_param.cc
  src/io/fileutils.cc
  src/io/import_3mf.cc
  src/io/import_amf.cc
  src/io/import_stl.cc
  src/io/import_off.cc
  src/io/import_svg.cc
  src/io/import_json.cc
  src/io/libsvg/circle.cc
  src/io/libsvg/data.cc
  src/io/libsvg/ellipse.cc
  src/io/libsvg/group.cc
  src/io/libsvg/libsvg.cc
  src/io/libsvg/line.cc
  src/io/libsvg/path.cc
  src/io/libsvg/polygon.cc
  src/io/libsvg/polyline.cc
  src/io/libsvg/rect.cc
  src/io/libsvg/shape.cc
  src/io/libsvg/svgpage.cc
  src/io/libsvg/text.cc
  src/io/libsvg/transformation.cc
  src/io/libsvg/tspan.cc
  src/io/libsvg/use.cc
  src/io/libsvg/util.cc
  src/platform/PlatformUtils.cc
  src/utils/boost-utils.cc
  src/utils/calc.cc
  src/utils/degree_trig.cc
  src/utils/hash.cc
  src/utils/printutils.cc
  src/utils/StackCheck.h
  src/utils/svg.cc
  src/utils/version_check.h
  ${PLATFORM_SOURCES}
  ${FLEX_openscad_lexer_OUTPUTS}
  ${BISON_openscad_parser_OUTPUTS}
  ${FLEX_comment_lexer_OUTPUTS}
  ${BISON_comment_parser_OUTPUTS})

#
# Offscreen OpenGL context source code
#
if(NULLGL)
  message(STATUS "NULLGL is set. Overriding OpenGL(TM) settings")
  set(OFFSCREEN_SOURCES
    src/glview/Renderer.cc
    src/glview/NULLGL.cc # contains several 'nullified' versions of above .cc files
    src/glview/OffscreenView.cc
    src/glview/OffscreenContextNULL.cc
    src/io/export_png.cc
    src/io/imageutils.cc
    ${OFFSCREEN_IMGUTILS_SOURCE})
else()
  set(OFFSCREEN_SOURCES
    src/glview/fbo.cc
    src/glview/Renderer.cc
    src/glview/system-gl.cc
    src/glview/VertexArray.cc
    src/glview/VBORenderer.cc
    src/glview/GLView.cc
    src/glview/OffscreenView.cc
    src/glview/cgal/CGALRenderer.cc
    src/glview/preview/OpenCSGRenderer.cc
    src/glview/preview/ThrownTogetherRenderer.cc
    src/io/export_png.cc
    src/io/imageutils.cc)
endif()


if(UNIX AND (NOT APPLE) AND (NOT HEADLESS))
  set(PLATFORM_INPUT_DRIVER_SOURCES src/gui/input/JoystickInputDriver.cc)
  foreach(library ${openscadEXECUTABLE} openscadGUI)
     target_compile_definitions(${library} PRIVATE ENABLE_JOYSTICK)
  endforeach(library)
endif()

set(INPUT_DRIVER_SOURCES
  ${PLATFORM_INPUT_DRIVER_SOURCES}
  ${INPUT_DRIVER_HIDAPI_SOURCES}
  ${INPUT_DRIVER_SPNAV_SOURCES}
  ${INPUT_DRIVER_DBUS_SOURCES})

set(GUI_SOURCES
  ${GUI_SOURCES}
  ${INPUT_DRIVER_SOURCES}
)

# header-only code
set(GUI_HEADERS
  src/gui/AboutDialog.h
  src/gui/Network.h
  src/gui/NetworkSignal.h
  )


if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  # Ignore specific warning on external lib
  set_source_files_properties("src/ext/polyclipping/clipper.cpp" PROPERTIES COMPILE_FLAGS "-Wno-class-memaccess")
endif()

list(APPEND Sources src/openscad.cc ${CORE_SOURCES} ${OFFSCREEN_SOURCES})

set(RESOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)
if(HEADLESS)
  target_compile_definitions(${openscadEXECUTABLE} PRIVATE OPENSCAD_NOGUI)
else()
  list(APPEND RESOURCE_FILES ${RESOURCE_DIR}/common.qrc)
  # list(APPEND Sources ${GUI_SOURCES} ${GUI_HEADERS})
  list(APPEND Sources ${GUI_SOURCES})
endif()

if (SNAPSHOT)
  target_compile_definitions(${openscadEXECUTABLE} PRIVATE OPENSCAD_SNAPSHOT)
  set(MACOSX_BUNDLE_ICON_FILE icon-nightly.icns)
  set(WINDOWS_RESOURCE_PATH ${RESOURCE_DIR}/openscad_win32-nightly.rc)
else()
  set(MACOSX_BUNDLE_ICON_FILE OpenSCAD.icns)
  set(WINDOWS_RESOURCE_PATH ${RESOURCE_DIR}/openscad_win32.rc)
endif()

if (APPLE)
  list(APPEND RESOURCE_FILES ${RESOURCE_DIR}/mac.qrc)
  list(APPEND RESOURCE_FILES ${RESOURCE_DIR}/icons/${MACOSX_BUNDLE_ICON_FILE})
elseif(WIN32)
  list(APPEND RESOURCE_FILES ${WINDOWS_RESOURCE_PATH})
endif()

if (SORT_BUILD)
  # Build the last modified sources first (to fail fast during development)
  execute_process(
    COMMAND ../scripts/sort_cmake_filelist.sh "${Sources}"
    OUTPUT_VARIABLE Sources)
endif()

target_sources(${openscadEXECUTABLE} PRIVATE ${Sources} ${RESOURCE_FILES})
find_program(SHELL_EXE NAMES sh bash $ENV{SHELL})
add_custom_command(TARGET ${openscadEXECUTABLE} POST_BUILD
    COMMAND "${SHELL_EXE}"
    ARGS "${CMAKE_CURRENT_LIST_DIR}/scripts/translation-make.sh" "${SUFFIX_WITH_DASH}"
    COMMENT "Compiling language files")

if(APPLE AND NOT APPLE_UNIX)
  set_target_properties(${openscadEXECUTABLE} PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Info.plist.in
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_ICON_FILE ${MACOSX_BUNDLE_ICON_FILE}
    MACOSX_BUNDLE_BUNDLE_VERSION ${OPENSCAD_YEAR}.${OPENSCAD_MONTH}.${OPENSCAD_DAY}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${OPENSCAD_YEAR}.${OPENSCAD_MONTH}
    RESOURCE "${RESOURCE_FILES}"
  )
  set(BUNDLE_RESOURCES_DIR ${CMAKE_CURRENT_BINARY_DIR}/OpenSCAD.app/Contents/Resources)
  file(COPY ${CMAKE_SOURCE_DIR}/color-schemes DESTINATION ${BUNDLE_RESOURCES_DIR})
  file(COPY ${CMAKE_SOURCE_DIR}/examples DESTINATION ${BUNDLE_RESOURCES_DIR})
  file(COPY ${CMAKE_SOURCE_DIR}/fonts DESTINATION ${BUNDLE_RESOURCES_DIR})
  file(COPY ${CMAKE_SOURCE_DIR}/libraries DESTINATION ${BUNDLE_RESOURCES_DIR})
  file(COPY ${CMAKE_SOURCE_DIR}/locale DESTINATION ${BUNDLE_RESOURCES_DIR})
  file(COPY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${BUNDLE_RESOURCES_DIR})
  file(COPY ${CMAKE_SOURCE_DIR}/templates DESTINATION ${BUNDLE_RESOURCES_DIR})
elseif(MINGW)
  set_target_properties(${openscadEXECUTABLE} PROPERTIES
    LINK_FLAGS "-Wl,--stack,${STACKSIZE}"
  )
elseif(MSVC)
  set_target_properties(${openscadEXECUTABLE} PROPERTIES
    LINK_FLAGS "-subsystem:windows -ENTRY:mainCRTStartup -stack:${STACKSIZE}"
  )
endif()

foreach(library ${openscadEXECUTABLE} openscadGUI)
   if(NOT HEADLESS)
        target_link_libraries(${library} PRIVATE
          Qt5::Core Qt5::Widgets Qt5::Multimedia Qt5::OpenGL Qt5::Concurrent Qt5::Network Qt5::Svg
          ${QT5QSCINTILLA_LIBRARY} ${Qt5DBus_LIBRARIES} ${Qt5Gamepad_LIBRARIES}
        )
   endif()
   if(MXECROSS)
     target_link_libraries(${library} PRIVATE Qt5::QSvgPlugin)
   endif()
endforeach(library)

# Configure icon-related files, for release vs nightly
configure_file(${CMAKE_CURRENT_LIST_DIR}/openscad.appdata.xml.in ${CMAKE_CURRENT_LIST_DIR}/openscad.appdata.xml.in2)
configure_file(${RESOURCE_DIR}/icons/openscad.desktop.in ${RESOURCE_DIR}/icons/openscad.desktop)
configure_file(${RESOURCE_DIR}/common.qrc.in ${RESOURCE_DIR}/common.qrc)
configure_file(${RESOURCE_DIR}/mac.qrc.in ${RESOURCE_DIR}/mac.qrc)

if(NOT APPLE OR APPLE_UNIX)
set_target_properties(${openscadEXECUTABLE} PROPERTIES OUTPUT_NAME openscad${SUFFIX_WITH_DASH})
install(TARGETS ${openscadEXECUTABLE} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${CMAKE_CURRENT_LIST_DIR}/doc/openscad.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 RENAME openscad${SUFFIX_WITH_DASH}.1)
install(FILES ${CMAKE_CURRENT_LIST_DIR}/openscad.appdata.xml DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/metainfo RENAME org.openscad.OpenSCAD${SUFFIX_WITH_DASH}.appdata.xml)
install(FILES ${RESOURCE_DIR}/icons/openscad.desktop DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications RENAME openscad${SUFFIX_WITH_DASH}.desktop)
install(FILES ${RESOURCE_DIR}/icons/openscad.xml DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/mime/packages RENAME openscad${SUFFIX_WITH_DASH}.xml)
install(FILES ${RESOURCE_DIR}/icons/openscad${SUFFIX_WITH_DASH}-48.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/48x48/apps RENAME openscad${SUFFIX_WITH_DASH}.png)
install(FILES ${RESOURCE_DIR}/icons/openscad${SUFFIX_WITH_DASH}-64.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/64x64/apps RENAME openscad${SUFFIX_WITH_DASH}.png)
install(FILES ${RESOURCE_DIR}/icons/openscad${SUFFIX_WITH_DASH}-128.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/128x128/apps RENAME openscad${SUFFIX_WITH_DASH}.png)
install(FILES ${RESOURCE_DIR}/icons/openscad${SUFFIX_WITH_DASH}-256.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/256x256/apps RENAME openscad${SUFFIX_WITH_DASH}.png)
install(FILES ${RESOURCE_DIR}/icons/openscad${SUFFIX_WITH_DASH}-512.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/512x512/apps RENAME openscad${SUFFIX_WITH_DASH}.png)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/color-schemes DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/openscad${SUFFIX_WITH_DASH})
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/examples DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/openscad${SUFFIX_WITH_DASH})
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/fonts DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/openscad${SUFFIX_WITH_DASH} PATTERN ".uuid" EXCLUDE)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/libraries DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/openscad${SUFFIX_WITH_DASH} PATTERN ".git*" EXCLUDE)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/locale DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/openscad${SUFFIX_WITH_DASH} FILES_MATCHING PATTERN "*/LC_MESSAGES/openscad.mo")
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/shaders DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/openscad${SUFFIX_WITH_DASH})
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/templates DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/openscad${SUFFIX_WITH_DASH} FILES_MATCHING PATTERN "*.json")
endif()

if(INFO)
  include(info)
endif()

if(ENABLE_TESTS)
  add_subdirectory(tests)
endif()

if(OFFLINE_DOCS)
  add_subdirectory(resources)
endif()
