/* 
 * CueMarkers.cpp is part of LoopAuditioneer software
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

#include "CueMarkers.h"

CueMarkers::CueMarkers() : dwCuePoints(0) {
}

CueMarkers::~CueMarkers() {
}

void CueMarkers::AddCue(CUEPOINT cuePoint) {
  cuePoints.push_back(cuePoint);
  dwCuePoints = cuePoints.size();
}

unsigned int CueMarkers::GetNumberOfCues() {
  return dwCuePoints;
}

void CueMarkers::GetCuePoint(int index, CUEPOINT& cData) {
  cData.dwName = cuePoints[index].dwName;
  cData.dwPosition = cuePoints[index].dwPosition;
  cData.fccChunk = cuePoints[index].fccChunk;
  cData.dwChunkStart = cuePoints[index].dwChunkStart;
  cData.dwBlockStart = cuePoints[index].dwBlockStart;
  cData.dwSampleOffset = cuePoints[index].dwSampleOffset;
  cData.keepThisCue = cuePoints[index].keepThisCue;
}

void CueMarkers::SetSaveOption(bool keep, unsigned index) {
  if (index < dwCuePoints)
    cuePoints[index].keepThisCue = keep;
}

void CueMarkers::ExportCues() {
  unsigned int idx = 0;
  for (unsigned int i = 0; i < dwCuePoints; i++) {
    if (cuePoints[i].keepThisCue) {
      CUEPOINT outCue;

      outCue.dwName = idx;
      outCue.dwPosition = cuePoints[i].dwPosition;
      outCue.fccChunk = cuePoints[i].fccChunk;
      outCue.dwChunkStart = cuePoints[i].dwChunkStart;
      outCue.dwBlockStart = cuePoints[i].dwBlockStart;
      outCue.dwSampleOffset = cuePoints[i].dwSampleOffset;
      outCue.keepThisCue = cuePoints[i].keepThisCue;

      exportedCues.push_back(outCue);
      idx += 1;
    }
  }
}

void CueMarkers::ChangePosition(unsigned offset, unsigned index) {
  if (index < dwCuePoints)
    cuePoints[index].dwSampleOffset = offset;
}

void CueMarkers::MoveCues(unsigned samples) {
  if (cuePoints.empty() == false) {
    for (unsigned i = 0; i < cuePoints.size() ; i++) {
      int difference = (int) cuePoints[i].dwSampleOffset - (int) samples;

      if (difference < 0)
        cuePoints[i].keepThisCue = false;
      else
        cuePoints[i].dwSampleOffset -= samples;
    }
  }
}

void CueMarkers::AreCuesValidStill(long unsigned dataLength) {
  if (cuePoints.empty() == false) {
    for (unsigned i = 0; i < cuePoints.size() ; i++) {
      long int difference = (long int) dataLength - (long int) cuePoints[i].dwSampleOffset;

      if (difference < 0)
        cuePoints[i].keepThisCue = false;
    }
  }
}
