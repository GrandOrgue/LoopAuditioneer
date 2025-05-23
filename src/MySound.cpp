/* 
 * MySound.cpp is a part of LoopAuditioneer software
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

#include "MySound.h"
#include "FileHandling.h"
#include "MyFrame.h"
#include <algorithm>
#include <climits>

MySound::MySound(wxString apiName, unsigned int deviceID) : m_audio(NULL), fmt(RTAUDIO_FLOAT32), bufferFrames(1024), sampleRateToUse(0), m_lastError(wxEmptyString) {
  for (int i = 0; i < 3; i++)
    pos[i] = 0;

  RtAudio::getCompiledApi(m_availableApis);

  m_isJackUsed = false;
  if (std::find(m_availableApis.begin(), m_availableApis.end(), RtAudio::Api::UNIX_JACK) != m_availableApis.end()) {
    RtAudio testForJack(RtAudio::Api::UNIX_JACK);
    if (testForJack.getDeviceCount() > 0)
      m_isJackUsed = true;
  }

  SetApiToUse(RtAudio::getCompiledApiByName(std::string(apiName.mb_str())));
  SetAudioDevice(deviceID);
  m_needsResampling = false;
}

MySound::~MySound() {
  delete m_audio;
}

void MySound::SetApiToUse(RtAudio::Api api) {
  if (m_audio != NULL) {
    StopAudioStream();
    CloseAudioStream();
    delete m_audio;
    m_audio = 0;
  }

  if (!m_isJackUsed) {
    if (std::find(m_availableApis.begin(), m_availableApis.end(), api) != m_availableApis.end() && api != RtAudio::Api::UNIX_JACK) {
      // the api should be valid
      m_audio = new RtAudio(api);
    } else {
      // use default api UNSPECIFIED as the asked api isn't currently valid
      m_audio = new RtAudio();
    }
  } else {
    // since Jack obviously anyway is available and used we just set that api
    m_audio = new RtAudio(RtAudio::Api::UNIX_JACK);
  }
  std::string api_str = RtAudio::getApiName(m_audio->getCurrentApi());
  m_api = wxString(api_str);

}

void MySound::SetAudioDevice(unsigned int devID) {
  std::vector<unsigned> deviceIds = m_audio->getDeviceIds();
  if (deviceIds.size() > 0) {
    if (std::find(deviceIds.begin(), deviceIds.end(), devID) != deviceIds.end()) {
      // this should be a valid device id
      m_deviceID = devID;
    } else {
      // just use the default device
      m_deviceID = m_audio->getDefaultOutputDevice();
    }
  } else {
    // we must try using default Api and device instead which might be invalid
    SetApiToUse(RtAudio::UNSPECIFIED);
    m_deviceID = m_audio->getDefaultOutputDevice();
  }

  parameters.deviceId = m_deviceID;
  info = m_audio->getDeviceInfo(m_deviceID);
  options.streamName = "LoopAuditioneer";
}

void MySound::SetSampleRate(int sampleRate) {
  if (std::find(info.sampleRates.begin(), info.sampleRates.end(), sampleRate) != info.sampleRates.end()) {
    // device supports this samplerate
    sampleRateToUse = sampleRate;
    m_needsResampling = false;
  } else {
    // device doesn't support this samplerate we need to resample
    sampleRateToUse = info.preferredSampleRate;
    m_needsResampling = true;
  }
}

void MySound::SetAudioFormat(int audioFormat) {
  (void)audioFormat;
  fmt = RTAUDIO_FLOAT32;
}

void MySound::OpenAudioStream() {
  if (
    m_audio->openStream(
      &parameters,
      NULL,
      fmt,
      sampleRateToUse,
      &bufferFrames,
      &MyFrame::AudioCallback,
      (void *)&pos,
      &options
    ) == RTAUDIO_NO_ERROR) {
    // All is fine
    m_lastError = wxEmptyString;
  } else {
    // Some kind of error has happened
    m_lastError = wxString(m_audio->getErrorText());
    m_audio->abortStream();
  }
}

void MySound::StartAudioStream() {
  if (m_audio->startStream() == RTAUDIO_NO_ERROR) {
    // All is fine
    m_lastError = wxEmptyString;
  } else {
    // Some kind of error has happened
    m_lastError = wxString(m_audio->getErrorText());
    m_audio->abortStream();
  }
}

void MySound::StopAudioStream() {
  if (m_audio->isStreamRunning()) {
    if (m_audio->stopStream() == RTAUDIO_NO_ERROR) {
      // All is fine
      m_lastError = wxEmptyString;
    } else {
      // Some kind of error has happened
      m_lastError = wxString(m_audio->getErrorText());
      m_audio->abortStream();
    }
  }
}

void MySound::CloseAudioStream() {
  if (m_audio->isStreamOpen()) 
    m_audio->closeStream();
}

void MySound::SetLoopPosition(unsigned int currentPos, unsigned int lStart, unsigned int lEnd, int n_channels) {
  // positions are all numbers in frames! and we need them in items for the data array
  pos[0] = currentPos * n_channels;
  pos[1] = lStart * n_channels;
  pos[2] = lEnd * n_channels + (n_channels - 1);
}

void MySound::SetStartPosition(unsigned int startPos, int n_channels) {
  pos[0] = startPos * n_channels;
}

void MySound::SetChannels(int channels) {
  if ((unsigned) channels <= info.outputChannels) {
    // we can safely use this number of channels
    parameters.nChannels = channels;
    m_channelsUsed = channels;
  } else {
    // the file contain more channels than device can handle
    parameters.nChannels = info.outputChannels;
    m_channelsUsed = info.outputChannels;
  }
}

bool MySound::IsStreamActive() {
  if (m_audio->isStreamRunning()) {
    return true;
  } else {
    return false;
  }
}

bool MySound::IsStreamAvailable() {
  return m_audio->isStreamOpen();
}

wxString MySound::GetApi() {
  return m_api;
}

unsigned int MySound::GetDevice() {
  return m_deviceID;
}

unsigned int MySound::GetSampleRateToUse() {
  return sampleRateToUse;
}

bool MySound::StreamNeedsResampling() {
  return m_needsResampling;
}

unsigned int MySound::GetChannelsUsed() {
  return m_channelsUsed;
}

wxString MySound::GetLastError() {
  return m_lastError;
}

bool MySound::IsJackUsed() {
  return m_isJackUsed;
}
