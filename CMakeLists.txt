# LoopAuditioneer software
# Copyright (C) 2024 Lars Palo 
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.10)

# Make sure that the version file will be considered by CMake
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/version.txt")

# read PROGRAM_VERSION from file
if(EXISTS "${CMAKE_SOURCE_DIR}/version.txt")
  file(READ "${CMAKE_SOURCE_DIR}/version.txt" PROGRAM_VERSION)
  string(STRIP "${PROGRAM_VERSION}" PROGRAM_VERSION)
else()
  message(FATAL_ERROR "File ${CMAKE_SOURCE_DIR}/version.txt not found")
endif()

# Set the project name
project(
  LoopAuditioneer
  VERSION "${PROGRAM_VERSION}"
  DESCRIPTION "LoopAuditioneer - Software for loop and cue handling in .wav files"
  LANGUAGES CXX C
)

option(RTAUDIO_USE_JACK "Enable RtAudio support for Jack (Rt and PortAudio)" ON)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set output locations
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(RESOURCE_INSTALL_DIR "share/${PROJECT_NAME}")
set(RESOURCE_DIR "${CMAKE_BINARY_DIR}/${RESOURCE_INSTALL_DIR}")

# Get wxWidgets
if(CMAKE_CROSSCOMPILING AND WIN32)
  set(wxWidgets_USE_UNICODE ON)
  set(wxWidgets_USE_STATIC ON)
endif()
find_package(wxWidgets REQUIRED html net adv core base)

# Get PkgConfig used for external libraries
find_package(PkgConfig REQUIRED)

if(UNIX AND NOT APPLE)
  if(${RTAUDIO_USE_JACK})
    pkg_check_modules(JACK REQUIRED IMPORTED_TARGET jack)
  endif()
  pkg_check_modules(ALSA REQUIRED IMPORTED_TARGET alsa)
  pkg_check_modules(PA_SIMPLE REQUIRED IMPORTED_TARGET libpulse-simple)
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

if(NOT EXISTS "${CMAKE_BINARY_DIR}/tmp")
  execute_process(COMMAND mkdir tmp WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

if(NOT EXISTS "${CMAKE_BINARY_DIR}/include")
  execute_process(COMMAND mkdir include WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
  execute_process(COMMAND mkdir rtaudio WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/include)
endif()

if(EXISTS "${CMAKE_BINARY_DIR}/lib/libsndfile.a")
  add_library(LA_sndfile STATIC IMPORTED GLOBAL)
  set_target_properties(LA_sndfile PROPERTIES IMPORTED_LOCATION "${CMAKE_BINARY_DIR}/lib/libsndfile.a")
  # (This actually doesn't work on cmake 3.10) target_include_directories(LA_sndfile INTERFACE ${CMAKE_BINARY_DIR}/include)
  set_target_properties(LA_sndfile PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_BINARY_DIR}/include)
else()
  if(CMAKE_CROSSCOMPILING AND WIN32)
    execute_process(COMMAND autoreconf -vif WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/submodules/libsndfile)
    execute_process(COMMAND ${CMAKE_SOURCE_DIR}/scripts/conf-libsndfile-cross.sh ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tmp)
  else()
    execute_process(COMMAND autoreconf -vif WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/submodules/libsndfile)
    execute_process(COMMAND ${CMAKE_SOURCE_DIR}/submodules/libsndfile/configure --prefix=${CMAKE_BINARY_DIR} --enable-static --disable-external-libs --disable-full-suite --disable-sqlite --disable-alsa WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tmp)
  endif()
  execute_process(COMMAND make WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tmp)
  execute_process(COMMAND make install WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tmp)
  execute_process(COMMAND rm -rf WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tmp)
  add_library(LA_sndfile STATIC IMPORTED GLOBAL)
  set_target_properties(LA_sndfile PROPERTIES IMPORTED_LOCATION "${CMAKE_BINARY_DIR}/lib/libsndfile.a")
  # (This actually doesn't work on cmake 3.10) target_include_directories(LA_sndfile INTERFACE ${CMAKE_BINARY_DIR}/include)
  set_target_properties(LA_sndfile PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_BINARY_DIR}/include)
endif()

if(EXISTS "${CMAKE_BINARY_DIR}/lib/librtaudio.a")
  add_library(LA_rtaudio STATIC IMPORTED GLOBAL)
  set_target_properties(LA_rtaudio PROPERTIES IMPORTED_LOCATION "${CMAKE_BINARY_DIR}/lib/librtaudio.a")
  # (This actually doesn't work on cmake 3.10) target_include_directories(LA_rtaudio INTERFACE ${CMAKE_BINARY_DIR}/include/rtaudio)
  set_target_properties(LA_rtaudio PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_BINARY_DIR}/include/rtaudio)
else()
  if (NOT EXISTS "${CMAKE_SOURCE_DIR}/submodules/rtaudio/configure")
    execute_process(COMMAND ./autogen.sh --no-configure WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/submodules/rtaudio)
  endif()
  if(CMAKE_CROSSCOMPILING AND WIN32)
    execute_process(COMMAND ${CMAKE_SOURCE_DIR}/submodules/rtaudio/configure --host=x86_64-w64-mingw32 --prefix=${CMAKE_BINARY_DIR} --enable-static --disable-shared --with-dsound --with-wasapi WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tmp)
  else()
    if(${RTAUDIO_USE_JACK})
      execute_process(COMMAND ${CMAKE_SOURCE_DIR}/submodules/rtaudio/configure --prefix=${CMAKE_BINARY_DIR} --with-pulse --with-jack --with-alsa WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tmp)
    else()
      execute_process(COMMAND ${CMAKE_SOURCE_DIR}/submodules/rtaudio/configure --prefix=${CMAKE_BINARY_DIR} --with-pulse --with-alsa WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tmp)
    endif()
  endif()
  execute_process(COMMAND make WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tmp)
