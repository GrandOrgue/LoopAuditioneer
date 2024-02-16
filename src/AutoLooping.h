/* 
 * AutoLooping.h tries to find natural good loop points in audio
 * Copyright (C) 2011-2024 Lars Palo 
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

#ifndef AUTOLOOPING_H
#define AUTOLOOPING_H

#include <vector>
#include "FileHandling.h"

class AutoLooping {
public:
  // the constructor sets up the general settings for loopfinding
  // default values if not sent with the call to the constructor
  AutoLooping(
    double = 0.03,
    double = 1.0,
    double = 0.3,
    double = 6,
    unsigned = 50000,
    unsigned = 6,
    unsigned = 10
  );

  ~AutoLooping();

  // this is the function that analyze the audio data for loops and will store
  // the best loops found in the vector sent by reference
  bool AutoFindLoops(
    FileHandling *audioFile, // an array of audio data from the strongest channel in file
    unsigned samplerate,
    std::vector<std::pair<std::pair<unsigned, unsigned>, double> > &loops,
    unsigned sustainStart,
    unsigned sustainEnd,
    std::vector<std::pair<unsigned, unsigned> > &loopsAlreadyInFile
  );

  // Functions for setting private variables
  void SetThreshold(double th);
  void SetDuration(double d);
  void SetBetween(double b);
  void SetQuality(double q);
  void SetCandidates(int c);
  void SetLoops(int l);
  void SetMultiple(int m);
  void SetBruteForce(bool b);

  double GetThreshold();
  double GetMinDuration();
  double GetMinDistance();
  double GetQuality();
  unsigned GetCandidates();
  unsigned GetLoopsToReturn();
  unsigned GetLoopMultiple();
  bool GetBruteForce();

private:
  double m_derivativeThreshold;  // 0.03 (3 %)
  double m_minLoopDuration;      // 1.0 seconds
  double m_distanceBetweenLoops; // 0.3 seconds
  double m_qualityFactor;        // value (6) /32767 (0.00006) for float)
  unsigned m_maxCandidates;      // 50000
  unsigned m_loopsToReturn;      // 6
  unsigned m_maxLoopsMultiple;   // 10
  bool m_useBruteForce;
};

#endif
