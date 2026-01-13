/* 
 * CueMarkers.h is a part of LoopAuditioneer software
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

#include <wx/wx.h>
#include <vector>

typedef struct {
  unsigned int dwName;
  unsigned int dwPosition;
  unsigned int fccChunk;
  unsigned int dwChunkStart;
  unsigned int dwBlockStart;
  unsigned int dwSampleOffset;
  bool keepThisCue;
} CUEPOINT;

class CueMarkers {
public:
  CueMarkers();
  ~CueMarkers();

  void AddCue(CUEPOINT cuePoint);
  unsigned int GetNumberOfCues();
  void GetCuePoint(int index, CUEPOINT& cData);
  void SetSaveOption(bool keep, unsigned index);
  void ChangePosition(unsigned offset, unsigned index);
  void ExportCues();
  void MoveCues(unsigned samples);
  void AreCuesValidStill(long unsigned dataLength);

  std::vector<CUEPOINT> exportedCues;

private:
  unsigned int dwCuePoints;
  std::vector<CUEPOINT> cuePoints;
};

