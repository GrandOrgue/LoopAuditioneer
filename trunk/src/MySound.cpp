/* 
 * MySound.cpp is a part of LoopAuditioneer software
 * Copyright (C) 2011-2021 Lars Palo 
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

MySound::MySound(wxString apiName, unsigned int deviceID) : m_audio(NULL), fmt(RTAUDIO_FLOAT32), bufferFrames(1024), sampleRateToUse(0) {
  for (int i = 0; i < 3; i++)
    pos[i] = 0;

  RtAudio::getCompiledApi(m_availableApis);
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
  if (std::find(m_availableApis.begin(), m_availableApis.end(), api) != m_availableApis.end()) {
    // the api should be valid
    m_audio = new RtAudio(api);
  } else {
    // use default api UNSPECIFIED
    m_audio = new RtAudio();
  }
  std::string api_str = RtAudio::getApiName(m_audio->getCurrentApi());
  m_api = wxString(api_str);
}

void MySound::SetAudioDevice(unsigned int devID) {
  if (m_audio->getDeviceCount() > 0) {
    if ((devID >= 0) && (devID < m_audio->getDeviceCount())) {
      // this should be a valid device
      m_deviceID = devID;
    } else {
      // just use the default device
      m_deviceID = m_audio->getDefaultOutputDevice();
    }
    parameters.deviceId = m_deviceID;
    info = m_audio->getDeviceInfo(m_deviceID);
  } else {
    // we must try using default Api and device instead
    SetApiToUse(RtAudio::UNSPECIFIED);
    SetAudioDevice(INT_MAX);
  }
}

void MySound::SetSampleRate(int sampleRate) {
  if (info.probed) {
    if (std::find(info.sampleRates.begin(), info.sampleRates.end(), sampleRate) != info.sampleRates.end()) {
      // device supports this samplerate
      sampleRateToUse = sampleRate;
      m_needsResampling = false;
    } else {
      // device doesn't support this samplerate we need to resample
      sampleRateToUse = info.preferredSampleRate;
      m_needsResampling = true;
    }
  } else {
    // TODO: this shouldn't happen, possibly replace with message
    sampleRateToUse = sampleRate;
    m_needsResampling = false;
  }
}

void MySound::SetAudioFormat(int audioFormat) {
  fmt = RTAUDIO_FLOAT32;
}

void MySound::OpenAudioStream() {
  try {
    m_audio->openStream(
      &parameters,
      NULL,
      fmt,
      sampleRateToUse,
      &bufferFrames,
      &MyFrame::AudioCallback,
      (void *)&pos,
      &options
    );
  } catch ( RtAudioError& e ) {
    e.printMessage();
  }
}

void MySound::StartAudioStream() {
  try {
    m_audio->startStream();
  } catch ( RtAudioError& e ) {
    e.printMessage();
  }
}

void MySound::StopAudioStream() {
  if (m_audio->isStreamRunning()) {
    try {
      // Stop the stream
      m_audio->stopStream();
    } catch (RtAudioError& e) {
      e.printMessage();
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
  if (info.probed) {
    if (channels <= info.outputChannels) {
      // we can safely use this number of channels
      parameters.nChannels = channels;
      m_channelsUsed = channels;
    } else {
      // the file contain more channels than device can handle
      parameters.nChannels = info.outputChannels;
      m_channelsUsed = info.outputChannels;
    }
  } else {
    // TODO: this shouldn't happen, possibly replace with message
    parameters.nChannels = channels;
    m_channelsUsed = channels;
  }
}

bool MySound::IsStreamActive() {
  if (m_audio->isStreamRunning()) {
    return true;
  } else {
    return false;
  }
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

