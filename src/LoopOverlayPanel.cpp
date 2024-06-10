/*
 * LoopOverlayPanel.cpp draws the waveforms overlayed at looppoints
 * Copyright (C) 2012-2024 Lars Palo
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

#include "LoopOverlayPanel.h"

BEGIN_EVENT_TABLE(LoopOverlayPanel, wxPanel)
  EVT_SIZE(LoopOverlayPanel::OnSize)
  EVT_PAINT(LoopOverlayPanel::OnPaintEvent)
END_EVENT_TABLE()

LoopOverlayPanel::LoopOverlayPanel(
  FileHandling *fh,
  int selectedLoop,
  wxWindow* parent,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size,
  long style ) : wxPanel(parent, id, pos, size, style) {

  m_numberOfSamples = 41;
  m_trackWidth = 376;
  m_maxSamplesSpinner = (m_trackWidth / 2) + 1;
  m_fileRef = fh;
  m_selectedLoop = selectedLoop;

  // get the audio data as doubles from
  audioData = new double[m_fileRef->ArrayLength];
  bool gotData = m_fileRef->GetDoubleAudioData(audioData);

  SetBackgroundColour(wxColour(244,242,239));
  SetMinSize(wxSize(400, 380));
  SetBackgroundStyle(wxBG_STYLE_PAINT);

  if (gotData) {
    ReadLoopData();
    UpdateAudioTracks();
  } else {
    this->Close(true);
  }
}

LoopOverlayPanel::~LoopOverlayPanel() {
  if (audioData)
    delete[] audioData;
}

int LoopOverlayPanel::GetCurrentLoopEnd() {
  return currentLoopend;
}

int LoopOverlayPanel::GetCurrentLoopStart() {
  return currentLoopstart;
}

int LoopOverlayPanel::GetNumberOfSamples() {
  return m_numberOfSamples;
}

int LoopOverlayPanel::GetMaxSamplesSpin() {
  return m_maxSamplesSpinner;
}

int LoopOverlayPanel::GetSelectedLoop() {
  return m_selectedLoop;
}

void LoopOverlayPanel::SetCurrentLoopEnd(int end) {
  currentLoopend = end;
}

void LoopOverlayPanel::SetCurrentLoopStart(int start) {
  currentLoopstart = start;
}

void LoopOverlayPanel::SetNumberOfSamples(int samples) {
  m_numberOfSamples = samples;
}

void LoopOverlayPanel::SetSelectedLoop(int loop) {
  m_selectedLoop = loop;
}

void LoopOverlayPanel::OnPaintEvent(wxPaintEvent& WXUNUSED(event)) {
  wxPaintDC dc(this);
  OnPaint(dc);
}

void LoopOverlayPanel::OnPaint(wxDC& dc) {
  wxSize size = this->GetClientSize();
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

  int trackHeight = (size.y - (topMargin + bottomMargin + ((m_fileRef->m_channels - 1) * marginBetweenTracks))) / m_fileRef->m_channels;

  dc.SetBackground(wxBrush());
  dc.Clear();
  dc.SetBrush(wxBrush(*wxWHITE));
  dc.SetPen(wxPen(*wxBLACK, 1, wxPENSTYLE_SOLID));

  // draw the track containing rectangles
  if (m_fileRef->m_channels > 0) {
    for (int i = 0; i < m_fileRef->m_channels; i++) {
      int x1, y1, x2, y2;
      x1 = leftMargin;
      y1 = topMargin + trackHeight * i + i * marginBetweenTracks;
      x2 = m_trackWidth + 1;
      y2 = trackHeight;
      dc.DrawRectangle(x1, y1, x2, y2);

      // text to the left of track rectangles
      dc.SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
      wxString ch_label = wxString::Format(wxT("Channel %i"), i + 1);
      wxSize extent = dc.GetTextExtent(ch_label);
      dc.DrawRotatedText(
        ch_label,
        (leftMargin - extent.y) / 2,
        topMargin + trackHeight / 2 + trackHeight * i + extent.x / 2,
        90
      );
      dc.SetFont(wxFont(6, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_LIGHT));
      wxString highValue = wxString::Format(wxT("%.2f"), m_maxValue);
      extent = dc.GetTextExtent(highValue);
      dc.DrawText(highValue, (leftMargin - extent.x) / 2, topMargin + trackHeight * i);
      wxString lowValue = wxString::Format(wxT("%.2f"), m_minValue);
      extent = dc.GetTextExtent(lowValue);
      dc.DrawText(lowValue, (leftMargin - extent.x) / 2, topMargin + (trackHeight - extent.y) + trackHeight * i);

      // draw the zero line only if it should be visible
      if (m_maxValue > 0 && m_minValue < 0) {
        dc.SetPen(wxPen(*wxGREEN, 1, wxPENSTYLE_SOLID));
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
      dc.SetPen(wxPen(*wxBLUE, 1, wxPENSTYLE_SOLID));
      wxPoint *startWave = new wxPoint[m_numberOfSamples];
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
      dc.SetPen(wxPen(*wxBLUE, 1, wxPENSTYLE_SOLID));
      dc.DrawLines(m_numberOfSamples, startWave);

      // draw in the sample points and lines indicating the endpoint data
      dc.SetPen(wxPen(*wxRED, 1, wxPENSTYLE_SOLID));
      wxPoint *endWave = new wxPoint[m_numberOfSamples];
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
      dc.SetPen(wxPen(*wxRED, 1, wxPENSTYLE_SOLID));
      dc.DrawLines(m_numberOfSamples, endWave);

      // reset pen for next channel
      dc.SetPen(wxPen(*wxBLACK, 1, wxPENSTYLE_SOLID));
      delete[] startWave;
      delete[] endWave;
    }
  }
}

void LoopOverlayPanel::UpdateAudioTracks() {
  if (!m_startTracks.empty())
    m_startTracks.clear();
  if (!m_endTracks.empty())
    m_endTracks.clear();

  // prepare to transfer data to internal storage
  m_maxValue = -1.0;
  m_minValue = 1.0;

  for (int i = 0; i < m_fileRef->m_channels; i++) {
    STARTAUDIO start;
    m_startTracks.push_back(start);
    ENDAUDIO end;
    m_endTracks.push_back(end);
  }

  // get the necessary data
  int index = 0;
  int halfOfSamples = m_numberOfSamples / 2;
  for (int i = 0; i < m_numberOfSamples * m_fileRef->m_channels; i++) {
    double startValue = 0;
    double endValue = 0;
    int startIdx = currentLoopstart * m_fileRef->m_channels - halfOfSamples * m_fileRef->m_channels + i;
    int endIdx = currentLoopend * m_fileRef->m_channels - (halfOfSamples - 1) * m_fileRef->m_channels + i;
    if (startIdx > 0)
      startValue = audioData[startIdx];
    if ((unsigned) endIdx < m_fileRef->ArrayLength - 1)
      endValue = audioData[endIdx];
    // de-interleaving
    m_startTracks[index].startData.push_back(startValue);
    m_endTracks[index].endData.push_back(endValue);
    index++;

    if (index == m_fileRef->m_channels)
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

void LoopOverlayPanel::ReadLoopData() {
  // set the currently selected loops positions
  LOOPDATA currentLoop;
  m_fileRef->m_loops->GetLoopData(m_selectedLoop, currentLoop);

  currentLoopstart = currentLoop.dwStart;
  currentLoopend = currentLoop.dwEnd;
}

void LoopOverlayPanel::PaintNow() {
  this->Refresh();
}

void LoopOverlayPanel::OnSize(wxSizeEvent& WXUNUSED(event)) {
  Refresh();
}
