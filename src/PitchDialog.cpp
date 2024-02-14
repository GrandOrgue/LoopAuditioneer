/* 
 * PitchDialog.cpp is a part of LoopAuditioneer software
 * Copyright (C) 2011-2024 Lars Palo 
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

#include "PitchDialog.h"
#include <wx/choice.h>
#include "SpectrumDialog.h"

IMPLEMENT_CLASS(PitchDialog, wxDialog )

BEGIN_EVENT_TABLE(PitchDialog, wxDialog)
  EVT_RADIOBOX(ID_PITCH_METHOD, PitchDialog::OnAutoDetectionCheck)
  EVT_COMBOBOX(ID_NOTECOMBO, PitchDialog::OnNoteChange)
  EVT_SLIDER(ID_PITCHFRACTION, PitchDialog::OnFractionChange)
  EVT_BUTTON(ID_SPECTRUM_BTN, PitchDialog::OnViewSpectrumButton)
END_EVENT_TABLE()

PitchDialog::PitchDialog(FileHandling *audioFile) {
  Init(audioFile);
}

PitchDialog::PitchDialog(
  FileHandling *audioFile,
  wxWindow* parent,
  wxWindowID id,
  const wxString& caption,
  const wxPoint& pos,
  const wxSize& size,
  long style
) {
  Init(audioFile);
  Create(parent, id, caption, pos, size, style);
}
 
void PitchDialog::Init(FileHandling *audioFile) {
  m_audioFile = audioFile;

  double midi_note_pitch;
  double hps_midi_note_pitch;
  double td_midi_note_pitch;

  double fftPitches[2];
  for (int i = 0; i < 2; i++)
    fftPitches[i] = 0;
  bool got_fftpitch = m_audioFile->GetFFTPitch(fftPitches);
  m_TDdetectedPitch = m_audioFile->GetTDPitch();
  m_fileMIDIUnityNote = (int) m_audioFile->m_loops->GetMIDIUnityNote();
  m_fileMIDIPitchFraction = (double) m_audioFile->m_loops->GetMIDIPitchFraction() / (double)UINT_MAX * 100.0;

  if (got_fftpitch) {
    // FFT detection
    m_detectedPitch = fftPitches[0];
    m_detectedMIDIUnityNote = (69 + 12 * (log10(fftPitches[0] / 440.0) / log10(2)));
    midi_note_pitch = 440.0 * pow(2, ((double)(m_detectedMIDIUnityNote - 69) / 12.0));
    m_detectedMIDIPitchFraction = 1200 * (log10(fftPitches[0] / midi_note_pitch) / log10(2));
    m_actualMIDIPitchFraction = ((double)UINT_MAX * (m_detectedMIDIPitchFraction / 100.0));

    m_hpsDetectedPitch = fftPitches[1];
    m_hpsDetectedMIDIUnityNote = (69 + 12 * (log10(fftPitches[1] / 440.0) / log10(2)));
    hps_midi_note_pitch = 440.0 * pow(2, ((double)(m_hpsDetectedMIDIUnityNote - 69) / 12.0));
    m_hpsDetectedMIDIPitchFraction = 1200 * (log10(fftPitches[1] / hps_midi_note_pitch) / log10(2));
    m_actualHpsMIDIPitchFraction = ((double)UINT_MAX * (m_hpsDetectedMIDIPitchFraction / 100.0));
  } else {
    m_detectedMIDIUnityNote = 0;
    midi_note_pitch = 0;
    m_detectedMIDIPitchFraction = 0;
    m_actualMIDIPitchFraction = 0;

    m_hpsDetectedMIDIUnityNote = 0;
    hps_midi_note_pitch = 0;
    m_hpsDetectedMIDIPitchFraction = 0;
    m_actualHpsMIDIPitchFraction = 0;
  }

  if (m_TDdetectedPitch != 0) {
    // TD detection
    m_TDdetectedMIDIUnityNote = (69 + 12 * (log10(m_TDdetectedPitch / 440.0) / log10(2)));
    td_midi_note_pitch = 440.0 * pow(2, ((double)(m_TDdetectedMIDIUnityNote - 69) / 12.0));
    m_TDdetectedMIDIPitchFraction = 1200 * (log10(m_TDdetectedPitch / td_midi_note_pitch) / log10(2));
    m_actualTdMIDIPitchFraction = ((double)UINT_MAX * (m_TDdetectedMIDIPitchFraction / 100.0));
  } else {
    m_TDdetectedMIDIUnityNote = 0;
    td_midi_note_pitch = 0;
    m_TDdetectedMIDIPitchFraction = 0;
    m_actualTdMIDIPitchFraction = 0;
  }

  CalculatingResultingPitch();
  m_useFFTDetection = true;
  m_useHpsFFTDetection = false;
  m_useTDDetection = false;
  m_useManual = false;

  pitchMethods.Add(wxT("FFT pitch"));
  pitchMethods.Add(wxT("HPS pitch"));
  pitchMethods.Add(wxT("Timedomain pitch"));
  pitchMethods.Add(wxT("Existing/manual pitch"));

  for (int i = 0; i < 128; i++)
    m_notenumbers.Add(wxString::Format(wxT("%d"), i));

  m_fftSizes.Add(wxT("1024")); // pow(2, 10 + choice number)
  m_fftSizes.Add(wxT("2048"));
  m_fftSizes.Add(wxT("4096"));
  m_fftSizes.Add(wxT("8192"));
  m_fftSizes.Add(wxT("16384"));
  m_fftSizes.Add(wxT("32768"));
  m_fftSizes.Add(wxT("65536"));

  m_windowTypes.Add(wxT("Rectangular"));
  m_windowTypes.Add(wxT("Bartlett"));
  m_windowTypes.Add(wxT("Hamming"));
  m_windowTypes.Add(wxT("Hanning"));
  m_windowTypes.Add(wxT("Blackman"));
  m_windowTypes.Add(wxT("Blackman-Harris"));
  m_windowTypes.Add(wxT("Welch"));
  m_windowTypes.Add(wxT("Gaussian(a=2.5)"));
  m_windowTypes.Add(wxT("Gaussian(a=3.5)"));
  m_windowTypes.Add(wxT("Gaussian(a=4.5)"));
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

  // Horizontal sizer for fft detection methods
  wxStaticBoxSizer *fftPitchContainer = new wxStaticBoxSizer(FFTPitchBox, wxHORIZONTAL);
  firstRow->Add(fftPitchContainer, 2, wxGROW|wxALL, 5);

  // Vertical sizer for first fft pitch subsections
  wxBoxSizer *autoPitchContainer = new wxBoxSizer(wxVERTICAL);
  fftPitchContainer->Add(autoPitchContainer, 1, wxGROW|wxALL, 5);

  // Label for the autodetected pitch frequency
  wxStaticText *pitchLabel = new wxStaticText ( 
    this, 
    wxID_STATIC,
    wxEmptyString, 
    wxDefaultPosition, 
    wxDefaultSize, 
    0
  );
  pitchLabel->SetLabel(wxString::Format(wxT("FFT pitch: %.2f Hz"), m_detectedPitch));
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

  // Vertical sizer for second fft pitch subsections
  wxBoxSizer *hpsPitchContainer = new wxBoxSizer(wxVERTICAL);
  fftPitchContainer->Add(hpsPitchContainer, 1, wxGROW|wxALL, 5);

  // Label for the autodetected pitch frequency
  wxStaticText *hpsPitchLabel = new wxStaticText ( 
    this, 
    wxID_STATIC,
    wxEmptyString, 
    wxDefaultPosition, 
    wxDefaultSize, 
    0
  );
  hpsPitchLabel->SetLabel(wxString::Format(wxT("HPS pitch: %.2f Hz"), m_hpsDetectedPitch));
  hpsPitchContainer->Add(hpsPitchLabel, 1, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 2);

  // Label for the calculated MIDIUnityNote
  wxStaticText *hpsMidiNoteLabel = new wxStaticText ( 
    this, 
    wxID_STATIC,
    wxEmptyString, 
    wxDefaultPosition, 
    wxDefaultSize, 
    0
  );
  hpsMidiNoteLabel->SetLabel(wxString::Format(wxT("MIDIUnityNote: %d"), m_hpsDetectedMIDIUnityNote));
  hpsPitchContainer->Add(hpsMidiNoteLabel, 1, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 2);

  // Label for the calculated MIDIPitchFraction
  wxStaticText *hpsPitchFractionLabel = new wxStaticText ( 
    this, 
    wxID_STATIC,
    wxEmptyString, 
    wxDefaultPosition, 
    wxDefaultSize, 
    0
  );
  hpsPitchFractionLabel->SetLabel(wxString::Format(wxT("PitchFraction: %.2f cent"), m_hpsDetectedMIDIPitchFraction));
  hpsPitchContainer->Add(hpsPitchFractionLabel, 1, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 2);

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

  // Horizontal sizer for options to display FFT spectrum of whole file
  wxBoxSizer* spectrumRow = new wxBoxSizer(wxHORIZONTAL);
  boxSizer->Add(spectrumRow, 0, wxGROW|wxALL, 5);

  // The View Spectrum button
  wxButton *m_spectrumButton = new wxButton(
    this,
    ID_SPECTRUM_BTN,
    wxT("View FFT spectrum"),
    wxDefaultPosition,
    wxDefaultSize,
    0
  );
  spectrumRow->Add(m_spectrumButton, 0, wxALL, 5);
  
  // FFT size choice
  wxChoice *fftsizeChoice = new wxChoice(
    this,
    ID_FFTSIZE_CHOICE,
    wxDefaultPosition,
    wxDefaultSize,
    m_fftSizes
  );
  spectrumRow->Add(fftsizeChoice, 0, wxALL, 5);
  fftsizeChoice->SetSelection(5);

  // Window type choice
  wxChoice *windowChoice = new wxChoice(
    this,
    ID_WINDOW_TYPE_CHOICE,
    wxDefaultPosition,
    wxDefaultSize,
    m_windowTypes
  );
  spectrumRow->Add(windowChoice, 0, wxALL, 5);
  windowChoice->SetSelection(9);

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
    wxT("127"),
    wxDefaultPosition,
    wxDefaultSize,
    m_notenumbers,
    wxCB_READONLY
  );
  noteCombo->SetValue(wxString::Format(wxT("%d"), m_fileMIDIUnityNote));
  noteCombo->Enable(false);
  unityNoteRow->Add(noteCombo, 1, wxALIGN_CENTER_VERTICAL|wxALL, 0);

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

  // Set OK button as default
  okButton->SetDefault();
  okButton->SetFocus();
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

void PitchDialog::TransferSelectedPitchToFile() {
    int selectedMethod = GetMethodUsed();
    if (selectedMethod == 0) {
      m_audioFile->m_loops->SetMIDIUnityNote((char) m_detectedMIDIUnityNote);
      m_audioFile->m_loops->SetMIDIPitchFraction(m_actualMIDIPitchFraction);
    } else if (selectedMethod == 1) {
      m_audioFile->m_loops->SetMIDIUnityNote((char) m_hpsDetectedMIDIUnityNote);
      m_audioFile->m_loops->SetMIDIPitchFraction(m_actualHpsMIDIPitchFraction);
    } else if (selectedMethod == 2) {
      m_audioFile->m_loops->SetMIDIUnityNote((char) m_TDdetectedMIDIUnityNote);
      m_audioFile->m_loops->SetMIDIPitchFraction(m_actualTdMIDIPitchFraction);
    } else if (selectedMethod == 3) {
      m_audioFile->m_loops->SetMIDIUnityNote((char) GetMIDINote());
      m_audioFile->m_loops->SetMIDIPitchFraction((unsigned)((double)UINT_MAX * (GetPitchFraction() / 100.0)));
    }
}

int PitchDialog::GetMethodUsed() {
  if (m_useFFTDetection)
    return 0;
  if (m_useHpsFFTDetection)
    return 1;
  if (m_useTDDetection)
    return 2;
  if (m_useManual)
    return 3;
  else 
    return 0;
}

void PitchDialog::OnAutoDetectionCheck(wxCommandEvent& WXUNUSED(event)) {
  wxRadioBox *radioBox = (wxRadioBox*) FindWindow(ID_PITCH_METHOD);
  wxComboBox *midinote = (wxComboBox*) FindWindow(ID_NOTECOMBO);
  wxSlider *pitchFract = (wxSlider*) FindWindow(ID_PITCHFRACTION);

  if (radioBox->GetSelection() == 0) {
    // first FFT method chosen
    m_useFFTDetection = true;
    m_useHpsFFTDetection = false;
    m_useTDDetection = false;
    m_useManual = false;
    midinote->Enable(false);
    pitchFract->Enable(false);
  } else if (radioBox->GetSelection() == 1) {
    // third FFT method chosen
    m_useFFTDetection = false;
    m_useHpsFFTDetection = true;
    m_useTDDetection = false;
    m_useManual = false;
    midinote->Enable(false);
    pitchFract->Enable(false);
  } else if (radioBox->GetSelection() == 2) {
    // Timedomain method chosen
    m_useFFTDetection = false;
    m_useHpsFFTDetection = false;
    m_useTDDetection = true;
    m_useManual = false;
    midinote->Enable(false);
    pitchFract->Enable(false);
  } else {
    // Existing/manual method chosen
    m_useFFTDetection = false;
    m_useHpsFFTDetection = false;
    m_useTDDetection = false;
    m_useManual = true;
    midinote->Enable(true);
    pitchFract->Enable(true);
  }
}

void PitchDialog::OnNoteChange(wxCommandEvent& WXUNUSED(event)) {
  wxComboBox *midinote = (wxComboBox*) FindWindow(ID_NOTECOMBO);
  m_fileMIDIUnityNote = wxAtoi(midinote->GetValue());

  CalculatingResultingPitch();
  resultingPitchLabel->SetLabel(wxString::Format(wxT("Resulting pitch: %.2f Hz"), m_resultingPitch));
}

void PitchDialog::OnFractionChange(wxCommandEvent& WXUNUSED(event)) {
  wxSlider *pitchFract = (wxSlider*) FindWindow(ID_PITCHFRACTION);
  m_fileMIDIPitchFraction = (double) pitchFract->GetValue() / 100.0;
  fractionLabel->SetLabel(wxString::Format(wxT("PitchFraction: %.2f cent"), m_fileMIDIPitchFraction));

  CalculatingResultingPitch();
  resultingPitchLabel->SetLabel(wxString::Format(wxT("Resulting pitch: %.2f Hz"), m_resultingPitch));
}

void PitchDialog::OnViewSpectrumButton(wxCommandEvent& WXUNUSED(event)) {
  wxChoice *fftChoice = (wxChoice*) FindWindow(ID_FFTSIZE_CHOICE);
  wxChoice *windowChoice = (wxChoice*) FindWindow(ID_WINDOW_TYPE_CHOICE);

  int fftSize = pow(2, 10 + fftChoice->GetSelection());
  int windowType = windowChoice->GetSelection();
  int half = fftSize / 2;
  double *fftResult = new double[half];
  for (int i = 0; i < half; i++)
    fftResult[i] = 0;

  if (m_audioFile->GetSpectrum(fftResult, fftSize, windowType)) {
    // We now have the spectrum in the fftResult array in dB scaled so that 1.0 in amplitude would be 0 dB
    SpectrumDialog spectrumDlg(fftResult, fftSize, m_audioFile->GetFileName(), (unsigned) m_audioFile->GetSampleRate(), this);
    if (spectrumDlg.ShowModal() == wxID_OK) {
      // There should be a pitch to use for the manual pitch
      double pitch = spectrumDlg.GetSelectedPitch();
      wxRadioBox *radioBox = (wxRadioBox*) FindWindow(ID_PITCH_METHOD);
      radioBox->SetSelection(3);
      // then we notify the box that selection has changed
      wxCommandEvent evt(wxEVT_RADIOBOX, ID_PITCH_METHOD);
      wxPostEvent(this, evt);
      int midi_note = (69 + 12 * (log10(pitch / 440.0) / log10(2)));
      double midi_note_pitch = 440.0 * pow(2, ((double)(midi_note - 69) / 12.0));
      double cent_deviation = 1200 * (log10(pitch / midi_note_pitch) / log10(2));

      wxComboBox *midinote = (wxComboBox*) FindWindow(ID_NOTECOMBO);
      midinote->SetSelection(midi_note);
      m_fileMIDIUnityNote = midi_note;
      wxSlider *pitchFract = (wxSlider*) FindWindow(ID_PITCHFRACTION);
      pitchFract->SetValue((int) (cent_deviation * 100));
      m_fileMIDIPitchFraction = cent_deviation;
      fractionLabel->SetLabel(wxString::Format(wxT("PitchFraction: %.2f cent"), m_fileMIDIPitchFraction));
      CalculatingResultingPitch();
      resultingPitchLabel->SetLabel(wxString::Format(wxT("Resulting pitch: %.2f Hz"), m_resultingPitch));
    }
  } else {
    // Notify that it was not possible to get spectrum
    
  }

  delete[] fftResult;
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
