/* 
 * LoopOverlay.cpp displays the waveforms overlayed at looppoints
 * Copyright (C) 2012-2024 Lars Palo and contributors (see AUTHORS file) 
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

#include "LoopOverlay.h"

BEGIN_EVENT_TABLE(LoopOverlay, wxDialog)
  EVT_BUTTON(ID_PREV_LOOP, LoopOverlay::OnPrevButton)
  EVT_BUTTON(ID_NEXT_LOOP, LoopOverlay::OnNextButton)
  EVT_BUTTON(ID_STORE_CHANGES, LoopOverlay::OnStoreChanges)
  EVT_SPINCTRL(ID_LOOPBEGIN, LoopOverlay::OnLoopStartChange)
  EVT_SPINCTRL(ID_LOOPSTOP, LoopOverlay::OnLoopEndChange)
  EVT_SPINCTRL(ID_WAVELENGTH, LoopOverlay::OnWaveLengthChange)
END_EVENT_TABLE()

LoopOverlay::LoopOverlay(
  FileHandling *fh,
  int selectedLoop,
  wxWindow* parent,
  wxWindowID id,
  const wxString& title,
  const wxPoint& pos,
  const wxSize& size,
  long style ) : wxDialog(parent, id, title, pos, size, style) {

  m_fileReference = fh;
  m_hasChanged = false;

  m_drawingPanel = new LoopOverlayPanel(m_fileReference, selectedLoop, this);

  if (m_drawingPanel) {
    // Create a top level sizer
    wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

    // Second box sizer to get nice margins
    wxBoxSizer *boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, 1, wxEXPAND|wxALL, 5);

    // Sizer for the topmost row
    wxBoxSizer *m_topRow = new wxBoxSizer(wxHORIZONTAL);
    boxSizer->Add(m_topRow, 0, wxEXPAND|wxALL, 5);

    // The previous loop button
    m_prevLoop = new wxButton(
      this,
      ID_PREV_LOOP,
      wxT("Previous loop"),
      wxDefaultPosition,
      wxDefaultSize,
      0
    );
    m_topRow->Add(m_prevLoop, 0, wxALIGN_CENTER|wxALL, 2);

    // Label for loop
    m_loopLabel = new wxStaticText(
      this,
      wxID_STATIC,
      wxEmptyString,
      wxDefaultPosition,
      wxDefaultSize,
      0
    );
    SetLoopString();
    m_topRow->AddStretchSpacer();
    m_topRow->Add(m_loopLabel, 0, wxALIGN_CENTER);
    m_topRow->AddStretchSpacer();

    // The next loop button
    m_nextLoop = new wxButton(
      this,
      ID_NEXT_LOOP,
      wxT("Next loop"),
      wxDefaultPosition,
      wxDefaultSize,
      0
    );
    m_topRow->Add(m_nextLoop, 0, wxALIGN_CENTER|wxALL, 2);

    DecideButtonState();

    // Sizer for the middle row
    wxBoxSizer *m_middleRow = new wxBoxSizer(wxHORIZONTAL);
    boxSizer->Add(m_middleRow, 2, wxEXPAND|wxALL, 5);

    // Sizer for loopstart items
    wxBoxSizer *loopStartSizer = new wxBoxSizer(wxVERTICAL);
    m_middleRow->Add(loopStartSizer, 0, wxEXPAND|wxALL, 0);

    // Label for loopstart
    wxStaticText *loopStartLabel = new wxStaticText(
      this,
      wxID_ANY,
      wxT("Loopstart"),
      wxDefaultPosition,
      wxDefaultSize,
      0
    );
    loopStartSizer->Add(loopStartLabel, 0, wxALIGN_CENTER_HORIZONTAL|wxBOTTOM|wxLEFT|wxRIGHT, 2);

    // A spin control for the loop start value
    loopStartSpin = new wxSpinCtrl ( 
      this, 
      ID_LOOPBEGIN,
      wxEmptyString, 
      wxDefaultPosition, 
      wxDefaultSize,
      wxSP_ARROW_KEYS, 
      0, 
      m_drawingPanel->GetCurrentLoopEnd() - 1,
      m_drawingPanel->GetCurrentLoopStart()
    );
    loopStartSizer->Add(loopStartSpin, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_TOP|wxTOP|wxLEFT|wxRIGHT, 2);

    loopStartSizer->AddStretchSpacer();

    // Label for the number of samples in waveform
    wxStaticText *nrSamples = new wxStaticText(
      this,
      wxID_ANY,
      wxT("Nr. of samples"),
      wxDefaultPosition,
      wxDefaultSize,
      0
    );
    loopStartSizer->Add(nrSamples, 0, wxALIGN_CENTER_HORIZONTAL|wxBOTTOM|wxLEFT|wxRIGHT, 2);

    // A spin control for the number of samples to show the waveform for
    m_waveLength = new wxSpinCtrl ( 
      this, 
      ID_WAVELENGTH,
      wxEmptyString,
      wxDefaultPosition,
      wxDefaultSize,
      wxSP_ARROW_KEYS,
      m_drawingPanel->GetNumberOfSamples(),
      m_drawingPanel->GetMaxSamplesSpin(),
      m_drawingPanel->GetNumberOfSamples()
    );
    loopStartSizer->Add(m_waveLength, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_TOP|wxTOP|wxLEFT|wxRIGHT, 2);

    // The drawing panel that show loop overlay
    m_middleRow->Add(m_drawingPanel, 1, wxEXPAND|wxALL, 2);

    // Sizer for loopend items
    wxBoxSizer *loopEndSizer = new wxBoxSizer(wxVERTICAL);
    m_middleRow->Add(loopEndSizer, 0, wxEXPAND|wxALL, 0);

    // Label for loopend
    wxStaticText *loopEndLabel = new wxStaticText(
      this,
      wxID_ANY,
      wxT("Loopend"),
      wxDefaultPosition,
      wxDefaultSize,
      0
    );
    loopEndSizer->Add(loopEndLabel, 0, wxALIGN_CENTER_HORIZONTAL|wxBOTTOM|wxLEFT|wxRIGHT, 2);

    // A spin control for the loop end value
    loopEndSpin = new wxSpinCtrl ( 
      this, 
      ID_LOOPSTOP,
      wxEmptyString,
      wxDefaultPosition,
      wxDefaultSize,
      wxSP_ARROW_KEYS,
      m_drawingPanel->GetCurrentLoopStart() + 1,
      m_fileReference->waveTracks[0].waveData.size() - 1,
      m_drawingPanel->GetCurrentLoopEnd()
    );
    loopEndSizer->Add(loopEndSpin, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_TOP|wxTOP|wxLEFT|wxRIGHT, 2);

    loopEndSizer->AddStretchSpacer();

    // The loopchange storage button
    m_storeChanges = new wxButton(
      this,
      ID_STORE_CHANGES,
      wxT("Save changes"),
      wxDefaultPosition,
      wxDefaultSize,
      0
    );
    m_storeChanges->Enable(false);
    loopEndSizer->Add(m_storeChanges, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_TOP|wxTOP|wxLEFT|wxRIGHT, 2);

    SetMinSize(wxSize(640, 480));
    SetAutoLayout(true);
    SetSizer(topSizer);
    topSizer->Fit(this);
    topSizer->SetSizeHints(this);
  } else {
    this->Close(true);
  }
}

LoopOverlay::~LoopOverlay() {

}

void LoopOverlay::SetLoopString() {
  wxString labelString = wxString::Format(wxT("Loop #: %i"), m_drawingPanel->GetSelectedLoop() + 1);
  m_loopLabel->SetLabel(labelString);
}

void LoopOverlay::DecideButtonState() {
  if (m_drawingPanel->GetSelectedLoop() + 1 < m_fileReference->m_loops->GetNumberOfLoops())
    m_nextLoop->Enable(true);
  else
    m_nextLoop->Enable(false);

  if (m_drawingPanel->GetSelectedLoop() > 0)
    m_prevLoop->Enable(true);
  else
    m_prevLoop->Enable(false);
}

void LoopOverlay::OnPrevButton(wxCommandEvent& WXUNUSED(event)) {
  if (m_drawingPanel->GetSelectedLoop() > 0) {
    m_drawingPanel->SetSelectedLoop(m_drawingPanel->GetSelectedLoop() - 1);

    SetLoopString();
    DecideButtonState();
    m_drawingPanel->ReadLoopData();
    m_drawingPanel->UpdateAudioTracks();
    UpdateSpinners();
    SetSaveButtonState();

    m_drawingPanel->PaintNow();
  }
}

void LoopOverlay::OnNextButton(wxCommandEvent& WXUNUSED(event)) {
  if (m_drawingPanel->GetSelectedLoop() < m_fileReference->m_loops->GetNumberOfLoops() - 1) {
    m_drawingPanel->SetSelectedLoop(m_drawingPanel->GetSelectedLoop() + 1);

    SetLoopString();
    DecideButtonState();
    m_drawingPanel->ReadLoopData();
    m_drawingPanel->UpdateAudioTracks();
    UpdateSpinners();
    SetSaveButtonState();

    m_drawingPanel->PaintNow();
  }
}

void LoopOverlay::UpdateSpinners() {
  loopStartSpin->SetRange(0, m_drawingPanel->GetCurrentLoopEnd() - 1);
  loopStartSpin->SetValue(m_drawingPanel->GetCurrentLoopStart());
  loopEndSpin->SetRange(m_drawingPanel->GetCurrentLoopStart() + 1, m_fileReference->waveTracks[0].waveData.size() - 1);
  loopEndSpin->SetValue(m_drawingPanel->GetCurrentLoopEnd());
}

void LoopOverlay::OnLoopStartChange(wxSpinEvent& WXUNUSED(event)) {
  m_drawingPanel->SetCurrentLoopStart(loopStartSpin->GetValue());
  m_drawingPanel->UpdateAudioTracks();
  SetSaveButtonState();
  m_drawingPanel->PaintNow();
}

void LoopOverlay::OnLoopEndChange(wxSpinEvent& WXUNUSED(event)) {
  m_drawingPanel->SetCurrentLoopEnd(loopEndSpin->GetValue());
  m_drawingPanel->UpdateAudioTracks();
  SetSaveButtonState();
  m_drawingPanel->PaintNow();
}

void LoopOverlay::SetSampleSpinnerValues() {
  m_waveLength->SetRange(41, m_drawingPanel->GetMaxSamplesSpin());
}

void LoopOverlay::OnWaveLengthChange(wxSpinEvent& WXUNUSED(event)) {
  m_drawingPanel->SetNumberOfSamples(m_waveLength->GetValue());
  if (m_drawingPanel->GetNumberOfSamples() > m_drawingPanel->GetMaxSamplesSpin()) {
    m_drawingPanel->SetNumberOfSamples(m_drawingPanel->GetMaxSamplesSpin());
    m_waveLength->SetValue(m_drawingPanel->GetNumberOfSamples());
  } else {
    m_drawingPanel->UpdateAudioTracks();
    m_drawingPanel->PaintNow();
  }
}

void LoopOverlay::OnStoreChanges(wxCommandEvent& WXUNUSED(event)) {
  m_fileReference->m_loops->SetLoopPositions(m_drawingPanel->GetCurrentLoopStart(), m_drawingPanel->GetCurrentLoopEnd(), m_drawingPanel->GetSelectedLoop());
  SetSaveButtonState();
  m_hasChanged = true;
}

void LoopOverlay::SetSaveButtonState() {
  LOOPDATA currentLoop;
  m_fileReference->m_loops->GetLoopData(m_drawingPanel->GetSelectedLoop(), currentLoop);

  if ((unsigned) loopStartSpin->GetValue() == currentLoop.dwStart && (unsigned) loopEndSpin->GetValue() == currentLoop.dwEnd)
    m_storeChanges->Enable(false);
  else
    m_storeChanges->Enable(true);
}

bool LoopOverlay::GetHasChanged() {
  return m_hasChanged;
}
