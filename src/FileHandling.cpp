/*
 * FileHandling.cpp is a part of LoopAuditioneer software
 * Copyright (C) 2011-2026 Lars Palo and contributors (see AUTHORS file)
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
#include <cfloat>
#include <algorithm>

FileHandling::FileHandling(wxString fileName, wxString path) : m_loops(NULL), m_cues(NULL), shortAudioData(NULL), intAudioData(NULL), floatAudioData(NULL), doubleAudioData(NULL), fileOpenWasSuccessful(false), m_fftPitch(0), m_fftHPS(0), m_fftPeakPitch(0), m_timeDomainPitch(0), m_autoSustainStart(0),
m_autoSustainEnd(0), m_sliderSustainStart(0), m_sliderSustainEnd(0) {
  m_fileName = fileName;
  m_loops = new LoopMarkers();
  m_cues = new CueMarkers();
  wxString filePath;
  filePath = path;
  filePath += wxFILE_SEP_PATH;
  filePath += fileName;

  // Here we get all the info about the file to be able to later open new file in write mode if changes should be saved
  SndfileHandle sfHandle;

  sfHandle = SndfileHandle(std::string(filePath.mb_str())); // Open file only for read first to get all info

  if (sfHandle) { // checking if opening file was succesful or not

    m_format = sfHandle.format();
    m_samplerate = sfHandle.samplerate();
    m_channels = sfHandle.channels();
    m_minorFormat = sfHandle.format() & SF_FORMAT_SUBMASK;

    // Try to get loop info from the file
    if (sfHandle.command(SFC_GET_INSTRUMENT, &instr, sizeof(instr)) == SF_TRUE) {
      // There are loops!

      m_loops->SetMIDIUnityNote(instr.basenote);
      m_loops->SetMIDIPitchFraction(instr.dwMIDIPitchFraction);

      // Store all the loops into m_loops loopsIn vector
      int loopsToRead = instr.loop_count;
      if (loopsToRead > 16)
        loopsToRead = 16;
      for (int i = 0; i < loopsToRead; i++) {
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
    if (sfHandle.command(SFC_GET_CUE, &cues, sizeof(cues)) == SF_TRUE) {
      // There are cues!

      // Check if the cue is a real cue or a label, only keep real cues!
      for (unsigned i = 0; i < cues.cue_count; i++) {
        bool toAdd = true;

        for (int j = 0; j < instr.loop_count; j++) {
          if (cues.cue_points[i].sample_offset == instr.loops[j].start)
            toAdd = false;
        }

        if (toAdd) {
          CUEPOINT tmp;
          tmp.dwName = cues.cue_points[i].indx;
          tmp.dwPosition = cues.cue_points[i].position;
          tmp.fccChunk = cues.cue_points[i].fcc_chunk;
          tmp.dwChunkStart = cues.cue_points[i].chunk_start;
          tmp.dwBlockStart = cues.cue_points[i].block_start;
          tmp.dwSampleOffset = cues.cue_points[i].sample_offset;
          tmp.keepThisCue = true;

          m_cues->AddCue(tmp);
        }
      }
    }

    // Decide what format to store audio data as and copy it
    if (m_minorFormat == SF_FORMAT_DOUBLE) {
      ArrayLength = sfHandle.frames() * sfHandle.channels();
      doubleAudioData = new double[ArrayLength];
      sfHandle.read(doubleAudioData, ArrayLength);

      fileOpenWasSuccessful = true;
    } else if (m_minorFormat == SF_FORMAT_FLOAT) {
      ArrayLength = sfHandle.frames() * sfHandle.channels();
      floatAudioData = new float[ArrayLength];
      sfHandle.read(floatAudioData, ArrayLength);

      fileOpenWasSuccessful = true;
    } else if ((m_minorFormat == SF_FORMAT_PCM_16) || (m_minorFormat == SF_FORMAT_PCM_S8) || (m_minorFormat == SF_FORMAT_PCM_U8)) {
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

    // also store audio data as doubles de-interleaved if successful
    // plus check if the float audio data needs to be read for playback
    if (fileOpenWasSuccessful) {
      // remember to "rewind" file before read
      sfHandle.seek(0, SEEK_SET);

      for (int i = 0; i < m_channels; i++) {
        WAVETRACK track;
        waveTracks.push_back(track);
      }
      double *buffer = new double[ArrayLength];
      sfHandle.read(buffer, ArrayLength);

      int index = 0;
      for (unsigned i = 0; i < ArrayLength; i++) {
        // de-interleaving
        waveTracks[index].waveData.push_back(buffer[i]);
        index++;

        if (index == m_channels)
          index = 0;
      }
      delete[] buffer;
      
      if (m_minorFormat != SF_FORMAT_FLOAT) {
        // if the format is something else than floats we also need data as
        // floats for audio playback reasons
        sfHandle.seek(0, SEEK_SET);
        floatAudioData = new float[ArrayLength];
        sfHandle.read(floatAudioData, ArrayLength);
      }
    }
    
    // Try to get LIST INFO strings
    if (sfHandle.getString(SF_STR_ARTIST) != NULL)
      m_info.artist = wxString::FromUTF8(sfHandle.getString(SF_STR_ARTIST));
    else
      m_info.artist = wxEmptyString;
    
    if (sfHandle.getString(SF_STR_COPYRIGHT) != NULL)
      m_info.copyright = wxString::FromUTF8(sfHandle.getString(SF_STR_COPYRIGHT));
    else
      m_info.copyright = wxEmptyString;
    
    if (sfHandle.getString(SF_STR_SOFTWARE) != NULL)
      m_info.software = wxString::FromUTF8(sfHandle.getString(SF_STR_SOFTWARE));
    else
      m_info.software = wxEmptyString;
    
    if (sfHandle.getString(SF_STR_COMMENT) != NULL)
      m_info.comment = wxString::FromUTF8(sfHandle.getString(SF_STR_COMMENT));
    else
      m_info.comment = wxEmptyString;
    
    // creation date is special
    if (sfHandle.getString(SF_STR_DATE) != NULL) {
      wxString dateString = wxString::FromUTF8(sfHandle.getString(SF_STR_DATE));
      wxDateTime dt;
      wxString::const_iterator end;
      if (dt.ParseDate(dateString, &end))
        m_info.creation_date = dt;
      else
        m_info.creation_date = wxDateTime::Now();
    } else {
      m_info.creation_date = wxDateTime::Now();
    }
    
    // Auto calculate the sustainsection too
    if (fileOpenWasSuccessful)
      CalculateSustainStartAndEnd();
    
    // set a default, this will be set when the file is already opened from MyFrame
    m_useAutoSustain = true;

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
  
  delete[] floatAudioData;
}

void FileHandling::SaveAudioFile(wxString fileName, wxString path) {
  wxString filePath;
  filePath = path;
  filePath += wxFILE_SEP_PATH;
  filePath += fileName;

  // This we open the file write
  sfh = SndfileHandle(std::string(filePath.mb_str()), SFM_WRITE, m_format, m_channels, m_samplerate);

  // Deal with the loops first
  m_loops->ExportLoops();
  instr.basenote = m_loops->GetMIDIUnityNote();
  instr.dwMIDIPitchFraction = m_loops->GetMIDIPitchFraction();
  int loopCount = m_loops->loopsOut.size();
  if (loopCount > 16)
    instr.loop_count = 16;
  else
    instr.loop_count = loopCount;
  for (int i = 0; i < instr.loop_count; i++) {
    instr.loops[i].mode = m_loops->loopsOut[i].dwType;
    instr.loops[i].start = m_loops->loopsOut[i].dwStart;
    instr.loops[i].end = m_loops->loopsOut[i].dwEnd;
    instr.loops[i].count = m_loops->loopsOut[i].dwPlayCount;
  }
  sfh.command(SFC_SET_INSTRUMENT, &instr, sizeof(instr)); // this writes the loops metadata

  // Then take care of the cues
  m_cues->ExportCues();
  int cueCount = m_cues->exportedCues.size();
  if (cueCount > 99)
    cues.cue_count = 99;
  else
    cues.cue_count = cueCount;
  for (unsigned i = 0; i < cues.cue_count; i++) {
    cues.cue_points[i].indx =  m_cues->exportedCues[i].dwName;
    cues.cue_points[i].position =  m_cues->exportedCues[i].dwPosition;
    cues.cue_points[i].fcc_chunk =  m_cues->exportedCues[i].fccChunk;
    cues.cue_points[i].chunk_start =  m_cues->exportedCues[i].dwChunkStart;
    cues.cue_points[i].block_start =  m_cues->exportedCues[i].dwBlockStart;
    cues.cue_points[i].sample_offset =  m_cues->exportedCues[i].dwSampleOffset;
  }
  sfh.command(SFC_SET_CUE, &cues, sizeof(cues));
  
  // Write LIST INFO strings if they are set
  if (m_info.artist != wxEmptyString)
    sfh.setString(SF_STR_ARTIST, (const char*)m_info.artist.mb_str());
    
  if (m_info.copyright != wxEmptyString)
    sfh.setString(SF_STR_COPYRIGHT, (const char*)m_info.copyright.mb_str());
  
  // Remember to set the software string of the used software!
  m_info.software = wxT("LoopAuditioneer");
  if (m_info.software != wxEmptyString)
    sfh.setString(SF_STR_SOFTWARE, (const char*)m_info.software.mb_str());
  
  if (m_info.comment != wxEmptyString)
    sfh.setString(SF_STR_COMMENT, (const char*)m_info.comment.mb_str());
  
  // the creation date must be formatted as YYYY-MM-DD and nothing else
  if (m_info.creation_date.IsValid()) {
    wxString dateString = m_info.creation_date.FormatISODate();
    sfh.setString(SF_STR_DATE, (const char*)dateString.mb_str());
  }
  
  // Finally write the data back
  if (m_minorFormat == SF_FORMAT_DOUBLE) {
    sfh.write(doubleAudioData, ArrayLength);
  } else if (m_minorFormat == SF_FORMAT_FLOAT) {
    sfh.write(floatAudioData, ArrayLength);
  } else if ((m_minorFormat == SF_FORMAT_PCM_16) || (m_minorFormat == SF_FORMAT_PCM_S8) || (m_minorFormat == SF_FORMAT_PCM_U8)) {
    sfh.write(shortAudioData, ArrayLength);
  } else {
    sfh.write(intAudioData, ArrayLength);
  }
  // File will be finally closed when the instance is deleted!
}

int FileHandling::GetSampleRate() {
  return m_samplerate;
}

void FileHandling::SetSampleRate(unsigned s_rate) {
  m_samplerate = s_rate;
}

int FileHandling::GetAudioFormat() {
  return m_minorFormat;
}

int FileHandling::GetWholeFormat() {
  return m_format;
}

wxString FileHandling::GetInfoString() {
  wxString info = wxEmptyString;
  switch (m_minorFormat) {
    case SF_FORMAT_PCM_S8:
      info = wxT("8 bit (signed)");
      break;
    case SF_FORMAT_PCM_16:
      info = wxT("16 bit");
      break;
    case SF_FORMAT_PCM_24:
      info = wxT("24 bit");
      break;
    case SF_FORMAT_PCM_32:
      info = wxT("32 bit");
      break;
    case SF_FORMAT_PCM_U8:
      info = wxT("8 bit (unsigned)");
      break;
    case SF_FORMAT_FLOAT:
      info = wxT("32 bit float");
      break;
    case SF_FORMAT_DOUBLE:
      info = wxT("64 bit float");
      break;
    default:
      SndfileHandle sndfile;
      SF_FORMAT_INFO format_info;
      format_info.format = m_minorFormat;
      sndfile.command(SFC_GET_FORMAT_INFO, &format_info, sizeof (format_info));
      info = wxString(format_info.name);
      break;
  }
  return wxString::Format(wxT("%s %u Hz"), info, m_samplerate);
}

bool FileHandling::FileCouldBeOpened() {
  if (fileOpenWasSuccessful)
    return true;
  else
    return false;
}

bool FileHandling::GetFFTPitch(double pitches[]) {
  bool gotPitch = DetectPitchByFFT();
  if (gotPitch) {
    pitches[0] = m_fftPitch;
    pitches[1] = m_fftHPS;
    pitches[2] = m_fftPeakPitch;

    return true;
  } else
    return false;
}

/*
 * GetSpectrum needs an array of doubles that is the size of (fftSize / 2) for the total output in dB for each bin
 * fftSize must be a power of 2
 * windowType must be in range 0 to 9
 */
