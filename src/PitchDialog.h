/* 
 * PitchDialog.h is a part of LoopAuditioneer software
 * Copyright (C) 2011-2025 Lars Palo and contributors (see AUTHORS file) 
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * You can contact the author on larspalo(at)yahoo.se
 */

#ifndef PITCHDIALOG_H
#define PITCHDIALOG_H

#include <wx/wx.h>
#include "FileHandling.h"

// Identifiers
enum {
  ID_PITCH_METHOD = wxID_HIGHEST + 400,
  ID_NOTECOMBO = wxID_HIGHEST + 401,
  ID_PITCHFRACTION = wxID_HIGHEST + 402,
  ID_SPECTRUM_BTN = wxID_HIGHEST + 403,
  ID_FFTSIZE_CHOICE = wxID_HIGHEST + 404,
  ID_WINDOW_TYPE_CHOICE = wxID_HIGHEST + 405,
};

class PitchDialog : public wxDialog {
  DECLARE_CLASS(PitchDialog)
  DECLARE_EVENT_TABLE()

public:
  // Constructors
  PitchDialog(FileHandling *audioFile);
  PitchDialog(
    FileHandling *audioFile,
    wxWindow* parent,
    wxWindowID id = wxID_ANY,
    const wxString& caption = wxT("MIDI Pitch settings"),
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
  );

  // Initialize our variables
  void Init(FileHandling *audioFile);

  // Creation
  bool Create( 
    wxWindow* parent,
    wxWindowID id = wxID_ANY,
    const wxString& caption = wxT("MIDI Pitch settings"),
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
  );

  // Creates the controls and sizers
  void CreateControls();

  // Accessing functions
  int GetMethodUsed();
  int GetMIDINote();
  double GetPitchFraction();
  int GetFftSize();
  int GetWindowType();
  bool GetInterpolatePitch();

  // Overrides
  bool TransferDataToWindow();
  bool TransferDataFromWindow();
  void TransferSelectedPitchToFile();

  // Event processing
  void OnAutoDetectionCheck(wxCommandEvent& event);
  void OnNoteChange(wxCommandEvent& event);
  void OnFractionChange(wxCommandEvent& event);
  void OnViewSpectrumButton(wxCommandEvent& event);

  // Setter functions
  void SetPreferredPitchMethod(int method);
  void SetPreferredFftSize(int size);
  void SetPreferredWindow(int window);
  void SetPreferredInterpolatePitch(bool interpolate);

private:
  double m_detectedPitch;
  int m_detectedMIDIUnityNote;
  double m_detectedMIDIPitchFraction;
  unsigned m_actualMIDIPitchFraction;
  double m_hpsDetectedPitch;
  int m_hpsDetectedMIDIUnityNote;
  double m_hpsDetectedMIDIPitchFraction;
  unsigned m_actualHpsMIDIPitchFraction;

  double m_fftPeakPitch;
  int m_fftPeakMIDIUnityNote;
  double m_fftPeakMIDIPitchFraction;
  unsigned m_actualFftPeakMIDIPitchFraction;

  int m_fileMIDIUnityNote;
  double m_fileMIDIPitchFraction;
  bool m_useFFTDetection;
  bool m_useHpsFFTDetection;
  bool m_useFftPeakDetection;
  bool m_useTDDetection;
  bool m_useManual;
  double m_resultingPitch;
  int m_TDdetectedMIDIUnityNote;
  double m_TDdetectedMIDIPitchFraction;
  unsigned m_actualTdMIDIPitchFraction;
  double m_TDdetectedPitch;
  wxArrayString pitchMethods;
  FileHandling *m_audioFile;

  wxArrayString m_notenumbers;
  wxStaticText *fractionLabel;
  wxStaticText *resultingPitchLabel;
  wxArrayString m_fftSizes;
  wxArrayString m_windowTypes;
  bool m_useInterpolatePitch;

  void CalculatingResultingPitch();
};

#endif
