This is LoopAuditioneer 0.1.2.1
Copyright (C) 2011 Lars Palo
Released under a GNU/GPL 3 license, see LICENCE.txt

LoopAuditioneer is a software for evaluating loops and cues existing in wav 
file metadata. It's envisionaged to be useful for sample production for virtual
pipe organs, like GrandOrgue software. LoopAuditioneer does not find any loops
by itself, you should use another software like Nick Appletons free Autoloop
for that.

In the src/ directory you'll find the source code for LoopAuditioneer.

The lib-src/ directory contains the external libraries that LoopAuditioneer is
dependant on. Libsndfile, originally written by Erik de Castro Lopo and 
avaiable at http://www.mega-nerd.com/libsndfile/, is provided here locally as
the version included contains non-official modifications (like the ability to 
handle cue points) that the LoopAuditioneer author has added, but not yet
been officially approved and included in the library. RtAudio, by Gary P. 
Scavone (http://www.music.mcgill.ca/~gary/rtaudio/) is used for audio output.

In the icons/ directory resides the icons used by the program. The complete set
is available at http://www.small-icons.com/packs/24x24-free-pixel-icons.htm and
distributed under a Creative Commons 3.0 license.

Basic installation instructions are available in the InstallInstructions.txt
file. At the moment the program developed and mostly tested under Linux
(Ubuntu 11.04).

OTHER DEPENDENCIES
------------------

LoopAuditioneer requires wxWidgets, available at http://www.wxwidgets.org/.

CONTACT THE AUTHOR:
-------------------

You can contact the author on larspalo AT yahoo DOT se.
