# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- Possibility to inspect and adjust cue position similar to looppoint overlay. (TODO)
- Possibility to adjust cue position on sample level detail. (TODO)

### Changed

- Re-worked the dialog showing waveform overlay at looppoints to be independent (modeless). (TODO)

### Fixed

- Drawing of the playposition marker so ports that doesn't support drawing with wxClientDC also behave correctly.
- Builds on Github by using ubuntu-22.04 instead of ubuntu-20.04.

## [0.11.1] - 2024-11-04

### Changed

- FFT pitch detection algorithms to take both individaul peaks and their harmonic relations into account.
- Pitch detection in time domain to exclude values that are wildly off when finally averaging the detected pitch.

### Fixed

- Bug that could cause a crash when trying to auto detect sustain section.
- Regression in LoopOverlay dialog that didn't take size changes into account for adjusting max number of samples to display.
- Bug that could cause a crash when adjusting sustain section on waveform manually.
- Bug in the GetSpectrum function in FileHandling.cpp that could cause a crash.

## [0.11.0] - 2024-06-19

### Added

- Builds for macOS.
- Config file setting for preferred pitch method.
- Config file setting for spectrum FFT size.
- Config file setting for spectrum window type.
- Config file setting for spectrum panel interpolate pitch option.
- Batch process to remove pitch information from files.
- Batch process to remove loops, cues and pitch.
- Option to make batch process files recursively.

### Changed

- Batch process naming "Kill" loops/cues to "Remove" instead.
- Loop overlay drawing on separate wxPanel.

### Fixed

- Sizer flag warnings for wxWidgets 3.2+ builds.

## [0.10.1] - 2024-06-06

### Changed

- Spectrum panel zoom removes current selected pitch
- Spectrum panel help lines for clicking pitch draw horizontal line at actual dB level.

### Fixed

- Spectrum panel zoom in to calculate pitch correctly with current offset taken into account.
- Spectrum panel selection zoom to behave correctly.

## [0.10.0] - 2024-02-27

### Added

- Possibility to inspect FFT power spectrum in a new dialog.
- Set pitch manually from selected peak in FFT power spectrum dialog.
- Export audio data from cue as separate release in batch mode.
- Export audio data to after last loop as attack in batch mode.

### Changed

- Changed version control system from svn to git and moved project to GitHub.
- External libraries converted to submodules.

## [0.9.0] - 2021-11-11

### Added

- New cmake build system.

### Fixed

- New project icon with higher resolution.

### Changed

- Re-organized some of the project files.

## [0.8.8] - 2021-09-26 

### Added

- Indication on the waveform for which loop or cue that's currently selected.

### Changed

- Updated internal help documentation.

## [0.8.7] - 2021-09-22

### Added

- Possibility to adjust the sustainsection (when in manual mode) directly on
  the waveform.
- If a new loop is manually created a basic loop search will be done at
  endpoints of sustainsection.
- Options to open next upper or lower file from the currently selected one was
  added and can be used either by the file menu items or key combination.

## [0.8.6] - 2021-05-27

### Added

- Possibility to select different sound Apis and Devices.
- Audio output can now be resampled if it's needed.

### Fixed

- Prevent crash if audio stream was playing while opening a file.

## [0.8.5] - 2020-07-12

### Added

- Possiblitiy to read/write some LIST INFO stings.

## [0.8.4] - 2020-07-04 

### Added

- Currently used sustainsection is marked on the waveform.

### Changed

- Audio data handling has been streamlined.
- New handling of the sustainsection developed.

## [0.8.3] - 2020-06-25

### Changed

- Updated the external libraries and build on a newer toolchain.
- Small code adjustments done to make it work with the new build tools.

## [0.8.2] - 2016-01-23

### Added

- Directory name prepend the open file name.
- Application window size and position is stored in config file
- Waveform overlay at looppoints allow stepping between loops, altering
  looppoints, choosing how many samples should be displayed.

### Changed

- Status bar division to enable longer paths displayed.
- Clarified the section on "Is there a standard change log format?".
- Major re-work of waveform overlay at looppoints.

### Fixed

- File list scrolling that made current file invisible.
- Loop selection after loopsearch.

## [0.8.1.3] - 2015-11-15 

### Changed

- Updated toolchain.
- Small changes to the code because of the newer wxWidgets library.

## [0.8.1.2] - 2015-02-15 

### Fixed

- Header display of the listctrl on Windows.
- Spacing of the file listctrl columns.

## [0.8.1.1] - 2015-02-12 

### Added

- Automatic loopsearch is now aware of already present loops in the file.

### Fixed

- Playback of 32 bit float files.

## [0.8.1] - 2015-02-05

