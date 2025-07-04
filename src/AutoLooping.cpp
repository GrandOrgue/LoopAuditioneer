/* 
 * AutoLooping.cpp tries to find natural good loop points in audio
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

#include "AutoLooping.h"
#include <cmath>

AutoLooping::AutoLooping(
    double threshold,
    double minLoopDuration,
    double distanceBetweenLoops,
    double quality,
    unsigned maxCandidates,
    unsigned loopsToReturn,
    unsigned maxLoopsMultiple) {
  m_derivativeThreshold = threshold;
  m_minLoopDuration = minLoopDuration;
  m_distanceBetweenLoops = distanceBetweenLoops;
  m_qualityFactor = quality;
  m_maxCandidates = maxCandidates;
  m_loopsToReturn = loopsToReturn;
  m_maxLoopsMultiple = maxLoopsMultiple;
  m_useBruteForce = false;
}

AutoLooping::~AutoLooping() {
}

bool AutoLooping::AutoFindLoops(
  FileHandling *audioFile,
  std::vector<std::pair<std::pair<unsigned, unsigned>, double> > &loops) {

  unsigned samplerate = audioFile->GetSampleRate();
  // retrieve the used sustainsection
  std::pair <unsigned, unsigned> sustainSection = audioFile->GetSustainsection();
  // another sustain section sanity check!
  // a pipe would normally need around 50 - 150 ms to settle after attack
  unsigned sustainStartIdx = sustainSection.first;
  unsigned sustainEndIdx = sustainSection.second;
  unsigned hundredMsSamples = samplerate / 10;
  if (sustainStartIdx < hundredMsSamples) {
    sustainStartIdx = hundredMsSamples;
    if (sustainStartIdx > sustainEndIdx) {
      // if start index is after end index we have a problem!
      if ((sustainEndIdx + hundredMsSamples) < (audioFile->ArrayLength / audioFile->m_channels)) {
        sustainEndIdx += hundredMsSamples;
      } else {
        // when having adjusted start index to a reasonable value it's now not
        // possible to move back the sustain end for some reason which means
        // that something's very wrong and we should just return false and quit
        return false;
      }
    }
  }

  double *data = new double[audioFile->ArrayLength / audioFile->m_channels];
  audioFile->SeparateStrongestChannel(data);
  // we find maximum derivative in audio data which is where the
  // waveform will change the most (the opposite of what we're interested in)
  double maxDerivative = 0;
  for (
    unsigned i = sustainStartIdx;
    i < sustainEndIdx - 1;
    i++) {

    double currentDerivative = fabs( (data[i + 1] - data[i]) );

    if (currentDerivative > maxDerivative)
      maxDerivative = currentDerivative;
  }

  // since we're interested in sections where the waveform doesn't change a lot
  // we now add all indexes with a derivative below the derivativeThreshold to 
  // the every candidate vector
  std::vector<unsigned> everyLoopCandidates;
  double derivativeThreshold = maxDerivative * m_derivativeThreshold;
  for (
    unsigned i = sustainStartIdx; 
    i < sustainEndIdx - 1;
    i++) {

    double currentDerivative = fabs( (data[i + 1] - data[i]) );

    if (currentDerivative < derivativeThreshold)
      everyLoopCandidates.push_back(i);
  }

  // we're done with the single channel data
  delete[] data;

  // first get all loops already in file
  std::vector<std::pair<unsigned, unsigned> > loopsAlreadyInFile;
  for (int i = 0; i < audioFile->m_loops->GetNumberOfLoops(); i++) {
    LOOPDATA aLoop;
    audioFile->m_loops->GetLoopData(i, aLoop);
    loopsAlreadyInFile.push_back(std::make_pair(aLoop.dwStart, aLoop.dwEnd));
  }

  std::vector<unsigned> loopCandidates;
  // to ensure even distribution of the candidates over sustainsection and
  // limit the number of candidates to m_maxCandidates if more are found
  if (everyLoopCandidates.size() > m_maxCandidates) {
    int totalAmountOfCandidates = everyLoopCandidates.size();
    double increment = (double) totalAmountOfCandidates / (double) m_maxCandidates;
    for (unsigned i = 0; i < m_maxCandidates; i++) {
      int indexToUse = i * increment;
      if (indexToUse < totalAmountOfCandidates - 1)
        loopCandidates.push_back(everyLoopCandidates[indexToUse]);
    }

    everyLoopCandidates.clear();
  } else {
    // just copy the content of every loop candidates vector and get rid of it
    for (unsigned i = 0; i < everyLoopCandidates.size(); i++)
      loopCandidates.push_back(everyLoopCandidates[i]);

    everyLoopCandidates.clear();
  }

  // Then we cross correlate the points and if we get a good match we push the
  // sample indexes of start and end into the foundLoops vector
  // Also note that for both the loopstart and end we compare a "window" of
  // four samples before the candidate plus the candidate which gives
  // five samples per channel to the window. If correlation is sufficiently
  // good then we'll add the loop
  std::vector<std::pair<std::pair<unsigned, unsigned>, double > > foundLoops;
  if (loopCandidates.empty() == true) {
    return false;
  }
  for (unsigned i = 0; i < loopCandidates.size() - 1; i++) {
    // this is for the start point
    unsigned loopStartIndex = loopCandidates[i];
    if (loopStartIndex < 4)
      continue;

    // if loop start point is too close to already stored loop continue
    if (!foundLoops.empty()) {
      if (
        (loopStartIndex - foundLoops.back().first.first) < 
        (samplerate * m_distanceBetweenLoops) && !m_useBruteForce
      ) {
        continue;
      }
    }

    // and now compare to end point candidates and we go from back to get the
    // longest possible loops first
    for (unsigned j = loopCandidates.size() - 1; j > i + 1; j--) {
      unsigned loopEndIndex = loopCandidates[j];
      
      // check if the endpoint is too close to startpoint
      if (loopEndIndex - loopStartIndex < samplerate * m_minLoopDuration)
        continue;

      // the end of a wave file loop should be compared against the sample just before start
      // now comes the actual comparison of the candidates
      double correlationValue = audioFile->GetLoopQuality(loopStartIndex, loopEndIndex);
      // if the quality of the correlation is better (lower) than threshold add the loop
      if (correlationValue <= m_qualityFactor) {
        // make sure the loop doesn't already exist in file, or that it's too close to an existing!
        bool loopAlreadyExist = false;
        for (unsigned k = 0; k < loopsAlreadyInFile.size(); k++) {
          unsigned startDifference = std::abs((int) loopStartIndex - (int) loopsAlreadyInFile[k].first);
          unsigned endDifference = std::abs((int) loopEndIndex - (int) loopsAlreadyInFile[k].second);
          if (loopsAlreadyInFile[k].first == loopStartIndex &&
            loopsAlreadyInFile[k].second == loopEndIndex
          ) {
            loopAlreadyExist = true;
            break;
          } else if (startDifference < (samplerate * m_distanceBetweenLoops) && !m_useBruteForce) {
            loopAlreadyExist = true;
            break;
          } else if (endDifference < (samplerate * m_distanceBetweenLoops) && !m_useBruteForce) {
            loopAlreadyExist = true;
            break;
          }
        }
        if (!loopAlreadyExist) {
          foundLoops.push_back(
            std::make_pair(
              std::make_pair(
                loopStartIndex,
                loopEndIndex
              ),
              correlationValue
            )
          );
        }
        break;
      }
    }
    // if enough loops to select from are found we abort
    if ((foundLoops.size() > m_loopsToReturn * m_maxLoopsMultiple - 1) && !m_useBruteForce)
      break;
  }

  // for easy handling the found loops vector should be sorted by quality
  // which will be done by searching for the best and exchange places so that
  // the best will be first in the vector
  for (unsigned i = 0; i < foundLoops.size(); i++) {
    // find smallest (best) quality among i to foundLoops.size() - 1
    int best = i;
    for (unsigned j = i + 1; j < foundLoops.size(); j++) {
      if (foundLoops[j].second < foundLoops[best].second)
        best = j;
    }

    // now we switch content of index i and best
    unsigned tempStart = foundLoops[i].first.first;
    unsigned tempEnd = foundLoops[i].first.second;
    double tempCorr = foundLoops[i].second;
    foundLoops[i].first.first = foundLoops[best].first.first;
    foundLoops[i].first.second = foundLoops[best].first.second;
    foundLoops[i].second = foundLoops[best].second;
    foundLoops[best].first.first = tempStart;
    foundLoops[best].first.second = tempEnd;
    foundLoops[best].second = tempCorr;
  }

  // the wished number of loops will be pushed back into the loops vector
  // selected from the best quality loops in the foundLoops vector
  // but we should make sure that all returned loops must overlap at least one other
  if (!foundLoops.empty()) {
    std::vector<unsigned> alreadyStoredLoopIndexes;
    bool addedLoop;
    // the first loop should be the best quality and is automatically added if no loops already exist
    if (loopsAlreadyInFile.empty()) {
      loops.push_back(foundLoops[0]);
      alreadyStoredLoopIndexes.push_back(0);
    } else {
      // we temporarily add the already existing loops to the vector
      for (unsigned i = 0; i < loopsAlreadyInFile.size(); i++) {
        loops.push_back(std::make_pair(std::make_pair(loopsAlreadyInFile[i].first, loopsAlreadyInFile[i].second), 0));
      }
    }

    addedLoop = true;

    while ((loops.size() < (m_loopsToReturn + loopsAlreadyInFile.size())) && addedLoop) {
      addedLoop = false;

      for (unsigned i = 0; i < foundLoops.size(); i++) {
        bool jumpToNext = false;
        for (unsigned l = 0; l < alreadyStoredLoopIndexes.size(); l++) {
          if (i == alreadyStoredLoopIndexes[l])
            jumpToNext = true;
        }

        if (jumpToNext)
          continue;
        
        bool overlaps = false;
        bool tooClose = true;
        for (unsigned j = 0; j < loops.size(); j++) {
          if ((foundLoops[i].first.first > loops[j].first.first && foundLoops[i].first.first < loops[j].first.second) ||
              (foundLoops[i].first.second < loops[j].first.second && foundLoops[i].first.second > loops[j].first.first)
          ) {
            overlaps = true;
            // also check if using brute force that it's not too close to any already added
            if (m_useBruteForce) {
              for (unsigned k = 0; k < loops.size(); k++) {
                if (std::abs((int) foundLoops[i].first.first - (int) loops[k].first.first) > samplerate * m_distanceBetweenLoops &&
                  std::abs((int) foundLoops[i].first.second - (int) loops[k].first.second) > samplerate * m_distanceBetweenLoops
                ) {
                  tooClose = false;
                } else {
                  tooClose = true;
                  break;
                }
              }
            } else {
              // the end point must still be examined against already added loops even if not using brute force
              for (unsigned k = 0; k < loops.size(); k++) {
                if (std::abs((int) foundLoops[i].first.second - (int) loops[k].first.second) > samplerate * m_distanceBetweenLoops) {
                  tooClose = false;
                } else {
                  tooClose = true;
                  break;
                }
              }
            }
          }
        }

        if (overlaps && !tooClose) {
          loops.push_back(foundLoops[i]);
          addedLoop = true;
          alreadyStoredLoopIndexes.push_back(i);

          // jump out and begin checking again from start
          break;
        }
      }
    }
    // now remove the temporarily added already existing loops (if previously added)
    if (!loopsAlreadyInFile.empty()) {
      int loopsToRemove = loopsAlreadyInFile.size();
      loops.erase(loops.begin(), loops.begin() + loopsToRemove);
    }
    if (!loops.empty())
      return true;
    else
      return false;
  } else {
    return false;
  }
}

void AutoLooping::SetThreshold(double th) {
  m_derivativeThreshold = th;
}

void AutoLooping::SetDuration(double d) {
  m_minLoopDuration = d;
}

void AutoLooping::SetBetween(double b) {
  m_distanceBetweenLoops = b;
}

void AutoLooping::SetQuality(double q) {
  m_qualityFactor = q;
}

void AutoLooping::SetCandidates(int c) {
  m_maxCandidates = c;
}

void AutoLooping::SetLoops(int l) {
  m_loopsToReturn = l;
}

void AutoLooping::SetMultiple(int m) {
  m_maxLoopsMultiple = m;
}

void AutoLooping::SetBruteForce(bool b) {
  if (b)
    m_useBruteForce = true;
  else
    m_useBruteForce = false;
}

double AutoLooping::GetThreshold() {
  return m_derivativeThreshold;
}

double AutoLooping::GetMinDuration() {
  return m_minLoopDuration;
}

double AutoLooping::GetMinDistance() {
  return m_distanceBetweenLoops;
}

double AutoLooping::GetQuality() {
  return m_qualityFactor;
}

unsigned AutoLooping::GetCandidates() {
  return m_maxCandidates;
}

unsigned AutoLooping::GetLoopsToReturn() {
  return m_loopsToReturn;
}

unsigned AutoLooping::GetLoopMultiple() {
  return m_maxLoopsMultiple;
}

bool AutoLooping::GetBruteForce() {
  return m_useBruteForce;
}
