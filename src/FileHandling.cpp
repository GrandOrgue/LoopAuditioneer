/*
 * FileHandling.cpp is a part of LoopAuditioneer software
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

#include "FileHandling.h"

FileHandling::FileHandling(wxString fileName, wxString path) : m_loops(NULL), m_cues(NULL), shortAudioData(NULL), intAudioData(NULL), doubleAudioData(NULL), fileOpenWasSuccessful(false) {
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

double FileHandling::GetPitch(double data[]) {
  bool gotPitch = AutoDetectPitch(data);
  if (gotPitch)
    return m_pitch;
  else
    return 0;
}

bool FileHandling::AutoDetectPitch(double data[]) {
  // Find strongest value
  double maxValue = 0;
  for (unsigned i = 0; i < ArrayLength; i += m_channels) {
    for (int j = 0; j < m_channels; j++) {
      double currentValue = fabs(data[i + j]);

      if (currentValue > maxValue) {
        maxValue = currentValue;
      }
    }
  }

  // Find a resonable sustainsection in the audio to check
  unsigned sustainStartIndex, sustainEndIndex;

  // set a windowsize for a 20 Hz frequency in current file
  int windowSize = m_samplerate / 20 * m_channels;

  // Find sustainstart by scanning from the beginning
  double maxAmplitudeValue = 0;
  
  for (unsigned i = 0; i < ArrayLength - windowSize; i += windowSize) {
    double maxValueInThisWindow = 0;
    for (int j = i; j < i + windowSize; j++) {
      double currentValue = fabs(data[j]);

      if (currentValue > maxValueInThisWindow)
        maxValueInThisWindow = currentValue;
    }

    if (maxValueInThisWindow > maxAmplitudeValue)
      maxAmplitudeValue = maxValueInThisWindow;
    else {
      // the max value in the window is not increasing anymore so 
      // sustainsection is reached
      sustainStartIndex = i + windowSize;
      break;
    }
  }

  // then we add an offset of 0.25 seconds to allow the tone to stabilize
  sustainStartIndex += m_samplerate * m_channels / 4;

  // make sure we're on the first channel now
  sustainStartIndex -= (sustainStartIndex % m_channels);

  // search forward for a zero crossing
  if (data[sustainStartIndex] > 0) {
    while (data[sustainStartIndex] > 0) {
      sustainStartIndex += m_channels;
    }
    sustainStartIndex -= m_channels;
  } else {
    while (data[sustainStartIndex] < 0) {
      sustainStartIndex += m_channels;
    }
    sustainStartIndex -= m_channels;
  }

  // now find sustainend by scanning from the end of audio data
  maxAmplitudeValue = 0;
  
  for (unsigned i = ArrayLength; i > 0 + windowSize; i -= windowSize) {
    double maxValueInThisWindow = 0;
    for (int j = i; j > i - windowSize; j--) {
      double currentValue = fabs(data[j]);

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
      // sustainsectionend is reached
      sustainEndIndex = i;
      break;
    }
  }

  sustainEndIndex -= (sustainEndIndex % m_channels);

  if (sustainStartIndex > sustainEndIndex)
    return false;

  // Find the approximate period of pattern repetition
  // We search a window with a length of at four times the longest period we
  // expect to find. A 32' C has 16.35 Herz, so we use 15 for good measure.
  // one complete period is sampleRate / 15 samples long and we need four
  // times the length in samples multiplied with numberOfChannels
  unsigned searchWindow = m_samplerate / 15 * 4 * m_channels;
  unsigned maxLag = searchWindow / 2;
  unsigned approximatePeriod = 0;

  std::vector<std::pair<unsigned, double> > possiblePeriod; // store lag and value for all max after start
  double currentCorr, previousCorr, beforePreviousCorr;

  for (unsigned i = 0; i < maxLag; i += m_channels) {
    // calculate normalized crosscorrelation
    double sum1 = 0, sum2 = 0, sum3 = 0;
    for (unsigned j = sustainStartIndex; j < sustainStartIndex + searchWindow - i - m_channels; j += m_channels) {
      sum1 += data[j] * data[j + i];
      sum2 += pow(data[j], 2);
      sum3 += pow(data[j + i], 2);
    }
    currentCorr = sum1 / sqrt(sum2 * sum3);

    if (i == 0)
      previousCorr = currentCorr;

    if (i == m_channels) {
      beforePreviousCorr = previousCorr;
      previousCorr = currentCorr;
    }

    if (i > m_channels) {
      if (currentCorr > previousCorr) {
        // we're going towards a possible max so just continue
        beforePreviousCorr = previousCorr;
        previousCorr = currentCorr;
      } else {
        // we're going away from a max so check if previous was a max
        if (previousCorr > beforePreviousCorr) {
          // previous lag was a max! So we store it in the vector
          possiblePeriod.push_back(std::make_pair(i - m_channels, previousCorr));

          // then we must continue
          beforePreviousCorr = previousCorr;
          previousCorr = currentCorr;
        } else {
          // no, we are just moving away from a max so just continue
          beforePreviousCorr = previousCorr;
          previousCorr = currentCorr;
        }
      }
    }
  }

  double derivativeAtZero = (data[sustainStartIndex + m_channels] - data[sustainStartIndex - m_channels]) / 2.0;
  // Then we look at the possiblePeriod vector and find the first value that is
  // above the threshold which "should" be the repetitionperiod...
  if (possiblePeriod.empty() == false) {
    // first we find the highest value in the maxima vector which we then will 
    // use to create a threshold of say max 5% lower
    double thresholdValue = 0;
    for (int i = 0; i < possiblePeriod.size(); i++) {
      if (possiblePeriod[i].second > thresholdValue)
        thresholdValue = possiblePeriod[i].second;
    }
    thresholdValue *= 0.95;

    for (int i = 0; i < possiblePeriod.size(); i++) {
      if (possiblePeriod[i].second > thresholdValue) {
        // this could be the period, we should double check that the derivative
        // of the first channel will actually match at start and this point then
        double derivativeAtThis = (data[sustainStartIndex + possiblePeriod[i].first + m_channels] - data[sustainStartIndex + possiblePeriod[i].first - m_channels]) / 2.0;

        if (derivativeAtZero < 0 == derivativeAtThis < 0) {
          approximatePeriod = possiblePeriod[i].first;
          break;
        }
      }
    }
  } else {
    return false;
  }

  if (approximatePeriod == 0)
    return false;

  // Now we know the approximate period so that we can search more efficently
  // and add the distance between every period within the sustainsection to
  // the pitch vector and then we calculate the average of them to get the
  // average pitch.
  std::vector<unsigned> repetitionPeriods;
  unsigned jumpThisManyIndexes = approximatePeriod - 2 * m_channels;
  unsigned lastMinIndex = sustainStartIndex;
  for (unsigned i = sustainStartIndex; i < sustainEndIndex - approximatePeriod; i += m_channels) {
    if (i < lastMinIndex + jumpThisManyIndexes)
      continue;

    double sum = 0;

    for (int j = 0; j < approximatePeriod; j++) {
      sum += pow(data[i] - data[j + i], 2);
    }
    currentCorr = sqrt(sum / (double) approximatePeriod);

    if (i > sustainStartIndex + m_channels) {
      if (currentCorr < previousCorr) {
        // we're going towards a possible min so just continue
        beforePreviousCorr = previousCorr;
        previousCorr = currentCorr;
      } else {
        // we're going away from a min so check if previous was a min
        if (previousCorr < beforePreviousCorr) {
          // previous lag was a min! So we store it in the vector
          repetitionPeriods.push_back(i - m_channels - lastMinIndex);
          lastMinIndex = i - m_channels;

          // then we must continue
          beforePreviousCorr = 1;
          previousCorr = 1;
        } else {
          // no, we are just moving away from a min so just continue
          beforePreviousCorr = previousCorr;
          previousCorr = currentCorr;
        }
      }
    }

    if (i == sustainStartIndex) {
      previousCorr = currentCorr;
    }

    if (i == sustainStartIndex + m_channels) {
      beforePreviousCorr = previousCorr;
      previousCorr = currentCorr;
    }
  }

  // calculate average pitch
  if (repetitionPeriods.empty() == false) {
    unsigned periodSum = 0;
    for (int i = 0; i < repetitionPeriods.size(); i++) {
      periodSum += repetitionPeriods[i];
    }
    double averagePeriod = (double) periodSum / (double) repetitionPeriods.size();
    m_pitch = m_samplerate * m_channels / averagePeriod;
    return true;
  }

  return false;
}

