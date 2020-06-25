/* 
 * LoopOverlay.cpp displays the waveforms overlayed at looppoints
 * Copyright (C) 2012-2020 Lars Palo 
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
  EVT_SIZE(LoopOverlay::OnSize)
  EVT_PAINT(LoopOverlay::OnPaintEvent)
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
  m_selectedLoop = selectedLoop;
  m_numberOfSamples = 41;
  m_trackWidth = 376;
  m_maxSamplesSpinner = (m_trackWidth / 2) + 1;
  m_hasChanged = false;
  SetBackgroundColour(wxColour(244,242,239));

  // get the audio data as doubles from
  audioData = new double[m_fileReference->ArrayLength];
  bool gotData = m_fileReference->GetDoubleAudioData(audioData);

  if (gotData) {
    ReadLoopData();
    UpdateAudioTracks();

    m_drawingPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE);
    m_drawingPanel->SetBackgroundColour(wxColour(244,242,239));
    m_drawingPanel->SetMinSize(wxSize(400, 380));
    m_drawingPanel->SetBackgroundStyle(wxBG_STYLE_PAINT);

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
    loopStartSizer->Add(loopStartLabel, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_BOTTOM|wxBOTTOM|wxLEFT|wxRIGHT, 2);

    // A spin control for the loop start value
    loopStartSpin = new wxSpinCtrl ( 
      this, 
      ID_LOOPBEGIN,
      wxEmptyString, 
      wxDefaultPosition, 
      wxSize(100, -1),
      wxSP_ARROW_KEYS, 
      0, 
      currentLoopend - 1, 
      currentLoopstart 
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
    loopStartSizer->Add(nrSamples, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_BOTTOM|wxBOTTOM|wxLEFT|wxRIGHT, 2);

    // A spin control for the number of samples to show the waveform for
    m_waveLength = new wxSpinCtrl ( 
      this, 
      ID_WAVELENGTH,
      wxEmptyString, 
      wxDefaultPosition, 
      wxSize(100, -1),
      wxSP_ARROW_KEYS, 
      m_numberOfSamples, 
      m_maxSamplesSpinner, 
      m_numberOfSamples 
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
    loopEndSizer->Add(loopEndLabel, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_BOTTOM|wxBOTTOM|wxLEFT|wxRIGHT, 2);

    // A spin control for the loop end value
    loopEndSpin = new wxSpinCtrl ( 
      this, 
      ID_LOOPSTOP,
      wxEmptyString, 
      wxDefaultPosition, 
      wxSize(100, -1),
      wxSP_ARROW_KEYS, 
      currentLoopstart + 1, 
      m_fileReference->waveTracks[0].waveData.size() - 1, 
      currentLoopend
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
  if (audioData)
    delete[] audioData;
}

void LoopOverlay::OnPaintEvent(wxPaintEvent & evt) {
  wxPaintDC dcp(this);
  wxClientDC dc(m_drawingPanel);
  OnPaint(dc);
}

void LoopOverlay::OnPaint(wxDC& dc) {
  wxSize size = m_drawingPanel->GetClientSize();
  int leftMargin = 20;
  int rightMargin = 10;
  int topMargin = 10;
  int bottomMargin = 10;
  int marginBetweenTracks = 0;
  m_trackWidth = size.x - (leftMargin + rightMargin);
  m_maxSamplesSpinner = (m_trackWidth / 2) + 1;
  leftMargin += (m_trackWidth % (m_numberOfSamples - 1)) / 2;
  // ensure width is divisible by m_numberOfSamples - 1
  m_trackWidth -= (m_trackWidth % (m_numberOfSamples - 1));

  int trackHeight = (size.y - (topMargin + bottomMargin + ((m_fileReference->m_channels - 1) * marginBetweenTracks))) / m_fileReference->m_channels;

  dc.SetBackground(wxBrush());
  dc.Clear();
  dc.SetBrush(wxBrush(*wxWHITE));
  dc.SetPen(wxPen(*wxBLACK, 1, wxSOLID));

  // draw the track containing rectangles
  if (m_fileReference->m_channels > 0) {
    for (int i = 0; i < m_fileReference->m_channels; i++) {
      int x1, y1, x2, y2;
      x1 = leftMargin;
      y1 = topMargin + trackHeight * i + i * marginBetweenTracks;
      x2 = m_trackWidth + 1;
      y2 = trackHeight;
      dc.DrawRectangle(x1, y1, x2, y2);

      // text to the left of track rectangles
      dc.SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxNORMAL, wxNORMAL));
      wxString ch_label = wxString::Format(wxT("Channel %i"), i + 1);
      wxSize extent = dc.GetTextExtent(ch_label);
      dc.DrawRotatedText(
        ch_label,
        (leftMargin - extent.y) / 2,
        topMargin + trackHeight / 2 + trackHeight * i + extent.x / 2,
        90
      );
      dc.SetFont(wxFont(6, wxFONTFAMILY_DEFAULT, wxNORMAL, wxFONTWEIGHT_LIGHT));
      wxString highValue = wxString::Format(wxT("%.2f"), m_maxValue);
      extent = dc.GetTextExtent(highValue);
      dc.DrawText(highValue, (leftMargin - extent.x) / 2, topMargin + trackHeight * i);
      wxString lowValue = wxString::Format(wxT("%.2f"), m_minValue);
      extent = dc.GetTextExtent(lowValue);
      dc.DrawText(lowValue, (leftMargin - extent.x) / 2, topMargin + (trackHeight - extent.y) + trackHeight * i);

      // draw the zero line only if it should be visible
      if (m_maxValue > 0 && m_minValue < 0) {
        dc.SetPen(wxPen(*wxGREEN, 1, wxSOLID));
        double normalizedZero = (0 - m_minValue) / m_valueRange;
        int zeroY = topMargin + trackHeight - trackHeight * normalizedZero + trackHeight * i;
        dc.DrawLine(
          leftMargin + 1,
          zeroY,
          leftMargin + m_trackWidth,
          zeroY
        );
      }

      // draw in the sample points and lines indicating the startpoint data
      dc.SetPen(wxPen(*wxBLUE, 1, wxSOLID));
      wxPoint startWave[m_numberOfSamples];
      for (int j = 0; j < m_numberOfSamples; j++) {
        int x_value = leftMargin + (m_trackWidth / (m_numberOfSamples - 1)) * j;
        double y = m_startTracks[i].startData[j]; // real value from audio data
        y = (y - m_minValue) / m_valueRange; // normalized between min and max
        int y_value = topMargin + trackHeight - trackHeight * y + trackHeight * i;
        startWave[j] = wxPoint(x_value, y_value);
        // draw the samplepoint as 3 x 3 points with center as value
        if (m_trackWidth / m_numberOfSamples > 5) {
          for (int x = 0; x < 3; x++) {
            for (int y = 0; y < 3; y++)
              dc.DrawPoint(x_value - 1 + x, y_value - 1 + y);
          }
        }
      }
      dc.SetPen(wxPen(*wxBLUE, 1, wxSOLID));
      dc.DrawLines(m_numberOfSamples, startWave);

      // draw in the sample points and lines indicating the endpoint data
      dc.SetPen(wxPen(*wxRED, 1, wxSOLID));
      wxPoint endWave[m_numberOfSamples];
      for (int j = 0; j < m_numberOfSamples; j++) {
        int x_value = leftMargin + (m_trackWidth / (m_numberOfSamples - 1)) * j;
        double y = m_endTracks[i].endData[j]; // real value from audio data
        y = (y - m_minValue) / m_valueRange; // normalized between min and max
        int y_value = topMargin + trackHeight - trackHeight * y + trackHeight * i;
        endWave[j] = wxPoint(x_value, y_value);
        // draw the samplepoint as 3 x 3 points with center as value
        if (m_trackWidth / m_numberOfSamples > 5) {
          for (int x = 0; x < 3; x++) {
            for (int y = 0; y < 3; y++)
              dc.DrawPoint(x_value - 1 + x, y_value - 1 + y);
          }
        }
      }
      dc.SetPen(wxPen(*wxRED, 1, wxSOLID));
      dc.DrawLines(m_numberOfSamples, endWave);

      // reset pen for next channel
      dc.SetPen(wxPen(*wxBLACK, 1, wxSOLID));
    }
  }
  SetSampleSpinnerValues();
}

void LoopOverlay::UpdateAudioTracks() {
  if (!m_startTracks.empty())
    m_startTracks.clear();
  if (!m_endTracks.empty())
    m_endTracks.clear();

  // prepare to transfer data to internal storage
  m_maxValue = -1.0;
  m_minValue = 1.0;

  for (int i = 0; i < m_fileReference->m_channels; i++) {
    STARTAUDIO start;
    m_startTracks.push_back(start);
    ENDAUDIO end;
    m_endTracks.push_back(end);
  }

  // get the necessary data
  int index = 0;
  int halfOfSamples = m_numberOfSamples / 2;
  for (int i = 0; i < m_numberOfSamples * m_fileReference->m_channels; i++) {
    double startValue = 0;
    double endValue = 0;
    int startIdx = currentLoopstart * m_fileReference->m_channels - halfOfSamples * m_fileReference->m_channels + i;
    int endIdx = currentLoopend * m_fileReference->m_channels - (halfOfSamples - 1) * m_fileReference->m_channels + i;
    if (startIdx > 0)
      startValue = audioData[startIdx];
    if (endIdx < m_fileReference->ArrayLength - 1)
      endValue = audioData[endIdx];
    // de-interleaving
    m_startTracks[index].startData.push_back(startValue);
    m_endTracks[index].endData.push_back(endValue);
    index++;

    if (index == m_fileReference->m_channels)
      index = 0;

    // max and min values will be used to scale the waveform
    if (startValue > m_maxValue)
      m_maxValue = startValue;

    if (startValue < m_minValue)
      m_minValue = startValue;

    if (endValue > m_maxValue)
      m_maxValue = endValue;

    if (endValue < m_minValue)
      m_minValue = endValue;
  }

  m_valueRange = m_maxValue - m_minValue;
}

void LoopOverlay::SetLoopString() {
  wxString labelString = wxString::Format(wxT("Loop #: %i"), m_selectedLoop + 1);
  m_loopLabel->SetLabel(labelString);
}

void LoopOverlay::DecideButtonState() {
  if (m_selectedLoop + 1 < m_fileReference->m_loops->GetNumberOfLoops())
    m_nextLoop->Enable(true);
  else
    m_nextLoop->Enable(false);

  if (m_selectedLoop > 0)
    m_prevLoop->Enable(true);
  else
    m_prevLoop->Enable(false);
}

void LoopOverlay::OnPrevButton(wxCommandEvent& event) {
  if (m_selectedLoop > 0) {
    m_selectedLoop--;

    SetLoopString();
    DecideButtonState();
    ReadLoopData();
    UpdateAudioTracks();
    UpdateSpinners();
    SetSaveButtonState();

    PaintNow();
  }
}

void LoopOverlay::OnNextButton(wxCommandEvent& event) {
  if (m_selectedLoop < m_fileReference->m_loops->GetNumberOfLoops() - 1) {
    m_selectedLoop++;

    SetLoopString();
    DecideButtonState();
    ReadLoopData();
    UpdateAudioTracks();
    UpdateSpinners();
    SetSaveButtonState();

    PaintNow();
  }
}

void LoopOverlay::UpdateSpinners() {
  loopStartSpin->SetRange(0, currentLoopend - 1);
  loopStartSpin->SetValue(currentLoopstart);
  loopEndSpin->SetRange(currentLoopstart + 1, m_fileReference->waveTracks[0].waveData.size() - 1);
  loopEndSpin->SetValue(currentLoopend);
}

void LoopOverlay::OnLoopStartChange(wxSpinEvent& event) {
  currentLoopstart = loopStartSpin->GetValue();
  UpdateAudioTracks();
  SetSaveButtonState();
  PaintNow();
}

void LoopOverlay::OnLoopEndChange(wxSpinEvent& event) {
  currentLoopend = loopEndSpin->GetValue();
  UpdateAudioTracks();
  SetSaveButtonState();
  PaintNow();
}

void LoopOverlay::ReadLoopData() {
  // set the currently selected loops positions
  LOOPDATA currentLoop;
  m_fileReference->m_loops->GetLoopData(m_selectedLoop, currentLoop);

  currentLoopstart = currentLoop.dwStart;
  currentLoopend = currentLoop.dwEnd;
}

void LoopOverlay::SetSampleSpinnerValues() {
  m_waveLength->SetRange(41, m_maxSamplesSpinner);
}

void LoopOverlay::OnWaveLengthChange(wxSpinEvent& event) {
  m_numberOfSamples = m_waveLength->GetValue();
  if (m_numberOfSamples > m_maxSamplesSpinner) {
    m_numberOfSamples = m_maxSamplesSpinner;
    m_waveLength->SetValue(m_numberOfSamples);
  } else {
    UpdateAudioTracks();
    PaintNow();
  }
}

void LoopOverlay::OnStoreChanges(wxCommandEvent& event) {
  m_fileReference->m_loops->SetLoopPositions(currentLoopstart, currentLoopend, m_selectedLoop);
  SetSaveButtonState();
  m_hasChanged = true;
}

void LoopOverlay::SetSaveButtonState() {
  LOOPDATA currentLoop;
  m_fileReference->m_loops->GetLoopData(m_selectedLoop, currentLoop);

  if (loopStartSpin->GetValue() == currentLoop.dwStart && loopEndSpin->GetValue() == currentLoop.dwEnd)
    m_storeChanges->Enable(false);
  else
    m_storeChanges->Enable(true);
}

bool LoopOverlay::GetHasChanged() {
  return m_hasChanged;
}

void LoopOverlay::PaintNow() {
  this->Refresh();
}

void LoopOverlay::OnSize(wxSizeEvent& event) {
  Layout();
}
