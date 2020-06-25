/* 
 * AutoLoopDialog.cpp provide a GUI for setting parameters for AutoLooping
 * Copyright (C) 2011-2020 Lars Palo
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

#include "AutoLoopDialog.h"
#include "wx/statline.h"

IMPLEMENT_CLASS(AutoLoopDialog, wxDialog )

BEGIN_EVENT_TABLE(AutoLoopDialog, wxDialog)
  EVT_CHECKBOX(ID_SEARCH_CHECK, AutoLoopDialog::OnAutosearchCheck)
  EVT_CHECKBOX(ID_BRUTE_FORCE_CHECK, AutoLoopDialog::OnBruteForceCheck)
  EVT_SLIDER(ID_SUSTAINSTART, AutoLoopDialog::OnStartSliderMove)
  EVT_SLIDER(ID_SUSTAINEND, AutoLoopDialog::OnEndSliderMove)
  EVT_SLIDER(ID_THRESHOLD, AutoLoopDialog::OnThresholdSlider)
  EVT_SLIDER(ID_DURATION, AutoLoopDialog::OnDurationSlider)
  EVT_SLIDER(ID_BETWEEN, AutoLoopDialog::OnBetweenSlider)
  EVT_SLIDER(ID_QUALITY, AutoLoopDialog::OnQuality)
END_EVENT_TABLE()

AutoLoopDialog::AutoLoopDialog() {
  Init();
}

AutoLoopDialog::AutoLoopDialog(
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
 
void AutoLoopDialog::Init() {
  m_threshold = 0.03;
  m_minDuration = 1.0;
  m_betweenLoops = 0.3;
  m_quality = 6;
  m_candidates = 50000;
  m_numberOfLoops = 6;
  m_loopMultiple = 10;
  m_autoSearchSustain = true;
  m_startPercentage = 20;
  m_endPercentage = 70;
  m_searchBruteForce = false;
}

bool AutoLoopDialog::Create( 
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

void AutoLoopDialog::CreateControls() {
  // Create a top level sizer
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  this->SetSizer(topSizer);

  // Second box sizer to get nice margins
  wxBoxSizer *boxSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(boxSizer, 1, wxEXPAND|wxALL, 5);

  // Horizontal sizer for first row
  wxBoxSizer *firstRow = new wxBoxSizer(wxHORIZONTAL);
  boxSizer->Add(firstRow, 0, wxGROW|wxALL, 5);

  // Grouping of sustainsection options
  wxStaticBox *staticBox = new wxStaticBox(
    this, 
    wxID_STATIC,
    wxT("Search options"), 
    wxDefaultPosition, 
    wxDefaultSize
  );

  // Vertical sizer for first rows subsections
  wxStaticBoxSizer *firstRowSub = new wxStaticBoxSizer(staticBox, wxVERTICAL);
  firstRow->Add(firstRowSub, 1, wxGROW|wxALL, 0);

  // Horizontal sizer for first sub row
  wxBoxSizer *firstSubRow = new wxBoxSizer(wxHORIZONTAL);
  firstRowSub->Add(firstSubRow, 0, wxGROW|wxALL, 2);

  // Checkbox for autosearching sustainsection
  wxCheckBox *autosearchCheck = new wxCheckBox(
    this,
    ID_SEARCH_CHECK,
    wxT("Autosearch sustainsection"),
    wxDefaultPosition,
    wxDefaultSize
  );
  autosearchCheck->SetValue(true);
  firstSubRow->Add(autosearchCheck, 1, wxGROW|wxALL, 2);

  // Checkbox for search with brute force
  wxCheckBox *bruteForceCheck = new wxCheckBox(
    this,
    ID_BRUTE_FORCE_CHECK,
    wxT("Search w. brute force (can take long time)"),
    wxDefaultPosition,
    wxDefaultSize
  );
  bruteForceCheck->SetValue(true);
  firstSubRow->Add(bruteForceCheck, 1, wxGROW|wxALL, 2);

  // Horizontal sizer for second sub row
  wxBoxSizer *secondSubRow = new wxBoxSizer(wxHORIZONTAL);
  firstRowSub->Add(secondSubRow, 0, wxGROW|wxALL, 2);

  // Label for the start
  m_startLabel = new wxStaticText ( 
    this, 
    wxID_STATIC,
    wxEmptyString, 
    wxDefaultPosition, 
    wxSize(220,-1), 
    0 
  );
  m_startLabel->SetLabel(wxString::Format(wxT("Sustain start at: %i %%"), m_startPercentage));
  secondSubRow->Add(m_startLabel, 1, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  // Slider for sustainsection start percentage 0 - 99
  wxSlider *startSlider = new wxSlider ( 
    this, 
    ID_SUSTAINSTART,
    20,
    0,
    99,
    wxDefaultPosition, 
    wxDefaultSize, 
    wxSL_HORIZONTAL
  );
  secondSubRow->Add(startSlider, 2, wxALIGN_CENTER_VERTICAL|wxALL, 0);
  startSlider->Enable(false);

  // Horizontal sizer for third sub row
  wxBoxSizer *thirdSubRow = new wxBoxSizer(wxHORIZONTAL);
  firstRowSub->Add(thirdSubRow, 0, wxGROW|wxALL, 2);

  // Label for the start
  m_endLabel = new wxStaticText ( 
    this, 
    wxID_STATIC,
    wxEmptyString, 
    wxDefaultPosition, 
    wxSize(220,-1), 
    0 
  );
  m_endLabel->SetLabel(wxString::Format(wxT("Sustain end at: %i %%"), m_endPercentage));
  thirdSubRow->Add(m_endLabel, 1, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  // Slider for sustainsection end percentage
  wxSlider *endSlider = new wxSlider ( 
    this, 
    ID_SUSTAINEND,
    70,
    1,
    100,
    wxDefaultPosition, 
    wxDefaultSize, 
    wxSL_HORIZONTAL
  );
  thirdSubRow->Add(endSlider, 2, wxALIGN_CENTER_VERTICAL|wxALL, 0);
  endSlider->Enable(false);

  // Horizontal sizer for second row
  wxBoxSizer *secondRow = new wxBoxSizer(wxHORIZONTAL);
  boxSizer->Add(secondRow, 0, wxGROW|wxALL, 5);

  // Label for the threshold
  m_thresholdLabel = new wxStaticText ( 
    this, 
    wxID_STATIC,
    wxEmptyString, 
    wxDefaultPosition, 
    wxSize(220,-1), 
    0 
  );
  m_thresholdLabel->SetLabel(wxString::Format(wxT("Derivative threshold: %.3f"), m_threshold));
  secondRow->Add(m_thresholdLabel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  // A slider for derivative threshold value / 1000
  wxSlider *thresholdSlider = new wxSlider ( 
    this, 
    ID_THRESHOLD,
    30,
    1,
    100,
    wxDefaultPosition, 
    wxDefaultSize, 
    wxSL_HORIZONTAL
  );
  secondRow->Add(thresholdSlider, 1, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  // Horizontal sizer for third row
  wxBoxSizer* thirdRow = new wxBoxSizer(wxHORIZONTAL);
  boxSizer->Add(thirdRow, 0, wxGROW|wxALL, 5);

  // Label for the loop length
  m_durationLabel = new wxStaticText ( 
    this, 
    wxID_STATIC,
    wxEmptyString, 
    wxDefaultPosition, 
    wxSize(220, -1), 
    0 
  );
  m_durationLabel->SetLabel(wxString::Format(wxT("Min. loop length: %.2f s"), m_minDuration));
  thirdRow->Add(m_durationLabel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  // A slider for minimum loop duration / 100
  wxSlider *durationSlider = new wxSlider ( 
    this, 
    ID_DURATION,
    100,
    0,
    500,
    wxDefaultPosition, 
    wxDefaultSize, 
    wxSL_HORIZONTAL
  );
  thirdRow->Add(durationSlider, 1, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  // Horizontal sizer for fourth row
  wxBoxSizer *fourthRow = new wxBoxSizer(wxHORIZONTAL);
  boxSizer->Add(fourthRow, 0, wxGROW|wxALL, 5);

  // Label for the threshold
  m_distanceLabel = new wxStaticText ( 
    this, 
    wxID_STATIC,
    wxEmptyString, 
    wxDefaultPosition, 
    wxSize(220,-1), 
    0 
  );
  m_distanceLabel->SetLabel(wxString::Format(wxT("Min. time between loops: %.2f s"), m_betweenLoops));
  fourthRow->Add(m_distanceLabel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  // A slider for distance between loops / 100
  wxSlider *distanceSlider = new wxSlider ( 
    this, 
    ID_BETWEEN,
    30,
    0,
    100,
    wxDefaultPosition, 
    wxDefaultSize, 
    wxSL_HORIZONTAL
  );
  fourthRow->Add(distanceSlider, 1, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  // Horizontal sizer for fifth row
  wxBoxSizer *fifthRow = new wxBoxSizer(wxHORIZONTAL);
  boxSizer->Add(fifthRow, 0, wxGROW|wxALL, 5);

  // Label for the quality
  m_qualityLabel = new wxStaticText ( 
    this, 
    wxID_STATIC,
    wxEmptyString, 
    wxDefaultPosition, 
    wxSize(220,-1), 
    0 
  );
  m_qualityLabel->SetLabel(wxString::Format(wxT("Quality factor: %.1f"), m_quality));
  fifthRow->Add(m_qualityLabel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  // A slider for quality factor / 10
  wxSlider *qualitySlider = new wxSlider ( 
    this, 
    ID_QUALITY,
    60,
    5,
    1000,
    wxDefaultPosition, 
    wxDefaultSize, 
    wxSL_HORIZONTAL
  );
  fifthRow->Add(qualitySlider, 1, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  // Horizontal sizer for sixth row
  wxBoxSizer *sixthRow = new wxBoxSizer(wxHORIZONTAL);
  boxSizer->Add(sixthRow, 0, wxGROW|wxALL, 5);

  // Loop candidates label
  wxStaticText* candidatesLabel = new wxStaticText(
    this, 
    wxID_STATIC,
    wxT("Number of candidates: "), 
    wxDefaultPosition,
    wxSize(220,-1), 
    0
  );
  sixthRow->Add(candidatesLabel, 0, wxALIGN_LEFT|wxALL, 0);

  // A slider for number of candidates
  wxSlider *candidatesSlider = new wxSlider ( 
    this, 
    ID_CANDIDATES,
    50000,
    1000,
    100000,
    wxDefaultPosition, 
    wxDefaultSize, 
    wxSL_HORIZONTAL|wxSL_LABELS
  );
  sixthRow->Add(candidatesSlider, 1, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  // Horizontal sizer for seventh row
  wxBoxSizer *seventhRow = new wxBoxSizer(wxHORIZONTAL);
  boxSizer->Add(seventhRow, 0, wxGROW|wxALL, 5);

  // Loops to return label
  wxStaticText* loopLabel = new wxStaticText(
    this, 
    wxID_STATIC,
    wxT("Loops to return: "), 
    wxDefaultPosition,
    wxSize(220,-1), 
    0
  );
  seventhRow->Add(loopLabel, 0, wxALIGN_LEFT|wxALL, 0);

  // A slider for number of loops
  wxSlider *loopSlider = new wxSlider ( 
    this, 
    ID_NR_LOOPS,
    6,
    1,
    16,
    wxDefaultPosition, 
    wxDefaultSize, 
    wxSL_HORIZONTAL|wxSL_LABELS
  );
  seventhRow->Add(loopSlider, 1, wxALIGN_CENTER_VERTICAL|wxALL, 0);

  // Horizontal sizer for eighth row
  wxBoxSizer *eighthRow = new wxBoxSizer(wxHORIZONTAL);
  boxSizer->Add(eighthRow, 0, wxGROW|wxALL, 5);

  // Loop multiple label
  wxStaticText* multipleLabel = new wxStaticText(
    this, 
    wxID_STATIC,
    wxT("Loop pool multiple: "), 
    wxDefaultPosition,
    wxSize(220,-1), 
    0
  );
  eighthRow->Add(multipleLabel, 0, wxALIGN_LEFT|wxALL, 0);

  // A slider for loop multiple
  wxSlider *multipleSlider = new wxSlider ( 
    this, 
    ID_LOOP_MULTIPLE,
    10,
    1,
    10,
    wxDefaultPosition, 
    wxDefaultSize, 
    wxSL_HORIZONTAL|wxSL_LABELS
  );
  eighthRow->Add(multipleSlider, 1, wxALIGN_CENTER_VERTICAL|wxALL, 0);

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
}

void AutoLoopDialog::SetThreshold(double th) {
  m_threshold = th;
}
void AutoLoopDialog::SetDuration(double d) {
  m_minDuration = d;
}
void AutoLoopDialog::SetBetween(double b) {
  m_betweenLoops = b;
}
void AutoLoopDialog::SetQuality(double q) {
  m_quality = q;
}
void AutoLoopDialog::SetCandidates(int c) {
  m_candidates = c;
}
void AutoLoopDialog::SetNrLoops(int l) {
  m_numberOfLoops = l;
}
void AutoLoopDialog::SetMultiple(int m) {
  m_loopMultiple = m;
}
void AutoLoopDialog::SetAutosearch(bool search) {
  wxCheckBox *autoCheck = (wxCheckBox*) FindWindow(ID_SEARCH_CHECK);
  wxSlider *startSl = (wxSlider*) FindWindow(ID_SUSTAINSTART);
  wxSlider *endSl = (wxSlider*) FindWindow(ID_SUSTAINEND);

  if (search) {
    startSl->Enable(false);
    endSl->Enable(false);
    m_autoSearchSustain = true;
    autoCheck->SetValue(true);
  } else {
    startSl->Enable(true);
    endSl->Enable(true);
    m_autoSearchSustain = false;
    autoCheck->SetValue(false);
  }
}
void AutoLoopDialog::SetStart(int start) {
  m_startPercentage = start;
}
void AutoLoopDialog::SetEnd(int end) {
  m_endPercentage = end;
}
void AutoLoopDialog::SetBruteForce(bool b) {
  if (b)
    m_searchBruteForce = true;
  else
    m_searchBruteForce = false;
}
double AutoLoopDialog::GetThreshold() {
  return m_threshold;
}
double AutoLoopDialog::GetDuration() {
  return m_minDuration;
}
double AutoLoopDialog::GetBetween() {
  return m_betweenLoops;
}
double AutoLoopDialog::GetQuality() {
  return m_quality;
}
int AutoLoopDialog::GetCandidates() {
  return m_candidates;
}
int AutoLoopDialog::GetNrLoops() {
  return m_numberOfLoops;
}
int AutoLoopDialog::GetMultiple() {
  return m_loopMultiple;
}
bool AutoLoopDialog::GetAutosearch() {
  return m_autoSearchSustain;
}
int AutoLoopDialog::GetStart() {
  return m_startPercentage;
}
int AutoLoopDialog::GetEnd() {
  return m_endPercentage;
}
bool AutoLoopDialog::GetBruteForce() {
  return m_searchBruteForce;
}

// Override of transfer data to the window
bool AutoLoopDialog::TransferDataToWindow() {
  wxCheckBox *autoCheck = (wxCheckBox*) FindWindow(ID_SEARCH_CHECK);
  wxCheckBox *bruteCheck = (wxCheckBox*) FindWindow(ID_BRUTE_FORCE_CHECK);
  wxSlider *startSl = (wxSlider*) FindWindow(ID_SUSTAINSTART);
  wxSlider *endSl = (wxSlider*) FindWindow(ID_SUSTAINEND);
  wxSlider *thresholdSl = (wxSlider*) FindWindow(ID_THRESHOLD);
  wxSlider *durationSl = (wxSlider*) FindWindow(ID_DURATION);
  wxSlider *betweenSl = (wxSlider*) FindWindow(ID_BETWEEN);
  wxSlider *qualitySl = (wxSlider*) FindWindow(ID_QUALITY);
  wxSlider *candidatesSl = (wxSlider*) FindWindow(ID_CANDIDATES);
  wxSlider *loopsSl = (wxSlider*) FindWindow(ID_NR_LOOPS);
  wxSlider *multipleSl = (wxSlider*) FindWindow(ID_LOOP_MULTIPLE);

  autoCheck->SetValue(m_autoSearchSustain);
  bruteCheck->SetValue(m_searchBruteForce);
  startSl->SetValue(m_startPercentage);
  endSl->SetValue(m_endPercentage);
  candidatesSl->SetValue(m_candidates);
  loopsSl->SetValue(m_numberOfLoops);
  multipleSl->SetValue(m_loopMultiple);

  int value = (m_threshold * 1000);
  thresholdSl->SetValue(value);
  value = (m_minDuration * 100);
  durationSl->SetValue(value);
  value = (m_betweenLoops * 100);
  betweenSl->SetValue(value);
  value = (m_quality * 10);
  qualitySl->SetValue(value);

  return true;
}

// Override of transfer data from the window
bool AutoLoopDialog::TransferDataFromWindow() {
  wxCheckBox *autoCheck = (wxCheckBox*) FindWindow(ID_SEARCH_CHECK);
  wxCheckBox *bruteCheck = (wxCheckBox*) FindWindow(ID_BRUTE_FORCE_CHECK);
  wxSlider *startSl = (wxSlider*) FindWindow(ID_SUSTAINSTART);
  wxSlider *endSl = (wxSlider*) FindWindow(ID_SUSTAINEND);
  wxSlider *thresholdSl = (wxSlider*) FindWindow(ID_THRESHOLD);
  wxSlider *durationSl = (wxSlider*) FindWindow(ID_DURATION);
  wxSlider *betweenSl = (wxSlider*) FindWindow(ID_BETWEEN);
  wxSlider *qualitySl = (wxSlider*) FindWindow(ID_QUALITY);
  wxSlider *candidatesSl = (wxSlider*) FindWindow(ID_CANDIDATES);
  wxSlider *loopsSl = (wxSlider*) FindWindow(ID_NR_LOOPS);
  wxSlider *multipleSl = (wxSlider*) FindWindow(ID_LOOP_MULTIPLE);

  m_candidates = candidatesSl->GetValue();
  m_numberOfLoops = loopsSl->GetValue();
  m_loopMultiple = multipleSl->GetValue();
  m_autoSearchSustain = autoCheck->GetValue();
  m_startPercentage = startSl->GetValue();
  m_endPercentage = endSl->GetValue();
  m_searchBruteForce = bruteCheck->GetValue();

  double value = (double) thresholdSl->GetValue() / 1000.0;
  m_threshold = value;
  value = (double) durationSl->GetValue() / 100.0;
  m_minDuration = value;
  value = (double) betweenSl->GetValue() / 100.0;
  m_betweenLoops = value;
  value = (double) qualitySl->GetValue() / 10.0;
  m_quality = value;

  return true;
}

void AutoLoopDialog::OnAutosearchCheck(wxCommandEvent& event) {
  wxCheckBox *autoCheck = (wxCheckBox*) FindWindow(ID_SEARCH_CHECK);
  wxSlider *startSl = (wxSlider*) FindWindow(ID_SUSTAINSTART);
  wxSlider *endSl = (wxSlider*) FindWindow(ID_SUSTAINEND);
  m_autoSearchSustain = autoCheck->GetValue();

  if (m_autoSearchSustain) {
    startSl->Enable(false);
    endSl->Enable(false);
  } else {
    startSl->Enable(true);
    endSl->Enable(true);
  }
}

void AutoLoopDialog::OnBruteForceCheck(wxCommandEvent& event) {
  wxCheckBox *bruteCheck = (wxCheckBox*) FindWindow(ID_BRUTE_FORCE_CHECK);
  m_searchBruteForce = bruteCheck->GetValue();
}

void AutoLoopDialog::OnStartSliderMove(wxCommandEvent& event) {
  wxSlider *startSl = (wxSlider*) FindWindow(ID_SUSTAINSTART);
  int value = startSl->GetValue();

  if (value < m_endPercentage) {
    m_startPercentage = value;
    m_startLabel->SetLabel(wxString::Format(wxT("Sustain start at: %i %%"), m_startPercentage));
  } else
    startSl->SetValue(m_startPercentage);
}

void AutoLoopDialog::OnEndSliderMove(wxCommandEvent& event) {
  wxSlider *endSl = (wxSlider*) FindWindow(ID_SUSTAINEND);
  int value = endSl->GetValue();

  if (value > m_startPercentage) {
    m_endPercentage = value;
    m_endLabel->SetLabel(wxString::Format(wxT("Sustain end at: %i %%"), m_endPercentage));
  } else
    endSl->SetValue(m_endPercentage);
}

void AutoLoopDialog::OnThresholdSlider(wxCommandEvent& event) {
  wxSlider *thresholdSl = (wxSlider*) FindWindow(ID_THRESHOLD);

  double value = (double) thresholdSl->GetValue() / 1000.0;
  m_threshold = value;

  m_thresholdLabel->SetLabel(wxString::Format(wxT("Derivative threshold: %.3f"), m_threshold));
}

void AutoLoopDialog::OnDurationSlider(wxCommandEvent& event) {
  wxSlider *durationSl = (wxSlider*) FindWindow(ID_DURATION);

  double value = (double) durationSl->GetValue() / 100.0;
  m_minDuration = value;

  m_durationLabel->SetLabel(wxString::Format(wxT("Min. loop lenght: %.2f s"), m_minDuration));
}

void AutoLoopDialog::OnBetweenSlider(wxCommandEvent& event) {
  wxSlider *betweenSl = (wxSlider*) FindWindow(ID_BETWEEN);

  double value = (double) betweenSl->GetValue() / 100.0;
  m_betweenLoops = value;

  m_distanceLabel->SetLabel(wxString::Format(wxT("Min. time between loops: %.2f s"), m_betweenLoops));
}

void AutoLoopDialog::OnQuality(wxCommandEvent& event) {
  wxSlider *qualitySl = (wxSlider*) FindWindow(ID_QUALITY);

  double value = (double) qualitySl->GetValue() / 10.0;
  m_quality = value;

  m_qualityLabel->SetLabel(wxString::Format(wxT("Quality factor: %.1f"), m_quality));
}

void AutoLoopDialog::UpdateLabels() {
  m_startLabel->SetLabel(wxString::Format(wxT("Sustain start at: %i %%"), m_startPercentage));
  m_endLabel->SetLabel(wxString::Format(wxT("Sustain end at: %i %%"), m_endPercentage));
  m_thresholdLabel->SetLabel(wxString::Format(wxT("Derivative threshold: %.3f"), m_threshold));
  m_durationLabel->SetLabel(wxString::Format(wxT("Min. loop lenght: %.2f s"), m_minDuration));
  m_distanceLabel->SetLabel(wxString::Format(wxT("Min. time between loops: %.2f s"), m_betweenLoops));
  m_qualityLabel->SetLabel(wxString::Format(wxT("Quality factor: %.1f"), m_quality));
}
