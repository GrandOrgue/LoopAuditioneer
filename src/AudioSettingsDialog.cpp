/* 
 * AudioSettingsDialog.cpp is a part of LoopAuditioneer software
 * Copyright (C) 2021-2023 Lars Palo 
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

#include "AudioSettingsDialog.h"
#include <wx/statline.h>
#include <wx/choice.h>
#include <algorithm>

IMPLEMENT_CLASS(AudioSettingsDialog, wxDialog )

// Event table
BEGIN_EVENT_TABLE(AudioSettingsDialog, wxDialog)
  EVT_CHOICE(ID_SOUND_API, AudioSettingsDialog::OnApiChoice)
  EVT_CHOICE(ID_SOUND_DEVICE, AudioSettingsDialog::OnDeviceChoice)
END_EVENT_TABLE()

AudioSettingsDialog::AudioSettingsDialog(MySound *my_snd) {
  Init(my_snd);
}

AudioSettingsDialog::AudioSettingsDialog(
    MySound *my_snd,
    wxWindow* parent,
    wxWindowID id,
    const wxString& caption,
    const wxPoint& pos,
    const wxSize& size,
    long style
  ) {
  Init(my_snd);
  Create(parent, id, caption, pos, size, style);
}

void AudioSettingsDialog::Init(MySound *my_snd) {
  m_snd_api = my_snd->GetApi();
  m_snd_device = my_snd->GetDevice();
  for (unsigned i = 0; i < my_snd->m_availableApis.size(); i++)
    m_availableApis.Add(wxString(RtAudio::getApiName(my_snd->m_availableApis[i])));
  UpdateAvailableDevices();
}

bool AudioSettingsDialog::Create(
  wxWindow* parent,
  wxWindowID id,
  const wxString& caption,
  const wxPoint& pos,
  const wxSize& size,
  long style
) {
  if (!wxDialog::Create(parent, id, caption, pos, size, style))
    return false;

  CreateControls();

  GetSizer()->Fit(this);
  GetSizer()->SetSizeHints(this);
  Centre();

  return true;
}

void AudioSettingsDialog::CreateControls() {
  // Create a top level sizer
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  this->SetSizer(topSizer);

  // Second box sizer to get nice margins
  wxBoxSizer *boxSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(boxSizer, 0, wxEXPAND|wxALL, 5);

  // Information message at top
  wxStaticText *infoMessage = new wxStaticText(
    this,
    wxID_STATIC,
    wxT("Select audio Api and Device."),
    wxDefaultPosition,
    wxDefaultSize,
    0
  );
  boxSizer->Add(infoMessage, 0, wxALIGN_LEFT|wxALL, 5);
  
  // A horizontal line after information messagew
  wxStaticLine *dividerline = new wxStaticLine(
    this,
    wxID_STATIC,
    wxDefaultPosition,
    wxDefaultSize,
    wxLI_HORIZONTAL
  );
  boxSizer->Add(dividerline, 0, wxGROW|wxALL, 5);

  // Label for api
  wxStaticText *apiLabel = new wxStaticText(
    this,
    wxID_STATIC,
    wxT("Api: "),
    wxDefaultPosition,
    wxSize(100,-1),
    0
  );
  boxSizer->Add(apiLabel, 0, wxALL, 2);

  // wxChoice for audio api
  m_apiChoice = new wxChoice(
    this,
    ID_SOUND_API,
    wxDefaultPosition,
    wxDefaultSize,
    m_availableApis
  );
  boxSizer->Add(m_apiChoice, 0, wxEXPAND|wxALL, 5);
  m_apiChoice->SetStringSelection(m_snd_api);

  // Label for audio device
  wxStaticText *deviceLabel = new wxStaticText(
    this,
    wxID_STATIC,
    wxT("Device: "),
    wxDefaultPosition,
    wxSize(100,-1),
    0
  );
  boxSizer->Add(deviceLabel, 0, wxALL, 2);

  // wxChoice for device
  m_deviceChoice = new wxChoice(
    this,
    ID_SOUND_DEVICE,
    wxDefaultPosition,
    wxDefaultSize,
    m_availableDevices
  );
  boxSizer->Add(m_deviceChoice, 0, wxEXPAND|wxALL, 5);
  if (ConvertDeviceIdToString() != wxEmptyString)
    m_deviceChoice->SetStringSelection(ConvertDeviceIdToString());

  // A horizontal line before the buttons
  wxStaticLine *bottomline = new wxStaticLine(
    this,
    wxID_STATIC,
    wxDefaultPosition,
    wxDefaultSize,
    wxLI_HORIZONTAL
  );
  boxSizer->Add(bottomline, 0, wxGROW|wxALL, 5);

  // Horizontal sizer for buttons
  wxBoxSizer* buttonRow = new wxBoxSizer(wxHORIZONTAL);
  boxSizer->Add(buttonRow, 0, wxEXPAND|wxALL, 5);

  // The OK button
  wxButton *okButton = new wxButton(
    this,
    wxID_OK,
    wxT("&Ok"),
    wxDefaultPosition,
    wxDefaultSize,
    0
  );
  buttonRow->Add(okButton, 0, wxALIGN_CENTER|wxALL, 5);

  // The Cancel button
  wxButton *cancelButton = new wxButton(
    this,
    wxID_CANCEL,
    wxT("&Cancel"),
    wxDefaultPosition,
    wxDefaultSize,
    0
  );
  buttonRow->Add(cancelButton, 0, wxALIGN_CENTER|wxALL, 5);

  CheckIfOkCanBeEnabled();
}

wxString AudioSettingsDialog::GetSoundApi() {  return m_snd_api;}
unsigned int AudioSettingsDialog::GetSoundDeviceId() {  return m_snd_device;}

// Override of transfer data to the window
bool AudioSettingsDialog::TransferDataToWindow() {
  m_apiChoice->SetStringSelection(m_snd_api);
  m_deviceChoice->SetStringSelection(ConvertDeviceIdToString());
  
  return true;
}

// Override of transfer data from the window
bool AudioSettingsDialog::TransferDataFromWindow() {
  m_snd_api = m_availableApis.Item(m_apiChoice->GetSelection());
  
  return true;
}

void AudioSettingsDialog::UpdateAvailableDevices() {
  RtAudio audio(RtAudio::getCompiledApiByName(std::string(m_snd_api.mb_str())));
  RtAudio::DeviceInfo info;
  m_availableDevices.Empty();
  std::vector<unsigned> ids = audio.getDeviceIds();
  if (ids.size() == 0) {
    return;
  }

  for (unsigned i = 0; i < ids.size(); i++) {
    info = audio.getDeviceInfo(ids[i]);
    m_availableDevices.Add(wxString(info.name));
  }
}

wxString AudioSettingsDialog::ConvertDeviceIdToString() {
  RtAudio audio(RtAudio::getCompiledApiByName(std::string(m_snd_api.mb_str())));
  std::vector<unsigned> ids = audio.getDeviceIds();

  if (std::find(ids.begin(), ids.end(), m_snd_device) != ids.end()) {
    RtAudio::DeviceInfo info;
    info = audio.getDeviceInfo(m_snd_device);
  	return wxString(info.name);
  } else {
    return wxEmptyString;
  }
}

void AudioSettingsDialog::OnApiChoice(wxCommandEvent& WXUNUSED(event)) {
  wxString choiceStr = m_availableApis.Item(m_apiChoice->GetSelection());
  if (!choiceStr.IsSameAs(m_snd_api)) {
    m_snd_api = choiceStr;
    UpdateAvailableDevices();
    
    m_deviceChoice->Set(m_availableDevices);
    m_deviceChoice->SetSelection(wxNOT_FOUND);
    m_snd_device = UINT_MAX; // since no device yet is chosen for this api
  }
  CheckIfOkCanBeEnabled();
}

void AudioSettingsDialog::OnDeviceChoice(wxCommandEvent& WXUNUSED(event)) {
  int selectedIdx = m_deviceChoice->GetSelection();
  RtAudio audio(RtAudio::getCompiledApiByName(std::string(m_snd_api.mb_str())));
  std::vector<unsigned> ids = audio.getDeviceIds();
  if (selectedIdx < 0 || ids.size() < 1 || selectedIdx > (int) (ids.size() - 1)) {
    m_deviceChoice->SetSelection(wxNOT_FOUND);
    m_snd_device = UINT_MAX; // since no device yet is chosen for this api
  } else
    m_snd_device = ids[selectedIdx];
  CheckIfOkCanBeEnabled();
}

void AudioSettingsDialog::CheckIfOkCanBeEnabled() {
  wxButton *okButton = (wxButton*) FindWindow(wxID_OK);
  if ((m_apiChoice->GetSelection() != wxNOT_FOUND) && (m_deviceChoice->GetSelection() != wxNOT_FOUND)) {
    okButton->Enable(true);
  } else {
    okButton->Enable(false);
  }
}

