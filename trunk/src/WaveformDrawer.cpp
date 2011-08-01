/* 
 * WaveformDrawer draws the waveform from an audio file
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

#include "WaveformDrawer.h"
#include "sndfile.hh"
#include "icons/PlayPositionMarker.xpm"
#include <cmath>
#include <cfloat>
#include "LoopAuditioneer.h"
#include "LoopAuditioneerDef.h"

BEGIN_EVENT_TABLE(WaveformDrawer, wxPanel)
  EVT_PAINT(WaveformDrawer::paintEvent)
  EVT_RIGHT_DOWN(WaveformDrawer::OnRightClick)
  EVT_MENU(ADD_CUE, WaveformDrawer::OnClickAddCue)
  EVT_LEFT_DOWN(WaveformDrawer::OnLeftClick)
END_EVENT_TABLE()

WaveformDrawer::WaveformDrawer(wxFrame *parent, wxString fileName) : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE), m_buffer(NULL) {
  CouldOpenFile = false;
  somethingHasChanged = false;
  white.Set(wxT("#ffffff"));
  black.Set(wxT("#000000"));
  blue.Set(wxT("#0d0060"));
  green.Set(wxT("#00C800"));
  red.Set(wxT("#f90000"));
  xSize = 1;
  ySize = 1;
  topMargin = 10;
  bottomMargin = 0;
  marginBetweenTracks = 0;
  leftMargin = 30;
  rightMargin = 10;
  trackWidth = 0;
  trackHeight = 0;
  playPosition = 0;
  playPositionMarker = wxIcon(PlayPositionMarker_xpm);
  m_background.Set(wxT("#f4f2ef"));
  SetBackgroundColour(m_background);
  selectedCueIndex = 0;
  cueIsSelected = false;
 
  SndfileHandle sfHandle;

  sfHandle = SndfileHandle(((const char*)fileName.mb_str()));

  if (sfHandle) {
    nrChannels = sfHandle.channels();

    for (int i = 0; i < nrChannels; i++) {
      WAVETRACK wt;
      waveTracks.push_back(wt);
    }

    m_buffer = new double[sfHandle.frames() * sfHandle.channels()];
    sfHandle.read(m_buffer, sfHandle.frames() * sfHandle.channels());

    int index = 0;
    for (int i = 0; i < sfHandle.frames() * sfHandle.channels(); i++) {
      // de-interleaving
      waveTracks[index].waveData.push_back(m_buffer[i]);
      index++;

      if (index == nrChannels)
        index = 0;
    }
    CouldOpenFile = true;
  }
  delete[] m_buffer;

  // create the popup menu for the waveform
  m_popupMenu = new wxMenu();
  m_popupMenu->Append(ADD_CUE, wxT("&Add cue"), wxT("Create a new cue at this position"));
}

// Called when the panel needs to be redrawn (if the panel is resized)
void WaveformDrawer::paintEvent(wxPaintEvent & evt) {
    wxPaintDC dc(this);
    OnPaint(dc);
}
 
// Method to call when one wants to force redrawing for playback
void WaveformDrawer::paintNow() {
    wxClientDC dc(this);
    OnPaintPlayPosition(dc);
}

// Here the actual drawing happens when either the panel is resized or something changes
void WaveformDrawer::OnPaint(wxDC& dc) {
  bool redrawCompletely = false;

  // First get the size of this panel to know if the panel is resized.
  wxSize size = this->GetSize();
  trackWidth = size.x - (leftMargin + rightMargin);
  trackHeight = (size.y - (topMargin + bottomMargin + ((nrChannels - 1) * marginBetweenTracks))) / nrChannels;

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
    dc.SetPen(wxPen(black, 1, wxSOLID));

    // draw playposition rectangle
    dc.DrawRectangle(leftMargin, 0, trackWidth, 10);

    // draw the track containing rectangles
    if (nrChannels > 0) {
      for (int i = 0; i < nrChannels; i++) {
        int x1, y1, x2, y2;
        x1 = leftMargin;
        y1 = topMargin + trackHeight * i + i * marginBetweenTracks; // top margin + trackheight * tracknumber + margin between tracks
        x2 = trackWidth;
        y2 = trackHeight;
        dc.DrawRectangle(x1, y1, x2, y2);
        dc.DrawText(wxT("1"), 12, topMargin + trackHeight * i + i * marginBetweenTracks);
        dc.DrawText(wxT("0"), 12, topMargin - 8 + trackHeight * i + i * marginBetweenTracks + (trackHeight / 2));
        dc.DrawText(wxT("-1"), 9, topMargin - 10 + trackHeight * i + i * marginBetweenTracks + trackHeight - 10);
      }
    }

    if (waveTracks[0].waveData.size() > 0) {
      int nrOfSamples = waveTracks[0].waveData.size();
      int samplesPerPixel;
    
      if (nrOfSamples % trackWidth == 0)
        samplesPerPixel = nrOfSamples / trackWidth;
      else
        samplesPerPixel = (nrOfSamples / trackWidth) + 1;
    
      double maxValue = 0, minValue = 0;
      int lineToDraw = 0;
      for (int j = 0; j < waveTracks.size(); j++) {
        for (int i = 0; i < waveTracks[0].waveData.size(); i++) {
          if (i % samplesPerPixel == 0 && i > 0) {
            // we should write the line representing the audio data and start a new count
            wxCoord x1 = leftMargin + lineToDraw, y1 = topMargin + trackHeight * j + j * marginBetweenTracks + (trackHeight / 2) - (maxValue * trackHeight / 2);
            wxCoord x2 = leftMargin + lineToDraw, y2 = topMargin + trackHeight * j + j * marginBetweenTracks + (trackHeight / 2) - (minValue * trackHeight / 2);

            dc.SetPen(wxPen(blue, 1, wxSOLID));
            dc.DrawLine(x1, y1, x2, y2);

            maxValue = waveTracks[j].waveData[i];
            minValue = waveTracks[j].waveData[i];
            lineToDraw++;
          } else {
            if (waveTracks[j].waveData[i] > maxValue)
              maxValue = waveTracks[j].waveData[i];
            else if (waveTracks[j].waveData[i] < minValue)
              minValue = waveTracks[j].waveData[i];
          }
        }
        // draw the 0 indicating line
        dc.SetPen(wxPen(blue, 1, wxSOLID));
        dc.DrawLine((leftMargin + 1), topMargin + trackHeight * j + j * marginBetweenTracks + (trackHeight / 2), size.x - (rightMargin + 1), topMargin + trackHeight * j + j * marginBetweenTracks + (trackHeight / 2)); 

        // reset the wave pixel position for the next track
        lineToDraw = 0;
      }
      // draw in eventual metadata (loops and cues)
      if (cueSampleOffset.size() > 0) {
        int overlap = 0;
        // here we draw the cues from the vector
        for (int i = 0; i < cueSampleOffset.size(); i++) {
          overlap = cueLayout[i].putInRow;
          // the positions from dwSampleOffset is in sample frames so it has to be re-calculated into pixels
          int xPosition = cueSampleOffset[i] / samplesPerPixel + leftMargin;
          int yPositionHigh = topMargin + 1;
          int yPositionLow = topMargin + trackHeight * waveTracks.size() + (marginBetweenTracks * (waveTracks.size() - 1) - 1);
          dc.SetPen(wxPen(green, 1, wxDOT));
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
        int yPositionLow = topMargin + trackHeight * waveTracks.size() + (marginBetweenTracks * (waveTracks.size() - 1) - 1);

        for (int i = 0; i < loopPositions.size(); i++) {
          // the loop start value (in samples) is in loopPositions[i].first
          int xPositionS = loopPositions[i].first / samplesPerPixel + leftMargin;

          overlap = loopLayout[i].placedInRow;

          dc.SetPen(wxPen(red, 1, wxDOT_DASH));
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
    }
    // draw the indicator for the playposition
    dc.DrawIcon(playPositionMarker, playPosition, 1);
    somethingHasChanged = false;
  } else {
    // the panel is not resized so the waveform doesn't need redrawing but the playposition should be redrawn
    dc.SetBackground(m_background);
    dc.SetClippingRegion(0, 0, leftMargin + trackWidth + rightMargin, 9);
    dc.Clear();
    dc.SetBrush(wxBrush(white));
    dc.SetPen(wxPen(black, 1, wxSOLID));

    // draw playposition rectangle
    dc.DrawRectangle(leftMargin, 0, trackWidth, 10);
 
    // draw the indicator for the playposition 
    dc.DrawIcon(playPositionMarker, playPosition, 1);
  }
}

void WaveformDrawer::OnPaintPlayPosition(wxDC& dc) {
  // the playposition should be redrawn during playback
  dc.SetBackground(m_background);
  dc.SetClippingRegion(0, 0, leftMargin + trackWidth + rightMargin, 9);
  dc.Clear();
  dc.SetPen(wxPen(black, 1, wxSOLID));
  dc.SetBrush(wxBrush(white, wxSOLID));

  // draw playposition rectangle
  dc.DrawRectangle(leftMargin, 0, trackWidth, 10);
 
  // draw the indicator for the playposition 
  dc.DrawIcon(playPositionMarker, playPosition, 1);
}

void WaveformDrawer::SetPlayPosition(unsigned int pPos) {
  // In comes a sample value and the playPosition is calculated in pixels from (leftMargin - 4) to (trackWidth - 4)
  int nrOfSamples = waveTracks[0].waveData.size();
  int samplesPerPixel;
    
  if (nrOfSamples % trackWidth == 0)
    samplesPerPixel = nrOfSamples / trackWidth;
  else
    samplesPerPixel = (nrOfSamples / trackWidth) + 1;
  playPosition = pPos / samplesPerPixel + (leftMargin - 4);
}

WaveformDrawer::~WaveformDrawer() {
}

void WaveformDrawer::AddCuePosition(unsigned int cuePos) {
  cueSampleOffset.push_back(cuePos);
  somethingHasChanged = true;
}

void WaveformDrawer::AddLoopPosition(unsigned int startPos, unsigned int endPos) {
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
  for (int i = 0; i < loopPositions.size(); i++) {
    // add a LOOPLAYOUT item to loopLayout vector
    LOOPLAYOUT item;
    loopLayout.push_back(item);
    // let's check if loop will overlap any previous and which
    // and preliminary set placedInRow to 0
    loopLayout[i].placedInRow = 0;
    for (int j = 0; j < i; j++) {
      bool isOverlapping = false;
      if (loopPositions[i].first > loopPositions[j].first && loopPositions[i].first < loopPositions[j].second)
        isOverlapping = true;
      else if (loopPositions[i].second > loopPositions[j].first && loopPositions[i].second < loopPositions[j].second)
        isOverlapping = true;

      if (isOverlapping) {
        // if it's overlapping then we add that loop to the loopLayout[i].overlappingLoops vector
        loopLayout[i].overlappingLoops.push_back(j);
      }
    }
  }
  // now all placedInRow is 0 and we know what loops overlaps which others
  // IF the loopLayout[i].overlappingLoops vector not is empty then placedInRow might need correction
  for (int i = 0; i < loopLayout.size(); i++) {
    // loopLayout[i].overlappingLoops[some valid index] contains the index of one overlapping loop
    // IF not the vector is empty...
    // IF last overlapping loop is in row 0 THEN this must be in row 1
    // ELSE we start at row 0 and check all the overlapping loops for a match
    // IF so THEN we continue with next row until we find one that's not matching (is unused)
    if (loopLayout[i].overlappingLoops.empty() == false) {
      if (loopLayout[loopLayout[i].overlappingLoops.back()].placedInRow == 0)
        loopLayout[i].placedInRow = 1;
      else {
        int unUsedRow = 0;
        for (int j = 0; j < loopLayout[i].overlappingLoops.size(); j++) {
          if (loopLayout[loopLayout[i].overlappingLoops[j]].placedInRow == unUsedRow)
            unUsedRow++;
          else
            break;
        }
        loopLayout[i].placedInRow = unUsedRow;
      }
    }
  }

  // And now it's the cue markers turn but first we get the samplesPerPixel value
  int nrOfSamples = waveTracks[0].waveData.size();
  int samplesPerPixel;
  int equalTo24px;

  if (nrOfSamples % trackWidth == 0)
    samplesPerPixel = nrOfSamples / trackWidth;
  else
    samplesPerPixel = (nrOfSamples / trackWidth) + 1;

  equalTo24px = 24 * samplesPerPixel;

  for (int i = 0; i < cueSampleOffset.size(); i++) {
    // Add a CUELAYOUT item to the cueLayout vector
    CUELAYOUT item;
    cueLayout.push_back(item);

    cueLayout[i].putInRow = 0;
    // with the cues we check if the marker is within any loop and if so treat as with the loops
    // but due to the possibility of marker and loop start flags overlapping we'll add samples to
    // the start equal to 24 pixels
    for (int j = 0; j < loopPositions.size(); j++) {
      bool isOverlapping = false;
      if (cueSampleOffset[i] > (loopPositions[j].first - equalTo24px) && cueSampleOffset[i] < loopPositions[j].second)
        isOverlapping = true;
     
      if (isOverlapping) {
        // if it's overlapping then we add that loop index to the cueLayout[i].withinLoop vector
        cueLayout[i].withinLoop.push_back(j);
      }
    }
    // and additionally we check if any previous marker is within 25 pixels around it
    for (int j = 0; j < i; j++) {
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
  for (int i = 0; i < cueLayout.size(); i++) {
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
        for (int j = 0; j < cueLayout[i].withinLoop.size(); j++) {
          if (loopLayout[cueLayout[i].withinLoop[j]].placedInRow == unUsedRow) {
            unUsedRow++;
          } else {
            if (cueLayout[i].markerClose.empty())
              break;
            else {
              for (int k = 0; k < cueLayout[i].markerClose.size(); k++) {
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

        for (int k = 0; k < cueLayout[i].markerClose.size(); k++) {
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

  if (m_x > leftMargin && m_x < (leftMargin + trackWidth) && m_y > topMargin && m_y <= (topMargin + trackHeight * nrChannels + marginBetweenTracks * nrChannels)) {
    // user have clicked on the track area
    int nrOfSamples = waveTracks[0].waveData.size();
    int samplesPerPixel;

    if (nrOfSamples % trackWidth == 0)
      samplesPerPixel = nrOfSamples / trackWidth;
    else
      samplesPerPixel = (nrOfSamples / trackWidth) + 1;

    if (cueIsSelected) {
      // a cue was selected so we'll change it's dwSampleOffset to a new position
      
      // first check that the pixels are valid
      if (m_x < leftMargin)
        m_x = leftMargin;
      if (m_x > leftMargin + trackWidth)
        m_x = leftMargin + trackWidth;

      int approximateSampleNumber = samplesPerPixel * (m_x - (leftMargin + 1));
      int earliestSampleToConsider = approximateSampleNumber - samplesPerPixel;
      int lastSampleToConsider = approximateSampleNumber + samplesPerPixel;

      if (earliestSampleToConsider < 0)
        earliestSampleToConsider = 0;

      if (lastSampleToConsider > waveTracks[0].waveData.size())
        lastSampleToConsider = waveTracks[0].waveData.size() - 1;

      unsigned int bestSample;
      double lowestRMSPower = DBL_MAX;
      double currentRMSPower = 0;
      // the sample values are in waveTracks[0].waveData
      for (int i = earliestSampleToConsider; i <= lastSampleToConsider; i++) {
        for (int j = 0; j < waveTracks.size(); j++)
          currentRMSPower += pow(waveTracks[j].waveData[i], 2);

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
      for (int i = 0; i < cueSampleOffset.size(); i++) {
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
        dc.SetBrush(wxBrush(white, wxBDIAGONAL_HATCH));
        dc.SetPen(wxPen(green, 1, wxSOLID));
        dc.DrawRectangle(cueLayout[selectedCueIndex].flagUpLeft.first, cueLayout[selectedCueIndex].flagUpLeft.second,
                         cueLayout[selectedCueIndex].flagDownRight.first - cueLayout[selectedCueIndex].flagUpLeft.first,
                         cueLayout[selectedCueIndex].flagDownRight.second - cueLayout[selectedCueIndex].flagUpLeft.second);

        ::wxGetApp().frame->SetStatusText(wxT("Move cue with next left click!"), 0);
      }
    }

  } else {
    cueIsSelected = false;
    ::wxGetApp().frame->SetStatusText(wxT("Ready"), 0);
    wxPaintEvent evt;
    Refresh();
    Update();
    AddPendingEvent(evt);
  }
}

void WaveformDrawer::OnRightClick(wxMouseEvent& event) {
  m_x = event.GetX(); 
  m_y = event.GetY();

  if (m_x > leftMargin && m_x < (leftMargin + trackWidth) && m_y > topMargin && m_y <= (topMargin + trackHeight * nrChannels + marginBetweenTracks * nrChannels)) {
    // user have rightclicked on the track area
    if (cueIsSelected) {
      cueIsSelected = false;
      ::wxGetApp().frame->SetStatusText(wxT("Ready"), 0);
      wxPaintEvent evt;
      Refresh();
      Update();
      AddPendingEvent(evt);
    }
    // draw an indication line approximately where cue will be inserted
    wxClientDC dc(this);
    int yPositionHigh = topMargin + 1;
    int yPositionLow = topMargin + trackHeight * waveTracks.size() + (marginBetweenTracks * (waveTracks.size() - 1) - 1);
    dc.SetPen(wxPen(green, 1, wxDOT));
    dc.DrawLine(m_x, yPositionLow, m_x, yPositionHigh);

    PopupMenu(m_popupMenu, event.GetPosition());

    wxPaintEvent evt;
    Refresh();
    Update();
    AddPendingEvent(evt);
  }
}

void WaveformDrawer::OnClickAddCue(wxCommandEvent& event) {
  // we should now calculate what sample have lowest RMS power around current position
  // so that a good dwSampleOffset value can be sent to the new cue
  int nrOfSamples = waveTracks[0].waveData.size();
  int samplesPerPixel;

  if (nrOfSamples % trackWidth == 0)
    samplesPerPixel = nrOfSamples / trackWidth;
  else
    samplesPerPixel = (nrOfSamples / trackWidth) + 1;

  int approximateSampleNumber = samplesPerPixel * (m_x - (leftMargin + 1));
  int earliestSampleToConsider = approximateSampleNumber - samplesPerPixel;
  int lastSampleToConsider = approximateSampleNumber + samplesPerPixel;

  if (earliestSampleToConsider < 0)
    earliestSampleToConsider = 0;

  if (lastSampleToConsider > waveTracks[0].waveData.size())
    lastSampleToConsider = waveTracks[0].waveData.size() - 1;

  unsigned int bestSample;
  double lowestRMSPower = DBL_MAX;
  double currentRMSPower = 0;
  // the sample values are in waveTracks[0].waveData
  for (int i = earliestSampleToConsider; i <= lastSampleToConsider; i++) {
    for (int j = 0; j < waveTracks.size(); j++)
      currentRMSPower += pow(waveTracks[j].waveData[i], 2);

    if (currentRMSPower < lowestRMSPower) {
      lowestRMSPower = currentRMSPower;
      bestSample = i;
    }
    currentRMSPower = 0;
  }

  ::wxGetApp().frame->AddNewCue(bestSample); // send offset value for the new cue creation
}


