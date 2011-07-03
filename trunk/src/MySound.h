/* 
 * MySound.h is a part of LoopAuditioneer software
 * Copyright (C) 2011 Lars Palo 
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

class MySound {
public:
  MySound();
  ~MySound();

  void SetSampleRate(int sampleRate);
  void SetAudioFormat(int audioFormat);
  void OpenAudioStream();
  void StartAudioStream();
  void StopAudioStream();
  void CloseAudioStream();
  void SetLoopPosition(unsigned int currentPos, unsigned int lStart, unsigned int lEnd, int n_channels);
  void SetStartPosition(unsigned int startPos, int n_channels);

private:
  RtAudio *m_audio;
  RtAudio::StreamParameters parameters;
  RtAudio::StreamOptions options;
  RtAudioFormat fmt;
  unsigned int bufferFrames;
  unsigned int sampleRateToUse;
  unsigned int pos[3]; // Used to keep track of position. [0] = current position, [1] = loop start, [2] = loop end
};

#endif
