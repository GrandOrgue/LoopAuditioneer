/*
 * FileHandling.h is a part of LoopAuditioneer software
 * Copyright (C) 2011-2024 Lars Palo and contributors (see AUTHORS file)
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
#include <wx/datetime.h>

typedef struct {
  std::vector<double> waveData;
} WAVETRACK;

typedef struct {
  // LIST INFO string data
  wxString artist;
  wxString copyright;
  wxString software;
  wxString comment;
  wxDateTime creation_date;
} WAV_LIST_INFO;

class SpectrumPeak {
public:
  SpectrumPeak() {
    m_binNbr = 0;
    m_dB = -200;
    m_pitch = 0;
    m_harmonicQuality = 0;
  }
  unsigned m_binNbr;
  double m_dB;
  double m_pitch;
  std::vector<unsigned> m_matchingHarmonics;
  double m_harmonicQuality;
};

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
  bool GetFFTPitch(double pitches[]);
  bool GetSpectrum(double *output, unsigned fftSize, int windowType);
  double GetTDPitch();
  void PerformCrossfade(int loopNumber, double fadeLength, int fadeType);
  void TrimExcessData();
  bool TrimStart(unsigned timeToTrim);
  bool TrimEnd(unsigned timeToTrim);
  bool TrimAsRelease();
  bool TrimAsAttack();
  void PerformFade(unsigned fadeLength, int fadeType);
  // Get audio data as doubles
  bool GetDoubleAudioData(double audio[]);
  // Update the wave data vector if audio is changed
  void UpdateWaveTracks(double audio[]);
  void SetAutoSustainSearch(bool choice);
  bool GetAutoSustainSearch();
  std::pair<unsigned, unsigned> GetSustainsection();
  void SetSliderSustainsection(int start, int end);
  // Get strongest channel of audio data as doubles
  void SeparateStrongestChannel(double outData[]);
  bool AutoCreateReleaseCue();
  wxString GetFileName();

  short *shortAudioData;
  int *intAudioData;
  float *floatAudioData; // always read and used for playback!
  double *doubleAudioData;
  std::vector<WAVETRACK> waveTracks;
  WAV_LIST_INFO m_info;

  long unsigned int ArrayLength;

  int m_channels;

private:
  SndfileHandle sfh;
  SF_INSTRUMENT instr;
  SF_CUES cues;

  wxString m_fileName;
  int m_format;
  int m_minorFormat;
  unsigned m_samplerate;
  bool fileOpenWasSuccessful;
  double m_fftPitch;
  double m_fftHPS;
  double m_timeDomainPitch;
  unsigned m_autoSustainStart;
  unsigned m_autoSustainEnd;
  unsigned m_sliderSustainStart;
  unsigned m_sliderSustainEnd;
  bool m_useAutoSustain;

  bool DetectPitchByFFT();
  bool DetectPitchInTimeDomain();
  double TranslateIndexToPitch(
    int idxAtPeak,
    double valueBeforePeak,
    double valueAtPeak,
    double valueAfterPeak,
    unsigned wSize
  );
  void CalculateSustainStartAndEnd();

};

#endif

