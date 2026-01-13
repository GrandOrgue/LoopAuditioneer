/* 
 * SpectrumDialog.cpp is a part of LoopAuditioneer software
 * Copyright (C) 2026 Lars Palo and contributors (see AUTHORS file) 
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

#include "SpectrumDialog.h"
#include <wx/statline.h>
#include "PitchDialog.h"

IMPLEMENT_CLASS(SpectrumDialog, wxDialog)

BEGIN_EVENT_TABLE(SpectrumDialog, wxDialog)
  EVT_BUTTON(ID_ZOOM_ALL_BTN, SpectrumDialog::OnZoomAllButton)
  EVT_BUTTON(ID_ZOOM_OUT_BTN, SpectrumDialog::OnZoomOutButton)
  EVT_BUTTON(ID_ZOOM_IN_BTN, SpectrumDialog::OnZoomInButton)
  EVT_BUTTON(ID_ZOOM_SEL_BTN, SpectrumDialog::OnZoomSelection)
  EVT_SLIDER(ID_ZOOM_SLIDER, SpectrumDialog::OnZoomSlider)
  EVT_CHECKBOX(ID_PITCH_INTERPOLATION_CHECK, SpectrumDialog::OnPitchInterpolationCheck)
END_EVENT_TABLE()

SpectrumDialog::SpectrumDialog(double *fftData, unsigned fftSize, wxString fileName, unsigned samplerate) {
  Init(fftData, fftSize, fileName, samplerate);
}

SpectrumDialog::SpectrumDialog(
  double *fftData,
  unsigned fftSize,
  wxString fileName,
  unsigned samplerate,
  wxWindow* parent,
  wxWindowID id,
  const wxString& title,
  const wxPoint& pos,
  const wxSize& size,
  long style) {
  Init(fftData, fftSize, fileName, samplerate);
  Create(parent, id, title, pos, size, style);
}

SpectrumDialog::~SpectrumDialog() {

}

void SpectrumDialog::Init(double *fftData, unsigned fftSize, wxString fileName, unsigned samplerate) {
  m_fftData = fftData;
  m_fftSize = fftSize;
  m_fileName = fileName;
  m_sampleRate = samplerate;
}

bool SpectrumDialog::Create(
  wxWindow* parent,
  wxWindowID id,
  const wxString& caption,
  const wxPoint& pos,
  const wxSize& size,
  long style) {
  if (!wxDialog::Create(parent, id, caption, pos, size, style))
    return false;

  CreateControls();
  SetTitle(wxT("Power spectrum for ") + m_fileName);

  DecideOkButtonState();
  DecideZoomButtonState();

  GetSizer()->Fit(this);
  GetSizer()->SetSizeHints(this);
  Centre();

  return true;
}

void SpectrumDialog::CreateControls() {
  // Create a top level sizer
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  // Sizer for the topmost row
  wxBoxSizer *m_topRow = new wxBoxSizer(wxHORIZONTAL);

  // The drawing panel that show both fft power spectrum and pitch/file details
  m_drawingPanel = new SpectrumPanel(m_fftData, m_fftSize, m_fileName, m_sampleRate, this);
  m_topRow->Add(m_drawingPanel, 1, wxEXPAND);

  topSizer->Add(m_topRow, 1, wxEXPAND);

  // Sizer for the zoom button row
  wxBoxSizer *m_zoomRow = new wxBoxSizer(wxHORIZONTAL);

  // The zoom all button
  m_zoomAllBtn = new wxButton(
    this,
    ID_ZOOM_ALL_BTN,
    wxT("All"),
    wxDefaultPosition,
    wxDefaultSize,
    0
  );
  m_zoomRow->Add(m_zoomAllBtn, 0, wxALIGN_CENTER|wxALL, 5);

  // The zoom out button
  m_zoomOutBtn = new wxButton(
    this,
    ID_ZOOM_OUT_BTN,
    wxT("Out"),
    wxDefaultPosition,
    wxDefaultSize,
    0
  );
  m_zoomRow->Add(m_zoomOutBtn, 0, wxALIGN_CENTER|wxALL, 5);

  // The zoom in button
  m_zoomInBtn = new wxButton(
    this,
    ID_ZOOM_IN_BTN,
    wxT("In"),
    wxDefaultPosition,
    wxDefaultSize,
    0
  );
  m_zoomRow->Add(m_zoomInBtn, 0, wxALIGN_CENTER|wxALL, 5);

  // The zoom selection button
  m_zoomSelectionBtn = new wxButton(
    this,
    ID_ZOOM_SEL_BTN,
    wxT("Selection"),
    wxDefaultPosition,
    wxDefaultSize,
    0
  );
  m_zoomRow->Add(m_zoomSelectionBtn, 0, wxALIGN_CENTER|wxALL, 5);

  m_zoomSlider = new wxSlider(
    this,
    ID_ZOOM_SLIDER,
    m_sampleRate / 4,
    0,
    m_sampleRate / 2
  );
  m_zoomRow->Add(m_zoomSlider, 1, wxEXPAND|wxALL, 5);

  topSizer->Add(m_zoomRow, 0, wxEXPAND);

  wxStaticLine *bottomDivider = new wxStaticLine(this);
  topSizer->Add(bottomDivider, 0, wxEXPAND);

  wxBoxSizer *lastRow = new wxBoxSizer(wxHORIZONTAL);

  m_interpolatePitchCheck = new wxCheckBox(
    this,
    ID_PITCH_INTERPOLATION_CHECK,
    wxT("Interpolate pitch")
  );
  lastRow->Add(m_interpolatePitchCheck, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
  m_interpolatePitchCheck->SetValue(m_drawingPanel->GetUsePitchInterpolation());

  lastRow->AddStretchSpacer();
  wxButton *usePitchButton = new wxButton(
    this,
    wxID_OK,
    wxT("Transfer selected pitch")
  );
  lastRow->Add(usePitchButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
  lastRow->AddStretchSpacer();
  wxButton *cancelButton = new wxButton(
    this,
    wxID_CANCEL,
    wxT("Cancel")
  );
  lastRow->Add(cancelButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
  lastRow->AddStretchSpacer();

  topSizer->Add(lastRow, 0, wxGROW);

  SetSizer(topSizer);
}

double SpectrumDialog::GetSelectedPitch() {
  return m_drawingPanel->GetSelectedPitch();
}

void SpectrumDialog::PitchSelectionHasChanged() {
  DecideOkButtonState();
}

void SpectrumDialog::SetInterpolatePitchOption(bool interpolate) {
  m_interpolatePitchCheck->SetValue(interpolate);
  m_drawingPanel->SetPitchInterpolation(m_interpolatePitchCheck->GetValue());
}

void SpectrumDialog::DecideOkButtonState() {
  wxButton *theOkBtn = (wxButton*) FindWindow(wxID_OK);
  if (m_drawingPanel->HasPitchSelection()) {
    theOkBtn->Enable();
  } else {
    theOkBtn->Disable();
  }

  if (m_drawingPanel->HasSelection()) {
    m_zoomSelectionBtn->Enable();
  } else {
    m_zoomSelectionBtn->Disable();
  }
}

void SpectrumDialog::DecideZoomButtonState() {
  if (m_drawingPanel->GetZoomLevel() > 0) {
    m_zoomAllBtn->Enable();
    m_zoomOutBtn->Enable();
    m_zoomSlider->Enable();
  } else {
    m_zoomAllBtn->Disable();
    m_zoomOutBtn->Disable();
    m_zoomSlider->Disable();
  }
  if (m_drawingPanel->HasSelection()) {
    m_zoomSelectionBtn->Enable();
  } else {
    m_zoomSelectionBtn->Disable();
  }
  if (m_drawingPanel->GetHasCustomZoom()) {
    m_zoomAllBtn->Enable();
    m_zoomSlider->Enable();
    m_zoomOutBtn->Disable();
    m_zoomInBtn->Disable();
  } else {
    m_zoomInBtn->Enable();
  }
}

void SpectrumDialog::OnZoomAllButton(wxCommandEvent& WXUNUSED(event)) {
  m_drawingPanel->DoZoomAll();
  m_zoomSlider->SetValue(m_drawingPanel->GetPossibleSliderPosition());
  DecideZoomButtonState();
}

void SpectrumDialog::OnZoomInButton(wxCommandEvent& WXUNUSED(event)) {
  m_drawingPanel->DoZoomIn();
  DecideZoomButtonState();
}

void SpectrumDialog::OnZoomOutButton(wxCommandEvent& WXUNUSED(event)) {
  m_drawingPanel->DoZoomOut();
  m_zoomSlider->SetValue(m_drawingPanel->GetPossibleSliderPosition());
  DecideZoomButtonState();
}

void SpectrumDialog::OnZoomSelection(wxCommandEvent& WXUNUSED(event)) {
  m_drawingPanel->DoZoomSelection();
  DecideZoomButtonState();
  m_zoomSlider->SetValue(m_drawingPanel->GetPossibleSliderPosition());
}

void SpectrumDialog::OnZoomSlider(wxCommandEvent& WXUNUSED(event)) {
  int sliderValue = m_zoomSlider->GetValue();
  m_drawingPanel->SetCurrentMidHz(sliderValue);
  m_zoomSlider->SetValue(m_drawingPanel->GetPossibleSliderPosition());
}

void SpectrumDialog::OnPitchInterpolationCheck(wxCommandEvent& WXUNUSED(event)) {
  m_drawingPanel->SetPitchInterpolation(m_interpolatePitchCheck->GetValue());
  PitchDialog *myParent = (PitchDialog*) GetParent();
  myParent->SetPreferredInterpolatePitch(m_interpolatePitchCheck->GetValue());
}