#  if(CMAKE_CROSSCOMPILING AND WIN32)
#    execute_process(COMMAND x86_64-w64-mingw32-ranlib .libs/librtaudio.a WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tmp)
#  endif()
  execute_process(COMMAND make install WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tmp)
  execute_process(COMMAND rm -rf WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tmp)
  add_library(LA_rtaudio STATIC IMPORTED GLOBAL)
  set_target_properties(LA_rtaudio PROPERTIES IMPORTED_LOCATION "${CMAKE_BINARY_DIR}/lib/librtaudio.a")
  # (This actually doesn't work on cmake 3.10) target_include_directories(LA_rtaudio INTERFACE ${CMAKE_BINARY_DIR}/include/rtaudio)
  set_target_properties(LA_rtaudio PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_BINARY_DIR}/include/rtaudio)
endif()

if(EXISTS "${CMAKE_BINARY_DIR}/lib/libsamplerate.a")
  add_library(LA_samplerate STATIC IMPORTED GLOBAL)
  set_target_properties(LA_samplerate PROPERTIES IMPORTED_LOCATION "${CMAKE_BINARY_DIR}/lib/libsamplerate.a")
  # (This actually doesn't work on cmake 3.10) target_include_directories(LA_samplerate INTERFACE ${CMAKE_BINARY_DIR}/include)
  set_target_properties(LA_samplerate PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_BINARY_DIR}/include)
else()
  if(CMAKE_CROSSCOMPILING AND WIN32)
    execute_process(COMMAND autoreconf -vif WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/submodules/libsamplerate)
    execute_process(COMMAND ${CMAKE_SOURCE_DIR}/submodules/libsamplerate/configure --host=x86_64-w64-mingw32 --prefix=${CMAKE_BINARY_DIR} --enable-static --disable-sndfile --disable-fftw --disable-shared WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tmp)
  else()
    execute_process(COMMAND autoreconf -vif WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/submodules/libsamplerate)
    execute_process(COMMAND ${CMAKE_SOURCE_DIR}/submodules/libsamplerate/configure --prefix=${CMAKE_BINARY_DIR} --enable-static WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tmp)
  endif()
  execute_process(COMMAND make WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tmp)
  execute_process(COMMAND make install WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tmp)
  execute_process(COMMAND rm -rf tmp WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
  execute_process(COMMAND rm -rf bin WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
  execute_process(COMMAND rm -rf share WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
  add_library(LA_samplerate STATIC IMPORTED GLOBAL)
  set_target_properties(LA_samplerate PROPERTIES IMPORTED_LOCATION "${CMAKE_BINARY_DIR}/lib/libsamplerate.a")
  # (This actually doesn't work on cmake 3.10) target_include_directories(LA_samplerate INTERFACE ${CMAKE_BINARY_DIR}/include)
  set_target_properties(LA_samplerate PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_BINARY_DIR}/include)
endif()

add_subdirectory(src)

message(STATUS "

============================================================================
CMake Generator            : ${CMAKE_GENERATOR}
Project name               : ${CMAKE_PROJECT_NAME}
Project version            : ${PROJECT_VERSION}
Build type                 : ${CMAKE_BUILD_TYPE}
Install prefix             : ${CMAKE_INSTALL_PREFIX}
Output directory           : ${CMAKE_BINARY_DIR}
============================================================================
wxWidgets Version          : ${wxWidgets_VERSION}
wxWidgets Unicode?         : ${wxWidgets_USE_UNICODE}
wxWidgets Debug?           : ${wxWidgets_USE_DEBUG}
wxWidgets Static linking   : ${wxWidgets_USE_STATIC}
wxWidgets config binary    : ${wxWidgets_CONFIG_EXECUTABLE}
wxWidgets configuration    : ${wxWidgets_CONFIGURATION}
============================================================================

")

# packaging
set(CPACK_PACKAGE_NAME "LoopAuditioneer")
set(CPACK_PACKAGE_VENDOR "LoopAuditioneer")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "LoopAuditioneer - Software for loop and cue handling in .wav files")
set(
  CPACK_PACKAGE_DESCRIPTION
  "LoopAuditioneer"
)
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "LoopAuditioneer")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.txt")
set(CPACK_PACKAGE_CONTACT "larspalo@yahoo.se")
set(CPACK_PACKAGE_EXECUTABLES "LoopAuditioneeer" "LoopAuditioneeer")
set(CPACK_SOURCE_IGNORE_FILES "/\\\\.svn/" "/build/" "/crossbuild/")
if(WIN32)
  set(CPACK_SYSTEM_NAME "windows")
  set(CPACK_GENERATOR ZIP)
  set(CMAKE_SYSTEM_PROCESSOR "x86_64")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(CPACK_SYSTEM_NAME "linux")
  set(CPACK_GENERATOR TGZ)
endif()
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME}.${CMAKE_SYSTEM_PROCESSOR}")
include(CPack)

message(STATUS "  Project                     : ${CMAKE_PROJECT_NAME}")
message(STATUS "  Description                 : ${CPACK_PACKAGE_DESCRIPTION_SUMMARY}")
message(STATUS "  Version                     : ${PROJECT_VERSION}")
message(STATUS "  Build type                  : ${CMAKE_BUILD_TYPE}")
message(STATUS "============================================================================")
message(STATUS " ")
