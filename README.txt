This is LoopAuditioneer
Copyright (C) 2011-2023 Lars Palo
Released under a GNU/GPL 3 license, see LICENCE.txt

LoopAuditioneer is a software for evaluating loops and cues existing in wav 
file metadata. It's envisionaged to be useful for sample production for virtual
pipe organs, like GrandOrgue software. In LA The user can also search loops
automatically with very user configurable settings both in single file mode and
in batch processing. Of course you can still use another software like Nick 
Appletons free Autoloop independantly for other loop options.

In the src/ directory you'll find the source code for LoopAuditioneer. The
icons in resources/icons are created by the author. The only code in the src/
folder that's not written by the author is the FFT.h and FFT.cpp files that are
slightly modified versions of those in Audacity source code.

The pitch detection algorithm for the time domain is taken and adapted from a
discussion on the GrandOrgue mailing list.

The lib-src/ directory contains the external libraries that LoopAuditioneer is
dependant on. Libsndfile, and Libsamplerate written by Erik de Castro Lopo and 
avaiable at https://github.com/libsndfile, are provided here locally as
especially the version of libsndfile included contains non-official 
modifications (like the ability to handle pitch fraction exactly) that the
LoopAuditioneer author has added that has not yet been officially approved of
and included in the library. RtAudio, by Gary P. Scavone
(https://github.com/thestk/rtaudio) is used for audio output.

In the resources/free-pixel-icons/ directory resides the icons used by the
program for the toolbar. The complete set is available at 
http://www.small-icons.com/packs/24x24-free-pixel-icons.htm and distributed
under a Creative Commons 3.0 license.

Basic compilation instructions are available in the BUILD.txt file. At the
moment the program developed and mostly tested under Linux
(Ubuntu 20.04) 64 bit. The Windows binaries are produced by cross-compilation
with i686-w64-mingw32.

At https://loopauditioneer.sourceforge.io/userguide.html official documentation
on how to use the software can be found. Inside the software the help can be
consulted for additional insights.

OTHER DEPENDENCIES
------------------

LoopAuditioneer requires wxWidgets, available at http://www.wxwidgets.org/. The
repository unicode version of wxWidgets that Ubuntu offer should work as well.
From 0.8.0.2 the 3.0+ version of wxWidgets is needed. Other dependencies are
documented in the BUILD.txt file.

CONTACT THE AUTHOR:
-------------------

You can contact the author on larspalo AT yahoo DOT se.