bool FileHandling::GetSpectrum(double *outInDb, unsigned fftSize, int windowType) {
  if (!waveTracks.empty()) {
    unsigned numberOfSamples = waveTracks[0].waveData.size();
    if (fftSize > numberOfSamples) {
      return false;
    }

    unsigned nbrWindows = 0;
    unsigned halfFFTsize = fftSize / 2;
    double *input = new double[fftSize];
    double *output = new double[fftSize];
    double *fftData = new double[fftSize];
    double *window = new double[fftSize];

    for (unsigned i = 0; i < fftSize; i++) {
      input[i] = 0.0f;
      output[i] = 0.0f;
      fftData[i] = 0.0f;
      window[i] = 1.0f;
    }

    // Create a window that will be applied to the in data later
    if (windowType > 0)
      WindowFunc(windowType, fftSize, window);

    // Scale window so an amplitude of 1.0 equals to 0 dB
    double winScale = 0;
    for (unsigned i = 0; i < fftSize; i++)
      winScale += window[i];
    if (winScale > 0)
      winScale = 4.0 / (winScale * winScale);
    else
      winScale = 1.0;

    for (unsigned i = 0; i < waveTracks.size(); i++) {
      unsigned currentStartIdx = 0;
      while (currentStartIdx + fftSize < waveTracks[i].waveData.size()) {
        // Fill this input window with audio data from current channel
        for (unsigned j = 0; j < fftSize; j++) {
          input[j] = window[j] * waveTracks[i].waveData[currentStartIdx + j];
        }

        // Perform the FFT
        PowerSpectrum(fftSize, input, output);

        for (unsigned j = 0; j < halfFFTsize; j++)
          fftData[j] += output[j];

        // Overlap each window 50%
        currentStartIdx += halfFFTsize;
        nbrWindows++;
      }
    }

    double scale = winScale / (double) nbrWindows;
    // Convert to decibels and store value in the double array sent as parameter
    for (unsigned i = 0; i < halfFFTsize; i++) {
      double temp = 10 * log10(fftData[i] * scale);
      if (temp > -145)
        outInDb[i] = temp;
      else
        outInDb[i] = -145;
    }

    delete[] input;
    delete[] output;
    delete[] fftData;
    delete[] window;
    return true;
  } else {
    return false;
  }
}