### Added

- Possibility to do crossfades as a batch process.
- A complete help is now built into the program.
- Possibility to save a file and open next in the file list with one command.

### Changed

- Some batch processes are re-worked to be more useful in providing lines for a
  GrandOrgue ODF
- Increased range of a few of the loop search settings parameters.

### Fixed

- A few minor bugs.

## [0.8.0.1] - 2014-04-23

### Added

- Option to toggle selected loop or cue save on/off by keyboard.
- Option to toggle all loops save option on/off
- Option to toggle all cues save options on/off

### Fixed

- Scrolling issue when a loop save option was clicked.
- Filelist entry for a saved file will now be updated properly with the saved
  loops/cues number correct.
- Crossfade option was grayed out unless a loop was clicked.

## [0.8.0] - 2014-04-20

### Added

- Navigation between files in the listctrl and between loops and cues can be
  done with keyboard keys.
- Saving many of the settings to a config text file.
- Playback of only the loop part of a sample.

### Changed

- File listing in the listctrl allow multiple columns showing number of loops,
  cues, MIDI note and Pitch fraction.
- Keyboard shortcut keys changed to be better placed for lefthad usage.
- Loopsearch in batch mode now share the settings of single file mode.

### Fixed

- Some bugs in different batch processes.

## [0.7.5] - 2014-04-13

### Added

- A brute force option for loopsearching.

### Changed

- Increased range of loop finding parameters.
- If more than one loop is found they must overlap.

## [0.7.4] - 2014-02-16

### Added

- An additional FFT based pitch detection method (harmonic product spectrum,
  HPS).

### Changed

- FFT method is slightly reworked in how the harmonic matching is performed.

## [0.7.3] - 2014-01-29

### Added

- Options for cutting and fading in/out samples both in single file mode and in
  batch process mode.
- Possibility to trim away unused sample data (wav data between last loop and
  cue, or after last loop if no cue exist) in batch mode.

## [0.7.0.3] - 2012-03-25

### Changes

- Improvements to the pitch detection algorithms.
-  The FFT based version can now (hopefully) better find what peak is the
  fundamental since some harmonics detection checks are implemented. 
- Some internal re-arranging of the FFT and time domain pitch detection has
  been done to improve batch processing speeds.
- The release files were more agressively optimized (-O3 and -ffast-math) in an
  attempt to improve performance speed.

## [0.7.0.2] - 2012-03-18

### Added

- More crossfade methods.

### Changed

- Pitch detection now works for shorter samples where the sustain section is
  very short

### Fixed 

- Crash if the crossfade would accidentally attempt to write outside the audio
  data array.

## [0.7.0.1] - 2012-02-19

### Added

- Safety checks to avoid crashes if samples couldn't have the pitch detected.
- Option to list existing pitch data in the smpl chunk in batch processing.

### Changed

- Waveform drawing from using a spline in the loopoverlay to use straight lines
  instead.
- Loopsearching parameter limits.

### Fixed

- Crossfade methods.

## [0.7.0] - 2012-02-13

### Added

- Possibility to modify the audio data of a file. 
- Crossfading of loops in a few different manners.
- View waveform at the looppoints overlayed for close up inspection.
- Possibility to cut and fade in/out both to single file mode and in batch
  processing.

## [Older releases]

It's now possible to edit cues by left clicking on the cue marker flag in the
waveform and then left click again at the desired new location. It's also now
possible to create new cue markers by right clicking at the desired position on
the waveform and select Add cue from the popup menu.

A new feature makes it possible to create loops from the toolbar or menu and 
also edit already existing loops by right clicking on the loop row in the 
table, which will trigger a dialog where the new start and end positions can be
written. It's however recommended to use another software to automatically find
good loop points, since that feature is not available in LoopAuditioneer.

It's possible to autosearch for good natural loop points either by using the
toolbar icon or the menu item. Fine control of the loopsearch settings is
possible in the settings dialog available from both toolbar and menu.

Now it's also possible to auto-detect the pitch of the sample and store this
information as midi note and pitch fraction. Of course it's also possible to
manually set this information. It's possible to use either the FFT based pitch
detection or the time domain based one. Time domain is better for single notes
at medium to lower frequencies while FFT is better for multiple notes 
(like mixtures) and higher frequencies (approximately at 072-C of a 8' stop).
A second FFT based pitch detection method is added (harmonic product spectrum)
which works slightly differently to get another option for difficult samples.

Batch processing of all wav files in directory is possible through the batch
process dialog available both on toolbar and in menu.

All menu items now also have keyboard shortcuts.

For soft samples it's now possible to boost the volume to be able to better
judge the quality of the found loops.
