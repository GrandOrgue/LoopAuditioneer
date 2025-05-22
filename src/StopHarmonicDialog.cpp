/* 
 * StopHarmonicDialog.cpp is a part of LoopAuditioneer software
 * Copyright (C) 2012-2025 Lars Palo and contributors (see AUTHORS file) 
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

#include "StopHarmonicDialog.h"
#include <wx/statline.h>

IMPLEMENT_CLASS(StopHarmonicDialog, wxDialog )

BEGIN_EVENT_TABLE(StopHarmonicDialog, wxDialog)
  EVT_CHOICE(ID_HARMONICBOX, StopHarmonicDialog::OnChoiceSelected)
  EVT_SPINCTRLDOUBLE(ID_PITCHCTRL, StopHarmonicDialog::OnPitchChanged)
END_EVENT_TABLE()

StopHarmonicDialog::StopHarmonicDialog() {
  Init();
}

StopHarmonicDialog::StopHarmonicDialog(
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

void StopHarmonicDialog::Init() {
  m_harmoniclist.Add(wxT("Prime 64' (1)"));
  m_harmoniclist.Add(wxT("Octave 32' (2)"));
  m_harmoniclist.Add(wxT("Fifth 21 1/3' (3)"));
  m_harmoniclist.Add(wxT("Octave 16' (4)"));
  m_harmoniclist.Add(wxT("Major third 12 4/5' (5)"));
  m_harmoniclist.Add(wxT("Fifth 10 2/3' (6)"));
  m_harmoniclist.Add(wxT("Minor seventh 9 1/7' (7)"));
  m_harmoniclist.Add(wxT("Octave 8' (8)"));
  m_harmoniclist.Add(wxT("Major second 7 1/9' (9)"));
  m_harmoniclist.Add(wxT("Major third 6 2/5' (10)"));
  m_harmoniclist.Add(wxT("Tritone 5 9/11' (11)"));
  m_harmoniclist.Add(wxT("Fifth 5 1/3' (12)"));
  m_harmoniclist.Add(wxT("Minor sixth 4 12/13' (13)"));
  m_harmoniclist.Add(wxT("Minor seventh 4 4/7' (14)"));
  m_harmoniclist.Add(wxT("Major seventh 4 4/15' (15)"));
  m_harmoniclist.Add(wxT("Octave 4' (16)"));
  m_harmoniclist.Add(wxT("Minor second 3 13/17' (17)"));
  m_harmoniclist.Add(wxT("Major second 3 5/9' (18)"));
  m_harmoniclist.Add(wxT("Minor third 3 7/19' (19)"));
  m_harmoniclist.Add(wxT("Major third 3 1/5' (20)"));
  m_harmoniclist.Add(wxT("Fourth 3 1/21' (21)"));
  m_harmoniclist.Add(wxT("Tritone 2 10/11' (22)"));
  m_harmoniclist.Add(wxT("Tritone 2 18/23' (23)"));
  m_harmoniclist.Add(wxT("Fifth 2 2/3' (24)"));
  m_harmoniclist.Add(wxT("Minor sixth 2 14/25' (25)"));
  m_harmoniclist.Add(wxT("Minor sixth 2 6/13' (26)"));
  m_harmoniclist.Add(wxT("Major sixth 2 10/27' (27)"));
  m_harmoniclist.Add(wxT("Minor seventh 2 2/7' (28)"));
  m_harmoniclist.Add(wxT("Minor seventh 2 6/29' (29)"));
  m_harmoniclist.Add(wxT("Major seventh 2 2/15' (30)"));
  m_harmoniclist.Add(wxT("Major seventh 2 2/31' (31)"));
  m_harmoniclist.Add(wxT("Octave 2' (32)"));
  m_harmoniclist.Add(wxT("Minor second 1 /33' (33)"));
  m_harmoniclist.Add(wxT("Minor second 1 15/17' (34)"));
  m_harmoniclist.Add(wxT("1 29/35' (35)"));
  m_harmoniclist.Add(wxT("Major second 1 7/9' (36)"));
  m_harmoniclist.Add(wxT("1 27/37' (37)"));
  m_harmoniclist.Add(wxT("Minor third 1 13/19' (38)"));
  m_harmoniclist.Add(wxT("1 25/39' (39)"));
  m_harmoniclist.Add(wxT("Major third 1 3/5' (40)"));
  m_harmoniclist.Add(wxT("1 23/41' (41)"));
  m_harmoniclist.Add(wxT("Fourth 1 11/21' (42)"));
  m_harmoniclist.Add(wxT("1 21/43' (43)"));
  m_harmoniclist.Add(wxT("Tritone 1 5/11' (44)"));
  m_harmoniclist.Add(wxT("1 19/45' (45)"));
  m_harmoniclist.Add(wxT("1 9/23' (46)"));
  m_harmoniclist.Add(wxT("1 17/47' (47)"));
  m_harmoniclist.Add(wxT("Fifth 1 1/3' (48)"));
  m_harmoniclist.Add(wxT("1 15/49' (49)"));
  m_harmoniclist.Add(wxT("1 7/25' (50)"));
  m_harmoniclist.Add(wxT("1 13/51' (51)"));
  m_harmoniclist.Add(wxT("Minor sixth 1 3/13' (52)"));
  m_harmoniclist.Add(wxT("1 11/53' (53)"));
  m_harmoniclist.Add(wxT("Major sixth 1 5/27' (54)"));
  m_harmoniclist.Add(wxT("1 9/55' (55)"));
  m_harmoniclist.Add(wxT("Minor seventh 1 1/7' (56)"));
  m_harmoniclist.Add(wxT("1 7/57' (57)"));
  m_harmoniclist.Add(wxT("1 3/29' (58)"));
  m_harmoniclist.Add(wxT("1 5/59' (59)"));
  m_harmoniclist.Add(wxT("Major seventh 1 1/15' (60)"));
  m_harmoniclist.Add(wxT("1 3/61' (61)"));
  m_harmoniclist.Add(wxT("1 1/31' (62)"));
  m_harmoniclist.Add(wxT("1 1/63' (63)"));
  m_harmoniclist.Add(wxT("Octave 1' (64)"));

  m_selectedHarmonic = 8;
  m_pitch = 440;
}

bool StopHarmonicDialog::Create(
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

void StopHarmonicDialog::CreateControls() {
  // Create a top level sizer
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  this->SetSizer(topSizer);

  // Second box sizer to get nice margins
  wxBoxSizer *boxSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(boxSizer, 0, wxEXPAND|wxALL, 5);

  // Horizontal sizer for harmonic selection
  wxBoxSizer *selectionRow = new wxBoxSizer(wxHORIZONTAL);
  boxSizer->Add(selectionRow, 0, wxEXPAND|wxALL, 5);

  // Label for dropdown selection
  wxStaticText *selectionLabel = new wxStaticText(
    this,
    wxID_STATIC,
    wxT("Harmonic: "),
    wxDefaultPosition,
    wxSize(100,-1),
    0
  );
  selectionRow->Add(selectionLabel, 0, wxALL, 2);

  // Dropdown for selecting process
  m_harmonicChoiceBox = new wxChoice(
    this,
    ID_HARMONICBOX,
    wxDefaultPosition,
    wxDefaultSize,
    m_harmoniclist
  );
  m_harmonicChoiceBox->SetSelection(m_selectedHarmonic - 1);
  selectionRow->Add(m_harmonicChoiceBox, 0, wxGROW|wxALL, 2);

  // Horizontal sizer for pitch selection
  wxBoxSizer *pitchRow = new wxBoxSizer(wxHORIZONTAL);
  boxSizer->Add(pitchRow, 0, wxEXPAND|wxALL, 5);

  // Label for pitch selection
  wxStaticText *pitchLabel = new wxStaticText(
    this,
    wxID_STATIC,
    wxT("Pitch of 8' a1 (Hz):"),
    wxDefaultPosition,
    wxSize(150,-1),
    0
  );
  pitchRow->Add(pitchLabel, 0, wxALL, 2);

  // Spincontrol for selecting pitch
  m_pitchCtrl = new wxSpinCtrlDouble(
    this,
    ID_PITCHCTRL,
    wxEmptyString,
    wxDefaultPosition,
    wxDefaultSize,
    wxSP_ARROW_KEYS,
    220,
    880,
    440,
    1,
    wxT("Pitch selection control")
  );
  m_pitchCtrl->SetValue(440);
  pitchRow->Add(m_pitchCtrl, 0, wxALL, 2);

  // Horizontal sizer for buttons
  wxBoxSizer* buttonRow = new wxBoxSizer(wxHORIZONTAL);
  boxSizer->Add(buttonRow, 0, wxEXPAND|wxALL, 5);

  // The OK button
  wxButton *okButton = new wxButton(
    this,
    wxID_OK,
    wxT("&Ok"),
    wxDefaultPosition,
    wxDefaultSize,
    0
  );
  buttonRow->Add(okButton, 0, wxALIGN_CENTER|wxALL, 5);

  // The Cancel button
  wxButton *cancelButton = new wxButton(
    this,
    wxID_CANCEL,
    wxT("&Cancel"),
    wxDefaultPosition,
    wxDefaultSize,
    0
  );
  buttonRow->Add(cancelButton, 0, wxALIGN_CENTER|wxALL, 5);

  // A horizontal line after the buttons
  wxStaticLine *dividerline = new wxStaticLine(
    this,
    wxID_STATIC,
    wxDefaultPosition,
    wxDefaultSize,
    wxLI_HORIZONTAL
  );
  boxSizer->Add(dividerline, 0, wxGROW|wxALL, 5);

  // Information message at bottom
  wxStaticText *infoMessage = new wxStaticText(
    this,
    wxID_STATIC,
    wxT("Select harmonic of stop/folder for reference of 036-C, and pitch of 8' a1."),
    wxDefaultPosition,
    wxDefaultSize,
    0
  );
  boxSizer->Add(infoMessage, 0, wxALIGN_LEFT|wxALL, 5);

}

int StopHarmonicDialog::GetSelectedHarmonic() {
  return m_selectedHarmonic;
}

double StopHarmonicDialog::GetSelectedPitch() {
  return m_pitch;
}

void StopHarmonicDialog::OnChoiceSelected(wxCommandEvent& WXUNUSED(event)) {
  m_selectedHarmonic = m_harmonicChoiceBox->GetSelection() + 1;
}

void StopHarmonicDialog::OnPitchChanged(wxSpinDoubleEvent& WXUNUSED(event)) {
  m_pitch = m_pitchCtrl->GetValue();
}
