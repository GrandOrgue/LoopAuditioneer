/* 
 * AudioSettingsDialog.cpp is a part of LoopAuditioneer software
 * Copyright (C) 2021 Lars Palo 
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
  for (int i = 0; i < my_snd->m_availableApis.size(); i++)
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
  wxChoice *apiChoice = new wxChoice(
    this,
    ID_SOUND_API,
    wxDefaultPosition,
    wxDefaultSize,
    m_availableApis
  );
  boxSizer->Add(apiChoice, 0, wxEXPAND|wxALL, 5);
  apiChoice->SetStringSelection(m_snd_api);

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
  wxChoice *deviceChoice = new wxChoice(
    this,
    ID_SOUND_DEVICE,
    wxDefaultPosition,
    wxDefaultSize,
    m_availableDevices
  );
  boxSizer->Add(deviceChoice, 0, wxEXPAND|wxALL, 5);
  deviceChoice->SetSelection(m_snd_device);

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
  wxChoice *apiChoice = (wxChoice*) FindWindow(ID_SOUND_API);
  wxChoice *deviceChoice = (wxChoice*) FindWindow(ID_SOUND_DEVICE);
  
  apiChoice->SetStringSelection(m_snd_api);
  deviceChoice->SetSelection(m_snd_device);
  
  return true;
}

// Override of transfer data from the window
bool AudioSettingsDialog::TransferDataFromWindow() {
  wxChoice *apiChoice = (wxChoice*) FindWindow(ID_SOUND_API);
  wxChoice *deviceChoice = (wxChoice*) FindWindow(ID_SOUND_DEVICE);
  
  m_snd_api = m_availableApis.Item(apiChoice->GetSelection());
  m_snd_device = deviceChoice->GetSelection();
  
  return true;
}

void AudioSettingsDialog::UpdateAvailableDevices() {
  RtAudio audio(RtAudio::getCompiledApiByName(std::string(m_snd_api.mb_str())));
  RtAudio::DeviceInfo info;
  unsigned int devices = audio.getDeviceCount();
  m_availableDevices.Empty();

  for (unsigned int i = 0; i < devices; i++) {
    info = audio.getDeviceInfo(i);
    m_availableDevices.Add(wxString(info.name));
  }
}

void AudioSettingsDialog::OnApiChoice(wxCommandEvent& event) {
  wxChoice *apiChoice = (wxChoice*) FindWindow(ID_SOUND_API);
  wxString choiceStr = m_availableApis.Item(apiChoice->GetSelection());
  if (!choiceStr.IsSameAs(m_snd_api)) {
    m_snd_api = choiceStr;
    UpdateAvailableDevices();
    
    wxChoice *deviceChoice = (wxChoice*) FindWindow(ID_SOUND_DEVICE);
    deviceChoice->Set(m_availableDevices);
    deviceChoice->SetSelection(wxNOT_FOUND);
    m_snd_device = UINT_MAX; // since no device yet is chosen for this api
  }
  CheckIfOkCanBeEnabled();
}

void AudioSettingsDialog::OnDeviceChoice(wxCommandEvent& event) {
  wxChoice *deviceChoice = (wxChoice*) FindWindow(ID_SOUND_DEVICE);
  unsigned int deviceIdx = deviceChoice->GetSelection();
  if (deviceIdx != m_snd_device) {
    m_snd_device = deviceIdx;
  }
  CheckIfOkCanBeEnabled();
}

void AudioSettingsDialog::CheckIfOkCanBeEnabled() {
  wxChoice *apiChoice = (wxChoice*) FindWindow(ID_SOUND_API);
  wxChoice *deviceChoice = (wxChoice*) FindWindow(ID_SOUND_DEVICE);
  wxButton *okButton = (wxButton*) FindWindow(wxID_OK);
  if ((apiChoice->GetSelection() != wxNOT_FOUND) && (deviceChoice->GetSelection() != wxNOT_FOUND)) {
    okButton->Enable(true);
  } else {
    okButton->Enable(false);
  }
}

