[![Release](https://img.shields.io/github/v/release/GrandOrgue/LoopAuditioneer)](https://github.com/GrandOrgue/LoopAuditioneer/releases)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

# LoopAuditioneer

LoopAuditioneer is a software for evaluating loops and cues existing in wav 
file metadata as well as generally being helpful in sample preparation for
usage in virtual pipe organs, like the
[GrandOrgue software](https://github.com/GrandOrgue/grandorgue).

## Features

- Load .wav sample files and display existing cue and smpl metadata
- See the waveform with loops and cues drawn upon it
- Zoom in/out on amplitude of the waveform
- Play back the loops and cues for aural evaluation
- Choose which loops/cues to keep when saved or saved as...
- Enjoy bit true data handling, even if header is re-written due to changes,
  there will be no degrading of the audio data unless actual changes are made
  to the audio data itself
- Edit/create cue (release marker) position directly on the waveform with
  automatic adjustment to find the position with lowest RMS power closest to
  where the user clicked
- Edit/create new loops manually
- Autosearch for good, natural loop points with a high degree of
  configurational control over the loopsearching process
- Perform crossfades of loops when it's difficult to find natural seamless ones
- View the waveform at the looppoints in detail
- Batch process all .wav files in any source directory with freely selectable
  target directory to either overwrite the existing files or create copies of
  them
- Autodetect pitch of sample and store information in file
- Edit dwMIDIUnityNote and dwMIDIPitchFraction
- View the FFT power spectrum and set pitch from chosen peak
- Perform cut and fade in/out both in single file mode and in batch mode
- Trim away unused wav data from looped samples in batch mode
- Export audio data from cue marker as separate release in batch mode
- Export audio data to after last loop as separate attack in batch mode

## Credit to others code

The only code in the src/ directory that's not written by the author is the
FFT.h and FFT.cpp files that are slightly modified versions of those in Audacity
source code. The pitch detection algorithm for the time domain is taken and
adapted from a discussion on the GrandOrgue mailing list.

Other external libraries that LoopAuditioneer is dependant on and use as
submodules are [Libsndfile](https://github.com/GrandOrgue/libsndfile) and
[Libsamplerate](https://github.com/libsndfile/libsamplerate) both originally
written by Erik de Castro Lopo. Libsndfile, is provided from a fork of the
official version to allow exact reading and writing of pitch fraction
metadata information. [RtAudio](https://github.com/thestk/rtaudio), by Gary P.
Scavone is used for audio output.

## Graphical credits

In the resources/free-pixel-icons/ directory resides the icons used by the
program for the toolbar. The complete set is available at 
http://www.small-icons.com/packs/24x24-free-pixel-icons.htm and distributed
under a Creative Commons 3.0 license. The icons in resources/icons are created
by the author (Lars Palo) except for the macOS adaptation which was done by
[vpoguru](https://github.com/vpoguru).

## Building

Basic compilation instructions are available in the BUILD.md file. At the
moment the program is developed and mostly tested under Linux (Ubuntu 20.04) 64
bit. The Windows binaries are produced by cross-compilation with
x86_64-w64-mingw32. Since june 2024 builds for macOS, both Intel and arm64 are
also available. All releases of this software here on Github are created on
action runners on Github.

LoopAuditioneer requires wxWidgets 3.0+, available at http://www.wxwidgets.org/.
The repository unicode version of wxWidgets that Ubuntu offer should work as
well. Other build dependencies are documented in the BUILD.md file. Both macOS
and Windows builds have wxWidgets included in them which means that they need
nothing extra installed in order to run.

## Help and documentation

At https://loopauditioneer.sourceforge.io/userguide.html older official
documentation on how to use the software can be found. Inside the software the
help can be consulted for additional insights.