bool FileHandling::DetectPitchByFFT() {
  if (waveTracks[0].waveData.size() < 1024) {
    // the file doesn't contain enough data...
    m_fftPitch = 0;
    m_fftHPS = 0;
    m_fftPeakPitch = 0;
    return false;
  }

  unsigned fftSize = 131072;
  bool foundLargestSize = false;
  while (!foundLargestSize) {
    if (fftSize < waveTracks[0].waveData.size()) {
      foundLargestSize = true;
    } else {
      fftSize /= 2;
    }
  }

  unsigned halfSize = fftSize / 2;
  double *pwrSpec = new double[halfSize];

  if (GetSpectrum(pwrSpec, fftSize, 3)) {
    // fft data is now available in pwrSpec array already as dB values
    // so we store all peaks and get largest (greatest) peak in one go
    std::vector<SpectrumPeak> allPeaks;
    double peakDb = -200;
    unsigned peakBin = 0;
    unsigned peakIdx = 0;
    double peakPitch = 0;
    for (unsigned i = 0; i < halfSize; i++) {
      // only care for storing peaks that are above -90 dB
      if (pwrSpec[i] > -90.0f && i > 0 && i < halfSize - 1) {
        if (pwrSpec[i] > pwrSpec[i - 1] && pwrSpec[i] > pwrSpec[i + 1]) {
          // this bin is a peak
          SpectrumPeak peak;
          peak.m_binNbr = i;
          peak.m_dB = pwrSpec[i];
          peak.m_pitch = i * (double) m_samplerate / fftSize;
          allPeaks.push_back(peak);
        }
      }
      // next comes storing the greatest peak that is expected to be significant
      if (pwrSpec[i] > peakDb && i > 0 && allPeaks.size() > 0) {
        peakDb = pwrSpec[i];
        peakBin = i;
        peakIdx = allPeaks.size() - 1;
        peakPitch = allPeaks[peakIdx].m_pitch;
      }
    }

    // initially we set the possible fundamental to the largest peak
    unsigned possibleF0 = peakIdx;
    m_fftPeakPitch = TranslateIndexToPitch(
      peakBin,
      pow(10, (pwrSpec[peakBin - 1] / 10)),
      pow(10, (pwrSpec[peakBin] / 10)),
      pow(10, (pwrSpec[peakBin + 1] / 10)),
      fftSize
    );

    if (peakIdx > 0 && allPeaks.size()) {
      // each peak up to and including max peak should be compared for harmonic quality relative to other peaks
      for (unsigned i = 0; i <= peakIdx; i++) {
        allPeaks[i].m_harmonicQuality = pow(10, (allPeaks[i].m_dB / 10.0));
        std::vector<double> harmonicsPitch;
        for (unsigned j = 2; j < 10; j++) {
          harmonicsPitch.push_back(allPeaks[i].m_pitch * j);
        }

        std::vector<unsigned> candidateHarmonics;
        for (unsigned j = 0; j < harmonicsPitch.size(); j++) {
          for (unsigned k = i + 1; k < allPeaks.size(); k++) {
            // the pitches might not be exact so we allow for 1% variance per subsequent harmonic
            if (fabs(allPeaks[k].m_pitch - harmonicsPitch[j]) < harmonicsPitch[j] * 0.01 * (j + 1)) {
              candidateHarmonics.push_back(k);
            }
          }
          if (candidateHarmonics.size() > 1) {
            // only select the strongest one
            unsigned strongestCandidate = 0;
            double dBvalue = allPeaks[candidateHarmonics[0]].m_dB;
            for (unsigned k = 1; k < candidateHarmonics.size(); k++) {
              if (allPeaks[candidateHarmonics[k]].m_dB > dBvalue) {
                dBvalue = allPeaks[candidateHarmonics[k]].m_dB;
                strongestCandidate = k;
              }
            }
            allPeaks[i].m_matchingHarmonics.push_back(candidateHarmonics[strongestCandidate]);
            allPeaks[i].m_harmonicQuality += pow(10, (allPeaks[candidateHarmonics[strongestCandidate]].m_dB / 10.0));
          } else if (candidateHarmonics.size()) {
            allPeaks[i].m_matchingHarmonics.push_back(candidateHarmonics[0]);
            allPeaks[i].m_harmonicQuality += pow(10, (allPeaks[candidateHarmonics[0]].m_dB / 10.0));
          }
          if (candidateHarmonics.empty()) {
            allPeaks[i].m_harmonicQuality *= 0.5;
          } else {
            candidateHarmonics.clear();
          }
        }
        harmonicsPitch.clear();
      }

      // now we need to look at peaks earlier than peakIdx and decide if there could be a possible fundamental there
      // we do this by first just checking the harmonic quality itself against the greatest peak and selecting first better one if it exist
      for (unsigned i = 0; i < peakIdx; i++) {
        if (10 * log10(allPeaks[i].m_harmonicQuality) > 10 * log10(allPeaks[possibleF0].m_harmonicQuality)) {
          possibleF0 = i;
          break;
        }
      }

      // then compare by pure number of peak strength against other peaks together with the harmonic quality
      for (unsigned i = peakIdx - 1; i > 0; i--) {
        if (i == possibleF0)
          continue;
        if (allPeaks[i].m_dB > allPeaks[possibleF0].m_dB - 12 && 10 * log10(allPeaks[i].m_harmonicQuality) > (10 * log10(allPeaks[possibleF0].m_harmonicQuality) - 3)) {
          possibleF0 = i;
        }
      }
    }

    if (possibleF0 != peakIdx) {
      // the possible peak must include strongest peak as a harmonic to be accepted
      bool peakContainStrongest = false;
      for (unsigned j = 0; j < allPeaks[possibleF0].m_matchingHarmonics.size(); j++) {
        if (allPeaks[possibleF0].m_matchingHarmonics[j] == peakIdx)
          peakContainStrongest = true;
      }
      // otherwise we'll just fall back to greatest peak
      if (!peakContainStrongest) {
        possibleF0 = peakIdx;
      }
    }
    m_fftPitch = allPeaks[possibleF0].m_pitch;

    // now try detecting pitch with HPS
    unsigned maxBin = 0;
    double *original = new double[halfSize];
    double *hps = new double[halfSize];

    for (unsigned i = 0; i < halfSize; i++) {
      original[i] = sqrt(pow(10, (pwrSpec[i] / 10)));
      if (i > 0)
        hps[i] = original[i];
      else
        hps[i] = 0;
    }

    for (unsigned i = 2; i < 5; i++) {
      for (unsigned j = 1; j < halfSize; j++)
        hps[j] *= GetDownsampledValue(original, halfSize, i, j);
    }

    for (unsigned i = 0; i < halfSize; i++) {
      if (hps[i] > hps[maxBin])
        maxBin = i;
    }

    if (!maxBin)
      maxBin = peakBin;

    m_fftHPS = TranslateIndexToPitch(
      maxBin,
      original[maxBin - 1],
      original[maxBin],
      original[maxBin + 1],
      fftSize
    );

    if (original) {
      delete[] original;
      original = NULL;
    }

    if (hps) {
      delete[] hps;
      hps = NULL;
    }

    if (pwrSpec) {
      delete[] pwrSpec;
      pwrSpec = NULL;
    }

    return true;
  } else {
    m_fftPitch = 0;
    m_fftHPS = 0;
    m_fftPeakPitch = 0;
    if (pwrSpec) {
      delete[] pwrSpec;
      pwrSpec = NULL;
    }
    return false;
  }
}

double FileHandling::GetDownsampledValue(double *fft, unsigned length, unsigned factor, unsigned index) {
  if (index * factor < length)
    return fft[factor * index];
  return 0;
}

double FileHandling::TranslateIndexToPitch(
  int idxAtPeak,
  double valueBeforePeak,
  double valueAtPeak,
  double valueAfterPeak,
  unsigned wSize) {
  double pitchToReturn;
  double centerPeakBin;

  centerPeakBin = (valueAfterPeak - valueBeforePeak) / (2 * ( 2 * valueAtPeak - valueBeforePeak - valueAfterPeak));
  pitchToReturn = (idxAtPeak + centerPeakBin) * m_samplerate / (double) wSize;

  return pitchToReturn;
}

