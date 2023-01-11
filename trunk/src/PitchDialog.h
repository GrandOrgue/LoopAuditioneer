/* 
 * PitchDialog.h is a part of LoopAuditioneer software
 * Copyright (C) 2011-2023 Lars Palo 
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

#ifndef PITCHDIALOG_H
#define PITCHDIALOG_H

#include <wx/wx.h>

// Identifiers
enum {
  ID_PITCH_METHOD = wxID_HIGHEST + 400,
  ID_NOTECOMBO = wxID_HIGHEST + 401,
  ID_PITCHFRACTION = wxID_HIGHEST + 402
};

class PitchDialog : public wxDialog {
  DECLARE_CLASS(PitchDialog)
  DECLARE_EVENT_TABLE()

public:
  // Constructors
  PitchDialog(
    double pitch,
    int midiNote,
    double pitchFraction,
    double hps_pitch,
    int hps_midiNote,
    double hps_pitchFraction,
    double td_pitch,
    int td_midiNote,
    double td_pitchFraction,
    int fileMidiNote,
    double filePitchFraction
  );
  PitchDialog(
    double pitch, 
    int midiNote,
    double pitchFraction,
    double hps_pitch,
    int hps_midiNote,
    double hps_pitchFraction,
    double td_pitch,
    int td_midiNote,
    double td_pitchFraction,
    int fileMidiNote,
    double filePitchFraction,
    wxWindow* parent,
    wxWindowID id = wxID_ANY,
    const wxString& caption = wxT("MIDI Pitch settings"),
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
  );

  // Initialize our variables
  void Init(
    double pitch,
    int midiNote,
    double pitchFraction,
    double hps_pitch,
    int hps_midiNote,
    double hps_pitchFraction,
    double td_pitch,
    int td_midiNote,
    double td_pitchFraction,
    int fileMidiNote,
    double filePitchFraction
  );

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

  // Overrides
  bool TransferDataToWindow();
  bool TransferDataFromWindow();

  // Event processing
  void OnAutoDetectionCheck(wxCommandEvent& event);
  void OnNoteChange(wxCommandEvent& event);
  void OnFractionChange(wxCommandEvent& event);

private:
  double m_detectedPitch;
  int m_detectedMIDIUnityNote;
  double m_detectedMIDIPitchFraction;
  double m_hpsDetectedPitch;
  int m_hpsDetectedMIDIUnityNote;
  double m_hpsDetectedMIDIPitchFraction;
  int m_fileMIDIUnityNote;
  double m_fileMIDIPitchFraction;
  bool m_useFFTDetection;
  bool m_useHpsFFTDetection;
  bool m_useTDDetection;
  bool m_useManual;
  double m_resultingPitch;
  int m_TDdetectedMIDIUnityNote;
  double m_TDdetectedMIDIPitchFraction;
  double m_TDdetectedPitch;
  wxArrayString pitchMethods;

  wxArrayString m_notenumbers;
  wxStaticText *fractionLabel;
  wxStaticText *resultingPitchLabel;

  void CalculatingResultingPitch();
};

#endif
