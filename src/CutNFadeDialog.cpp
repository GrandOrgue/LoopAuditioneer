/* 
 * CutNFadeDialog.cpp is a part of LoopAuditioneer software
 * Copyright (C) 2014-2021 Lars Palo 
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

#include "CutNFadeDialog.h"
#include "wx/statline.h"

IMPLEMENT_CLASS(CutNFadeDialog, wxDialog )

BEGIN_EVENT_TABLE(CutNFadeDialog, wxDialog)
  EVT_SPINCTRL(ID_CUTSTART, CutNFadeDialog::OnCutStartSpin)
  EVT_SPINCTRL(ID_CUTEND, CutNFadeDialog::OnCutEndSpin)
  EVT_SPINCTRL(ID_FADESTART, CutNFadeDialog::OnFadeStartSpin)
  EVT_SPINCTRL(ID_FADEEND, CutNFadeDialog::OnFadeEndSpin)
END_EVENT_TABLE()

CutNFadeDialog::CutNFadeDialog() {
  Init();
}

CutNFadeDialog::CutNFadeDialog(
  wxWindow* parent,
  wxWindowID id,
  const wxString& caption,
  const wxPoint& pos,
  const wxSize& size,
  long style
) {
  Init();
  Create(parent, id, caption, pos, size, style);
}
 
void CutNFadeDialog::Init() {
  m_cutStart = 0;
  m_cutEnd = 0;
  m_fadeStart = 0;
  m_fadeEnd = 0;
}

bool CutNFadeDialog::Create( 
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

void CutNFadeDialog::CreateControls() {
  // Create a top level sizer
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  this->SetSizer(topSizer);

  // Second box sizer to get nice margins
  wxBoxSizer *boxSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(boxSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  // Information message at top
  wxStaticText* info = new wxStaticText(
    this, 
    wxID_STATIC,
    wxT("All values in milliseconds. Value 0 means don't! Cuts are done before fades."), 
    wxDefaultPosition,
    wxDefaultSize, 
    0
  );
  boxSizer->Add(info, 0, wxALIGN_LEFT|wxALL, 5);

  // Spacer
  boxSizer->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  // First row
  wxBoxSizer* firstRow = new wxBoxSizer(wxHORIZONTAL);
  boxSizer->Add(firstRow, 0, wxGROW|wxALL, 5);

  // Label for the cut start
  wxStaticText* cutStartLabel = new wxStaticText ( 
    this, 
    wxID_STATIC,
    wxT("Cut from beginning:"), 
    wxDefaultPosition, 
    wxDefaultSize, 
    0 
  );
  firstRow->Add(cutStartLabel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  // A spin control for the cut start value
  wxSpinCtrl* cutStartSpin = new wxSpinCtrl ( 
    this, 
    ID_CUTSTART,
    wxEmptyString, 
    wxDefaultPosition, 
    wxDefaultSize,
    wxSP_ARROW_KEYS, 
    0, 
    2000, 
    m_cutStart 
  );
  firstRow->Add(cutStartSpin, 0, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxALL, 5);

  // Second row
  wxBoxSizer* secondRow = new wxBoxSizer(wxHORIZONTAL);
  boxSizer->Add(secondRow, 0, wxGROW|wxALL, 5);

  // Label for the fade in
  wxStaticText* fadeStartLabel = new wxStaticText ( 
    this, 
    wxID_STATIC,
    wxT("Fade in:"), 
    wxDefaultPosition, 
    wxDefaultSize, 
    0 
  );
  secondRow->Add(fadeStartLabel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  // A spin control for the fade start value
  wxSpinCtrl* fadeStartSpin = new wxSpinCtrl ( 
    this, 
    ID_FADESTART,
    wxEmptyString, 
    wxDefaultPosition, 
    wxDefaultSize,
    wxSP_ARROW_KEYS, 
    0, 
    2000, 
    m_fadeStart 
  );
  secondRow->Add(fadeStartSpin, 0, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxALL, 5);

  // Third row
  wxBoxSizer* thirdRow = new wxBoxSizer(wxHORIZONTAL);
  boxSizer->Add(thirdRow, 0, wxGROW|wxALL, 5);

  // Label for the cut start
  wxStaticText* cutEndLabel = new wxStaticText ( 
    this, 
    wxID_STATIC,
    wxT("Cut from end:"), 
    wxDefaultPosition, 
    wxDefaultSize, 
    0 
  );
  thirdRow->Add(cutEndLabel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  // A spin control for the cut start value
  wxSpinCtrl* cutEndSpin = new wxSpinCtrl ( 
    this, 
    ID_CUTEND,
    wxEmptyString, 
    wxDefaultPosition, 
    wxDefaultSize,
    wxSP_ARROW_KEYS, 
    0, 
    2000, 
    m_cutEnd 
  );
  thirdRow->Add(cutEndSpin, 0, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxALL, 5);

  // Fourth row
  wxBoxSizer* fourthRow = new wxBoxSizer(wxHORIZONTAL);
  boxSizer->Add(fourthRow, 0, wxGROW|wxALL, 5);

  // Label for the fade out
  wxStaticText* fadeEndLabel = new wxStaticText ( 
    this, 
    wxID_STATIC,
    wxT("Fade out:"), 
    wxDefaultPosition, 
    wxDefaultSize, 
    0 
  );
  fourthRow->Add(fadeEndLabel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  // A spin control for the fade end value
  wxSpinCtrl* fadeEndSpin = new wxSpinCtrl ( 
    this, 
    ID_FADEEND,
    wxEmptyString, 
    wxDefaultPosition, 
    wxDefaultSize,
    wxSP_ARROW_KEYS, 
    0, 
    2000, 
    m_fadeEnd 
  );
  fourthRow->Add(fadeEndSpin, 0, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxALL, 5);

  // A horizontal line before the OK and Cancel buttons
  wxStaticLine *line = new wxStaticLine(
    this, 
    wxID_STATIC,
    wxDefaultPosition, 
    wxDefaultSize, 
    wxLI_HORIZONTAL
  );
  boxSizer->Add(line, 0, wxGROW|wxALL, 5);

  // A horizontal box sizer for the third row
  wxBoxSizer* fifthRow = new wxBoxSizer(wxHORIZONTAL);
  boxSizer->Add(fifthRow, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  // The OK button
  wxButton* okButton = new wxButton(
    this, 
    wxID_OK, 
    wxT("&OK"),
    wxDefaultPosition, 
    wxDefaultSize, 
    0
  );
  fifthRow->Add(okButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  // The Cancel button
  wxButton* cancelButton = new wxButton(
    this, 
    wxID_CANCEL,
    wxT("&Cancel"), 
    wxDefaultPosition, 
    wxDefaultSize, 
    0
  );
  fifthRow->Add(cancelButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
}

void CutNFadeDialog::SetCutStart(unsigned cutStart) {
  m_cutStart = cutStart;
}

unsigned CutNFadeDialog::GetCutStart() {
  return m_cutStart;
}

void CutNFadeDialog::SetCutEnd(unsigned cutEnd) {
  m_cutEnd = cutEnd;
}

unsigned CutNFadeDialog::GetCutEnd() {
  return m_cutEnd;
}

void CutNFadeDialog::SetFadeStart(unsigned fadeStart) {
  m_fadeStart = fadeStart;
}

unsigned CutNFadeDialog::GetFadeStart() {
  return m_fadeStart;
}

void CutNFadeDialog::SetFadeEnd(unsigned fadeEnd) {
  m_fadeEnd = fadeEnd;
}

unsigned CutNFadeDialog::GetFadeEnd() {
  return m_fadeEnd;
}

// Override of transfer data to the window
bool CutNFadeDialog::TransferDataToWindow() {
  wxSpinCtrl *cutStartCtrl = (wxSpinCtrl*) FindWindow(ID_CUTSTART);
  wxSpinCtrl *cutEndCtrl = (wxSpinCtrl*) FindWindow(ID_CUTEND);
  wxSpinCtrl *fadeStartCtrl = (wxSpinCtrl*) FindWindow(ID_FADESTART);
  wxSpinCtrl *fadeEndCtrl = (wxSpinCtrl*) FindWindow(ID_FADEEND);

  cutStartCtrl->SetValue(m_cutStart);
  cutEndCtrl->SetValue(m_cutEnd);
  fadeStartCtrl->SetValue(m_fadeStart);
  fadeEndCtrl->SetValue(m_fadeEnd);

  return true;
}

// Override of transfer data from the window
bool CutNFadeDialog::TransferDataFromWindow() {
  wxSpinCtrl *cutStartCtrl = (wxSpinCtrl*) FindWindow(ID_CUTSTART);
  wxSpinCtrl *cutEndCtrl = (wxSpinCtrl*) FindWindow(ID_CUTEND);
  wxSpinCtrl *fadeStartCtrl = (wxSpinCtrl*) FindWindow(ID_FADESTART);
  wxSpinCtrl *fadeEndCtrl = (wxSpinCtrl*) FindWindow(ID_FADEEND);

  m_cutStart = cutStartCtrl->GetValue();
  m_cutEnd = cutEndCtrl->GetValue();
  m_fadeStart = fadeStartCtrl->GetValue();
  m_fadeEnd = fadeEndCtrl->GetValue();

  return true;
}

void CutNFadeDialog::OnCutStartSpin(wxSpinEvent& event) {
  if (event.GetPosition() < 0) {
    wxSpinCtrl *cutStartCtrl = (wxSpinCtrl*) FindWindow(ID_CUTSTART);
    cutStartCtrl->SetValue(0);
  } else {
    wxSpinCtrl *cutStartCtrl = (wxSpinCtrl*) FindWindow(ID_CUTSTART);
    cutStartCtrl->SetValue(event.GetPosition());
  }
}

void CutNFadeDialog::OnCutEndSpin(wxSpinEvent& event) {
  if (event.GetPosition() < 0) {
    wxSpinCtrl *cutEndCtrl = (wxSpinCtrl*) FindWindow(ID_CUTEND);
    cutEndCtrl->SetValue(0);
  } else {
    wxSpinCtrl *cutEndCtrl = (wxSpinCtrl*) FindWindow(ID_CUTEND);
    cutEndCtrl->SetValue(event.GetPosition());
  }
}

void CutNFadeDialog::OnFadeStartSpin(wxSpinEvent& event) {
  if (event.GetPosition() < 0) {
    wxSpinCtrl *fadeStartCtrl = (wxSpinCtrl*) FindWindow(ID_FADESTART);
    fadeStartCtrl->SetValue(0);
  } else {
    wxSpinCtrl *fadeStartCtrl = (wxSpinCtrl*) FindWindow(ID_FADESTART);
    fadeStartCtrl->SetValue(event.GetPosition());
  }
}

void CutNFadeDialog::OnFadeEndSpin(wxSpinEvent& event) {
  if (event.GetPosition() < 0) {
    wxSpinCtrl *fadeEndCtrl = (wxSpinCtrl*) FindWindow(ID_FADEEND);
    fadeEndCtrl->SetValue(0);
  } else {
    wxSpinCtrl *fadeEndCtrl = (wxSpinCtrl*) FindWindow(ID_FADEEND);
    fadeEndCtrl->SetValue(event.GetPosition());
  }
}
