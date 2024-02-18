# LoopAuditioneer

LoopAuditioneer is a software for evaluating loops and cues existing in wav 
file metadata as well as generally being helpful in sample peparation for
virtual pipe organs, like the
[GrandOrgue software](https://github.com/GrandOrgue/grandorgue).

## Things you can do with LoopAuditioneer

- Load .wav files and display existing cue and smpl metadata
- See the waveform with loops and cues drawn upon it
- zoom in/out on amplitude of the waveform
- Play back the loops and cues for aural evaluation
- Choose which loops/cues to keep when saved or saved as...
- Enjoy bit true data handling, even if header is re-written due to changes,
  there will be no degrading of the audio data unless actual changes are made
  to the audio data itself
- Edit/create cue (release marker) position directly on the waveform with
  automatic adjustment to find the position with lowest RMS power closest to
  where the user clicked
- Edit loops by right clicking on the loop table row
- Create new loops manually if one knows the right sample numbers
- Autosearch for good, natural loop points with a high degree of
  configurational control over the loopsearching process
- Perform crossfades of loops when it's difficult to find natural seamless ones
- View the waveform at the looppoints in detail
- Batch process all wav files in any source folder with freely selectable target
  folder to either overwrite the existing files or create copies of them
- Autodetect pitch of sample and store information in file and manually edit
  dwMIDIUnityNote and dwMIDIPitchFraction
- View the FFT power spectrum and set pitch from chosen peak
- Perform cut and fade in/out both in single file mode and in batch mode
- Trim away unused wav data from looped samples in batch mode
- Export audio data from cue marker as separate release in batch mode
- Export audio data to after last loop as separate attack in batch mode

## Dependencies on others code

The only code in the src/ directory that's not written by the author is the
FFT.h and FFT.cpp files that are slightly modified versions of those in Audacity
source code. The pitch detection algorithm for the time domain is taken and
adapted from a discussion on the GrandOrgue mailing list.

Other external libraries that LoopAuditioneer is dependant on and use as
submodules are [Libsndfile](https://github.com/larspalo/libsndfile),
[Libsamplerate](https://github.com/libsndfile/libsamplerate) both originally
written by Erik de Castro Lopo. Libsndfile, are provided from a fork of the
official version to include changes that allow exact handling of pitch fraction
metadata information. [RtAudio](https://github.com/thestk/rtaudio), by Gary P.
Scavone is used for audio output.

## Graphical credits

In the resources/free-pixel-icons/ directory resides the icons used by the
program for the toolbar. The complete set is available at 
http://www.small-icons.com/packs/24x24-free-pixel-icons.htm and distributed
under a Creative Commons 3.0 license. The icons in resources/icons are created
by the author (Lars Palo).

## Building

Basic compilation instructions are available in the BUILD.md file. At the
moment the program is developed and mostly tested under Linux (Ubuntu 20.04) 64
bit. The Windows binaries are produced by cross-compilation with
i686-w64-mingw32.

LoopAuditioneer requires wxWidgets 3.0+, available at http://www.wxwidgets.org/. The
repository unicode version of wxWidgets that Ubuntu offer should work as well.
Other dependencies are documented in the BUILD.md file.

## Help and documentation

At https://loopauditioneer.sourceforge.io/userguide.html official documentation
on how to use the software can be found. Inside the software the help can be
consulted for additional insights.
