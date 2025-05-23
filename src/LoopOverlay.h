/* 
 * LoopOverlay.h displays the waveforms overlayed at looppoints
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

#ifndef LOOPOVERLAY_H
#define LOOPOVERLAY_H

#include <wx/wx.h>
#include <vector>
#include <wx/spinctrl.h>
#include "FileHandling.h"
#include "LoopOverlayPanel.h"

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
    long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER
  );
  ~LoopOverlay();

  bool GetHasChanged();
  void SetSampleSpinnerValues();

private:
  LoopOverlayPanel *m_drawingPanel;
  wxButton *m_prevLoop;
  wxButton *m_nextLoop;
  wxButton *m_storeChanges;
  wxStaticText *m_loopLabel;
  wxSpinCtrl* loopStartSpin;
  wxSpinCtrl* loopEndSpin;
  wxSpinCtrl* m_waveLength;
  FileHandling *m_fileReference;
  bool m_hasChanged;

  void SetLoopString();
  void DecideButtonState();
  void OnPrevButton(wxCommandEvent& event);
  void OnNextButton(wxCommandEvent& event);
  void UpdateSpinners();
  void OnLoopStartChange(wxSpinEvent& event);
  void OnLoopEndChange(wxSpinEvent& event);
  void OnWaveLengthChange(wxSpinEvent& event);
  void OnStoreChanges(wxCommandEvent& event);
  void SetSaveButtonState();

  // handle events
  DECLARE_EVENT_TABLE()
};

#endif
