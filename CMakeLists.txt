################################################################################
cmake_minimum_required(VERSION 3.4.3)

################################################################################
## CEF OpenGL Win main project/solution
project(cef_opengl_win)

################################################################################
## generics

# ensure location of CEF files is set at command line
IF("${CEF_BUILD_DIR}" STREQUAL "" )
    MESSAGE(FATAL_ERROR "CEF_BUILD_DIR not set")
endif()

# set other dirs based off what was passsed in
set(CEF_INCLUDE_DIR "${CEF_BUILD_DIR}/include")
set(CEF_LIB_DIR "${CEF_BUILD_DIR}")
set(CEF_LIB_DLL_WRAPPER_DIR "${CEF_BUILD_DIR}/build/libcef_dll_wrapper")
set(CEF_RESOURCES_DIR "${CEF_BUILD_DIR}/Resources")

# location of CEF libraries we link against
find_library(
    CEF_LIBRARY_DEBUG
    NAMES libcef.lib
    PATHS ${CEF_LIB_DIR}
    PATH_SUFFIXES Debug
)
find_library(
    CEF_DLL_LIBRARY_DEBUG
    NAMES libcef_dll_wrapper.lib
    PATHS ${CEF_LIB_DLL_WRAPPER_DIR}
    PATH_SUFFIXES Debug
)
find_library(
    CEF_LIBRARY_RELEASE
    NAMES libcef.lib
    PATHS ${CEF_LIB_DIR}
    PATH_SUFFIXES Release
)
find_library(
    CEF_DLL_LIBRARY_RELEASE
    NAMES libcef_dll_wrapper.lib
    PATHS ${CEF_LIB_DLL_WRAPPER_DIR}
    PATH_SUFFIXES Release
)

set(CEF_LIBRARY
    debug ${CEF_LIBRARY_DEBUG}
    optimized ${CEF_LIBRARY_RELEASE}
)
set(CEF_DLL_LIBRARY
    debug ${CEF_DLL_LIBRARY_DEBUG}
    optimized ${CEF_DLL_LIBRARY_RELEASE}
)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -W3 /MTd")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -W3 /MT")

################################################################################
## cef_opengl_win executable

# add source file to the application
add_executable(
    cef_opengl_win
    src/cef_opengl_win.cpp
)

# define which include directories to pull in
target_include_directories(
    cef_opengl_win
    PUBLIC
    ${CEF_INCLUDE_DIR}
    ${CEF_INCLUDE_DIR}/..
)

# define which libs to link against
target_link_libraries(
    cef_opengl_win
    ${CEF_LIBRARY}
    ${CEF_DLL_LIBRARY}
    OpenGL32
)

################################################################################
# we are building Windows executable, not a console app (default)
set_target_properties(cef_opengl_win PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS")
set_target_properties(cef_opengl_win PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")

################################################################################
# Visual Studio batch commands to copy CEF runtime files along side executable
add_custom_command(
    TARGET cef_opengl_win POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
            "${CEF_BUILD_DIR}/$<CONFIGURATION>"
            "$<TARGET_FILE_DIR:cef_opengl_win>"
    COMMENT "Copying runtime files to executable directory")

add_custom_command(
    TARGET cef_opengl_win POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
            "${CEF_RESOURCES_DIR}"
            "$<TARGET_FILE_DIR:cef_opengl_win>"
    COMMENT "Copying resource files to executable directory")

# set the test application as the default startup project in Visual Studio
if("${CMAKE_VERSION}" VERSION_GREATER 3.6.2)
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT "cef_opengl_win")
endif()

################################################################################
# debug version needs a manifest file that allows retrieval of the Windows
# version number - otherwise the app in Debug configurations fails to start
add_custom_command(
    TARGET cef_opengl_win POST_BUILD
    COMMAND "mt.exe" -manifest \"$(ProjectDir)..\\src\\compatibility.manifest\" -outputresource:"$(TargetDir)$(TargetFileName)"\;\#1
    COMMENT "Adding custom manifest...")

################################################################################
# generic commands that have to go after everything else
if(CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
endif()
