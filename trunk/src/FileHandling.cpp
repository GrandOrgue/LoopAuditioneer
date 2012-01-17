/*
 * FileHandling.cpp is a part of LoopAuditioneer software
 * Copyright (C) 2011-2012 Lars Palo
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

#include "FileHandling.h"
#include "FFT.h"

FileHandling::FileHandling(wxString fileName, wxString path) : m_loops(NULL), m_cues(NULL), shortAudioData(NULL), intAudioData(NULL), doubleAudioData(NULL), fileOpenWasSuccessful(false), m_fftPitch(0), m_timeDomainPitch(0) {
  m_loops = new LoopMarkers();
  m_cues = new CueMarkers();
  wxString filePath;
  filePath = path.Append(wxT("/"));
  filePath += fileName;

  // Here we get all the info about the file to be able to later open new file in write mode if changes should be saved
  SndfileHandle sfHandle;

  sfHandle = SndfileHandle(((const char*)filePath.mb_str())); // Open file only for read first to get all info

  if (sfHandle) { // checking if opening file was succesful or not

    m_format = sfHandle.format();
    m_samplerate = sfHandle.samplerate();
    m_channels = sfHandle.channels();
    m_minorFormat = sfHandle.format() & SF_FORMAT_SUBMASK;

    // Try to get loop info from the file
    if (sfHandle.command(4304, &instr, sizeof(instr)) == SF_TRUE) {
      // There are loops!

      m_loops->SetMIDIUnityNote(instr.basenote);
      m_loops->SetMIDIPitchFraction(instr.dwMIDIPitchFraction);

      // Stort all the loops into m_loops loopsIn vector
      for (int i = 0; i < instr.loop_count; i++) {
        LOOPDATA temp;
        temp.dwType = instr.loops[i].mode;
        temp.dwStart = instr.loops[i].start;
        temp.dwEnd = instr.loops[i].end - 1; // -1 to compensate for libsndfile behaviour
        temp.dwPlayCount = instr.loops[i].count;
        temp.shouldBeSaved = true;
        m_loops->AddLoop(temp);
      }
    }

    // Try to get cue info from file
    if (sfHandle.command(4302, &cues, sizeof(cues)) == SF_TRUE) {
      // There are cues!

      // Check if the cue is a real cue or a label, only keep real cues!
      for (int i = 0; i < cues.cue_count; i++) {
        bool toAdd = true;

        for (int j = 0; j < instr.loop_count; j++) {
          if (cues.cue_points[i].dwSampleOffset == instr.loops[j].start)
            toAdd = false;
        }

        if (toAdd) {
          CUEPOINT tmp;
          tmp.dwName = cues.cue_points[i].dwName;
          tmp.dwPosition = cues.cue_points[i].dwPosition;
          tmp.fccChunk = cues.cue_points[i].fccChunk;
          tmp.dwChunkStart = cues.cue_points[i].dwChunkStart;
          tmp.dwBlockStart = cues.cue_points[i].dwBlockStart;
          tmp.dwSampleOffset = cues.cue_points[i].dwSampleOffset;
          tmp.keepThisCue = true;

          m_cues->AddCue(tmp);
        }
      }
    }

    // Decide what format to store audio data as and copy it
    if ((m_minorFormat == SF_FORMAT_DOUBLE) || (m_minorFormat == SF_FORMAT_FLOAT)) {
      ArrayLength = sfHandle.frames() * sfHandle.channels();
      doubleAudioData = new double[ArrayLength];
      sfHandle.read(doubleAudioData, ArrayLength);

      fileOpenWasSuccessful = true;
    } else if ((m_minorFormat == SF_FORMAT_PCM_16) || (m_minorFormat == SF_FORMAT_PCM_S8)) {
      ArrayLength = sfHandle.frames() * sfHandle.channels();
      shortAudioData = new short[ArrayLength];
      sfHandle.read(shortAudioData, ArrayLength);

      fileOpenWasSuccessful = true;
    } else if ((m_minorFormat == SF_FORMAT_PCM_24) || (m_minorFormat == SF_FORMAT_PCM_32)) {
      ArrayLength = sfHandle.frames() * sfHandle.channels();
      intAudioData = new int[ArrayLength];
      sfHandle.read(intAudioData, ArrayLength);

      fileOpenWasSuccessful = true;
    } else {
      // file didn't contain any audio data
      fileOpenWasSuccessful = false;
    }

  } else { // if file open didn't succeed we make a note of that
    fileOpenWasSuccessful = false;
  }
}

FileHandling::~FileHandling() {
  delete m_loops;

  delete m_cues;

  delete[] shortAudioData;

  delete[] intAudioData;

  delete[] doubleAudioData;
}

void FileHandling::SaveAudioFile(wxString fileName, wxString path) {
  wxString filePath;
  filePath = path.Append(wxT("/"));
  filePath += fileName;

  // This we open the file write
  sfh = SndfileHandle(((const char*)filePath.mb_str()), SFM_WRITE, m_format, m_channels, m_samplerate);

  // Deal with the loops first
  m_loops->ExportLoops();
  instr.basenote = m_loops->GetMIDIUnityNote();
  instr.dwMIDIPitchFraction = m_loops->GetMIDIPitchFraction();
  instr.loop_count = m_loops->loopsOut.size();
  for (int i = 0; i < instr.loop_count; i++) {
    if (i > 15)
      break;
    instr.loops[i].mode = m_loops->loopsOut[i].dwType;
    instr.loops[i].start = m_loops->loopsOut[i].dwStart;
    instr.loops[i].end = m_loops->loopsOut[i].dwEnd;
    instr.loops[i].count = m_loops->loopsOut[i].dwPlayCount;
  }
  sfh.command(4305, &instr, sizeof(instr)); // this writes the loops metadata

  // Then take care of the cues
  m_cues->ExportCues();
  cues.cue_count = m_cues->exportedCues.size();
  for (int i = 0; i < cues.cue_count; i++) {
    if (i > 99)
      break;
    cues.cue_points[i].dwName =  m_cues->exportedCues[i].dwName;
    cues.cue_points[i].dwPosition =  m_cues->exportedCues[i].dwPosition;
    cues.cue_points[i].fccChunk =  m_cues->exportedCues[i].fccChunk;
    cues.cue_points[i].dwChunkStart =  m_cues->exportedCues[i].dwChunkStart;
    cues.cue_points[i].dwBlockStart =  m_cues->exportedCues[i].dwBlockStart;
    cues.cue_points[i].dwSampleOffset =  m_cues->exportedCues[i].dwSampleOffset;
  }
  sfh.command(4303, &cues, sizeof(cues));

  // Finally write the data back
  if ((m_minorFormat == SF_FORMAT_DOUBLE) || (m_minorFormat == SF_FORMAT_FLOAT)) {
    sfh.write(doubleAudioData, ArrayLength);
  } else if ((m_minorFormat == SF_FORMAT_PCM_16) || (m_minorFormat == SF_FORMAT_PCM_S8)) {
    sfh.write(shortAudioData, ArrayLength);
  } else {
    sfh.write(intAudioData, ArrayLength);
  }
  // File will be finally closed when the instance is deleted!
}

int FileHandling::GetSampleRate() {
  return m_samplerate;
}

int FileHandling::GetAudioFormat() {
  return m_minorFormat;
}

bool FileHandling::FileCouldBeOpened() {
  if (fileOpenWasSuccessful)
    return true;
  else
    return false;
}

double FileHandling::GetFFTPitch(double data[]) {
  bool gotPitch = DetectPitchByFFT(data);
  if (gotPitch)
    return m_fftPitch;
  else
    return 0;
}

bool FileHandling::DetectPitchByFFT(double data[]) {
  std::vector<double> detectedPitches;
  unsigned numberOfSamples = ArrayLength / m_channels;
  unsigned sustainStartIndex, sustainEndIndex;
  double *channel_data = new double[numberOfSamples];
  int channel_idx;

  // detect strongest channel
  if (m_channels > 1) {
    double strongestValue = 0;
    for (int i = 0; i < m_channels; i++) {
      for (unsigned j = i; j < ArrayLength; j += m_channels) {
        if (fabs(data[j]) > strongestValue) {
          strongestValue = fabs(data[j]);
          channel_idx = i;
        }
      }
    }
  } else {
    channel_idx = 0;
  }
 
  // fill channel_data array with values from data[]
  int ch_idx = 0;
  for (int data_idx = channel_idx; data_idx < ArrayLength; data_idx += m_channels) {
    channel_data[ch_idx] = data[data_idx];
    ch_idx++;
  }

  // Find strongest value
  double maxValue = 0;
  for (unsigned idx = 0; idx < numberOfSamples; idx++) {
    double currentValue = fabs(channel_data[idx]);

    if (currentValue > maxValue)
      maxValue = currentValue;
  }

  // now detect sustain section
  // set a windowsize for a 20 Hz frequency in current file (mono now!)
  int windowSize = m_samplerate / 20;

  // Find sustainstart by scanning from the beginning
  double maxAmplitudeValue = 0;
  
  for (unsigned idx = 0; idx < numberOfSamples - windowSize; idx += windowSize) {
    double maxValueInThisWindow = 0;
    for (int j = idx; j < idx + windowSize; j++) {
      double currentValue = fabs(channel_data[j]);

      if (currentValue > maxValueInThisWindow)
        maxValueInThisWindow = currentValue;
    }

    if (maxValueInThisWindow > maxAmplitudeValue)
      maxAmplitudeValue = maxValueInThisWindow;
    else {
      // the max value in the window is not increasing anymore so 
      // sustainsection is reached
      sustainStartIndex = idx + windowSize;
      break;
    }
  }

  // then we add an offset of 0.25 seconds to allow the tone to stabilize
  sustainStartIndex += m_samplerate / 4;

  // now find sustainend by scanning from the end of audio data
  maxAmplitudeValue = 0;
  
  for (unsigned idx = numberOfSamples - 1; idx > windowSize; idx -= windowSize) {
    double maxValueInThisWindow = 0;
    for (int j = idx; j > idx - windowSize; j--) {
      double currentValue = fabs(channel_data[j]);

      if (currentValue > maxValueInThisWindow)
        maxValueInThisWindow = currentValue;
    }

    // if current max is less than one fourth of max value, or 12 dB lower
    // we just continue searching backwards
    if (maxValueInThisWindow < maxValue / 4) {
      maxAmplitudeValue = maxValueInThisWindow;
      continue;
    }

    if (maxValueInThisWindow > maxAmplitudeValue) {
      maxAmplitudeValue = maxValueInThisWindow;
    } else {
      // the max value in the window is not increasing anymore so 
      // sustainsectionend should be reached
      sustainEndIndex = idx;
      break;
    }
  }

  // then we remove an offset of 0.25 seconds to be sure
  sustainEndIndex -= m_samplerate / 4;

  if (sustainStartIndex > sustainEndIndex) {
    // this is an error situation where no sustainsection could be found
    delete[] channel_data;
    return false;
  }

  // find out how large the analyze window can be
  int analyzeWindowSize = 2;
  bool keepIncreasing = true;
  while (keepIncreasing) {
    if (analyzeWindowSize * 2 < sustainEndIndex - sustainStartIndex) {
      if (analyzeWindowSize < m_samplerate) {
        if (analyzeWindowSize < 65536)
          analyzeWindowSize *= 2;
        else
          keepIncreasing = false;
      } else {
        keepIncreasing = false;
      }
    } else {
      keepIncreasing = false;
    }
  }

  // if not more than one window will be analyzed put it in the middle
  if ((sustainEndIndex - sustainStartIndex) / analyzeWindowSize < 2) {
    sustainStartIndex += (sustainEndIndex - sustainStartIndex - analyzeWindowSize) / 2;
  }

  double *in = new double[analyzeWindowSize];
  double *out = new double[analyzeWindowSize / 2];
   
  unsigned current_idx = sustainStartIndex;

  while (current_idx < sustainEndIndex - analyzeWindowSize) {
    // fill in array
    int index = 0;
    for (unsigned x = current_idx; x < current_idx + analyzeWindowSize; x++) {
      in[index] = channel_data[x];
      index++;
    }

    // Apply a Gaussian window to the data
    WindowFunc(9, analyzeWindowSize, in);

    // Perform the FFT
    PowerSpectrum(analyzeWindowSize, in, out);

    // now we should find the first peak above the average value which should
    // indicate what frequency is the fundamental
    double sum = 0, averageValue;
    for (int x = 0; x < (analyzeWindowSize / 2); x++)
      sum += out[x];

    averageValue = sum / (double) (analyzeWindowSize / 2);

    int peakIndex = 0;
    double current, before, beforeBefore;
    for (int x = 0; x < (analyzeWindowSize / 2); x++) {
      current = out[x];

      if (x > 1) {
        if (current > before) {
          // we're going towards a possible peak so just continue
          beforeBefore = before;
          before = current;
        } else {
          // we're going away from a max so check if previous was a max
          if (before > beforeBefore) {
            // previous index was a max! check if we're above average
            if (before > averageValue) {
              // So, this value is a max and it's above average but can
              // we be ceratin that this is truly a local max?
              bool greaterMaxSoon = false;
              for (int t = 0; t < x / 10; t++) {
                if (out[x + t] > current) {
                  // No, there's a larger max soon to come
                  greaterMaxSoon = true;
                }
              }
              if (greaterMaxSoon) {
                // No! We must continue
                beforeBefore = before;
                before = current;
              } else {
                // Yes! we store before index as peakIndex
                peakIndex = x - 1;
                break;
              }
            } else {
              // No! Then we must continue
              beforeBefore = before;
              before = current;
            }
          } else {
            // no, we are just moving away from a max so just continue
            beforeBefore = before;
            before = current;
          }
        }
      }

      if (x == 0)
        before = current;

      if (x == 1) {
        beforeBefore = before;
        before = current;
      }
    }

    // translate peakIndex to a frequency
    double centerPeakBin; 
    double valueBeforePeak = out[peakIndex - 1];
    double valueAtPeak = out[peakIndex];
    double valueAfterPeak = out[peakIndex + 1]; 

    centerPeakBin = (valueAfterPeak - valueBeforePeak) / (2 * ( 2 * valueAtPeak - valueBeforePeak - valueAfterPeak));
    double peakFrequency = (peakIndex + centerPeakBin) * m_samplerate / (double) analyzeWindowSize;
    detectedPitches.push_back(peakFrequency);

    // proceed to next window
    current_idx += analyzeWindowSize / 2;
  }

  delete[] in;
  delete[] out;

  m_timeDomainPitch = DetectedPitchInTimeDomain(channel_data, sustainStartIndex, sustainEndIndex);

  delete[] channel_data;

  if (detectedPitches.empty() == false) {
    double pitchSum;
    for (int i = 0; i < detectedPitches.size(); i++)
      pitchSum += detectedPitches[i];

    m_fftPitch = pitchSum / (double) detectedPitches.size();

    return true;
  } else {
    return false;
  }
}

double FileHandling::DetectedPitchInTimeDomain(double audio[], unsigned start, unsigned end) {
  if (end - start < 2)
    return 0; // cannot calculate pitch

  std::vector<double> allDetectedPitches;
  double prev = audio[end]; // Last sample
  unsigned end_point = 0; // Preliminary value of the last sample of period

  for (unsigned i = end - 2; i > start; i--) {
    double v = audio[i];

    /* We are interested in positive zero crossings */
    if ((v > 0.0) && (prev <= 0.0)) {
      if (!end_point) {
        end_point = i;
      } else {
        /* Found the next zero crossing - is this a good loop? */
        unsigned len = end_point - i; /* no +1 as we don't want to look at the second crossover point */
        if (i > len) {
          unsigned prev_start_point = i - len;

          /* Find the RMS of the first signal and compute the error of the second signal. */
          double rms = 0.0;
          double error_rms = 0.0;

          for (unsigned j = 0; j < len; j++) {
            double error = audio[j + prev_start_point] - audio[j + i];
            double d     = audio[j + prev_start_point];

            error *= error;
            d *= d;
            error_rms = error_rms * j / ((double)(j + 1)) + error / ((double)(j + 1));
            rms = rms * j / ((double)(j + 1)) + d / ((double)(j + 1));
          }

          if ((error_rms > 0.0) && (rms > 0.0)) {
            error_rms = sqrt(error_rms);
            rms = sqrt(rms);

            if ((error_rms / rms) < 0.55) {
              // store pitch
              allDetectedPitches.push_back( ((double) m_samplerate) / (double) len );
              // set endpoint for next period
              end_point = i;
            }
          }
        }
      }
    }
    prev = v;
  }

  if (allDetectedPitches.empty() == false) {
    double pitchSum = 0.0;
    for (int i = 0; i < allDetectedPitches.size(); i++)
      pitchSum += allDetectedPitches[i];

    return pitchSum / allDetectedPitches.size();
  } else {
    return 0; /* Couldn't find out the pitch */
  }
}

double FileHandling::GetTDPitch() {
  return m_timeDomainPitch;
}

