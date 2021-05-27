/* 
 * StopHarmonicDialog.h is a part of LoopAuditioneer software
 * Copyright (C) 2012-2021 Lars Palo 
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

#ifndef STOPHARMONICDIALOG_H
#define STOPHARMONICDIALOG_H

#include <wx/wx.h>
#include <wx/spinctrl.h>

// Identifiers
enum {
  ID_HARMONICBOX = wxID_HIGHEST + 700,
  ID_PITCHCTRL = wxID_HIGHEST + 701
};

class StopHarmonicDialog : public wxDialog {
  DECLARE_CLASS(StopHarmonicDialog)
  DECLARE_EVENT_TABLE()

public:
  // Constructors
  StopHarmonicDialog();
  StopHarmonicDialog(
    wxWindow* parent,
    wxWindowID id = wxID_ANY,
    const wxString& caption = wxT("Stop harmonic selection"),
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
  );

  // Initialize our variables
  void Init();

  // Creation
  bool Create( 
    wxWindow* parent,
    wxWindowID id = wxID_ANY,
    const wxString& caption = wxT("Stop harmonic selection"),
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
  );

  // Creates the controls and sizers
  void CreateControls();

  // Accessor
  int GetSelectedHarmonic();
  double GetSelectedPitch();

private:
  int m_selectedHarmonic;
  double m_pitch;
  wxArrayString m_harmoniclist;
  wxChoice *m_harmonicChoiceBox;
  wxSpinCtrlDouble *m_pitchCtrl;

  // Event methods
  void OnChoiceSelected(wxCommandEvent& event);
  void OnPitchChanged(wxSpinDoubleEvent& event);

};

#endif

