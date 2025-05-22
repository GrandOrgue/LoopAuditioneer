/* 
 * LoopParametersDialog.h is a part of LoopAuditioneer software
 * Copyright (C) 2011-2025 Lars Palo and contributors (see AUTHORS file) 
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

#ifndef LOOPPARAMETERSDIALOG
#define LOOPPARAMETERSDIALOG

#include <wx/wx.h>
#include <wx/spinctrl.h>

// Identifiers
enum {
  ID_LOOPSTART = wxID_HIGHEST + 100,
  ID_LOOPEND = wxID_HIGHEST + 101
};

class LoopParametersDialog : public wxDialog {
  DECLARE_CLASS(LoopParametersDialog)
  DECLARE_EVENT_TABLE()

public:
  // Constructors
  LoopParametersDialog(unsigned int start, unsigned int end, unsigned int length);
  LoopParametersDialog(
    unsigned int start, 
    unsigned int end,
    unsigned int length,
    wxWindow* parent,
    wxWindowID id = wxID_ANY,
    const wxString& caption = wxT("Loop parameters"),
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
  );

  // Initialize our variables
  void Init(unsigned int start, unsigned int end, unsigned int length);

  // Creation
  bool Create( 
    wxWindow* parent,
    wxWindowID id = wxID_ANY,
    const wxString& caption = wxT("Loop parameters"),
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
  );

  // Creates the controls and sizers
  void CreateControls();

  // Accessing functions
  void SetLoopStart(unsigned int loopStart);
  unsigned int GetLoopStart();
  void SetLoopEnd(unsigned int loopEnd);
  unsigned int GetLoopEnd();
  void SetLastSample(unsigned int lastSample);

  // Overrides
  bool TransferDataToWindow();
  bool TransferDataFromWindow();

private:
  unsigned int m_loopStart;
  unsigned int m_loopEnd;
  unsigned int m_lastSample;

  void OnLoopStartSpin(wxSpinEvent& event);
  void OnLoopEndSpin(wxSpinEvent& event);
};

#endif