bool FileHandling::DetectPitchInTimeDomain() {
  unsigned numberOfSamples = waveTracks[0].waveData.size();
  if (!numberOfSamples)
    return false;
  std::pair <unsigned, unsigned> sustainStartAndEnd;
  sustainStartAndEnd.first = 0;
  sustainStartAndEnd.second = 0;
  double *channel_data = new double[numberOfSamples];

  // Get channel data
  SeparateStrongestChannel(channel_data);

  // Get sustainsection start and end
  sustainStartAndEnd.first = m_autoSustainStart;
  sustainStartAndEnd.second = m_autoSustainEnd;
  
  // Check if sustainsection is not valid and if so just set the whole channel as sustain
  if (sustainStartAndEnd.first == 0 && sustainStartAndEnd.second == 0) {
    sustainStartAndEnd.first = 0;
    sustainStartAndEnd.second = numberOfSamples - 1;
  }

  int startValue = sustainStartAndEnd.first;
  int endValue = sustainStartAndEnd.second;
  if (endValue - startValue < 2)
    return false; // cannot calculate pitch

  // if sustainsection is longer than 2 seconds we limit it
  if (sustainStartAndEnd.second - sustainStartAndEnd.first > m_samplerate * 2)
    sustainStartAndEnd.second = sustainStartAndEnd.first + m_samplerate * 2;

  std::vector<double> allDetectedPitches;
  double prev = channel_data[sustainStartAndEnd.second]; // Last sample
  unsigned end_point = 0; // Preliminary value of the last sample of period

  for (unsigned i = sustainStartAndEnd.second - 2; i > sustainStartAndEnd.first; i--) {
    double v = channel_data[i];

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
            double error = channel_data[j + prev_start_point] - channel_data[j + i];
            double d     = channel_data[j + prev_start_point];

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

  delete[] channel_data;

  if (!allDetectedPitches.empty() && allDetectedPitches.size() > 1) {
    double pitchSum = 0.0;
    for (unsigned i = 0; i < allDetectedPitches.size(); i++)
      pitchSum += allDetectedPitches[i];

    double meanTdPitch = pitchSum / allDetectedPitches.size();

    double varianceSum = 0;
    for (unsigned i = 0; i < allDetectedPitches.size(); i++) {
      varianceSum += pow(allDetectedPitches[i] - meanTdPitch, 2);
    }
    double variance = varianceSum / allDetectedPitches.size();
    double stdDeviation = sqrt(variance);

    std::vector<double> usableValues;
    for (unsigned i = 0; i < allDetectedPitches.size(); i++) {
      if (allDetectedPitches[i] > (meanTdPitch - stdDeviation / 2.0) && allDetectedPitches[i] < (meanTdPitch + stdDeviation / 2.0))
        usableValues.push_back(allDetectedPitches[i]);
    }

    if (!usableValues.empty()) {
      pitchSum = 0;
      for (unsigned i = 0; i < usableValues.size(); i++)
        pitchSum += usableValues[i];

      m_timeDomainPitch = pitchSum / usableValues.size();
      return true;
    } else {
      m_timeDomainPitch = meanTdPitch; /* Falling back to mean value */
      return true;
    }
  } else if (allDetectedPitches.size() == 1) {
    m_timeDomainPitch = allDetectedPitches[0];
    return true;
  } else {
    m_timeDomainPitch = 0; /* Couldn't find out the pitch */
    return false;
  }
}

double FileHandling::GetTDPitch() {
  bool gotPitch = DetectPitchInTimeDomain();
  if (gotPitch)
    return m_timeDomainPitch;
  else
    return 0;
}

void FileHandling::PerformCrossfade(int loopNumber, double fadeLength, int fadeType) {
  // get the audio data as doubles
  double *audioData = new double[ArrayLength];
  bool gotData = GetDoubleAudioData(audioData);
  
  if (!gotData)
    return;
  
  LOOPDATA loopToCrossfade;
  m_loops->GetLoopData(loopNumber, loopToCrossfade);
  unsigned samplesToFade = m_samplerate * fadeLength;
  unsigned samplesToFadeOut = m_samplerate * fadeLength;

  // check to not read outside the audiodata array
  if (samplesToFade > loopToCrossfade.dwStart)
    samplesToFade = loopToCrossfade.dwStart;

  // check to not write outside the audiodata array
  if ((samplesToFadeOut + loopToCrossfade.dwEnd + 1) * m_channels > ArrayLength)
    samplesToFadeOut = (ArrayLength / m_channels) - (loopToCrossfade.dwEnd + 1);

  unsigned firstTargetIdx = (loopToCrossfade.dwEnd - (samplesToFade - 1)) * m_channels;
  unsigned firstSourceIdx = (loopToCrossfade.dwStart - samplesToFade) * m_channels;
  unsigned secondTargetIdx = (loopToCrossfade.dwEnd + 1) * m_channels;
  unsigned secondSourceIdx = loopToCrossfade.dwStart * m_channels;

  // prepare arrays for the crossfade curve data
  double *fadeData = new double[samplesToFade];
  double *fadeOutData = new double[samplesToFadeOut];

  switch(fadeType) {
    case 0:
      // linear data from 0 to 1
      for (unsigned i = 0; i < samplesToFade; i++)
        fadeData[i] = i * 1.0 / (samplesToFade - 1);
      for (unsigned i = 0; i < samplesToFadeOut; i++)
        fadeOutData[i] = i * 1.0 / (samplesToFadeOut - 1);
      break;

    case 1:
      // create a S curve table from 0 to 1
      for (unsigned i = 0; i < samplesToFade; i++) {
        double linear = i * 1.0 / (samplesToFade - 1);
        fadeData[i] = 0.5 * (1.0 + cos((1.0 - linear) * M_PI));
      }
      for (unsigned i = 0; i < samplesToFadeOut; i++) {
        double linear = i * 1.0 / (samplesToFadeOut - 1);
        fadeOutData[i] = 0.5 * (1.0 + cos((1.0 - linear) * M_PI));
      }
      break;

    case 2:
      // create a curve from 0 to 1 with equal power/gain
      for (unsigned i = 0; i < samplesToFade; i++) {
        double linear = i * 1.0 / (samplesToFade - 1);
        fadeData[i] = linear / sqrt( pow(linear, 2) + pow((1 - linear), 2) );
      }
      for (unsigned i = 0; i < samplesToFadeOut; i++) {
        double linear = i * 1.0 / (samplesToFadeOut - 1);
        fadeOutData[i] = linear / sqrt( pow(linear, 2) + pow((1 - linear), 2) );
      }
      break;

    case 3:
       // create a sine curve table from 0 to 1
      for (unsigned i = 0; i < samplesToFade; i++) {
        double linear = i * 1.0 / (samplesToFade - 1);
        fadeData[i] = sin(M_PI / 2 * linear);
      }
      for (unsigned i = 0; i < samplesToFadeOut; i++) {
        double linear = i * 1.0 / (samplesToFadeOut - 1);
        fadeOutData[i] = sin(M_PI / 2 * linear);
      }
      break;

    default:
      // linear data from 0 to 1
      for (unsigned i = 0; i < samplesToFade; i++)
        fadeData[i] = i * 1.0 / (samplesToFade - 1);
      for (unsigned i = 0; i < samplesToFadeOut; i++)
        fadeOutData[i] = i * 1.0 / (samplesToFadeOut - 1);
  }

  // crossfade around the endpoint so that the audio won't click
  // the new loopEnd will be identical to the sample before loopStart
  // and the new sample after loopEnd will be identical to loopStart
  // with crossfades to either side of loopEnd with source from loopStart

  for (unsigned i = 0; i < samplesToFade; i++) {
    for (int j = 0; j < m_channels; j++) {
      audioData[firstTargetIdx + j] = 
        audioData[firstTargetIdx + j] * fadeData[samplesToFade - 1 - i] +
        audioData[firstSourceIdx + j] * fadeData[i];
    }
    firstTargetIdx += m_channels;
    firstSourceIdx += m_channels;
  }

  for (unsigned i = 0; i < samplesToFadeOut; i++) {
    for (int j = 0; j < m_channels; j++) {
      audioData[secondTargetIdx + j] = 
        audioData[secondTargetIdx + j] * fadeOutData[i] +
        audioData[secondSourceIdx + j] * fadeOutData[samplesToFadeOut - 1 - i];
    }
    secondTargetIdx += m_channels;
    secondSourceIdx += m_channels;
  }
  
  // update the waveform data with the new data
  UpdateWaveTracks(audioData);

  // change the current audiodata stored after crossfade is done
  if (shortAudioData != NULL) {
    for (unsigned i = 0; i < ArrayLength; i++)
      shortAudioData[i] = lrint(audioData[i] * (1.0 * 0x7FFF));
  } else if (intAudioData != NULL) {
    for (unsigned i = 0; i < ArrayLength; i++)
      intAudioData[i] = lrint(audioData[i] * (1.0 * 0x7FFFFFFF));
  } else if (doubleAudioData != NULL) {
    for (unsigned i = 0; i < ArrayLength; i++)
      doubleAudioData[i] = audioData[i];
  }
  if (floatAudioData != NULL) {
    // this should always be the case as it's used for playback
    for (unsigned i = 0; i < ArrayLength; i++)
      floatAudioData[i] = (float) audioData[i];
  }
  delete[] audioData;
  delete[] fadeData;
  delete[] fadeOutData;
}

void FileHandling::SeparateStrongestChannel(double outData[]) {
  if (!waveTracks.empty()) {
    if (m_channels > 1) {
      // we have more than one channel so deal with that
      double maxRMS = 0.0;
      unsigned strongestChannelIdx = 0;
      for (unsigned i = 0; i < waveTracks.size(); i++) {
        // this is done for each channel
        double channelRMS = 0.0;
        double totalValues = 0.0;
        for (unsigned j = 0; j < waveTracks[i].waveData.size(); j++) {
          double currentValue = pow(waveTracks[i].waveData[j], 2);
          totalValues += currentValue;
        }
        channelRMS = sqrt((totalValues / waveTracks[i].waveData.size()));

        if (channelRMS > maxRMS) {
          maxRMS = channelRMS;
          strongestChannelIdx = i;
        }
      }
      // now we should know which channel has the highest RMS
      for (unsigned i = 0; i < waveTracks[strongestChannelIdx].waveData.size(); i++)
        outData[i] = waveTracks[strongestChannelIdx].waveData[i];
    } else {
      // there's just one channel so copy that double data
      for (unsigned i = 0; i < waveTracks[0].waveData.size(); i++)
        outData[i] = waveTracks[0].waveData[i];
    }
  } else {
    // for some reason there's no data in the waveTracks!
    // for safety we then fill the outData array with zeros
    for (unsigned i = 0; i < ArrayLength / m_channels; i++)
        outData[i] = 0.0;
  }
}

void FileHandling::CalculateSustainStartAndEnd() {
  // prepare array for a single channel of audio data
  unsigned numberOfSamples = ArrayLength / m_channels;
  if (numberOfSamples < 1) {
    // there's no data to talk about!
    m_autoSustainStart = 0;
    m_autoSustainEnd = 0;
    return;
  }
  double *ch_data = new double[numberOfSamples];

  // populate channel data
  SeparateStrongestChannel(ch_data);

  // now detect sustain section
  // set a window size (mono now!)
  std::vector<double> rmsWindowValues;
  unsigned windowSize = m_samplerate / 50;
  if (windowSize > numberOfSamples)
    windowSize = numberOfSamples - 1;

  // Find strongest value of any windowSize in file audio
  double maxValue = 0.0;
  double individualMax = 0.0;
  unsigned indexWithMaxValue = 0;
  double rmsOfWholeFile = 0.0;
  for (unsigned idx = 0; idx < numberOfSamples - windowSize; idx += windowSize) {
    double totalValues = 0.0;
    double rmsInThisWindow = 0.0;
    for (unsigned j = idx; j < idx + windowSize; j++) {
      double currentValue = pow(ch_data[j], 2);
      totalValues += currentValue;
      if (currentValue > individualMax) {
        individualMax = currentValue;
        indexWithMaxValue = j;
      }
    }
    rmsInThisWindow = sqrt((totalValues / windowSize));
    rmsWindowValues.push_back(rmsInThisWindow);
    rmsOfWholeFile += rmsInThisWindow;

    if (rmsInThisWindow > maxValue) {
      maxValue = rmsInThisWindow;
    }
  }

  // clean up
  delete[] ch_data;

  if (rmsWindowValues.size()) {
    rmsOfWholeFile /= rmsWindowValues.size();
    unsigned firstStartIndexAboveAverageRMS = 0;
    if (rmsWindowValues[0] > rmsOfWholeFile * 1.75) {
      // This file is likely a separate release so sustain start should be from the very beginning of file
      m_autoSustainStart = 0;
    } else {
      // First index above the average RMS should be a decent starting point
      for (unsigned i = 0; i < rmsWindowValues.size(); i++) {
        if (rmsWindowValues[i] > rmsOfWholeFile) {
          firstStartIndexAboveAverageRMS = i;
          break;
        }
      }

      double startTime = (double) (firstStartIndexAboveAverageRMS * windowSize) / (double) m_samplerate;
      if (startTime < 0.300 && firstStartIndexAboveAverageRMS < rmsWindowValues.size() - 2) {
        m_autoSustainStart = (firstStartIndexAboveAverageRMS + 1) * windowSize;
      } else {
        m_autoSustainStart = firstStartIndexAboveAverageRMS * windowSize;
      }
    }

    unsigned firstEndIndexAboveAverageRMS = rmsWindowValues.size() - 1;
    if (rmsWindowValues[firstEndIndexAboveAverageRMS] < rmsOfWholeFile * 0.75) {
      // This file most likely have some kind of a release in it
      if (m_autoSustainStart) {
        // This file will most likely have both an attack and a release in it
        // Method is to now scan backwards for first occasion greater than average RMS
        for (unsigned i = rmsWindowValues.size() - 1; i > 0; i--) {
          if (rmsWindowValues[i] > rmsOfWholeFile) {
            firstEndIndexAboveAverageRMS = i;
            break;
          }
        }
        m_autoSustainEnd = firstEndIndexAboveAverageRMS * windowSize;
      } else {
        //This should be a release only file then
        // First step is to scan forwards until an obvious drop in RMS happen
        firstEndIndexAboveAverageRMS = 0;
        for (unsigned i = 0; i < rmsWindowValues.size() - 2; i++) {
          if (rmsWindowValues[i + 1] * 1.2 > rmsWindowValues[i] && rmsWindowValues[i] > rmsOfWholeFile * 2) {
            firstEndIndexAboveAverageRMS = i;
          } else {
            break;
          }
        }
        if (firstEndIndexAboveAverageRMS)
          m_autoSustainEnd = firstEndIndexAboveAverageRMS * windowSize;
        else {
          if (rmsWindowValues.size() > 1)
            m_autoSustainEnd = (rmsWindowValues[1] / rmsWindowValues[0]) * windowSize;
          else
            m_autoSustainEnd = windowSize;
        }
      }
    } else {
      // This file very likely doesn't have any release in it - it is an attack sample only
      m_autoSustainEnd = numberOfSamples - 1;
    }
  } else {
    // No RMS data is existing in the vector
    if (numberOfSamples) {
        m_autoSustainStart = 0;
        m_autoSustainEnd = numberOfSamples - 1;
    } else {
      m_autoSustainStart = 0;
      m_autoSustainEnd = 0;
    }
  }
}

void FileHandling::TrimExcessData() {
  // Remove data between last loop and first cue larger than last loop
  // First get last loop sample
  unsigned lastEndSample = 0;
  for (int i = 0; i < m_loops->GetNumberOfLoops(); i++) {
    LOOPDATA currentLoop;
    m_loops->GetLoopData(i, currentLoop);
    if (currentLoop.dwEnd > lastEndSample)
      lastEndSample = currentLoop.dwEnd;
  }
  // only continue if there were any loops
  if (lastEndSample) {
    // now get cues greater than loopend (if there are any)
    unsigned firstCuePosAfter = 0;
    for (unsigned i = 0; i < m_cues->GetNumberOfCues(); i++) {
      CUEPOINT currentCue;
      m_cues->GetCuePoint(i, currentCue);
      if (currentCue.dwSampleOffset > lastEndSample) {
        if (!firstCuePosAfter)
          firstCuePosAfter = currentCue.dwSampleOffset;
        else if (currentCue.dwSampleOffset < firstCuePosAfter)
          firstCuePosAfter = currentCue.dwSampleOffset;
      }
    }

    long unsigned newArrayLength = (lastEndSample + 3) * m_channels;

    if (firstCuePosAfter) {
      newArrayLength += (ArrayLength - ((firstCuePosAfter - 1) * m_channels));

      // we must also move one or more cues to new positions
      unsigned samplesToRemove = (firstCuePosAfter - 1) - (lastEndSample + 3);
      for (unsigned i = 0; i < m_cues->GetNumberOfCues(); i++) {
        CUEPOINT currentCue;
        m_cues->GetCuePoint(i, currentCue);
        if (currentCue.dwSampleOffset > lastEndSample) {
          unsigned newCuePosition = currentCue.dwSampleOffset - samplesToRemove;
          m_cues->ChangePosition(newCuePosition, i);
        }
      }
    }

    // Time to copy data to a temporary array and then to cleaned target array
    if ((m_minorFormat == SF_FORMAT_DOUBLE) || (m_minorFormat == SF_FORMAT_FLOAT)) {
      double *audioData = new double[newArrayLength];

      for (unsigned i = 0; i < (lastEndSample + 3) * m_channels; i++)
        audioData[i] = doubleAudioData[i];

      if (firstCuePosAfter) {
        unsigned newIdx = (lastEndSample + 3) * m_channels;
        for (unsigned i = (firstCuePosAfter - 1) * m_channels; i < ArrayLength; i++) {
          audioData[newIdx] = doubleAudioData[i];
          newIdx++;
        }
      }

      delete[] doubleAudioData;
      doubleAudioData = new double[newArrayLength];
      ArrayLength = newArrayLength;

      for (unsigned i = 0; i < ArrayLength; i++)
        doubleAudioData[i] = audioData[i];

      delete[] audioData;
    } else if ((m_minorFormat == SF_FORMAT_PCM_16) || (m_minorFormat == SF_FORMAT_PCM_S8) || (m_minorFormat == SF_FORMAT_PCM_U8)) {
      short *audioData = new short[newArrayLength];

      for (unsigned i = 0; i < (lastEndSample + 3) * m_channels; i++)
        audioData[i] = shortAudioData[i];

      if (firstCuePosAfter) {
        unsigned newIdx = (lastEndSample + 3) * m_channels;
        for (unsigned i = (firstCuePosAfter - 1) * m_channels; i < ArrayLength; i++) {
          audioData[newIdx] = shortAudioData[i];
          newIdx++;
        }
      }

      delete[] shortAudioData;
      shortAudioData = new short[newArrayLength];
      ArrayLength = newArrayLength;

      for (unsigned i = 0; i < ArrayLength; i++)
        shortAudioData[i] = audioData[i];

      delete[] audioData;
    } else {
      int *audioData = new int[newArrayLength];

      for (unsigned i = 0; i < (lastEndSample + 3) * m_channels; i++)
        audioData[i] = intAudioData[i];

      if (firstCuePosAfter) {
        unsigned newIdx = (lastEndSample + 3) * m_channels;
        for (unsigned i = (firstCuePosAfter - 1) * m_channels; i < ArrayLength; i++) {
          audioData[newIdx] = intAudioData[i];
          newIdx++;
        }
      }

      delete[] intAudioData;
      intAudioData = new int[newArrayLength];
      ArrayLength = newArrayLength;

      for (unsigned i = 0; i < ArrayLength; i++)
        intAudioData[i] = audioData[i];

      delete[] audioData;
    }
  }
}

bool FileHandling::TrimStart(unsigned timeToTrim) {
  // convert time to samples
  unsigned samples = (timeToTrim / 1000.0) * m_samplerate;
  unsigned samplesToCut = samples * m_channels;

  if (samplesToCut < ArrayLength) {
    // calculate new arraylength
    long unsigned newArrayLength = ArrayLength - samplesToCut;

    if (m_minorFormat == SF_FORMAT_DOUBLE) {
      double *audioData = new double[newArrayLength];

      for (unsigned i = 0; i < newArrayLength; i++)
        audioData[i] = doubleAudioData[samplesToCut + i];

      delete[] doubleAudioData;
      doubleAudioData = new double[newArrayLength];
      ArrayLength = newArrayLength;

      for (unsigned i = 0; i < ArrayLength; i++)
        doubleAudioData[i] = audioData[i];

      delete[] audioData;

    } else if ((m_minorFormat == SF_FORMAT_PCM_16) || (m_minorFormat == SF_FORMAT_PCM_S8) || (m_minorFormat == SF_FORMAT_PCM_U8)) {
      short *audioData = new short[newArrayLength];

      double *oldDblData = new double[ArrayLength];
      bool gotData = GetDoubleAudioData(oldDblData);

      if (!gotData)
        return false;

      for (unsigned i = 0; i < newArrayLength; i++)
        audioData[i] = shortAudioData[samplesToCut + i];

      delete[] shortAudioData;
      shortAudioData = new short[newArrayLength];

      for (unsigned i = 0; i < newArrayLength; i++)
        shortAudioData[i] = audioData[i];
      
      delete[] audioData;

    } else if ((m_minorFormat == SF_FORMAT_PCM_24) || (m_minorFormat == SF_FORMAT_PCM_32)) {
      int *audioData = new int[newArrayLength];

      double *oldDblData = new double[ArrayLength];
      bool gotData = GetDoubleAudioData(oldDblData);

      if (!gotData)
        return false;

      for (unsigned i = 0; i < newArrayLength; i++)
        audioData[i] = intAudioData[samplesToCut + i];

      delete[] intAudioData;
      intAudioData = new int[newArrayLength];

      for (unsigned i = 0; i < newArrayLength; i++)
        intAudioData[i] = audioData[i];

      delete[] audioData;

    }

    // always update float data as it's used for playback
    float *audioData = new float[newArrayLength];

    double *oldDblData = new double[ArrayLength];
    bool gotData = GetDoubleAudioData(oldDblData);

    if (!gotData)
      return false;

    for (unsigned i = 0; i < newArrayLength; i++)
    audioData[i] = floatAudioData[samplesToCut + i];

    delete[] floatAudioData;
    floatAudioData = new float[newArrayLength];

    for (unsigned i = 0; i < newArrayLength; i++)
      floatAudioData[i] = audioData[i];

    // update the double data in wavetracks too!
    double *newDblData = new double[newArrayLength];
    for (unsigned i = 0; i < newArrayLength; i++)
      newDblData[i] = oldDblData[samplesToCut + i];
        
    ArrayLength = newArrayLength;
      
    UpdateWaveTracks(newDblData);
      
    delete[] audioData;
    delete[] newDblData;
    delete[] oldDblData;

    // if loops and/or cues exist they must now be moved!
    m_loops->MoveLoops(samples);
    m_cues->MoveCues(samples);

    return true;
  } else {
    return false;
  }
}

bool FileHandling::TrimEnd(unsigned timeToTrim) {
  // convert time to samples
  unsigned samples = (timeToTrim / 1000.0) * m_samplerate;
  unsigned samplesToCut = samples * m_channels;

  if (samplesToCut < ArrayLength) {
    // calculate new arraylength
    long unsigned newArrayLength = ArrayLength - samplesToCut;

    if (m_minorFormat == SF_FORMAT_DOUBLE) {
      double *audioData = new double[newArrayLength];

      for (unsigned i = 0; i < newArrayLength; i++)
        audioData[i] = doubleAudioData[i];

      delete[] doubleAudioData;
      doubleAudioData = new double[newArrayLength];
      ArrayLength = newArrayLength;

      for (unsigned i = 0; i < ArrayLength; i++)
        doubleAudioData[i] = audioData[i];

      delete[] audioData;

    } else if ((m_minorFormat == SF_FORMAT_PCM_16) || (m_minorFormat == SF_FORMAT_PCM_S8) || (m_minorFormat == SF_FORMAT_PCM_U8)) {
      short *audioData = new short[newArrayLength];
      
      double *oldDblData = new double[ArrayLength];
      bool gotData = GetDoubleAudioData(oldDblData);

      if (!gotData)
        return false;

      for (unsigned i = 0; i < newArrayLength; i++)
        audioData[i] = shortAudioData[i];

      delete[] shortAudioData;
      shortAudioData = new short[newArrayLength];

      for (unsigned i = 0; i < newArrayLength; i++)
        shortAudioData[i] = audioData[i];

      delete[] audioData;

    } else if ((m_minorFormat == SF_FORMAT_PCM_24) || (m_minorFormat == SF_FORMAT_PCM_32)){
      int *audioData = new int[newArrayLength];

      for (unsigned i = 0; i < newArrayLength; i++)
        audioData[i] = intAudioData[i];

      delete[] intAudioData;
      intAudioData = new int[newArrayLength];

      for (unsigned i = 0; i < newArrayLength; i++)
        intAudioData[i] = audioData[i];

      delete[] audioData;
    }
    
    // always update float audio data since it's used for playback
    float *audioData = new float[newArrayLength];

    double *oldDblData = new double[ArrayLength];
    bool gotData = GetDoubleAudioData(oldDblData);

    if (!gotData)
      return false;

    for (unsigned i = 0; i < newArrayLength; i++)
      audioData[i] = floatAudioData[i];

    delete[] floatAudioData;
    floatAudioData = new float[newArrayLength];
    for (unsigned i = 0; i < newArrayLength; i++)
      floatAudioData[i] = audioData[i];

    // update the double data in wavetracks too
    double *dblData = new double[newArrayLength];
    for (unsigned i = 0; i < newArrayLength; i++)
      dblData[i] = oldDblData[i];

    ArrayLength = newArrayLength;

    UpdateWaveTracks(dblData);

    delete[] audioData;
    delete[] dblData;
    delete[] oldDblData;

    // Check if loops and/or cues still is within audio data!
    m_loops->AreLoopsStillValid(ArrayLength);
    m_cues->AreCuesValidStill(ArrayLength);

    return true;
  } else {
    return false;
  }
}

/*
 * TrimAsRelease() is used in batch mode to extract the audio data from last
 * cue marker existing in file and export it to a (new) file
 */
bool FileHandling::TrimAsRelease() {
  // There must be at least one cue point existing to use this function
  if (!m_cues->GetNumberOfCues())
    return false;

  // Get the last cue position in data
  unsigned lastCuePos = 0;
  for (unsigned i = 0; i < m_cues->GetNumberOfCues(); i++) {
    CUEPOINT currentCue;
    m_cues->GetCuePoint(i, currentCue);
    if (currentCue.dwSampleOffset > lastCuePos) {
      lastCuePos = currentCue.dwSampleOffset;
    }
  }

  unsigned cuePositionInData = lastCuePos * m_channels;
  long unsigned int newArrayLength = ArrayLength - (cuePositionInData);

  if ((m_minorFormat == SF_FORMAT_DOUBLE) || (m_minorFormat == SF_FORMAT_FLOAT)) {
    double *audioData = new double[newArrayLength];

    for (unsigned i = 0; i < newArrayLength; i++)
      audioData[i] = doubleAudioData[cuePositionInData + i];

    delete[] doubleAudioData;
    doubleAudioData = new double[newArrayLength];

    for (unsigned i = 0; i < newArrayLength; i++)
      doubleAudioData[i] = audioData[i];

    delete[] audioData;

  } else if ((m_minorFormat == SF_FORMAT_PCM_16) || (m_minorFormat == SF_FORMAT_PCM_S8) || (m_minorFormat == SF_FORMAT_PCM_U8)) {
    short *audioData = new short[newArrayLength];

    for (unsigned i = 0; i < newArrayLength; i++)
      audioData[i] = shortAudioData[cuePositionInData + i];

    delete[] shortAudioData;
    shortAudioData = new short[newArrayLength];

    for (unsigned i = 0; i < newArrayLength; i++)
      shortAudioData[i] = audioData[i];

    delete[] audioData;

  } else if ((m_minorFormat == SF_FORMAT_PCM_24) || (m_minorFormat == SF_FORMAT_PCM_32)) {
    int *audioData = new int[newArrayLength];

    for (unsigned i = 0; i < newArrayLength; i++)
      audioData[i] = intAudioData[cuePositionInData + i];

    delete[] intAudioData;
    intAudioData = new int[newArrayLength];

    for (unsigned i = 0; i < newArrayLength; i++)
      intAudioData[i] = audioData[i];

    delete[] audioData;
  }

  // also update float data as it always exist
  float *audioData = new float[newArrayLength];

  for (unsigned i = 0; i < newArrayLength; i++)
    audioData[i] = floatAudioData[cuePositionInData + i];

  delete[] floatAudioData;
  floatAudioData = new float[newArrayLength];

  for (unsigned i = 0; i < newArrayLength; i++)
    floatAudioData[i] = audioData[i];

  ArrayLength = newArrayLength;

  delete[] audioData;

  // if loops and/or cues exist they must now be removed!
  for (unsigned i = 0; i < (unsigned) m_loops->GetNumberOfLoops(); i++)
    m_loops->SetSaveOption(false, i);

  for (unsigned i = 0; i < m_cues->GetNumberOfCues(); i++)
    m_cues->SetSaveOption(false, i);

  return true;
}

/*
 * TrimAsAttack is used in batch mode to extract the audio data from start of
 * the file to slightly after last loop and export it to a (new) file.
 */
bool FileHandling::TrimAsAttack() {
  // There must be at least one loop existing to use this function
  if (!m_loops->GetNumberOfLoops())
    return false;

  // Get last position of loop in the audio data
  unsigned lastEndSample = 0;
  for (int i = 0; i < m_loops->GetNumberOfLoops(); i++) {
    LOOPDATA currentLoop;
    m_loops->GetLoopData(i, currentLoop);
    if (currentLoop.dwEnd > lastEndSample)
      lastEndSample = currentLoop.dwEnd;
  }

  long unsigned newArrayLength = (lastEndSample + 3) * m_channels;

  if ((m_minorFormat == SF_FORMAT_DOUBLE) || (m_minorFormat == SF_FORMAT_FLOAT)) {
    double *audioData = new double[newArrayLength];

    for (unsigned i = 0; i < newArrayLength; i++)
      audioData[i] = doubleAudioData[i];

    delete[] doubleAudioData;
    doubleAudioData = new double[newArrayLength];

    for (unsigned i = 0; i < newArrayLength; i++)
      doubleAudioData[i] = audioData[i];

    delete[] audioData;

  } else if ((m_minorFormat == SF_FORMAT_PCM_16) || (m_minorFormat == SF_FORMAT_PCM_S8) || (m_minorFormat == SF_FORMAT_PCM_U8)) {
    short *audioData = new short[newArrayLength];

    for (unsigned i = 0; i < newArrayLength; i++)
      audioData[i] = shortAudioData[i];

    delete[] shortAudioData;
    shortAudioData = new short[newArrayLength];

    for (unsigned i = 0; i < newArrayLength; i++)
      shortAudioData[i] = audioData[i];

    delete[] audioData;

  } else if ((m_minorFormat == SF_FORMAT_PCM_24) || (m_minorFormat == SF_FORMAT_PCM_32)) {
    int *audioData = new int[newArrayLength];

    for (unsigned i = 0; i < newArrayLength; i++)
      audioData[i] = intAudioData[i];

    delete[] intAudioData;
    intAudioData = new int[newArrayLength];

    for (unsigned i = 0; i < newArrayLength; i++)
      intAudioData[i] = audioData[i];

    delete[] audioData;
  }

  // also update float data as it always exist
  float *audioData = new float[newArrayLength];

  for (unsigned i = 0; i < newArrayLength; i++)
    audioData[i] = floatAudioData[i];

  delete[] floatAudioData;
  floatAudioData = new float[newArrayLength];

  for (unsigned i = 0; i < newArrayLength; i++)
    floatAudioData[i] = audioData[i];

  ArrayLength = newArrayLength;

  delete[] audioData;

  // if cues exist they must now be removed!
  for (unsigned i = 0; i < m_cues->GetNumberOfCues(); i++)
    m_cues->SetSaveOption(false, i);

  return true;

}

/*
 * ExportLoopAsNewFile is used in batch mode to export the audio data in
 * loop(s) to individual new file(s).
 */
bool FileHandling::ExportLoopAsNewFile(wxString fileName, wxString path, int loopIdx) {
  // There must be at least one loop existing to use this function
  if (!m_loops->GetNumberOfLoops() || loopIdx >= m_loops->GetNumberOfLoops())
    return false;

  LOOPDATA loopToExport;
  m_loops->GetLoopData(loopIdx, loopToExport);
  if (loopToExport.dwEnd <= loopToExport.dwStart)
    return false;

  unsigned arrayLength = (loopToExport.dwEnd - loopToExport.dwStart + 1) * m_channels;
  unsigned loopStartIdx = loopToExport.dwStart * m_channels;
  wxString filePath = path + wxFILE_SEP_PATH + fileName;

  // Open the file to write
  SndfileHandle sf = SndfileHandle(std::string(filePath.mb_str()), SFM_WRITE, m_format, m_channels, m_samplerate);

  // Write LIST INFO strings if they are set in the original file
  if (m_info.artist != wxEmptyString)
    sf.setString(SF_STR_ARTIST, (const char*)m_info.artist.mb_str());

  if (m_info.copyright != wxEmptyString)
    sf.setString(SF_STR_COPYRIGHT, (const char*)m_info.copyright.mb_str());

  // Remember to set the software string of the used software!
  m_info.software = wxT("LoopAuditioneer");
  if (m_info.software != wxEmptyString)
    sf.setString(SF_STR_SOFTWARE, (const char*)m_info.software.mb_str());

  if (m_info.comment != wxEmptyString)
    sf.setString(SF_STR_COMMENT, (const char*)m_info.comment.mb_str());

  // the creation date must be formatted as YYYY-MM-DD and nothing else
  if (m_info.creation_date.IsValid()) {
    wxString dateString = m_info.creation_date.FormatISODate();
    sf.setString(SF_STR_DATE, (const char*)dateString.mb_str());
  }

  // Copy/write the audio data
  if (m_minorFormat == SF_FORMAT_DOUBLE) {
    double *audioData = new double[arrayLength];
    for (unsigned i = 0; i < arrayLength; i++)
      audioData[i] = doubleAudioData[loopStartIdx + i];
    sf.write(audioData, arrayLength);
    delete[] audioData;
  } else if (m_minorFormat == SF_FORMAT_FLOAT) {
    float *audioData = new float[arrayLength];
    for (unsigned i = 0; i < arrayLength; i++)
      audioData[i] = floatAudioData[loopStartIdx + i];
    sf.write(audioData, arrayLength);
    delete[] audioData;
  } else if ((m_minorFormat == SF_FORMAT_PCM_16) || (m_minorFormat == SF_FORMAT_PCM_S8) || (m_minorFormat == SF_FORMAT_PCM_U8)) {
    short *audioData = new short[arrayLength];
    for (unsigned i = 0; i < arrayLength; i++)
      audioData[i] = shortAudioData[loopStartIdx + i];
    sf.write(audioData, arrayLength);
    delete[] audioData;
  } else {
    int *audioData = new int[arrayLength];
    for (unsigned i = 0; i < arrayLength; i++)
      audioData[i] = intAudioData[loopStartIdx + i];
    sf.write(audioData, arrayLength);
    delete[] audioData;
  }

  // The new file will be finally closed when the instance of sf is deleted!
  return true;
}

void FileHandling::PerformFade(unsigned fadeLength, int fadeType) {
  double *audioData = new double[ArrayLength];
  bool gotData = GetDoubleAudioData(audioData);

  if (!gotData)
    return;
  
  // fadeType 0 == fade in, anything else is a fade out

  unsigned samplesToFade = (fadeLength / 1000.0) * m_samplerate;

  // prepare an array for the crossfade curve data
  double *fadeData = new double[samplesToFade];

  // linear data from 0 to 1
  for (unsigned i = 0; i < samplesToFade; i++)
    fadeData[i] = i * 1.0 / (samplesToFade - 1);

  if (fadeType == 0) {
    for (unsigned i = 0; i < samplesToFade; i++) {
      for (int j = 0; j < m_channels; j++) {
        audioData[i * m_channels + j] *= fadeData[i];
        // also change stored audio data as type which might need conversion
        if (shortAudioData != NULL) {
          shortAudioData[i * m_channels + j] = lrint(audioData[i * m_channels + j] * (1.0 * 0x7FFF));
        } else if (intAudioData != NULL) {
          intAudioData[i * m_channels + j] = lrint(audioData[i * m_channels + j] * (1.0 * 0x7FFFFFFF));
        } else if (doubleAudioData != NULL) {
          doubleAudioData[i * m_channels + j] = audioData[i * m_channels + j];
        }
        // this should always be the case as floats are used for playback
        floatAudioData[i * m_channels + j] = (float) audioData[i * m_channels + j];
      }
    }
  } else {
    for (unsigned i = 0; i < samplesToFade; i++) {
      for (int j = 0; j < m_channels; j++) {
        audioData[(ArrayLength - 1) - (i + j)] *= fadeData[i];
        // also change stored audio data as type which might need conversion
        if (shortAudioData != NULL) {
          shortAudioData[(ArrayLength - 1) - (i + j)] = lrint(audioData[(ArrayLength - 1) - (i + j)] * (1.0 * 0x7FFF));
        } else if (intAudioData != NULL) {
          intAudioData[(ArrayLength - 1) - (i + j)] = lrint(audioData[(ArrayLength - 1) - (i + j)] * (1.0 * 0x7FFFFFFF));
        } else if (doubleAudioData != NULL) {
          doubleAudioData[(ArrayLength - 1) - (i + j)] = audioData[(ArrayLength - 1) - (i + j)];
        }
        // this should always be the case as floats are used for playback
        floatAudioData[(ArrayLength - 1) - (i + j)] = (float) audioData[(ArrayLength - 1) - (i + j)];
      }
    }
  }
  UpdateWaveTracks(audioData);
  delete[] audioData;
  delete[] fadeData;
}

bool FileHandling::GetDoubleAudioData(double audio[]) {
  if (!waveTracks.empty()) {
    unsigned length = waveTracks.size() * waveTracks[0].waveData.size();

    if (ArrayLength == length) {
      for (unsigned i = 0; i < waveTracks.size(); i++) {
        for (unsigned j = 0; j < waveTracks[0].waveData.size(); j++) {
          audio[i + j * waveTracks.size()] = waveTracks[i].waveData[j];
        }
      }
      return true;
    }
    return false;
  } else {
    return false;
  }
}

void FileHandling::UpdateWaveTracks(double audio[]) {
  // first empty old wavetracks
  for (unsigned i = 0; i < waveTracks.size(); i++)
    waveTracks[i].waveData.clear();

  // copy the new track data to right tracks
  int index = 0;
  for (unsigned i = 0; i < ArrayLength; i++) {
    // de-interleaving
    waveTracks[index].waveData.push_back(audio[i]);
    index++;

    if (index == m_channels)
      index = 0;
  }
}

std::pair<unsigned, unsigned> FileHandling::GetSustainsection() {
  std::pair <unsigned, unsigned> sustainStartAndEnd;
  
  if (m_useAutoSustain) {
    sustainStartAndEnd.first = m_autoSustainStart;
    sustainStartAndEnd.second = m_autoSustainEnd;
  } else {
    sustainStartAndEnd.first = m_sliderSustainStart;
    sustainStartAndEnd.second = m_sliderSustainEnd;
  }
  return sustainStartAndEnd;
}

void FileHandling::SetSliderSustainsection(int start, int end) {
  m_sliderSustainStart = ((double) start / 100.0) * ArrayLength / m_channels;
  m_sliderSustainEnd = ((double) end / 100.0) * ArrayLength / m_channels;
}

void FileHandling::SetAutoSustainSearch(bool choice) {
  m_useAutoSustain = choice;
}

bool FileHandling::GetAutoSustainSearch() {
  return m_useAutoSustain;
}

bool FileHandling::AutoCreateReleaseCue() {
  // from auto sustain end we back until we find a zero crossing in strongest channel
  unsigned nbrSamples = ArrayLength / m_channels;
  double *data = new double[nbrSamples];
  SeparateStrongestChannel(data);
  unsigned cueSampleOffset = m_autoSustainEnd;
  if (cueSampleOffset < nbrSamples) {
    if (data[cueSampleOffset] > 0) {
      while (data[cueSampleOffset] > 0) {
        cueSampleOffset--;
      }
    } else {
      while (data[cueSampleOffset] < 0) {
        cueSampleOffset--;
      }
    }
    if (fabs(data[cueSampleOffset]) > fabs(data[cueSampleOffset + 1])) {
      cueSampleOffset += 1;
    }

    CUEPOINT newCue;
    newCue.dwName = m_cues->GetNumberOfCues(); // this should be the new cues index
    newCue.dwPosition = 0;
    newCue.fccChunk = 1635017060; // value for data chunk
    newCue.dwChunkStart = 0;
    newCue.dwBlockStart = 0;
    newCue.dwSampleOffset = cueSampleOffset;
    newCue.keepThisCue = true;

    m_cues->AddCue(newCue); // add the cue to the file cue vector
    delete[] data;
    return true;
  } else {
    delete[] data;
    return false;
  }
}

wxString FileHandling::GetFileName() {
  return m_fileName;
}

double FileHandling::GetLoopQuality(unsigned loopNbr) {
  if (loopNbr < m_loops->GetNumberOfLoops()) {
    LOOPDATA loop;
    m_loops->GetLoopData(loopNbr, loop);
    unsigned compareStartIndex = loop.dwStart;
    unsigned compareEndIndex = loop.dwEnd;
    std::vector<double> loopQualityChannels = CalculateLoopQuality(compareStartIndex, compareEndIndex);
    double lowestQuality = 11;
    if (!loopQualityChannels.empty()) {
      std::vector<double>::iterator worst = std::max_element(loopQualityChannels.begin(), loopQualityChannels.end());
      lowestQuality = *worst;
    }
    return lowestQuality;
  } else {
    return 12;
  }
}

double FileHandling::GetLoopQuality(unsigned start, unsigned end) {
    std::vector<double> loopQualityChannels = CalculateLoopQuality(start, end);
    double lowestQuality = 11;
    if (!loopQualityChannels.empty()) {
      std::vector<double>::iterator worst = std::max_element(loopQualityChannels.begin(), loopQualityChannels.end());
      lowestQuality = *worst;
    }
    return lowestQuality;
}

std::vector<double> FileHandling::CalculateLoopQuality(unsigned startIdx, unsigned endIdx) {
  std::vector<double> qualityValues;
  if (startIdx > 4 && endIdx > startIdx && endIdx < waveTracks[0].waveData.size() - 1) {
    startIdx -= 5;
    endIdx -= 4;
  } else {
    return qualityValues;
  }

  for (unsigned i = 0; i < waveTracks.size(); i++) {
    double channelDiff = 0;
    for (int j = 0; j < 5; j++) {
      double difference = fabs(waveTracks[i].waveData[startIdx + j] - waveTracks[i].waveData[endIdx + j]);
      channelDiff += difference;
    }
    qualityValues.push_back(channelDiff);
  }

  return qualityValues;
}

double FileHandling::GetStrongestSampleValue() {
  double strongestValue = 0;
  for (unsigned i = 0; i < waveTracks.size(); i++) {
    for (unsigned j = 0; j < waveTracks[i].waveData.size(); j++) {
      double currentValue = sqrt(pow(waveTracks[i].waveData[j], 2));
      if (currentValue > strongestValue)
        strongestValue = currentValue;
    }
  }
  return strongestValue;
}
