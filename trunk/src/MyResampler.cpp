/*
 * MyResampler.cpp is a part of LoopAuditioneer software
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

#include "MyResampler.h"
#include <vector>

MyResampler::MyResampler(int channels) : resampledAudioData(NULL) {
  // initialize samplerate converter
  src_state = src_new(SRC_SINC_MEDIUM_QUALITY, channels, &src_error);
}

MyResampler::~MyResampler() {
  // Cleanup samplerate converter
  if (src_state)
    src_delete(src_state);

  delete[] resampledAudioData;
}

void MyResampler::ResetState() {
  src_reset(src_state);
  delete[] resampledAudioData;
}

wxString MyResampler::GetErrorString() {
  wxString error_message = wxString::FromUTF8(src_strerror(src_error));
  return error_message;
}

double MyResampler::GetRatioUsed() {
  return src_data.src_ratio;
}

bool MyResampler::HasError() {
  if (src_error)
    return true;
  else
    return false;
}

void MyResampler::SetDataIn(float *in) {
  src_data.data_in = in;
}

void MyResampler::SetDataInputFrames(long inFrames) {
  src_data.input_frames = inFrames;
}

void MyResampler::SetDataEndOfInput(int end) {
  src_data.end_of_input = end;
}

void MyResampler::SetDataSrcRatio(double ratio) {
  src_data.src_ratio = ratio;
}

void MyResampler::SimpleResample(int channels) {
  float output[4096];
  src_data.data_out = output;
  src_data.output_frames = 4096 / channels;
  std::vector<float> tempData;
  
  while (1) {
    if (src_data.input_frames < 4096 / channels)
      src_data.end_of_input = 1;

    src_process(src_state, &src_data);
    for (long int i = 0; i < src_data.output_frames_gen * channels; i++) {
      tempData.push_back(output[i]);
    }
    
    /* Terminate if done. */
    if (src_data.end_of_input && src_data.output_frames_gen == 0)
      break ;
			
    src_data.data_in += src_data.input_frames_used * channels;
    src_data.input_frames -= src_data.input_frames_used;
  }
  
  m_resampledDataLength = tempData.size();
  resampledAudioData = new float[m_resampledDataLength];
  for (long unsigned int i = 0; i < m_resampledDataLength; i++)
    resampledAudioData[i] = tempData[i];
}

