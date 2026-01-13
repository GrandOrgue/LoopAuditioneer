/* 
 * AudioSettingsDialog.h is a part of LoopAuditioneer software
 * Copyright (C) 2021-2026 Lars Palo and contributors (see AUTHORS file) 
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

#ifndef AUDIOSETTINGSDIALOG_H
#define AUDIOSETTINGSDIALOG_H

#include <wx/wx.h>
#include "MySound.h"
#include "RtAudio.h"

// Identifiers
enum {
  ID_SOUND_API = wxID_HIGHEST + 570,
  ID_SOUND_DEVICE = wxID_HIGHEST + 571
};

class AudioSettingsDialog : public wxDialog {
  DECLARE_CLASS(AudioSettingsDialog)
  DECLARE_EVENT_TABLE()

public:
  // Constructors
  AudioSettingsDialog(MySound *my_snd);
  AudioSettingsDialog(
    MySound *my_snd,
    wxWindow* parent,
    wxWindowID id = wxID_ANY,
    const wxString& caption = wxT("Audio Settings"),
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
  );

  // Initialize our variables
  void Init(MySound *my_snd);

  // Creation
  bool Create(
    wxWindow* parent,
    wxWindowID id = wxID_ANY,
    const wxString& caption = wxT("Audio Settings"),
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
  );

  // Creates the controls and sizers
  void CreateControls();

  // Accessors
  wxString GetSoundApi();
  unsigned int GetSoundDeviceId();
  
    // Overrides
  bool TransferDataToWindow();
  bool TransferDataFromWindow();

private:
  wxChoice *m_apiChoice;
  wxChoice *m_deviceChoice;
  wxString m_snd_api;
  unsigned int m_snd_device;
  wxArrayString m_availableApis;
  wxArrayString m_availableDevices;
  
  void UpdateAvailableDevices();
  wxString ConvertDeviceIdToString();
  void OnApiChoice(wxCommandEvent& event);
  void OnDeviceChoice(wxCommandEvent& event);
  void CheckIfOkCanBeEnabled();

};

#endif

