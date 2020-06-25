This is LoopAuditioneer 0.8.3
Copyright (C) 2011-2020 Lars Palo
Released under a GNU/GPL 3 license, see LICENCE.txt

LoopAuditioneer is a software for evaluating loops and cues existing in wav 
file metadata. It's envisionaged to be useful for sample production for virtual
pipe organs, like GrandOrgue software. LoopAuditioneer can also now find loops
by itself automatically from very user configurable settings both in single
file mode and in batch processing. Of course you can still use another software
like Nick Appletons free Autoloop independantly for other loop options.

In the src/ directory you'll find the source code for LoopAuditioneer. The
icons in src/icons are created by the author. The only code in the src/ folder
that's not written by the author is the FFT.h and FFT.cpp files that are
slightly modified versions of those in Audacity source code.

The pitch detection algorithm for the time domain is taken and adapted from a
discussion on the GrandOrgue mailing list.

The lib-src/ directory contains the external libraries that LoopAuditioneer is
dependant on. Libsndfile, originally written by Erik de Castro Lopo and 
avaiable at http://www.mega-nerd.com/libsndfile/, is provided here locally as
the version included contains non-official modifications (like the ability to 
handle pitch fraction) that the LoopAuditioneer author has added that has not
yet been officially approved of and included in the library. RtAudio, by Gary
P. Scavone (http://www.music.mcgill.ca/~gary/rtaudio/) is used for audio
output.

In the icons/ directory resides the icons used by the program. The complete set
is available at http://www.small-icons.com/packs/24x24-free-pixel-icons.htm and
distributed under a Creative Commons 3.0 license.

Basic installation instructions are available in the InstallInstructions.txt
file. At the moment the program developed and mostly tested under Linux
(Ubuntu 20.04).

OTHER DEPENDENCIES
------------------

LoopAuditioneer requires wxWidgets, available at http://www.wxwidgets.org/. The
repository unicode version of wxWidgets that Ubuntu offer should work as well.
From 0.8.0.2 the 3.0+ version of wxWidgets is needed.

CONTACT THE AUTHOR:
-------------------

You can contact the author on larspalo AT yahoo DOT se.
