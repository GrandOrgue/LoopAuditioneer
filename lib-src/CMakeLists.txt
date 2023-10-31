# LoopAuditioneer software
# Copyright (C) 2023 Lars Palo 
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

if(NOT EXISTS "${CMAKE_BINARY_DIR}/tmp")
  execute_process(COMMAND mkdir tmp WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

if(EXISTS "${CMAKE_BINARY_DIR}/lib/libsndfile.a")
  add_library(LA_sndfile STATIC IMPORTED GLOBAL)
  set_target_properties(LA_sndfile PROPERTIES IMPORTED_LOCATION "${CMAKE_BINARY_DIR}/lib/libsndfile.a")
  # (This actually doesn't work on cmake 3.10) target_include_directories(LA_sndfile INTERFACE ${CMAKE_BINARY_DIR}/include)
  set_target_properties(LA_sndfile PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_BINARY_DIR}/include)
else()
  if(CMAKE_CROSSCOMPILING AND WIN32)
    execute_process(COMMAND ${CMAKE_SOURCE_DIR}/scripts/conf-libsndfile-cross.sh ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tmp)
  else()
    execute_process(COMMAND ${CMAKE_SOURCE_DIR}/lib-src/libsndfile/configure --prefix=${CMAKE_BINARY_DIR} --enable-static --disable-external-libs --disable-full-suite --disable-sqlite --disable-alsa WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tmp)
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
  if(CMAKE_CROSSCOMPILING AND WIN32)
    execute_process(COMMAND ${CMAKE_SOURCE_DIR}/lib-src/rtaudio/configure --host=i686-w64-mingw32 --prefix=${CMAKE_BINARY_DIR} --enable-static --disable-shared --with-dsound --with-wasapi WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tmp)
  else()
    if(${RTAUDIO_USE_JACK})
      execute_process(COMMAND ${CMAKE_SOURCE_DIR}/lib-src/rtaudio/configure --prefix=${CMAKE_BINARY_DIR} --with-pulse --with-jack --with-alsa WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tmp)
    else()
      execute_process(COMMAND ${CMAKE_SOURCE_DIR}/lib-src/rtaudio/configure --prefix=${CMAKE_BINARY_DIR} --with-pulse --with-alsa WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tmp)
    endif()
  endif()
  execute_process(COMMAND make WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tmp)
#  if(CMAKE_CROSSCOMPILING AND WIN32)
#    execute_process(COMMAND i686-w64-mingw32-ranlib .libs/librtaudio.a WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tmp)
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
    execute_process(COMMAND ${CMAKE_SOURCE_DIR}/lib-src/libsamplerate/configure --host=i686-w64-mingw32 --prefix=${CMAKE_BINARY_DIR} --enable-static --disable-sndfile --disable-fftw --disable-shared WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tmp)
  else()
    execute_process(COMMAND ${CMAKE_SOURCE_DIR}/lib-src/libsamplerate/configure --prefix=${CMAKE_BINARY_DIR} --enable-static WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tmp)
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
