/* 
 * WaveformDrawer draws the waveform from an audio file
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

#include "WaveformDrawer.h"
#include "PlayPositionMarker.xpm"
#include <cmath>
#include <cfloat>
#include "LoopAuditioneer.h"
#include "LoopAuditioneerDef.h"
#include <wx/image.h>
#include <wx/bitmap.h>

BEGIN_EVENT_TABLE(WaveformDrawer, wxPanel)
  EVT_PAINT(WaveformDrawer::paintEvent)
  EVT_RIGHT_DOWN(WaveformDrawer::OnRightClick)
  EVT_MENU(ADD_CUE, WaveformDrawer::OnClickAddCue)
  EVT_LEFT_DOWN(WaveformDrawer::OnLeftClick)
  EVT_LEFT_UP(WaveformDrawer::OnLeftRelease)
  EVT_KEY_DOWN(WaveformDrawer::OnKeyDown)
  EVT_CHAR(WaveformDrawer::OnKeyDown)
  EVT_MOTION(WaveformDrawer::OnMouseMotion)
  EVT_LEAVE_WINDOW(WaveformDrawer::OnMouseLeave)
  EVT_ENTER_WINDOW(WaveformDrawer::OnMouseEnter)
END_EVENT_TABLE()

WaveformDrawer::WaveformDrawer(wxFrame *parent, FileHandling *fh) : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE) {
  m_fileReference = fh;
  somethingHasChanged = false;
  white.Set(wxT("#ffffff"));
  black.Set(wxT("#000000"));
  blue.Set(wxT("#0d0060"));
  green.Set(wxT("#00C800"));
  red.Set(wxT("#f90000"));
  yellow.Set(wxT("#ffff00"));
  xSize = 1;
  ySize = 1;
  topMargin = 10;
  bottomMargin = 10;
  marginBetweenTracks = 0;
  leftMargin = 30;
  rightMargin = 10;
  trackWidth = 0;
  trackHeight = 0;
  playPosition = 0;
  playPositionMarker = wxIcon(PlayPositionMarker_xpm);
  selectedCueIndex = 0;
  cueIsSelected = false;
  m_amplitudeZoomLevel = 1;
  mouseWithinSustainSection = false;
  withinLeftChangeBorder = false;
  withinRightChangeBorder = false;
  isChangingSustainSection = false;
  outlineAlreadyDrawn = false;
  outlineHasChanged = false;
  loopIndexSelection = -1;
  hasLoopSelection = false;
  cueIndexSelection = -1;
  hasCueSelection = false;
  m_x = 0;
  m_y = 0;
  m_prev_x = 0;
  m_prev_y = 0;
  m_leftBorderX = 0;
  m_rightBorderX = 0;

  // create the popup menu for the waveform
  m_popupMenu = new wxMenu();
  m_popupMenu->Append(ADD_CUE, wxT("&Add cue"), wxT("Create a new cue at this position"));
}

// Called when the panel needs to be redrawn (if the panel is resized)
void WaveformDrawer::paintEvent(wxPaintEvent& WXUNUSED(event)) {
  wxPaintDC dc(this);
  OnPaint(dc);
}
 
// Method to call when one wants to force redrawing for playback
void WaveformDrawer::paintNow() {
  RefreshRect(wxRect(0, 0, leftMargin + trackWidth + rightMargin, 9));
}

// Here the actual drawing happens when either the panel is resized or something changes
void WaveformDrawer::OnPaint(wxDC& dc) {
  bool redrawCompletely = false;

  // First get the size of this panel to know if the panel is resized.
  wxSize size = this->GetSize();
  trackWidth = size.x - (leftMargin + rightMargin);
  trackHeight = (size.y - (topMargin + bottomMargin + ((m_fileReference->m_channels - 1) * marginBetweenTracks))) / m_fileReference->m_channels;

  // if playPosition == 0 reset playPosition to correct pixel value instead
  if (playPosition == 0)
    SetPlayPosition(0);

  // Compare with the xSize and ySize members and decide if whole panel should be redrawn
  if (size.x == xSize && size.y == ySize)
    redrawCompletely = false;
  else
    redrawCompletely = true;

  // Check if metadata has changed which calls for a layout calculation and complete redraw
  if (somethingHasChanged) {
    redrawCompletely = true;
  }

  // This is for a complete redraw of the panel which includes waveform and metadata
  if (redrawCompletely) {
    // first calculate the layout
    CalculateLayout();

    dc.SetBrush(wxBrush(white));
    dc.SetPen(wxPen(black, 1, wxPENSTYLE_SOLID));
    dc.SetFont(wxFont(6, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_LIGHT));

    // draw playposition rectangle
    dc.DrawRectangle(leftMargin, 0, trackWidth, 10);

    // draw the track containing rectangles
    if (m_fileReference->m_channels > 0) {
      for (int i = 0; i < m_fileReference->m_channels; i++) {
        int x1, y1, x2, y2;
        x1 = leftMargin;
        y1 = topMargin + trackHeight * i + i * marginBetweenTracks; // top margin + trackheight * tracknumber + margin between tracks
        x2 = trackWidth;
        y2 = trackHeight;
        dc.DrawRectangle(x1, y1, x2, y2);
        dc.DrawText(wxString::Format(wxT("%.2f"), (double) 1.0 / (double) m_amplitudeZoomLevel), 9, topMargin + trackHeight * i + i * marginBetweenTracks);
        dc.DrawText(wxT("0"), 16, topMargin - 5 + trackHeight * i + i * marginBetweenTracks + (trackHeight / 2));
        dc.DrawText(wxString::Format(wxT("-%.2f"), (double) 1.0 / (double) m_amplitudeZoomLevel), 6, topMargin - 2 + trackHeight * i + i * marginBetweenTracks + trackHeight - 10);
      }
    }

    if (m_fileReference->waveTracks[0].waveData.size() > 0) {
      int nrOfSamples = m_fileReference->waveTracks[0].waveData.size();
      int samplesPerPixel;
    
      if (nrOfSamples % trackWidth == 0)
        samplesPerPixel = nrOfSamples / trackWidth;
      else
        samplesPerPixel = (nrOfSamples / trackWidth) + 1;

      double maxValue = 0, minValue = 0;
      int lineToDraw = 0;
      for (unsigned j = 0; j < m_fileReference->waveTracks.size(); j++) {
        for (unsigned i = 0; i < m_fileReference->waveTracks[0].waveData.size(); i++) {
          if (i % samplesPerPixel == 0 && i > 0) {
            // we should write the line representing the audio data and start a new count
            // but first we adjust max and min values with the m_amplitudeZoomLevel
            maxValue *= m_amplitudeZoomLevel;
            minValue *= m_amplitudeZoomLevel;
            if (maxValue > 1)
              maxValue = 1;
            if (minValue < -1)
              minValue = -1;

            // calculate coordinates
            wxCoord x1 = leftMargin + lineToDraw, y1 = topMargin + trackHeight * j + j * marginBetweenTracks + (trackHeight / 2) - (maxValue * trackHeight / 2);
            wxCoord x2 = leftMargin + lineToDraw, y2 = topMargin + trackHeight * j + j * marginBetweenTracks + (trackHeight / 2) - (minValue * trackHeight / 2);

            dc.SetPen(wxPen(blue, 1, wxPENSTYLE_SOLID));
            dc.DrawLine(x1, y1, x2, y2);

            // proceed with next frame
            maxValue = m_fileReference->waveTracks[j].waveData[i];
            minValue = m_fileReference->waveTracks[j].waveData[i];
            lineToDraw++;
          } else {
            if (m_fileReference->waveTracks[j].waveData[i] > maxValue)
              maxValue = m_fileReference->waveTracks[j].waveData[i];
            else if (m_fileReference->waveTracks[j].waveData[i] < minValue)
              minValue = m_fileReference->waveTracks[j].waveData[i];
          }
        }
        // draw the 0 indicating line
        dc.SetPen(wxPen(blue, 1, wxPENSTYLE_SOLID));
        dc.DrawLine((leftMargin + 1), topMargin + trackHeight * j + j * marginBetweenTracks + (trackHeight / 2), size.x - (rightMargin + 1), topMargin + trackHeight * j + j * marginBetweenTracks + (trackHeight / 2));

        // reset the wave pixel position for the next track
        lineToDraw = 0;
        maxValue = 0;
        minValue = 0;
      }
      // draw in eventual metadata (loops and cues)
      dc.SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
      if (cueSampleOffset.size() > 0) {
        int overlap = 0;
        // here we draw the cues from the vector
        for (unsigned i = 0; i < cueSampleOffset.size(); i++) {
          overlap = cueLayout[i].putInRow;
          // the positions from dwSampleOffset is in sample frames so it has to be re-calculated into pixels
          int xPosition = cueSampleOffset[i] / samplesPerPixel + leftMargin;
          int yPositionHigh = topMargin + 1;
          int yPositionLow = topMargin + trackHeight * m_fileReference->waveTracks.size() + (marginBetweenTracks * (m_fileReference->waveTracks.size() - 1) - 1);
          if (hasCueSelection && i == (unsigned) cueIndexSelection) {
            dc.SetPen(wxPen(green, 1, wxPENSTYLE_SOLID));
          } else {
            dc.SetPen(wxPen(green, 1, wxPENSTYLE_DOT));
          }
          wxSize extent = dc.GetTextExtent(wxString::Format(wxT("M%i"), i + 1));
          dc.DrawLine(xPosition, yPositionLow, xPosition, yPositionHigh + overlap * (extent.GetHeight() + 5));
          dc.DrawRectangle(xPosition, yPositionHigh + overlap * (extent.GetHeight() + 5), extent.GetWidth() + 2, extent.GetHeight());
          cueLayout[i].flagUpLeft = std::make_pair(xPosition, yPositionHigh + overlap * (extent.GetHeight() + 5)); 
          cueLayout[i].flagDownRight = std::make_pair(xPosition + extent.GetWidth() + 2, yPositionHigh + overlap * (extent.GetHeight() + 5) + extent.GetHeight());
          dc.DrawText(wxString::Format(wxT("M%i"), i + 1), xPosition + 1, yPositionHigh + overlap * (extent.GetHeight() + 5));
        }
      }
      if (loopPositions.size() > 0) {
        // here we draw the loops from the vector
        int overlap = 0;
        int yPositionHigh = topMargin + 1;
        int yPositionLow = topMargin + trackHeight * m_fileReference->waveTracks.size() + (marginBetweenTracks * (m_fileReference->waveTracks.size() - 1) - 1);

        for (unsigned i = 0; i < loopPositions.size(); i++) {
          // the loop start value (in samples) is in loopPositions[i].first
          int xPositionS = loopPositions[i].first / samplesPerPixel + leftMargin;

          overlap = loopLayout[i].placedInRow;

          if (hasLoopSelection && i == (unsigned) loopIndexSelection) {
            dc.SetPen(wxPen(red, 1, wxPENSTYLE_SOLID));
          } else {
            dc.SetPen(wxPen(red, 1, wxPENSTYLE_DOT_DASH));
          }
          wxSize extent = dc.GetTextExtent(wxString::Format(wxT("L%i"), i + 1));
          dc.DrawLine(xPositionS, yPositionLow, xPositionS, yPositionHigh + overlap * (extent.GetHeight() + 5));
          dc.DrawRectangle(xPositionS, yPositionHigh + overlap * (extent.GetHeight() + 5), extent.GetWidth() + 2, extent.GetHeight());
          dc.DrawText(wxString::Format(wxT("L%i"), i + 1), xPositionS + 1, yPositionHigh + overlap * (extent.GetHeight() + 5));

          // the loop end value (in samples) is in loopPositions[i].second
          int xPositionE = loopPositions[i].second / samplesPerPixel + leftMargin;
          dc.DrawLine(xPositionE, yPositionLow, xPositionE, yPositionHigh + overlap * (extent.GetHeight() + 5));
          dc.DrawRectangle(xPositionE - (extent.GetWidth() + 1), yPositionHigh + overlap * (extent.GetHeight() + 5), extent.GetWidth() + 2, extent.GetHeight());
          dc.DrawText(wxString::Format(wxT("L%i"), i + 1), xPositionE - (extent.GetWidth() + 1), yPositionHigh + overlap * (extent.GetHeight() + 5));

          // draw line at top connecting loop start and end lines
          dc.DrawLine(xPositionS, yPositionHigh + overlap * (extent.GetHeight() + 5), xPositionE, yPositionHigh + overlap * (extent.GetHeight() + 5));
        }
      }
      
      // draw time indicating lines at bottom
      dc.SetPen(wxPen(black, 1, wxPENSTYLE_SOLID));
      // starting at zero
      dc.DrawLine(leftMargin, size.y - 11, leftMargin, size.y - 1);
      dc.SetFont(wxFont(6, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_LIGHT));
      wxSize extent = dc.GetTextExtent(wxT("0"));
      dc.DrawText(wxT("0"), leftMargin + 2, size.y - extent.GetHeight());
      int lastLineX = 0;
      int lineNbr = 1;
      for (int i = 0; i < nrOfSamples; i++) {
        // only care if the sample make at least 0.25s
        if ((i * 4) % m_fileReference->GetSampleRate() == 0) {
          // make sure at least 25 pixels are passed since last line
          int xCoordinate = i / samplesPerPixel;
          if (xCoordinate - lastLineX >= 25) {
            if (lineNbr % 2 == 0) {
              // at this line we also write a number
              dc.DrawLine(leftMargin + xCoordinate, size.y - 11, leftMargin + xCoordinate, size.y - 1);
              wxString timeString = wxString::Format(wxT("%.1f"), ((double) i / (double) m_fileReference->GetSampleRate()));
              wxSize extent = dc.GetTextExtent(timeString);
              dc.DrawText(timeString, leftMargin + xCoordinate + 2, size.y - extent.GetHeight());
            } else {
              dc.DrawLine(leftMargin + xCoordinate, size.y - 11, leftMargin + xCoordinate, size.y - 5);
            }
            lineNbr++;
            lastLineX = xCoordinate;
          }
        }
      }
      // draw transparent rectangle that indicate current sustainsection in the file
      CalculateSustainIndication();
      if (mouseWithinSustainSection) {
        DrawSustainIndication(dc);
        DrawSustainSectionRectOutline();
      } else
        DrawSustainIndication(dc);
    }
    // draw the indicator for the playposition
    dc.DrawIcon(playPositionMarker, playPosition, 1);
    somethingHasChanged = false;
    m_overlay.Reset();
  } else {
    // the panel is not resized so the waveform doesn't need redrawing but the playposition should be redrawn
    dc.SetClippingRegion(0, 0, leftMargin + trackWidth + rightMargin, 9);
    dc.Clear();
    dc.SetBrush(wxBrush(white));
    dc.SetPen(wxPen(black, 1, wxPENSTYLE_SOLID));

    // draw playposition rectangle
    dc.DrawRectangle(leftMargin, 0, trackWidth, 10);
 
    // draw the indicator for the playposition 
    dc.DrawIcon(playPositionMarker, playPosition, 1);
  }
}

void WaveformDrawer::SetPlayPosition(unsigned int pPos) {
  // In comes a sample value and the playPosition is calculated in pixels from (leftMargin - 4) to (trackWidth - 4)
  int nrOfSamples = m_fileReference->waveTracks[0].waveData.size();
  int samplesPerPixel;

  if (trackWidth > 0) {
    if (nrOfSamples % trackWidth == 0)
      samplesPerPixel = nrOfSamples / trackWidth;
    else
      samplesPerPixel = (nrOfSamples / trackWidth) + 1;
    playPosition = pPos / samplesPerPixel + (leftMargin - 4);
  } else {
    playPosition = leftMargin - 4;
  }
}

WaveformDrawer::~WaveformDrawer() {
}

void WaveformDrawer::AddCuePosition(unsigned int cuePos) {
  cueSampleOffset.push_back(cuePos);
  somethingHasChanged = true;
}

void WaveformDrawer::AddLoopPosition(unsigned int startPos, unsigned int endPos) {
  m_overlay.Reset();
  loopPositions.push_back(std::make_pair(startPos, endPos));
  somethingHasChanged = true;
}

void WaveformDrawer::ClearMetadata() {
  cueSampleOffset.clear();
  loopPositions.clear();
  somethingHasChanged = true;
}

void WaveformDrawer::CalculateLayout() {
  // This function will check if loops and markers will collide and store in what "row" they should be placed

  // first we empty the vectors storing the layout information
  loopLayout.clear();
  cueLayout.clear();

  // then we deal with the loops
  for (unsigned i = 0; i < loopPositions.size(); i++) {
    // add a LOOPLAYOUT item to loopLayout vector
    LOOPLAYOUT item;
    loopLayout.push_back(item);
    // let's check if loop will overlap any previous and which
    // and preliminary set placedInRow to 0
    loopLayout[i].placedInRow = 0;
    for (unsigned j = 0; j < i; j++) {
      bool isOverlapping = false;
      if (loopPositions[i].first >= loopPositions[j].first && loopPositions[i].first <= loopPositions[j].second)
        isOverlapping = true;
      else if (loopPositions[i].second >= loopPositions[j].first && loopPositions[i].second <= loopPositions[j].second)
        isOverlapping = true;
      else if (loopPositions[i].first <= loopPositions[j].first && loopPositions[i].second >= loopPositions[j].second)
        isOverlapping = true;

      if (isOverlapping) {
        // if it's overlapping then we add that loop to the loopLayout[i].overlappingLoops vector
        loopLayout[i].overlappingLoops.push_back(j);
      }
    }
  }
  // now all placedInRow is 0 and we know what loops overlaps which others
  // IF the loopLayout[i].overlappingLoops vector not is empty then placedInRow might need correction
  for (unsigned i = 0; i < loopLayout.size(); i++) {
    // loopLayout[i].overlappingLoops[some valid index] contains the index of one overlapping loop
    // IF not the vector is empty...
    // IF last overlapping loop is in row 0 THEN this must be in row 1 (if there's only one)
    // ELSE we start at row 0 and check all the overlapping loops for a match
    // IF so THEN we continue with next row until we find one that's not matching (is unused)
    if (loopLayout[i].overlappingLoops.empty() == false) {
      if (loopLayout[loopLayout[i].overlappingLoops.back()].placedInRow == 0 && 
          loopLayout[i].overlappingLoops.size() == 1)
        loopLayout[i].placedInRow = 1;
      else {
        int unUsedRow = 0;
        for (unsigned j = 0; j < loopLayout[i].overlappingLoops.size(); j++) {
          bool alreadyTaken = false;
          if (loopLayout[loopLayout[i].overlappingLoops[j]].placedInRow == unUsedRow) {
            alreadyTaken = true;
          }

          // we should check if any of the overlapped loops already got this
          // row, in which case we must continue looking for an unused one
          for (unsigned k = 0; k < loopLayout[i].overlappingLoops.size(); k++) {
            if (loopLayout[loopLayout[i].overlappingLoops[k]].placedInRow ==
                unUsedRow) {
              alreadyTaken = true;
            }
          }

          if (alreadyTaken) {
            unUsedRow++;
            continue;
          } else {
            break;
          }
        } 
        loopLayout[i].placedInRow = unUsedRow;
      }
    }
  }

  // And now it's the cue markers turn but first we get the samplesPerPixel value
  int nrOfSamples = m_fileReference->waveTracks[0].waveData.size();
  int samplesPerPixel;
  int equalTo24px;

  if (nrOfSamples % trackWidth == 0)
    samplesPerPixel = nrOfSamples / trackWidth;
  else
    samplesPerPixel = (nrOfSamples / trackWidth) + 1;

  equalTo24px = 24 * samplesPerPixel;

  for (unsigned i = 0; i < cueSampleOffset.size(); i++) {
    // Add a CUELAYOUT item to the cueLayout vector
    CUELAYOUT item;
    cueLayout.push_back(item);

    cueLayout[i].putInRow = 0;
    // with the cues we check if the marker is within any loop and if so treat as with the loops
    // but due to the possibility of marker and loop start flags overlapping we'll add samples to
    // the start equal to 24 pixels
    for (unsigned j = 0; j < loopPositions.size(); j++) {
      bool isOverlapping = false;
      if (cueSampleOffset[i] > (loopPositions[j].first - equalTo24px) && cueSampleOffset[i] < loopPositions[j].second)
        isOverlapping = true;
     
      if (isOverlapping) {
        // if it's overlapping then we add that loop index to the cueLayout[i].withinLoop vector
        cueLayout[i].withinLoop.push_back(j);
      }
    }
    // and additionally we check if any previous marker is within 25 pixels around it
    for (unsigned j = 0; j < i; j++) {
      int differenceInSamples;
      if (cueSampleOffset[i] > cueSampleOffset[j])
        differenceInSamples = cueSampleOffset[i] - cueSampleOffset[j];
      else
        differenceInSamples = cueSampleOffset[j] - cueSampleOffset[i];

      if (differenceInSamples / samplesPerPixel < 25)
        cueLayout[i].markerClose.push_back(j);
    }
  }
  // so now we know what loops the marker is within and what other cues could be close
  for (unsigned i = 0; i < cueLayout.size(); i++) {
    // cueLayout[i].withinLoop[some valid index] contains the index of one overlapping loop
    // IF not the vector is empty...
    // IF last overlapping loop is in row 0 THEN this must be in row 1
    // ELSE we start at row 0 and check all the overlapping loops for a match
    // IF so THEN we continue with next row until we find one that's not matching (is unused)
    if (cueLayout[i].withinLoop.empty() == false) {
      if (loopLayout[cueLayout[i].withinLoop.back()].placedInRow == 0)
        cueLayout[i].putInRow = 1;
      else {
        int unUsedRow = 0;
        for (unsigned j = 0; j < cueLayout[i].withinLoop.size(); j++) {
          if (loopLayout[cueLayout[i].withinLoop[j]].placedInRow == unUsedRow) {
            unUsedRow++;
          } else {
            if (cueLayout[i].markerClose.empty())
              break;
            else {
              for (unsigned k = 0; k < cueLayout[i].markerClose.size(); k++) {
                if (cueLayout[cueLayout[i].markerClose[k]].putInRow == unUsedRow)
                  unUsedRow++;
              }
            }
          }
        }
        cueLayout[i].putInRow = unUsedRow;
      }
    } else {
      // not within any loop but maybe close to another marker
      if (cueLayout[i].markerClose.empty() == false) {
        int unUsedRow = 0;

        for (unsigned k = 0; k < cueLayout[i].markerClose.size(); k++) {
          if (cueLayout[cueLayout[i].markerClose[k]].putInRow == unUsedRow)
            unUsedRow++;
        }
        cueLayout[i].putInRow = unUsedRow;
      }
    }
  }
}

void WaveformDrawer::OnLeftClick(wxMouseEvent& event) {
  m_x = event.GetX(); 
  m_y = event.GetY();

  if (m_x > leftMargin && m_x < (leftMargin + trackWidth) && m_y > topMargin && m_y <= (topMargin + trackHeight * m_fileReference->m_channels + marginBetweenTracks * m_fileReference->m_channels)) {
    // user have clicked on the track area
    int nrOfSamples = m_fileReference->waveTracks[0].waveData.size();
    int samplesPerPixel;

    if (nrOfSamples % trackWidth == 0)
      samplesPerPixel = nrOfSamples / trackWidth;
    else
      samplesPerPixel = (nrOfSamples / trackWidth) + 1;

    if (mouseWithinSustainSection) {
      if (!isChangingSustainSection) {
        m_prev_x = m_x;
        m_prev_y = m_y;

        m_old_sustainsection_rect.yPosHigh = m_sustainsection_rect.yPosHigh;
        m_old_sustainsection_rect.yExtent = m_sustainsection_rect.yExtent;
        m_old_sustainsection_rect.xPosLeft = m_sustainsection_rect.xPosLeft;
        m_old_sustainsection_rect.xExtent = m_sustainsection_rect.xExtent;
      }
      if (withinLeftChangeBorder || withinRightChangeBorder) {
        isChangingSustainSection = true;
      }
    }

    if (cueIsSelected) {
      // a cue was selected so we'll change it's dwSampleOffset to a new position
      // first check that the pixels are valid
      if (m_x < leftMargin)
        m_x = leftMargin;
      if (m_x > leftMargin + trackWidth)
        m_x = leftMargin + trackWidth;

      int approximateSampleNumber = samplesPerPixel * (m_x - (leftMargin + 1));
      int earliestSampleToConsider = approximateSampleNumber - samplesPerPixel;
      unsigned lastSampleToConsider = approximateSampleNumber + samplesPerPixel;

      if (earliestSampleToConsider < 0)
        earliestSampleToConsider = 0;

      if (lastSampleToConsider > m_fileReference->waveTracks[0].waveData.size())
        lastSampleToConsider = m_fileReference->waveTracks[0].waveData.size() - 1;

      unsigned int bestSample = 0;
      double lowestRMSPower = DBL_MAX;
      double currentRMSPower = 0;
      // the sample values are in waveTracks[0].waveData
      for (unsigned i = earliestSampleToConsider; i <= lastSampleToConsider; i++) {
        for (unsigned j = 0; j < m_fileReference->waveTracks.size(); j++)
          currentRMSPower += pow(m_fileReference->waveTracks[j].waveData[i], 2);

        if (currentRMSPower < lowestRMSPower) {
          lowestRMSPower = currentRMSPower;
          bestSample = i;
        }
        currentRMSPower = 0;
      }

      cueSampleOffset[selectedCueIndex] = bestSample; // change the cues position in this class
      ::wxGetApp().frame->ChangeCuePosition(bestSample, selectedCueIndex); // send offset value for changed cue

      cueIsSelected = false;
      ::wxGetApp().frame->SetStatusText(wxT("Ready"), 0);
      return;
    }

    if (!cueIsSelected) {
      // check if click is in a cue flag and if so select that cue for re-positioning
      bool inCue = false;
      for (unsigned i = 0; i < cueSampleOffset.size(); i++) {
        if (m_x > cueLayout[i].flagUpLeft.first && m_x <= cueLayout[i].flagDownRight.first && 
            m_y > cueLayout[i].flagUpLeft.second && m_y <= cueLayout[i].flagDownRight.second) {
          inCue = true;
          selectedCueIndex = i;
          break;
        }
      } 

      if (inCue) {
        cueIsSelected = true;
        wxClientDC dc(this);
        dc.SetBrush(wxBrush(white, wxBRUSHSTYLE_BDIAGONAL_HATCH));
        dc.SetPen(wxPen(green, 1, wxPENSTYLE_SOLID));
        dc.DrawRectangle(cueLayout[selectedCueIndex].flagUpLeft.first, cueLayout[selectedCueIndex].flagUpLeft.second,
                         cueLayout[selectedCueIndex].flagDownRight.first - cueLayout[selectedCueIndex].flagUpLeft.first,
                         cueLayout[selectedCueIndex].flagDownRight.second - cueLayout[selectedCueIndex].flagUpLeft.second);

        ::wxGetApp().frame->SetStatusText(wxT("Move cue with next left click!"), 0);
      }
    }

  } else {
    cueIsSelected = false;
    ::wxGetApp().frame->SetStatusText(wxT("Ready"), 0);
    Refresh();
    Update();
  }
}

void WaveformDrawer::OnLeftRelease(wxMouseEvent& WXUNUSED(event)) {
  if (!m_fileReference->GetAutoSustainSearch()) {
    if (isChangingSustainSection) {
      // we must update the sustain section in the file reference as it could have changed
      // the values to send is in percentage of track size
      isChangingSustainSection = false;
      CalculateSustainRectZones();
      int start = ((double) (m_sustainsection_rect.xPosLeft - leftMargin + 1) / trackWidth) * 1000 + 0.5;
      int end = ((double) (m_sustainsection_rect.xPosLeft + m_sustainsection_rect.xExtent - leftMargin + 2) / trackWidth) * 1000 + 0.5;
      m_fileReference->SetSliderSustainsection(start, end);

      // also the auto loopsearch parameters must be updated
      ::wxGetApp().frame->UpdateAutoloopSliderSustainsection(start, end);

      // update display
      m_overlay.Reset();
      Update();
      Refresh();
    }
  }
}

void WaveformDrawer::OnRightClick(wxMouseEvent& event) {
  m_x = event.GetX(); 
  m_y = event.GetY();

  if (m_x > leftMargin && m_x < (leftMargin + trackWidth) && m_y > topMargin && m_y <= (topMargin + trackHeight * m_fileReference->m_channels + marginBetweenTracks * m_fileReference->m_channels)) {
    // user have rightclicked on the track area
    if (cueIsSelected) {
      cueIsSelected = false;
      ::wxGetApp().frame->SetStatusText(wxT("Ready"), 0);
      Refresh();
      Update();
    }
    // draw an indication line approximately where cue will be inserted
    wxClientDC dc(this);
    int yPositionHigh = topMargin + 1;
    int yPositionLow = topMargin + trackHeight * m_fileReference->waveTracks.size() + (marginBetweenTracks * (m_fileReference->waveTracks.size() - 1) - 1);
    dc.SetPen(wxPen(green, 1, wxPENSTYLE_DOT));
    dc.DrawLine(m_x, yPositionLow, m_x, yPositionHigh);

    PopupMenu(m_popupMenu, event.GetPosition());

    Refresh();
    Update();
  }
}

void WaveformDrawer::OnMouseMotion(wxMouseEvent& WXUNUSED(event)) {
  if (!m_fileReference->GetAutoSustainSearch()) {
    wxPoint pt = wxGetMousePosition();
    int mouseX = pt.x - this->GetScreenPosition().x;
    int mouseY = pt.y - this->GetScreenPosition().y;

    // check if mouse is within sustainsection
    if ((mouseX >= m_sustainsection_rect.xPosLeft && mouseX <= (m_sustainsection_rect.xPosLeft + m_sustainsection_rect.xExtent) && mouseY >= m_sustainsection_rect.yPosHigh && mouseY <= (m_sustainsection_rect.yPosHigh + m_sustainsection_rect.yExtent)) || isChangingSustainSection) { 
      mouseWithinSustainSection = true;

      if (!isChangingSustainSection) {
        if (mouseX <= m_leftBorderX) {
          // within left sustainsection range
          if (!withinLeftChangeBorder)
            outlineAlreadyDrawn = false;
          withinLeftChangeBorder = true;
          withinRightChangeBorder = false;
          ::wxGetApp().frame->SetStatusText(wxT("Click/drag to change start!"), 0);
        } else if (mouseX >= m_rightBorderX) {
          // within right sustainsection range
          if (!withinRightChangeBorder)
            outlineAlreadyDrawn = false;
          withinLeftChangeBorder = false;
          withinRightChangeBorder = true;
          ::wxGetApp().frame->SetStatusText(wxT("Click/drag to change end!"), 0);
        } else {
          // in the middle part of sustainsection
          ::wxGetApp().frame->SetStatusText(wxT("Sustainsection can be modified!"), 0);
          if (withinLeftChangeBorder || withinRightChangeBorder)
            outlineAlreadyDrawn = false;
          withinLeftChangeBorder = false;
          withinRightChangeBorder = false;
        }
      }

      if (isChangingSustainSection) {
        if (withinLeftChangeBorder) {
          if (mouseX < m_prev_x && m_sustainsection_rect.xPosLeft > leftMargin) {
            m_sustainsection_rect.xExtent += (m_prev_x - mouseX);
            m_sustainsection_rect.xPosLeft -= (m_prev_x - mouseX);
            outlineHasChanged = true;
          } else if (mouseX > m_prev_x && ((double) m_sustainsection_rect.xExtent / trackWidth * 100 + 0.5 > 1.0f)) {
            if ((m_sustainsection_rect.xExtent - (mouseX - m_prev_x)) / (double) trackWidth * 100 + 0.5 > 1.0f) {
              m_sustainsection_rect.xExtent -= (mouseX - m_prev_x);
              m_sustainsection_rect.xPosLeft += (mouseX - m_prev_x);
              outlineHasChanged = true;
            } else {
              outlineHasChanged = false;
            }
          } else {
            outlineHasChanged = false;
          }
          m_prev_x = mouseX;
        } else if (withinRightChangeBorder) {
          if (mouseX < m_prev_x && (m_sustainsection_rect.xExtent / (double) trackWidth * 100 > 1.0f)) {
            if ((m_sustainsection_rect.xExtent - (m_prev_x - mouseX)) / (double) trackWidth * 100 > 1.0f) {
              m_sustainsection_rect.xExtent -= (m_prev_x - mouseX);
              outlineHasChanged = true;
            } else {
              outlineHasChanged = false;
            }
          } else if (mouseX > m_prev_x && (m_sustainsection_rect.xPosLeft + m_sustainsection_rect.xExtent) < (leftMargin + trackWidth)) {
            m_sustainsection_rect.xExtent += (mouseX - m_prev_x);
            outlineHasChanged = true;
          } else {
            outlineHasChanged = false;
          }
          m_prev_x = mouseX;
        }
        if (outlineHasChanged) {
          CalculateSustainRectZones();
          DrawSustainSectionRectOutline();
        }
      }
      if (!outlineAlreadyDrawn) {
        DrawSustainSectionRectOutline();
        outlineAlreadyDrawn = true;
      }
    } else {
      ::wxGetApp().frame->SetStatusText(wxT("Ready"), 0);
      mouseWithinSustainSection = false;
      if (outlineAlreadyDrawn) {
        // Undo the rectangle indication sustainsection selection
        Update();
        Refresh();
      }
      outlineAlreadyDrawn = false;
    }
  }
}

void WaveformDrawer::OnMouseLeave(wxMouseEvent& WXUNUSED(event)) {
  if (!m_fileReference->GetAutoSustainSearch()) {
    mouseWithinSustainSection = false;
    withinLeftChangeBorder = false;
    withinRightChangeBorder = false;
    if (isChangingSustainSection) {
      // we must reset to old sustainsection
      m_sustainsection_rect.yPosHigh = m_old_sustainsection_rect.yPosHigh;
      m_sustainsection_rect.yExtent = m_old_sustainsection_rect.yExtent;
      m_sustainsection_rect.xPosLeft = m_old_sustainsection_rect.xPosLeft;
      m_sustainsection_rect.xExtent = m_old_sustainsection_rect.xExtent;    
    }
    isChangingSustainSection = false;
    if (outlineAlreadyDrawn) {
      // Undo the rectangle indication sustainsection selection
      Update();
      Refresh();
      ::wxGetApp().frame->SetStatusText(wxT("Ready"), 0);
    }
    outlineAlreadyDrawn = false;
  }
}

void WaveformDrawer::OnMouseEnter(wxMouseEvent& WXUNUSED(event)) {
}

void WaveformDrawer::DrawSustainSectionRectOutline() {
  // draw a rectangle indication sustainsection selection
  wxClientDC dc(this);
  wxDCOverlay overlaydc(m_overlay, &dc);
  overlaydc.Clear();
  if (isChangingSustainSection)
    DrawSustainIndication(dc);
  dc.SetBrush(*wxTRANSPARENT_BRUSH);
  dc.SetPen(wxPen(yellow, 1, wxPENSTYLE_SOLID));
  dc.DrawRectangle(m_sustainsection_rect.xPosLeft, m_sustainsection_rect.yPosHigh, m_sustainsection_rect.xExtent, m_sustainsection_rect.yExtent);
  if (withinLeftChangeBorder) {
    dc.DrawLine(m_leftBorderX, m_sustainsection_rect.yPosHigh + 1, m_leftBorderX, m_sustainsection_rect.yPosHigh + m_sustainsection_rect.yExtent - 1); 	
  } else if (withinRightChangeBorder) {
    dc.DrawLine(m_rightBorderX, m_sustainsection_rect.yPosHigh + 1, m_rightBorderX, m_sustainsection_rect.yPosHigh + m_sustainsection_rect.yExtent - 1);
  }
}

void WaveformDrawer::CalculateSustainRectZones() {
  if (m_sustainsection_rect.xExtent > 4 && m_sustainsection_rect.xExtent < 81) {
    m_leftBorderX = m_sustainsection_rect.xPosLeft + m_sustainsection_rect.xExtent / 4;
    m_rightBorderX = m_sustainsection_rect.xPosLeft + m_sustainsection_rect.xExtent * 0.75;
  } else if (m_sustainsection_rect.xExtent > 80) {
    m_leftBorderX = m_sustainsection_rect.xPosLeft + 20;
    m_rightBorderX = m_sustainsection_rect.xPosLeft + m_sustainsection_rect.xExtent - 20;
  } else {
    // sustain section is very small on the screen
    m_leftBorderX = m_sustainsection_rect.xPosLeft;
    m_rightBorderX = m_sustainsection_rect.xPosLeft + m_sustainsection_rect.xExtent - 1;
  }
}

void WaveformDrawer::DrawSustainIndication(wxDC &dc) {
  if (m_sustainsection_rect.xExtent > 0) {
    wxImage sustainImage(m_sustainsection_rect.xExtent, m_sustainsection_rect.yExtent);
    sustainImage.Clear(211);
    sustainImage.InitAlpha();
    if (sustainImage.HasAlpha()) {
      for (int i = 0; i < m_sustainsection_rect.xExtent; i++) {
        for (int j = 0; j < m_sustainsection_rect.yExtent; j++) {
          sustainImage.SetAlpha(i, j, 128);
        }
      }
    }
    wxBitmap bmp( sustainImage );
    dc.DrawBitmap( bmp, m_sustainsection_rect.xPosLeft, m_sustainsection_rect.yPosHigh, true );
  }
}

void WaveformDrawer::CalculateSustainIndication() {
  int nrOfSamples = m_fileReference->waveTracks[0].waveData.size();
  int samplesPerPixel;
    
  if (nrOfSamples % trackWidth == 0)
    samplesPerPixel = nrOfSamples / trackWidth;
  else
    samplesPerPixel = (nrOfSamples / trackWidth) + 1;
  std::pair<unsigned, unsigned> currentSustain = m_fileReference->GetSustainsection();
  int yPosHigh = topMargin + 1;
  int yExtent = topMargin + trackHeight * m_fileReference->waveTracks.size() + (marginBetweenTracks * (m_fileReference->waveTracks.size() - 1) - 1) - yPosHigh;
  int xPosLeft = currentSustain.first / samplesPerPixel + leftMargin;
  int xExtent = ((currentSustain.second - currentSustain.first + 1) / samplesPerPixel) + 1.5;
  m_sustainsection_rect.yPosHigh = yPosHigh;
  m_sustainsection_rect.yExtent = yExtent;
  m_sustainsection_rect.xPosLeft = xPosLeft;
  m_sustainsection_rect.xExtent = xExtent;
  CalculateSustainRectZones();
}

void WaveformDrawer::OnClickAddCue(wxCommandEvent& WXUNUSED(event)) {
  // we should now calculate what sample have lowest RMS power around current position
  // so that a good dwSampleOffset value can be sent to the new cue
  int nrOfSamples = m_fileReference->waveTracks[0].waveData.size();
  int samplesPerPixel;

  if (nrOfSamples % trackWidth == 0)
    samplesPerPixel = nrOfSamples / trackWidth;
  else
    samplesPerPixel = (nrOfSamples / trackWidth) + 1;

  int approximateSampleNumber = samplesPerPixel * (m_x - (leftMargin + 1));
  int earliestSampleToConsider = approximateSampleNumber - samplesPerPixel;
  unsigned lastSampleToConsider = approximateSampleNumber + samplesPerPixel;

  if (earliestSampleToConsider < 0)
    earliestSampleToConsider = 0;

  if (lastSampleToConsider > m_fileReference->waveTracks[0].waveData.size())
    lastSampleToConsider = m_fileReference->waveTracks[0].waveData.size() - 1;

  unsigned int bestSample = 0;
  double lowestRMSPower = DBL_MAX;
  double currentRMSPower = 0;
  // the sample values are in waveTracks[0].waveData
  for (unsigned i = earliestSampleToConsider; i <= lastSampleToConsider; i++) {
    for (unsigned j = 0; j < m_fileReference->waveTracks.size(); j++)
      currentRMSPower += pow(m_fileReference->waveTracks[j].waveData[i], 2);

    if (currentRMSPower < lowestRMSPower) {
      lowestRMSPower = currentRMSPower;
      bestSample = i;
    }
    currentRMSPower = 0;
  }

  ::wxGetApp().frame->AddNewCue(bestSample); // send offset value for the new cue creation
}

void WaveformDrawer::ChangeLoopPositions(unsigned int start, unsigned int end, int idx) {
  loopPositions[idx].first = start;
  loopPositions[idx].second = end;
  somethingHasChanged = true;
}

int WaveformDrawer::GetAmplitudeZoomLevel() {
  return m_amplitudeZoomLevel;
}

void WaveformDrawer::ZoomInAmplitude() {
  if (m_amplitudeZoomLevel < 1024)
    m_amplitudeZoomLevel *= 2;

  somethingHasChanged = true;
}

void WaveformDrawer::ZoomOutAmplitude() {
  if (m_amplitudeZoomLevel > 1)
    m_amplitudeZoomLevel /= 2;

  somethingHasChanged = true;
}

void WaveformDrawer::AutoCalculateZoomLevel() {
  double strongestSampleValue = m_fileReference->GetStrongestSampleValue();
  if (strongestSampleValue < 0.5 && strongestSampleValue > 0) {
    double zoomLvl = 0.5 / strongestSampleValue;
    int roundedUpZoom = ceil(zoomLvl);
    while (m_amplitudeZoomLevel < roundedUpZoom) {
      m_amplitudeZoomLevel *= 2;
    }
  }
}

void WaveformDrawer::OnKeyDown(wxKeyEvent& event) {
  MyFrame *myParent = (MyFrame *) GetParent();
  myParent->OnKeyboardInput(event);
}

void WaveformDrawer::SetLoopSelection(int idx) {
  if (idx < 0) {
    loopIndexSelection = -1;
    hasLoopSelection = false;
  } else {
    loopIndexSelection = idx;
    hasLoopSelection = true;
    cueIndexSelection = -1;
    hasCueSelection = false;
  }
  somethingHasChanged = true;
}

void WaveformDrawer::SetCueSelection(int idx) {
  if (idx < 0) {
    cueIndexSelection = -1;
    hasCueSelection = false;
  } else {
    loopIndexSelection = -1;
    hasLoopSelection = false;
    cueIndexSelection = idx;
    hasCueSelection = true;  
  }
  somethingHasChanged = true;
}
