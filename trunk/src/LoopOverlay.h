/* 
 * LoopPointOverlay.h displays the waveforms overlayed at looppoints
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

#ifndef LOOPOVERLAY_H
#define LOOPOVERLAY_H

#include <wx/wx.h>
#include <vector>
#include <wx/spinctrl.h>
#include "FileHandling.h"

typedef struct {
  std::vector<double> startData;
} STARTAUDIO;

typedef struct {
  std::vector<double> endData;
} ENDAUDIO;

// Identifiers
enum {
  ID_PREV_LOOP = wxID_HIGHEST + 350,
  ID_NEXT_LOOP = wxID_HIGHEST + 351,
  ID_LOOPBEGIN = wxID_HIGHEST + 352,
  ID_LOOPSTOP = wxID_HIGHEST + 353,
  ID_WAVELENGTH = wxID_HIGHEST + 354,
  ID_STORE_CHANGES = wxID_HIGHEST + 355
};

class LoopOverlay : public wxDialog {
public:
  LoopOverlay(
    FileHandling *fh,
    int selectedLoop,
    wxWindow* parent,
    wxWindowID id = wxID_ANY,
    const wxString& title = wxT("Waveform overlay at looppoints"),
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxCLIP_CHILDREN|wxFULL_REPAINT_ON_RESIZE
  );
  ~LoopOverlay();

  bool GetHasChanged();

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
  wxPanel *m_drawingPanel;
  wxButton *m_prevLoop;
  wxButton *m_nextLoop;
  wxButton *m_storeChanges;
  wxStaticText *m_loopLabel;
  wxSpinCtrl* loopStartSpin;
  wxSpinCtrl* loopEndSpin;
  wxSpinCtrl* m_waveLength;
  FileHandling *m_fileReference;
  double *audioData;
  int m_selectedLoop;
  bool m_hasChanged;

  void OnPaintEvent(wxPaintEvent& evt);
  void OnPaint(wxDC& dc);
  void OnSize(wxSizeEvent& event);
  void UpdateAudioTracks();
  void SetLoopString();
  void DecideButtonState();
  void OnPrevButton(wxCommandEvent& event);
  void OnNextButton(wxCommandEvent& event);
  void UpdateSpinners();
  void OnLoopStartChange(wxSpinEvent& event);
  void OnLoopEndChange(wxSpinEvent& event);
  void ReadLoopData();
  void SetSampleSpinnerValues();
  void OnWaveLengthChange(wxSpinEvent& event);
  void OnStoreChanges(wxCommandEvent& event);
  void SetSaveButtonState();
  void PaintNow();

  // handle events
  DECLARE_EVENT_TABLE()
};

#endif
