/* 
 * LoopMarkers.cpp is a part of LoopAuditioneer software
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

#include "LoopMarkers.h"

LoopMarkers::LoopMarkers() : dwMIDIUnityNote(0), dwMIDIPitchFraction(0), cSampleLoops(0) {
}

LoopMarkers::~LoopMarkers() {
}

void LoopMarkers::AddLoop(LOOPDATA loopData) {
  loopsIn.push_back(loopData);
  cSampleLoops = loopsIn.size();
}

void LoopMarkers::SetSaveOption(bool toBeSaved, unsigned index) {
  if (index < loopsIn.size())
    loopsIn[index].shouldBeSaved = toBeSaved;
}

void LoopMarkers::SetMIDIUnityNote(char note) {
  dwMIDIUnityNote = note;
}

void LoopMarkers::SetMIDIPitchFraction(unsigned int pitch) {
  dwMIDIPitchFraction = pitch;
}

char LoopMarkers::GetMIDIUnityNote() {
  return dwMIDIUnityNote;
}

unsigned int LoopMarkers::GetMIDIPitchFraction() {
  return dwMIDIPitchFraction;
}

int LoopMarkers::GetNumberOfLoops() {
  return cSampleLoops;
}

void LoopMarkers::GetLoopData(int index, LOOPDATA& lData) {
  lData.dwType = loopsIn[index].dwType;
  lData.dwStart = loopsIn[index].dwStart;
  lData.dwEnd = loopsIn[index].dwEnd;
  lData.dwPlayCount = loopsIn[index].dwPlayCount;
  lData.shouldBeSaved = loopsIn[index].shouldBeSaved;
}

void LoopMarkers::ExportLoops() {
  for (int i = 0; i < cSampleLoops; i++) {
    if (loopsIn[i].shouldBeSaved) {
      LOOPDATA outLoop;

      outLoop.dwType = loopsIn[i].dwType;
      outLoop.dwStart = loopsIn[i].dwStart;
      outLoop.dwEnd = loopsIn[i].dwEnd + 1; // +1 to compensate for libsndfile behaviour
      outLoop.dwPlayCount = loopsIn[i].dwPlayCount;
      outLoop.shouldBeSaved = loopsIn[i].shouldBeSaved;

      loopsOut.push_back(outLoop);
    }
  }
}

void LoopMarkers::SetLoopPositions(unsigned int start, unsigned int end, int index) {
  loopsIn[index].dwStart = start;
  loopsIn[index].dwEnd = end;
}

void LoopMarkers::MoveLoops(unsigned samples) {
  if (loopsIn.empty() == false) {
    for (unsigned i = 0; i < loopsIn.size() ; i++) {
      int difference = (int) loopsIn[i].dwStart - (int) samples;

      if (difference < 0)
        loopsIn[i].shouldBeSaved = false;
      else {
        loopsIn[i].dwStart -= samples;
        loopsIn[i].dwEnd -= samples;
      }
    }
  }
}

void LoopMarkers::AreLoopsStillValid(long unsigned dataLength) {
  if (loopsIn.empty() == false) {
    for (unsigned i = 0; i < loopsIn.size() ; i++) {
      long int difference = (long int) dataLength - (long int) loopsIn[i].dwEnd;

      if (difference < 0)
        loopsIn[i].shouldBeSaved = false;
    }
  }
}
