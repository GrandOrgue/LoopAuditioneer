/* 
 * MySound.cpp is a part of LoopAuditioneer software
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

#include "MySound.h"
#include "FileHandling.h"
#include "MyFrame.h"

MySound::MySound() : m_audio(NULL), fmt(RTAUDIO_SINT16), bufferFrames(1024), sampleRateToUse(0) {
  for (int i = 0; i < 3; i++)
    pos[i] = 0;
  
  m_audio = new RtAudio();
  if ( m_audio->getDeviceCount() > 0 ) {
    parameters.deviceId = m_audio->getDefaultOutputDevice();
    parameters.nChannels = 2;
    parameters.firstChannel = 0;
  } else {
    // There was no audio device found!
  }
}

MySound::~MySound() {
  delete m_audio;
}

void MySound::SetSampleRate(int sampleRate) {
  sampleRateToUse = sampleRate;
}

void MySound::SetAudioFormat(int audioFormat) {
  if (audioFormat == 1)
    fmt = RTAUDIO_SINT8;   // Signed 8-bit integer
  else if (audioFormat == 2)
    fmt = RTAUDIO_SINT16;  // Signed 16-bit integer
  else if (audioFormat == 3)
    fmt = RTAUDIO_SINT32;  // Signed 24-bit integer (lower 3 bytes of 32-bit signed integer.)
  else if (audioFormat == 4)
    fmt = RTAUDIO_SINT32;  // Signed 32-bit integer
  else if (audioFormat == 6)
    fmt = RTAUDIO_FLOAT32; // 32-bit float normalized between +/- 1.0
  else if (audioFormat == 7)
    fmt = RTAUDIO_FLOAT64;
  else {
    // Unknown audio format
  }
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
      &options );   
  } catch ( RtError& e ) {
    e.printMessage();
  }
}

void MySound::StartAudioStream() {
  try {
    m_audio->startStream();
  } catch ( RtError& e ) {
    e.printMessage();
  }
}

void MySound::StopAudioStream() {
  if (m_audio->isStreamRunning()) {
    try {
      // Stop the stream
      m_audio->stopStream();
    } catch (RtError& e) {
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
  parameters.nChannels = channels;
}

