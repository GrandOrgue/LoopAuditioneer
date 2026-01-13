/* 
 * CrossfadeDialog.cpp provide a GUI for setting parameters for Crossfading
 * Copyright (C) 2012-2026 Lars Palo and contributors (see AUTHORS file) 
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

#include "CrossfadeDialog.h"
#include "wx/statline.h"

IMPLEMENT_CLASS(CrossfadeDialog, wxDialog )

BEGIN_EVENT_TABLE(CrossfadeDialog, wxDialog)
  EVT_SLIDER(ID_FADEDURATION, CrossfadeDialog::OnFadedurationSlider)
  EVT_RADIOBOX(ID_FADEMETHOD, CrossfadeDialog::OnFademethodSelection)
END_EVENT_TABLE()

CrossfadeDialog::CrossfadeDialog() {
  Init();
}

CrossfadeDialog::CrossfadeDialog(
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
 
void CrossfadeDialog::Init() {
  m_fadeduration = 0.05;
  m_fademethods.Add(wxT("Linear"));
  m_fademethods.Add(wxT("S-shape (cos)"));
  m_fademethods.Add(wxT("Equal power/gain"));
  m_fademethods.Add(wxT("Equal power (sin)"));
  selectedMethod = 0;
}

bool CrossfadeDialog::Create( 
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

void CrossfadeDialog::CreateControls() {
  // Create a top level sizer
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  this->SetSizer(topSizer);

  // Second box sizer to get nice margins
  wxBoxSizer *boxSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(boxSizer, 1, wxEXPAND|wxALL, 5);

  // Horizontal sizer for first row
  wxBoxSizer *firstRow = new wxBoxSizer(wxHORIZONTAL);
  boxSizer->Add(firstRow, 1, wxGROW|wxALL, 5);

  // Radiobox for selecting fademethod to use
  wxRadioBox *fadeMethod = new wxRadioBox(
    this,
    ID_FADEMETHOD,
    wxT("Crossfade method"),
    wxDefaultPosition,
    wxDefaultSize,
    m_fademethods,
    1,
    wxRA_SPECIFY_ROWS
  );
  fadeMethod->Enable(0, true);
  firstRow->Add(fadeMethod, 1, wxGROW|wxALL, 2);

  // Horizontal sizer for second row
  wxBoxSizer *secondRow = new wxBoxSizer(wxVERTICAL);
  boxSizer->Add(secondRow, 1, wxGROW|wxALL, 5);

  // Label for the threshold
  wxStaticText *durationLabel = new wxStaticText ( 
    this, 
    wxID_STATIC,
    wxEmptyString, 
    wxDefaultPosition, 
    wxDefaultSize, 
    0 
  );
  durationLabel->SetLabel(wxT("Duration of crossfade (in milliseconds):"));
  secondRow->Add(durationLabel, 0, wxALIGN_LEFT|wxALL, 2);

  // A slider for derivative threshold value / 1000
  wxSlider *durationSlider = new wxSlider ( 
    this, 
    ID_FADEDURATION,
    50,
    1,
    1000,
    wxDefaultPosition, 
    wxDefaultSize, 
    wxSL_HORIZONTAL|wxSL_LABELS
  );
  secondRow->Add(durationSlider, 1, wxGROW|wxALL, 2);

  // A horizontal line before the OK and Cancel buttons
  wxStaticLine *line = new wxStaticLine(
    this, 
    wxID_STATIC,
    wxDefaultPosition, 
    wxDefaultSize, 
    wxLI_HORIZONTAL
  );
  boxSizer->Add(line, 0, wxGROW|wxALL, 5);

  // A horizontal box sizer for the last row
  wxBoxSizer* lastRow = new wxBoxSizer(wxHORIZONTAL);
  boxSizer->Add(lastRow, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

  // The OK button
  wxButton* okButton = new wxButton(
    this, 
    wxID_OK, 
    wxT("&OK"),
    wxDefaultPosition, 
    wxDefaultSize, 
    0
  );
  lastRow->Add(okButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  // The Cancel button
  wxButton* cancelButton = new wxButton(
    this, 
    wxID_CANCEL,
    wxT("&Cancel"), 
    wxDefaultPosition, 
    wxDefaultSize, 
    0
  );
  lastRow->Add(cancelButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

  // Set OK button as default
  okButton->SetDefault();
  okButton->SetFocus();
}

double CrossfadeDialog::GetFadeduration() {
  return m_fadeduration;
}

int CrossfadeDialog::GetFadetype() {
  return selectedMethod;
}

// Override of transfer data to the window
bool CrossfadeDialog::TransferDataToWindow() {
  wxSlider *durationSl = (wxSlider*) FindWindow(ID_FADEDURATION);

  int value = (m_fadeduration * 1000);
  durationSl->SetValue(value);

  return true;
}

// Override of transfer data from the window
bool CrossfadeDialog::TransferDataFromWindow() {
  wxSlider *durationSl = (wxSlider*) FindWindow(ID_FADEDURATION);

  double value = (double) durationSl->GetValue() / 1000.0;
  m_fadeduration = value;

  return true;
}

void CrossfadeDialog::OnFadedurationSlider(wxCommandEvent& WXUNUSED(event)) {
  wxSlider *durationSl = (wxSlider*) FindWindow(ID_FADEDURATION);

  double value = (double) durationSl->GetValue() / 1000.0;
  m_fadeduration = value;
}

void CrossfadeDialog::OnFademethodSelection(wxCommandEvent& WXUNUSED(event)) {
  wxRadioBox *radioBox = (wxRadioBox*) FindWindow(ID_FADEMETHOD);

  selectedMethod = radioBox->GetSelection();
}

void CrossfadeDialog::SetCaption(wxString str) {
  SetTitle(str);
}

