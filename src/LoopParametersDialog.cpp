/* 
 * LoopParametersDialog.cpp is a part of LoopAuditioneer software
 * Copyright (C) 2011-2021 Lars Palo 
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

#include "LoopParametersDialog.h"
#include "wx/statline.h"

IMPLEMENT_CLASS(LoopParametersDialog, wxDialog )

BEGIN_EVENT_TABLE(LoopParametersDialog, wxDialog)
  EVT_SPINCTRL(ID_LOOPSTART, LoopParametersDialog::OnLoopStartSpin)
  EVT_SPINCTRL(ID_LOOPEND, LoopParametersDialog::OnLoopEndSpin)
END_EVENT_TABLE()

LoopParametersDialog::LoopParametersDialog(unsigned int start, unsigned int end, unsigned int length) {
  Init(start, end, length);
}

LoopParametersDialog::LoopParametersDialog(
  unsigned int start, 
  unsigned int end,
  unsigned int length,
  wxWindow* parent,
  wxWindowID id,
  const wxString& caption,
  const wxPoint& pos,
  const wxSize& size,
  long style
) {
  Init(start, end, length);
  Create(parent, id, caption, pos, size, style);
}
 
void LoopParametersDialog::Init(unsigned int start, unsigned int end, unsigned int length) {
  m_loopStart = start;
  m_loopEnd = end;
  m_lastSample = length;
}

bool LoopParametersDialog::Create( 
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

void LoopParametersDialog::CreateControls() {
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
    wxT("Enter loop start and end values in samples."), 
    wxDefaultPosition,
    wxDefaultSize, 
    0
  );
  boxSizer->Add(info, 0, wxALIGN_LEFT|wxALL, 5);

  // Spacer
  boxSizer->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  // Horizontal spacer for first row
  wxBoxSizer* firstRow = new wxBoxSizer(wxHORIZONTAL);
  boxSizer->Add(firstRow, 0, wxGROW|wxALL, 5);

  // Label for the loop start
  wxStaticText* loopStartLabel = new wxStaticText ( 
    this, 
    wxID_STATIC,
    wxT("Loop start:"), 
    wxDefaultPosition, 
    wxDefaultSize, 
    0 
  );
  firstRow->Add(loopStartLabel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  // A spin control for the loop start value
  wxSpinCtrl* loopStartSpin = new wxSpinCtrl ( 
    this, 
    ID_LOOPSTART,
    wxEmptyString, 
    wxDefaultPosition, 
    wxSize(100, -1),
    wxSP_ARROW_KEYS, 
    0, 
    m_lastSample - 1, 
    m_loopStart 
  );
  firstRow->Add(loopStartSpin, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  // Horizontal spacer for second row
  wxBoxSizer* secondRow = new wxBoxSizer(wxHORIZONTAL);
  boxSizer->Add(secondRow, 0, wxGROW|wxALL, 5);

  // Label for the loop end
  wxStaticText* loopEndLabel = new wxStaticText ( 
    this, 
    wxID_STATIC,
    wxT("Loop end:"), 
    wxDefaultPosition, 
    wxDefaultSize, 
    0 
  );
  secondRow->Add(loopEndLabel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  // A spin control for the loop end value
  wxSpinCtrl* loopEndSpin = new wxSpinCtrl ( 
    this, 
    ID_LOOPEND,
    wxEmptyString, 
    wxDefaultPosition, 
    wxSize(100, -1),
    wxSP_ARROW_KEYS, 
    0, 
    m_lastSample, 
    m_loopEnd 
  );
  secondRow->Add(loopEndSpin, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

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

void LoopParametersDialog::SetLoopStart(unsigned int loopStart) {
  m_loopStart = loopStart;
}

unsigned int LoopParametersDialog::GetLoopStart() {
  return m_loopStart;
}

void LoopParametersDialog::SetLoopEnd(unsigned int loopEnd) {
  m_loopEnd = loopEnd;
}

unsigned int LoopParametersDialog::GetLoopEnd() {
  return m_loopEnd;
}

void LoopParametersDialog::SetLastSample(unsigned int lastSample) {
  m_lastSample = lastSample;
}

// Override of transfer data to the window
bool LoopParametersDialog::TransferDataToWindow() {
  wxSpinCtrl *startCtrl = (wxSpinCtrl*) FindWindow(ID_LOOPSTART);
  wxSpinCtrl *endCtrl = (wxSpinCtrl*) FindWindow(ID_LOOPEND);

  startCtrl->SetValue(m_loopStart);
  endCtrl->SetValue(m_loopEnd);

  return true;
}

// Override of transfer data from the window
bool LoopParametersDialog::TransferDataFromWindow() {
  wxSpinCtrl *startCtrl = (wxSpinCtrl*) FindWindow(ID_LOOPSTART);
  wxSpinCtrl *endCtrl = (wxSpinCtrl*) FindWindow(ID_LOOPEND);

  startCtrl->GetValue();
  endCtrl->GetValue();

  return true;
}

void LoopParametersDialog::OnLoopStartSpin(wxSpinEvent& event) {
  if (event.GetPosition() < m_loopEnd  && event.GetPosition() < m_lastSample) {
    m_loopStart = event.GetPosition();
    wxSpinCtrl *startCtrl = (wxSpinCtrl*) FindWindow(ID_LOOPSTART);
    startCtrl->SetValue(event.GetPosition());
  } else {
    wxSpinCtrl *startCtrl = (wxSpinCtrl*) FindWindow(ID_LOOPSTART);
    startCtrl->SetValue(0);
  }
}

void LoopParametersDialog::OnLoopEndSpin(wxSpinEvent& event) {
  if (event.GetPosition() > m_loopStart && event.GetPosition() <= m_lastSample) {
    m_loopEnd = event.GetPosition();
    wxSpinCtrl *endCtrl = (wxSpinCtrl*) FindWindow(ID_LOOPEND);
    endCtrl->SetValue(event.GetPosition());
  } else {
    wxSpinCtrl *endCtrl = (wxSpinCtrl*) FindWindow(ID_LOOPEND);
    endCtrl->SetValue(m_lastSample);
  }
}

