/* 
 * MySound.h is a part of LoopAuditioneer software
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

#ifndef MYSOUND_H
#define MYSOUND_H

#include <wx/wx.h>
#include "RtAudio.h"
#include <vector>

class MySound {
public:
  MySound(wxString apiName, unsigned int deviceID);
  ~MySound();

  void SetApiToUse(RtAudio::Api api);
  void SetAudioDevice(unsigned int devID);
  wxString GetApi();
  unsigned int GetDevice();
  unsigned int GetSampleRateToUse();
  unsigned int GetChannelsUsed();
  wxString GetLastError();
  void SetSampleRate(int sampleRate);
  void SetAudioFormat(int audioFormat);
  void SetChannels(int channels);
  void OpenAudioStream();
  void StartAudioStream();
  void StopAudioStream();
  void CloseAudioStream();
  void SetLoopPosition(unsigned int currentPos, unsigned int lStart, unsigned int lEnd, int n_channels);
  void SetStartPosition(unsigned int startPos, int n_channels);
  bool IsStreamActive();
  bool IsStreamAvailable();
  bool IsJackUsed();
  bool StreamNeedsResampling();
  unsigned int pos[3]; // Used to keep track of position. [0] = current position, [1] = loop start, [2] = loop end
  std::vector< RtAudio::Api > m_availableApis;

private:
  RtAudio *m_audio;
  RtAudio::StreamParameters parameters;
  RtAudio::StreamOptions options;
  RtAudio::DeviceInfo info;
  RtAudioFormat fmt;
  unsigned int bufferFrames;
  unsigned int sampleRateToUse;
  unsigned int m_deviceID;
  unsigned int m_channelsUsed;
  wxString m_api;
  bool m_needsResampling;
  wxString m_lastError;
  bool m_isJackUsed;
};

#endif
