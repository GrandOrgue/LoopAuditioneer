/*
 * MyResampler.h is a part of LoopAuditioneer software
 * Copyright (C) 2021-2024 Lars Palo and contributors (see AUTHORS file)
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

#ifndef MYRESAMPLER_H
#define MYRESAMPLER_H

#include <samplerate.h>
#include <wx/wx.h>

class MyResampler {
public:
  MyResampler(int channels);
  ~MyResampler();

  void ResetState();
  wxString GetErrorString();
  double GetRatioUsed();
  bool HasError();
  void SetDataIn(float *in);
  void SetDataInputFrames(long inFrames);
  void SetDataEndOfInput(int end);
  void SetDataSrcRatio(double ratio);
  void SimpleResample(int channels);
  
  float *resampledAudioData;
  long unsigned int m_resampledDataLength;

private:
  SRC_STATE *src_state;
  SRC_DATA src_data;
  int src_error;

};

#endif

