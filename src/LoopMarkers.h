/* 
 * LoopMarkers.h is a part of LoopAuditioneer software
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
  int dwType;
  unsigned int dwStart;
  unsigned int dwEnd;
  unsigned int dwPlayCount;
  bool shouldBeSaved;
} LOOPDATA;

class LoopMarkers {
public:
  LoopMarkers();
  ~LoopMarkers();

  void AddLoop(LOOPDATA loopData);
  void SetSaveOption(bool toBeSaved, unsigned index);
  void SetLoopPositions(unsigned int start, unsigned int end, int index);
  void SetMIDIUnityNote(char note);
  void SetMIDIPitchFraction(unsigned int pitch);
  char GetMIDIUnityNote();
  unsigned int GetMIDIPitchFraction();
  int GetNumberOfLoops();
  void GetLoopData(int index, LOOPDATA& lData);
  void ExportLoops();
  void MoveLoops(unsigned samples);
  void AreLoopsStillValid(long unsigned dataLength);

  std::vector<LOOPDATA> loopsOut;

private:
  char dwMIDIUnityNote;
  unsigned int dwMIDIPitchFraction;
  int cSampleLoops; // loop_count

  std::vector<LOOPDATA> loopsIn;
};

