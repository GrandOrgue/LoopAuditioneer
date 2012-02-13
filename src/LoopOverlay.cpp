/* 
 * LoopOverlay.cpp displays the waveforms overlayed at looppoints
 * Copyright (C) 2012 Lars Palo 
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
  EVT_PAINT(LoopOverlay::OnPaintEvent)
END_EVENT_TABLE()

LoopOverlay::LoopOverlay(
  double audioData[],
  unsigned startSample,
  unsigned endSample,
  int channels,
  wxWindow* parent,
  wxWindowID id,
  const wxString& title,
  const wxPoint& pos,
  const wxSize& size,
  long style ) : wxDialog(parent, id, title, pos, size, style) {

  // prepare to transfer data to internal storage
  m_maxValue = -1.0;
  m_minValue = 1.0;

  for (int i = 0; i < channels; i++) {
    STARTAUDIO start;
    m_startTracks.push_back(start);
    ENDAUDIO end;
    m_endTracks.push_back(end);
  }

  // get the necessary data
  int index = 0;
  for (int i = 0; i < 41 * channels; i++) {
    double startValue = audioData[startSample * channels - 20 * channels + i];
    double endValue = audioData[endSample * channels - 19 * channels + i];
    // de-interleaving
    m_startTracks[index].startData.push_back(startValue);
    m_endTracks[index].endData.push_back(endValue);
    index++;

    if (index == channels)
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

  m_channels = channels;

  // Layout issues
  this->SetSizeHints(wxDefaultSize, wxDefaultSize);

  wxBoxSizer* topSizer;
  topSizer = new wxBoxSizer(wxVERTICAL);

  m_drawPanel = new wxPanel(
    this,
    wxID_ANY,
    wxDefaultPosition,
    wxDefaultSize,
    wxFULL_REPAINT_ON_RESIZE
  );
  m_drawPanel->SetMinSize(wxSize(400, 300));

  topSizer->Add(m_drawPanel, 1, wxEXPAND|wxALL, 1);

  this->SetSizer(topSizer);
  this->Layout();
  topSizer->Fit(this);
}

LoopOverlay::~LoopOverlay() {
}

void LoopOverlay::OnPaintEvent(wxPaintEvent & evt) {
    wxPaintDC dc(m_drawPanel);
    OnPaint(dc);
}

void LoopOverlay::OnPaint(wxDC& dc) {
  wxSize size = m_drawPanel->GetSize();
  int leftMargin = 20;
  int rightMargin = 10;
  int topMargin = 10;
  int bottomMargin = 10;
  int marginBetweenTracks = 0;
  int trackWidth = size.x - (leftMargin + rightMargin);
  leftMargin += (trackWidth % 40) / 2;
  // ensure width is divisible by 40
  trackWidth -= (trackWidth % 40);

  int trackHeight = (size.y - (topMargin + bottomMargin + ((m_channels - 1) * marginBetweenTracks))) / m_channels;

  dc.SetBrush(wxBrush(*wxWHITE));
  dc.SetPen(wxPen(*wxBLACK, 1, wxSOLID));

  // draw the track containing rectangles
  if (m_channels > 0) {
    for (int i = 0; i < m_channels; i++) {
      int x1, y1, x2, y2;
      x1 = leftMargin;
      y1 = topMargin + trackHeight * i + i * marginBetweenTracks;
      x2 = trackWidth + 1;
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
          leftMargin + trackWidth,
          zeroY
        );
      }

      // draw in the spline and sample points indicating the startpoint data
      dc.SetPen(wxPen(*wxBLUE, 1, wxSOLID));
      wxPoint startWave[41];
      for (int j = 0; j < 41; j++) {
        int x_value = leftMargin + (trackWidth / 40) * j;
        double y = m_startTracks[i].startData[j]; // real value from audio data
        y = (y - m_minValue) / m_valueRange; // normalized between min and max
        int y_value = topMargin + trackHeight - trackHeight * y + trackHeight * i;
        startWave[j] = wxPoint(x_value, y_value);
        // draw the samplepoint as 3 x 3 points with center as value
        for (int x = 0; x < 3; x++) {
          for (int y = 0; y < 3; y++)
            dc.DrawPoint(x_value - 1 + x, y_value - 1 + y);
        }
      }
      dc.SetPen(wxPen(*wxBLUE, 1, wxDOT));
      dc.DrawSpline(WXSIZEOF(startWave), startWave);

      // draw in the spline and sample points indicating the endpoint data
      dc.SetPen(wxPen(*wxRED, 1, wxSOLID));
      wxPoint endWave[41];
      for (int j = 0; j < 41; j++) {
        int x_value = leftMargin + (trackWidth / 40) * j;
        double y = m_endTracks[i].endData[j]; // real value from audio data
        y = (y - m_minValue) / m_valueRange; // normalized between min and max
        int y_value = topMargin + trackHeight - trackHeight * y + trackHeight * i;
        endWave[j] = wxPoint(x_value, y_value);
        // draw the samplepoint as 3 x 3 points with center as value
        for (int x = 0; x < 3; x++) {
          for (int y = 0; y < 3; y++)
            dc.DrawPoint(x_value - 1 + x, y_value - 1 + y);
        }
      }
      dc.SetPen(wxPen(*wxRED, 1, wxLONG_DASH));
      dc.DrawSpline(WXSIZEOF(endWave), endWave);

      // draw the vertical line that indicate where loopstart sample is
      dc.SetPen(wxPen(*wxLIGHT_GREY, 1, wxDOT_DASH));
      dc.DrawLine(
        leftMargin + (trackWidth / 40) * 20,
        topMargin + 1 + trackHeight * i,
        leftMargin + (trackWidth / 40) * 20,
        topMargin + 1 + trackHeight + trackHeight * i
      );

      // reset pen for next channel
      dc.SetPen(wxPen(*wxBLACK, 1, wxSOLID));
    }
  }
}

