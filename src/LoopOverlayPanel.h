/*
 * LoopOverlayPanel.h draws the waveforms overlayed at looppoints
 * Copyright (C) 2024 Lars Palo and contributors (see AUTHORS file)
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

#ifndef LOOPOVERLAYPANEL_H
#define LOOPOVERLAYPANEL_H

#include <wx/wx.h>
#include <vector>
#include "FileHandling.h"

typedef struct {
  std::vector<double> startData;
} STARTAUDIO;

typedef struct {
  std::vector<double> endData;
} ENDAUDIO;

class LoopOverlayPanel : public wxPanel {
public:
  LoopOverlayPanel(
    FileHandling *fh,
    int selectedLoop,
    wxWindow* parent,
    wxWindowID id = wxID_ANY,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxFULL_REPAINT_ON_RESIZE
  );
  ~LoopOverlayPanel();

  int GetCurrentLoopEnd();
  int GetCurrentLoopStart();
  int GetNumberOfSamples();
  int GetMaxSamplesSpin();
  int GetSelectedLoop();

  void SetCurrentLoopEnd(int end);
  void SetCurrentLoopStart(int start);
  void SetNumberOfSamples(int samples);
  void SetSelectedLoop(int loop);

  void UpdateAudioTracks();
  void ReadLoopData();
  void PaintNow();

private:
  std::vector<STARTAUDIO> m_startTracks;
  std::vector<ENDAUDIO> m_endTracks;
  double m_maxValue;
  double m_minValue;
  double m_valueRange;
  int currentLoopstart;
  int currentLoopend;
  int m_numberOfSamples;
  int m_trackWidth;
  int m_maxSamplesSpinner;
  FileHandling *m_fileRef;
  double *audioData;
  int m_selectedLoop;

  void OnPaintEvent(wxPaintEvent& event);
  void OnPaint(wxDC& dc);
  void OnSize(wxSizeEvent& event);

  // handle events
  DECLARE_EVENT_TABLE()
};

#endif
