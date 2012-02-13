/* 
 * CrossfadeDialog.h provide a GUI for setting parameters for Crossfading
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

#ifndef CROSSFADEDIALOG_H
#define CROSSFADEDIALOG_H

#include <wx/wx.h>

// Identifiers
enum {
  ID_FADEDURATION = wxID_HIGHEST + 500,
  ID_FADEMETHOD = wxID_HIGHEST + 501
};

class CrossfadeDialog : public wxDialog {
  DECLARE_CLASS(CrossfadeDialog)
  DECLARE_EVENT_TABLE()

public:
  // Constructors
  CrossfadeDialog();
  CrossfadeDialog(
    wxWindow* parent,
    wxWindowID id = wxID_ANY,
    const wxString& caption = wxT("Parameters for crossfading loops"),
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
    const wxString& caption = wxT("Parameters for crossfading loops"),
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
  );

  // Creates the controls and sizers
  void CreateControls();

  // Accessing functions
  double GetFadeduration();
  int GetFadetype();

  // Overrides
  bool TransferDataToWindow();
  bool TransferDataFromWindow();
  void SetCaption(wxString str);

  // Event processing methods
  void OnFadedurationSlider(wxCommandEvent& event);
  void OnFademethodSelection(wxCommandEvent& event);

private:
  double m_fadeduration;  // in seconds (default 50 ms = 0.05, range 1 ms to 1000 ms)
  wxArrayString m_fademethods; // linear, equal power
  int selectedMethod; // index of fademethods linear = 0 as default
};


#endif
