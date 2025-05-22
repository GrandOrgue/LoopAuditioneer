# LoopAuditioneer software
# Copyright (C) 2021-2025 Lars Palo and contributors (see AUTHORS file)
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

# create definition file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/LoopAuditioneerDef.h.in ${CMAKE_CURRENT_BINARY_DIR}/LoopAuditioneerDef/LoopAuditioneerDef.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/LoopAuditioneerDef)

include_directories(${CMAKE_SOURCE_DIR}/resources/icons)

# embed free-pixel-icons
include(${CMAKE_SOURCE_DIR}/scripts/CreateResources.cmake)
create_resources(${CMAKE_SOURCE_DIR}/resources/free-pixel-icons/24x24/ ${CMAKE_CURRENT_BINARY_DIR}/FreePixelIcons/FreePixelIcons.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/FreePixelIcons)

# include wxWidgets convenience file
include(${wxWidgets_USE_FILE})

# general compile options
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_options(-g -Wall -Wextra -pedantic)
else()
  add_compile_options(-O3 -ffast-math)
endif ()

# rtaudio options
if(CMAKE_CROSSCOMPILING AND WIN32)
  add_compile_options(-D__WINDOWS_DS__ -D__WINDOWS_WASAPI__ -static)
elseif(APPLE)
  add_compile_options(-D__MACOSX_CORE__ --std=c++11)
else()
  if(${RTAUDIO_USE_JACK})
    add_compile_options(-D__LINUX_ALSA__ -D__LINUX_PULSE__ -D__UNIX_JACK__)
  else()
    add_compile_options(-D__LINUX_ALSA__ -D__LINUX_PULSE__)
  endif()
endif()

# add windows resources
if(CMAKE_CROSSCOMPILING AND WIN32)
  configure_file(${CMAKE_SOURCE_DIR}/resources/LoopAuditioneer.rc.in ${CMAKE_CURRENT_BINARY_DIR}/LoopAuditioneer.rc)
  configure_file(${CMAKE_SOURCE_DIR}/resources/LoopAuditioneer.ico ${CMAKE_CURRENT_BINARY_DIR}/LoopAuditioneer.ico COPYONLY)
endif()

set(LA_SRC
  LoopAuditioneer.cpp
  MyFrame.cpp
  MyPanel.cpp
  CueMarkers.cpp
  LoopMarkers.cpp
  FileHandling.cpp
  MySound.cpp
  WaveformDrawer.cpp
  LoopParametersDialog.cpp
  BatchProcessDialog.cpp
  AutoLoopDialog.cpp
  AutoLooping.cpp
  PitchDialog.cpp
  CrossfadeDialog.cpp
  LoopOverlay.cpp
  LoopOverlayPanel.cpp
  FFT.cpp
  StopHarmonicDialog.cpp
  CutNFadeDialog.cpp
  MyListCtrl.cpp
  ListInfoDialog.cpp
  AudioSettingsDialog.cpp
  MyResampler.cpp
  SpectrumPanel.cpp
  SpectrumDialog.cpp
)

# add the executable
if(CMAKE_CROSSCOMPILING AND WIN32)
  add_executable(${CMAKE_PROJECT_NAME}
    WIN32
    ${LA_SRC}
    LoopAuditioneer.rc
  )
else()
  add_executable(${CMAKE_PROJECT_NAME}
    ${LA_SRC}
  )
endif()

# include lib-src libraries
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC
  ${CMAKE_BINARY_DIR}/include
  ${CMAKE_BINARY_DIR}/include/rtaudio
)

# link with wxWidgets and external libraries
target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC
  m
  ${wxWidgets_LIBRARIES}
  Threads::Threads
  LA_sndfile
  LA_rtaudio
  LA_samplerate
)

# set windows specific linker options
if(CMAKE_CROSSCOMPILING AND WIN32)
  target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC
    -static
    -lole32
    -lwinmm
    -lm
    -Wl,--as-needed
    -lssp
    -lksuser
    -lmfplat
    -lmfuuid
    -lwmcodecdspuuid
    -ldsound
  )
endif()

if(UNIX AND NOT APPLE)
  if(${RTAUDIO_USE_JACK})
    target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC
      PkgConfig::JACK
      PkgConfig::ALSA
      PkgConfig::PA_SIMPLE
    )
  else()
    target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC
      PkgConfig::ALSA
      PkgConfig::PA_SIMPLE
    )
  endif()
endif()

if(APPLE)
  target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC
    "-framework CoreAudio"
    "-framework AudioToolbox"
  )
endif()

# Strip binary for release builds
if(CMAKE_BUILD_TYPE STREQUAL "Release")
  if(CMAKE_CROSSCOMPILING AND WIN32)
    add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD COMMAND strip --strip-all LoopAuditioneer.exe WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
  elseif(APPLE)
    add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD COMMAND strip ${CMAKE_PROJECT_NAME} WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
  else()
    add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD COMMAND strip --strip-all ${CMAKE_PROJECT_NAME} WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
  endif()
endif()

# Zip help files to the build
if(NOT EXISTS "${RESOURCE_DIR}/help")
  file(MAKE_DIRECTORY ${RESOURCE_DIR}/help)
endif()
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD COMMAND zip -r ${RESOURCE_DIR}/help/help.zip . WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/help)

# Copy icons to the build
set(ICON_SIZES 16 24 32 48 64 128 256 512 1024)
foreach(ICON_SIZE IN ITEMS ${ICON_SIZES})
  configure_file(${CMAKE_SOURCE_DIR}/resources/icons/${ICON_SIZE}x${ICON_SIZE}/${CMAKE_PROJECT_NAME}.png ${CMAKE_BINARY_DIR}/share/icons/hicolor/${ICON_SIZE}x${ICON_SIZE}/apps/${CMAKE_PROJECT_NAME}.png COPYONLY)
endforeach()

# Copy desktop file for linux
if(UNIX AND NOT APPLE)
  configure_file(${CMAKE_SOURCE_DIR}/resources/${CMAKE_PROJECT_NAME}.desktop.in ${CMAKE_BINARY_DIR}/share/${CMAKE_PROJECT_NAME}/applications/${CMAKE_PROJECT_NAME}.desktop COPYONLY)
endif()

# Installation rules
if (APPLE)
  install(DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.app DESTINATION .)
  # Explicitly set the executable permission
  install(PROGRAMS "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.app/Contents/MacOS/${CMAKE_PROJECT_NAME}"
    DESTINATION "${PROJECT_NAME}.app/Contents/MacOS"
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
else()
  install(TARGETS ${CMAKE_PROJECT_NAME} DESTINATION bin)
  install(DIRECTORY ${CMAKE_BINARY_DIR}/share/icons/ DESTINATION share/icons FILES_MATCHING PATTERN "*.png")
  install(FILES ${CMAKE_BINARY_DIR}/share/${CMAKE_PROJECT_NAME}/help/help.zip DESTINATION share/${CMAKE_PROJECT_NAME}/help)
endif()
if(UNIX AND NOT APPLE)
  install(FILES ${CMAKE_BINARY_DIR}/share/${CMAKE_PROJECT_NAME}/applications/${CMAKE_PROJECT_NAME}.desktop DESTINATION share/${CMAKE_PROJECT_NAME}/applications)
endif()
