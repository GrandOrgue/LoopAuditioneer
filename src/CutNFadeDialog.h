/* 
 * CutNFadeDialog.h is a part of LoopAuditioneer software
 * Copyright (C) 2014-2024 Lars Palo and contributors (see AUTHORS file) 
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

#ifndef CUTNFADEDIALOG
#define CUTNFADEDIALOG

#include <wx/wx.h>
#include <wx/spinctrl.h>

// Identifiers
enum {
  ID_CUTSTART = wxID_HIGHEST + 550,
  ID_CUTEND = wxID_HIGHEST + 551,
  ID_FADESTART = wxID_HIGHEST + 552,
  ID_FADEEND = wxID_HIGHEST + 553
};

class CutNFadeDialog : public wxDialog {
  DECLARE_CLASS(CutNFadeDialog)
  DECLARE_EVENT_TABLE()

public:
  // Constructors
  CutNFadeDialog();
  CutNFadeDialog(
    wxWindow* parent,
    wxWindowID id = wxID_ANY,
    const wxString& caption = wxT("Cut & Fade"),
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
    const wxString& caption = wxT("Cut & Fade"),
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
  );

  // Creates the controls and sizers
  void CreateControls();

  // Accessing functions
  void SetCutStart(unsigned cutStart);
  unsigned GetCutStart();
  void SetCutEnd(unsigned cutEnd);
  unsigned GetCutEnd();
  void SetFadeStart(unsigned fadeStart);
  unsigned GetFadeStart();
  void SetFadeEnd(unsigned fadeEnd);
  unsigned GetFadeEnd();

  // Overrides
  bool TransferDataToWindow();
  bool TransferDataFromWindow();

private:
  unsigned m_cutStart;
  unsigned m_cutEnd;
  unsigned m_fadeStart;
  unsigned m_fadeEnd;

  void OnCutStartSpin(wxSpinEvent& event);
  void OnCutEndSpin(wxSpinEvent& event);
  void OnFadeStartSpin(wxSpinEvent& event);
  void OnFadeEndSpin(wxSpinEvent& event);
};

#endif
