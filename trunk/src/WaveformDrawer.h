/* 
 * WaveformDrawer draws the waveform from an audio file
 * Copyright (C) 2011-2021 Lars Palo 
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

#ifndef WAVEFORMDRAWER_H
#define WAVEFORMDRAWER_H

#include <wx/wx.h>
#include <vector>
#include "FileHandling.h"
#include "wx/overlay.h"

typedef struct {
  int placedInRow;
  std::vector<int> overlappingLoops;
} LOOPLAYOUT;

typedef struct {
  int putInRow;
  std::vector<int> withinLoop;
  std::vector<int> markerClose;
  std::pair<wxCoord, wxCoord> flagUpLeft;
  std::pair<wxCoord, wxCoord> flagDownRight;
} CUELAYOUT;

typedef struct {
  int yPosHigh;
  int yExtent;
  int xPosLeft;
  int xExtent;
} SUSTAINSECTION_RECT;

class WaveformDrawer : public wxPanel {
public:
  WaveformDrawer(wxFrame *parent, FileHandling *fh);
  ~WaveformDrawer();

  bool somethingHasChanged;

  void paintEvent(wxPaintEvent& event);
  void OnLeftClick(wxMouseEvent& event);
  void OnLeftRelease(wxMouseEvent& event);
  void OnRightClick(wxMouseEvent& event);
  void OnMouseMotion(wxMouseEvent& event);
  void OnMouseLeave(wxMouseEvent& event);
  void OnMouseEnter(wxMouseEvent& event);
  void DrawSustainSectionRectOutline();
  void CalculateSustainRectZones();
  void DrawSustainIndication(wxDC &dc);
  void CalculateSustainIndication();
  void paintNow();
  void OnPaint(wxDC& dc);
  void OnPaintPlayPosition(wxDC& dc);
  void SetPlayPosition(unsigned int pPos);
  void AddCuePosition(unsigned int cuePos);
  void AddLoopPosition(unsigned int startPos, unsigned int endPos);
  void ChangeLoopPositions(unsigned int start, unsigned int end, int idx);
  void ClearMetadata();
  void CalculateLayout();
  void OnKeyDown(wxKeyEvent& event);
  void SetLoopSelection(int idx);
  void SetCueSelection(int idx);

  // Methods for dealing with amplitude zoom level
  int GetAmplitudeZoomLevel();
  void ZoomInAmplitude();
  void ZoomOutAmplitude();

private:
  std::vector<unsigned int> cueSampleOffset;
  std::vector<std::pair<unsigned int, unsigned int> > loopPositions;
  std::vector<LOOPLAYOUT> loopLayout;
  std::vector<CUELAYOUT> cueLayout;
  int xSize;
  int ySize;
  int topMargin;
  int bottomMargin;
  int marginBetweenTracks;
  int leftMargin;
  int rightMargin;
  unsigned int playPosition;
  wxColour white;
  wxColour black;
  wxColour blue;
  wxColour green;
  wxColour red;
  wxColour yellow;
  int trackWidth;
  int trackHeight;
  wxIcon playPositionMarker;
  wxMenu *m_popupMenu;
  wxCoord m_x;
  wxCoord m_y;
  wxCoord m_prev_x;
  wxCoord m_prev_y;
  int selectedCueIndex; // used when changing cue position
  bool cueIsSelected; // used when changing cue position
  int m_amplitudeZoomLevel;
  FileHandling *m_fileReference;
  SUSTAINSECTION_RECT m_sustainsection_rect;
  SUSTAINSECTION_RECT m_old_sustainsection_rect;
  bool mouseWithinSustainSection;
  bool withinLeftChangeBorder;
  bool withinRightChangeBorder;
  wxOverlay m_overlay;
  int m_leftBorderX;
  int m_rightBorderX;
  bool isChangingSustainSection;
  bool outlineHasChanged;
  bool outlineAlreadyDrawn;
  int loopIndexSelection; // -1 when cue is selected otherwise index
  bool hasLoopSelection;
  int cueIndexSelection; // -1 when loop is selected otherwise index
  bool hasCueSelection;

  void OnClickAddCue(wxCommandEvent& event);

  // This class handles events
  DECLARE_EVENT_TABLE()
};

#endif
