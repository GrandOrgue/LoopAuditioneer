/*
 * FileHandling.h is a part of LoopAuditioneer software
 * Copyright (C) 2011-2012 Lars Palo
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * You can contact the author on larspalo(at)yahoo.se
 */

#ifndef FILEHANDLING_H
#define FILEHANDLING_H

#include <wx/wx.h>
#include "sndfile.hh"
#include "LoopMarkers.h"
#include "CueMarkers.h"
#include <vector>
#include "RtAudio.h"

class FileHandling {
public:
  FileHandling(wxString fileName, wxString path);
  ~FileHandling();

  LoopMarkers *m_loops;
  CueMarkers *m_cues;

  void SaveAudioFile(wxString fileName, wxString path);
  int GetSampleRate();
  void SetSampleRate(unsigned s_rate);
  int GetAudioFormat();
  int GetWholeFormat();
  bool FileCouldBeOpened();
  double GetFFTPitch(double data[]);
  double GetTDPitch();
  void PerformCrossfade(double audioData[], int loopNumber, double fadeLength, int fadeType);

  short *shortAudioData;
  int *intAudioData;
  double *doubleAudioData;

  long unsigned int ArrayLength;

  int m_channels;

private:
  SndfileHandle sfh;
  SF_INSTRUMENT instr;
  SF_CUES cues;

  int m_format;
  int m_minorFormat;
  unsigned m_samplerate;
  bool fileOpenWasSuccessful;
  double m_fftPitch;
  double m_timeDomainPitch;

  bool DetectPitchByFFT(double data[]);
  double DetectedPitchInTimeDomain(double audio[], unsigned start, unsigned end);

};

#endif

