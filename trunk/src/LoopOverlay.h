/* 
 * LoopPointOverlay.h displays the waveforms overlayed at looppoints
 * Copyright (C) 2012-2015 Lars Palo 
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

#ifndef LOOPOVERLAY_H
#define LOOPOVERLAY_H

#include <wx/wx.h>
#include <vector>

typedef struct {
  std::vector<double> startData;
} STARTAUDIO;

typedef struct {
  std::vector<double> endData;
} ENDAUDIO;

class LoopOverlay : public wxDialog {
public:
  LoopOverlay(
    double audioData[],
    unsigned startSample,
    unsigned endSample,
    int channels,
    wxWindow* parent,
    wxWindowID id = wxID_ANY,
    const wxString& title = wxT("Waveform overlay at looppoints"),
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxBG_STYLE_PAINT|wxFULL_REPAINT_ON_RESIZE
  );
  ~LoopOverlay();

private:
  std::vector<STARTAUDIO> m_startTracks;
  std::vector<ENDAUDIO> m_endTracks;
  int m_channels;
  double m_maxValue;
  double m_minValue;
  double m_valueRange;

  void OnEraseBackground(wxEraseEvent& WXUNUSED(event));
  void OnPaintEvent(wxPaintEvent& evt);
  void OnPaint(wxDC& dc);

  // handle events
  DECLARE_EVENT_TABLE()
};

#endif
