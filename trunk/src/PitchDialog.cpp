/* 
 * PitchDialog.cpp is a part of LoopAuditioneer software
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

#include "PitchDialog.h"

IMPLEMENT_CLASS(PitchDialog, wxDialog )

BEGIN_EVENT_TABLE(PitchDialog, wxDialog)
  EVT_RADIOBOX(ID_PITCH_METHOD, PitchDialog::OnAutoDetectionCheck)
  EVT_COMBOBOX(ID_NOTECOMBO, PitchDialog::OnNoteChange)
  EVT_SLIDER(ID_PITCHFRACTION, PitchDialog::OnFractionChange)
END_EVENT_TABLE()

PitchDialog::PitchDialog(
  double pitch,
  int midiNote,
  double pitchFraction,
  double td_pitch,
  int td_midiNote,
  double td_pitchFraction,
  int fileMidiNote,
  double filePitchFraction
) {
  Init(pitch, midiNote, pitchFraction, td_pitch, td_midiNote, td_pitchFraction, fileMidiNote, filePitchFraction);
}

PitchDialog::PitchDialog(
  double pitch,
  int midiNote,
  double pitchFraction,
  double td_pitch,
  int td_midiNote,
  double td_pitchFraction,
  int fileMidiNote,
  double filePitchFraction,
  wxWindow* parent,
  wxWindowID id,
  const wxString& caption,
  const wxPoint& pos,
  const wxSize& size,
  long style
) {
  Init(pitch, midiNote, pitchFraction, td_pitch, td_midiNote, td_pitchFraction, fileMidiNote, filePitchFraction);
  Create(parent, id, caption, pos, size, style);
}
 
void PitchDialog::Init(
  double pitch,
  int midiNote,
  double pitchFraction,
  double td_pitch,
  int td_midiNote,
  double td_pitchFraction,
  int fileMidiNote,
  double filePitchFraction
) {
  m_detectedMIDIUnityNote = midiNote;
  m_detectedMIDIPitchFraction = pitchFraction;
  m_fileMIDIUnityNote = fileMidiNote;
  m_fileMIDIPitchFraction = filePitchFraction;
  CalculatingResultingPitch();
  m_detectedPitch = pitch;
  m_TDdetectedMIDIUnityNote = td_midiNote;
  m_TDdetectedMIDIPitchFraction = td_pitchFraction;
  m_TDdetectedPitch = td_pitch;
  m_useFFTDetection = true;
  m_useTDDetection = false;
  m_useManual = false;

  pitchMethods.Add(wxT("FFT pitch"));
  pitchMethods.Add(wxT("Timedomain pitch"));
  pitchMethods.Add(wxT("Existing/manual pitch"));

  for (int i = 0; i < 128; i++)
    m_notenumbers.Add(wxString::Format(wxT("%d"), i));
}

bool PitchDialog::Create( 
  wxWindow* parent,
  wxWindowID id,
  const wxString& caption,
  const wxPoint& pos,
  const wxSize& size,
  long style
) {
  if (!wxDialog::Create(parent, id, caption, pos, size, style))
    return false;

  CreateControls();

  GetSizer()->Fit(this);
  GetSizer()->SetSizeHints(this);
  Centre();

  return true;
}

void PitchDialog::CreateControls() {
  // Create a top level sizer
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  this->SetSizer(topSizer);

  // Second box sizer to get nice margins
  wxBoxSizer *boxSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(boxSizer, 1, wxEXPAND|wxALL, 5);

  // Horizontal sizer for first row
  wxBoxSizer* firstRow = new wxBoxSizer(wxHORIZONTAL);
  boxSizer->Add(firstRow, 1, wxGROW|wxALL, 5);

  // Grouping of FFT pitch detection information
  wxStaticBox *FFTPitchBox = new wxStaticBox(
    this, 
    wxID_STATIC,
    wxT("FFT-based pitch detection"), 
    wxDefaultPosition, 
    wxDefaultSize
  );

  // Vertical sizer for auto pitch subsections
  wxStaticBoxSizer *autoPitchContainer = new wxStaticBoxSizer(FFTPitchBox, wxVERTICAL);
  firstRow->Add(autoPitchContainer, 1, wxGROW|wxALL, 5);

  // Label for the autodetected pitch frequency
  wxStaticText *pitchLabel = new wxStaticText ( 
    this, 
    wxID_STATIC,
    wxEmptyString, 
    wxDefaultPosition, 
    wxDefaultSize, 
    0
  );
  pitchLabel->SetLabel(wxString::Format(wxT("Detected pitch: %.2f Hz"), m_detectedPitch));
  autoPitchContainer->Add(pitchLabel, 1, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 2);

  // Label for the calculated MIDIUnityNote
  wxStaticText *midiNoteLabel = new wxStaticText ( 
    this, 
    wxID_STATIC,
    wxEmptyString, 
    wxDefaultPosition, 
    wxDefaultSize, 
    0
  );
  midiNoteLabel->SetLabel(wxString::Format(wxT("MIDIUnityNote: %d"), m_detectedMIDIUnityNote));
  autoPitchContainer->Add(midiNoteLabel, 1, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 2);

  // Label for the calculated MIDIPitchFraction
  wxStaticText *pitchFractionLabel = new wxStaticText ( 
    this, 
    wxID_STATIC,
    wxEmptyString, 
    wxDefaultPosition, 
    wxDefaultSize, 
    0
  );
  pitchFractionLabel->SetLabel(wxString::Format(wxT("PitchFraction: %.2f cent"), m_detectedMIDIPitchFraction));
  autoPitchContainer->Add(pitchFractionLabel, 1, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 2);

  // Grouping of TimeDomain pitch detection information
  wxStaticBox *TDPitchBox = new wxStaticBox(
    this, 
    wxID_STATIC,
    wxT("Timedomain-based pitch detection "), 
    wxDefaultPosition, 
    wxDefaultSize
  );

  // Vertical sizer for auto pitch subsections
  wxStaticBoxSizer *TDPitchContainer = new wxStaticBoxSizer(TDPitchBox, wxVERTICAL);
  firstRow->Add(TDPitchContainer, 1, wxGROW|wxALL, 5);

  // Label for the autodetected pitch frequency
  wxStaticText *td_pitchLabel = new wxStaticText ( 
    this, 
    wxID_STATIC,
    wxEmptyString, 
    wxDefaultPosition, 
    wxDefaultSize, 
    0
  );
  td_pitchLabel->SetLabel(wxString::Format(wxT("Detected pitch: %.2f Hz"), m_TDdetectedPitch));
  TDPitchContainer->Add(td_pitchLabel, 1, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 2);

  // Label for the calculated MIDIUnityNote
  wxStaticText *td_midiNoteLabel = new wxStaticText ( 
    this, 
    wxID_STATIC,
    wxEmptyString, 
    wxDefaultPosition, 
    wxDefaultSize, 
    0
  );
  td_midiNoteLabel->SetLabel(wxString::Format(wxT("MIDIUnityNote: %d"), m_TDdetectedMIDIUnityNote));
  TDPitchContainer->Add(td_midiNoteLabel, 1, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 2);

  // Label for the calculated MIDIPitchFraction
  wxStaticText *td_pitchFractionLabel = new wxStaticText ( 
    this, 
    wxID_STATIC,
    wxEmptyString, 
    wxDefaultPosition, 
    wxDefaultSize, 
    0
  );
  td_pitchFractionLabel->SetLabel(wxString::Format(wxT("PitchFraction: %.2f cent"), m_TDdetectedMIDIPitchFraction));
  TDPitchContainer->Add(td_pitchFractionLabel, 1, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 2);

  // Horizontal sizer for selection row
  wxBoxSizer* selectionRow = new wxBoxSizer(wxHORIZONTAL);
  boxSizer->Add(selectionRow, 0, wxGROW|wxALL, 5);

  // Checkbox for using FFT pitch information
  wxRadioBox *pitchMethod = new wxRadioBox(
    this,
    ID_PITCH_METHOD,
    wxT("Pitch method to use"),
    wxDefaultPosition,
    wxDefaultSize,
    pitchMethods,
    1,
    wxRA_SPECIFY_ROWS
  );
  pitchMethod->Enable(0, true);
  selectionRow->Add(pitchMethod, 1, wxGROW|wxALL, 2);

  // Horizontal sizer for second row
  wxBoxSizer *secondRow = new wxBoxSizer(wxHORIZONTAL);
  boxSizer->Add(secondRow, 1, wxGROW|wxALL, 5);

  // Grouping of existing/manual pitch information
  wxStaticBox *existingPitchBox = new wxStaticBox(
    this, 
    wxID_STATIC,
    wxT("Existing/Manual pitch information"), 
    wxDefaultPosition, 
    wxDefaultSize
  );

  // Vertical sizer for existing pitch subsections
  wxStaticBoxSizer *filePitchContainer = new wxStaticBoxSizer(existingPitchBox, wxVERTICAL);
  secondRow->Add(filePitchContainer, 1, wxGROW|wxALL, 5);

  // A horizontal box sizer MIDIUnityNote information
  wxBoxSizer* unityNoteRow = new wxBoxSizer(wxHORIZONTAL);
  filePitchContainer->Add(unityNoteRow, 1, wxGROW|wxALL, 5);

  // Label for the unity note
  wxStaticText *unityNoteLabel = new wxStaticText ( 
    this, 
    wxID_STATIC,
    wxT("MIDIUnityNote: "), 
    wxDefaultPosition, 
    wxDefaultSize, 
    0
  );
  unityNoteRow->Add(unityNoteLabel, 1, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  // Combobox for showing/setting MIDIUnityNote 
  wxComboBox *noteCombo = new wxComboBox(
    this,
    ID_NOTECOMBO,
    wxT("0"),
    wxDefaultPosition,
    wxDefaultSize,
    m_notenumbers,
    wxCB_READONLY
  );
  noteCombo->SetValue(wxString::Format(wxT("%d"), m_fileMIDIUnityNote));
  noteCombo->Enable(false);
  unityNoteRow->Add(noteCombo, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  // A horizontal box sizer MIDIPitchFraction information
  wxBoxSizer* fractionRow = new wxBoxSizer(wxHORIZONTAL);
  filePitchContainer->Add(fractionRow, 1, wxGROW|wxALL, 5);

  // Label for the pitch fraction
  fractionLabel = new wxStaticText ( 
    this, 
    wxID_STATIC,
    wxEmptyString, 
    wxDefaultPosition, 
    wxDefaultSize, 
    0
  );
  fractionLabel->SetLabel(wxString::Format(wxT("PitchFraction: %.2f cent"), m_fileMIDIPitchFraction));
  fractionRow->Add(fractionLabel, 1, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  // A slider for setting pitchfraction value
  wxSlider *pitchFractionSlider = new wxSlider ( 
    this, 
    ID_PITCHFRACTION,
    0,
    0,
    9999,
    wxDefaultPosition, 
    wxDefaultSize, 
    wxSL_HORIZONTAL
  );
  pitchFractionSlider->SetValue((int) (m_fileMIDIPitchFraction * 100));
  pitchFractionSlider->Enable(false);
  fractionRow->Add(pitchFractionSlider, 2, wxGROW|wxALL, 5);

  // A horizontal box sizer for displaying resulting pitch
  wxBoxSizer* pitchRow = new wxBoxSizer(wxHORIZONTAL);
  filePitchContainer->Add(pitchRow, 1, wxGROW|wxALL, 5);

  // Label for the resulting pitch
  resultingPitchLabel = new wxStaticText ( 
    this, 
    wxID_STATIC,
    wxEmptyString, 
    wxDefaultPosition, 
    wxDefaultSize, 
    0
  );
  resultingPitchLabel->SetLabel(wxString::Format(wxT("Resulting pitch: %.2f Hz"), m_resultingPitch));
  pitchRow->Add(resultingPitchLabel, 1, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  // A horizontal box sizer for the third row
  wxBoxSizer* thirdRow = new wxBoxSizer(wxHORIZONTAL);
  boxSizer->Add(thirdRow, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  // The OK button
  wxButton* okButton = new wxButton(
    this, 
    wxID_OK, 
    wxT("&OK"),
    wxDefaultPosition, 
    wxDefaultSize, 
    0
  );
  thirdRow->Add(okButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  // The Cancel button
  wxButton* cancelButton = new wxButton(
    this, 
    wxID_CANCEL,
    wxT("&Cancel"), 
    wxDefaultPosition, 
    wxDefaultSize, 
    0
  );
  thirdRow->Add(cancelButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
}

// Override of transfer data to the window
bool PitchDialog::TransferDataToWindow() {
  wxComboBox *midinote = (wxComboBox*) FindWindow(ID_NOTECOMBO);
  wxSlider *pitchFract = (wxSlider*) FindWindow(ID_PITCHFRACTION);

  midinote->SetValue(wxString::Format(wxT("%i"), m_fileMIDIUnityNote));
  pitchFract->SetValue((int) m_fileMIDIPitchFraction * 100);

  return true;
}

// Override of transfer data from the window
bool PitchDialog::TransferDataFromWindow() {
  wxComboBox *midinote = (wxComboBox*) FindWindow(ID_NOTECOMBO);
  wxSlider *pitchFract = (wxSlider*) FindWindow(ID_PITCHFRACTION);

  m_fileMIDIUnityNote = wxAtoi(midinote->GetValue());
  m_fileMIDIPitchFraction = (double) pitchFract->GetValue() / 100.0;

  return true;
}

int PitchDialog::GetMethodUsed() {
  if (m_useFFTDetection)
    return 0;
  if (m_useTDDetection)
    return 1;
  if (m_useManual)
    return 2;
}

void PitchDialog::OnAutoDetectionCheck(wxCommandEvent& event) {
  wxRadioBox *radioBox = (wxRadioBox*) FindWindow(ID_PITCH_METHOD);
  wxComboBox *midinote = (wxComboBox*) FindWindow(ID_NOTECOMBO);
  wxSlider *pitchFract = (wxSlider*) FindWindow(ID_PITCHFRACTION);

  if (radioBox->GetSelection() == 0) {
    // FFT method chosen
    m_useFFTDetection = true;
    m_useTDDetection = false;
    m_useManual = false;
    midinote->Enable(false);
    pitchFract->Enable(false);
  } else if (radioBox->GetSelection() == 1) {
    // Timedomain method chosen
    m_useFFTDetection = false;
    m_useTDDetection = true;
    m_useManual = false;
    midinote->Enable(false);
    pitchFract->Enable(false);
  } else {
    // Existing/manual method chosen
    m_useFFTDetection = false;
    m_useTDDetection = false;
    m_useManual = true;
    midinote->Enable(true);
    pitchFract->Enable(true);
  }
}

void PitchDialog::OnNoteChange(wxCommandEvent& event) {
  wxComboBox *midinote = (wxComboBox*) FindWindow(ID_NOTECOMBO);
  m_fileMIDIUnityNote = wxAtoi(midinote->GetValue());

  CalculatingResultingPitch();
  resultingPitchLabel->SetLabel(wxString::Format(wxT("Resulting pitch: %.2f Hz"), m_resultingPitch));
}

void PitchDialog::OnFractionChange(wxCommandEvent& event) {
  wxSlider *pitchFract = (wxSlider*) FindWindow(ID_PITCHFRACTION);
  m_fileMIDIPitchFraction = (double) pitchFract->GetValue() / 100.0;
  fractionLabel->SetLabel(wxString::Format(wxT("PitchFraction: %.2f cent"), m_fileMIDIPitchFraction));

  CalculatingResultingPitch();
  resultingPitchLabel->SetLabel(wxString::Format(wxT("Resulting pitch: %.2f Hz"), m_resultingPitch));
}

int PitchDialog::GetMIDINote() {
  return m_fileMIDIUnityNote;
}

double PitchDialog::GetPitchFraction() {
  return m_fileMIDIPitchFraction;
}

void PitchDialog::CalculatingResultingPitch() {
  double midi_note_pitch = 440.0 * pow(2, ((double)(m_fileMIDIUnityNote - 69) / 12.0));
  m_resultingPitch = midi_note_pitch * pow(2, (m_fileMIDIPitchFraction / 1200.0));
}

