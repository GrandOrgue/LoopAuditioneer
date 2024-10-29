/*
 * SpectrumPanel.cpp is a part of LoopAuditioneer software
 * Copyright (C) 2024 Lars Palo and contributors (see AUTHORS file)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY {

}
 without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * You can contact the author on larspalo(at)yahoo.se
 */

#include "SpectrumPanel.h"
#include "SpectrumDialog.h"

// Event table
BEGIN_EVENT_TABLE(SpectrumPanel, wxPanel)
  EVT_PAINT(SpectrumPanel::OnPaintEvent)
  EVT_LEFT_DOWN(SpectrumPanel::OnLeftClick)
  EVT_MOTION(SpectrumPanel::OnMouseMotion)
  EVT_LEFT_UP(SpectrumPanel::OnLeftRelease)
  EVT_SIZE(SpectrumPanel::OnPanelSize)
END_EVENT_TABLE()

SpectrumPanel::SpectrumPanel(
  double *fftData,
  unsigned fftSize,
  wxString fileName,
  unsigned samplerate,
  wxWindow *parent) : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE) {
  m_fftData = fftData;
  m_fftSize = fftSize;
  m_fileName = fileName;
  m_sampleRate = samplerate;
  m_zoomLevel = 0;
  m_hasSelection = false;
  m_hasPitchSelected = false;
  m_frequencyRange = m_sampleRate / 2;
  m_visibleHzRange = m_sampleRate / 2;
  m_currentLeftmostHz = 0;
  m_currentRightmostHz = m_sampleRate / 2;
  m_currentMidHz = m_visibleHzRange / 2;
  m_lastClickedFftAreaXpos = -1;
  m_lastClickedFftAreaYpos = -1;
  m_usePitchInterpolation = false;
  m_hasCustomZoom = false;
  m_selectionStartBin = 0;
  m_selectionEndBin = 0;
  m_startSelectionX = wxCoord(-1);
  m_currentSelectionX = wxCoord(-1);
  m_isSelecting = false;
  m_selectedPitch = 0;

  SetBackgroundColour(wxColour(244,242,239));
  SetMinSize(wxSize(640, 480));
}

SpectrumPanel::~SpectrumPanel() {

}

double SpectrumPanel::GetSelectedPitch() {
  if (m_hasPitchSelected)
    return m_selectedPitch;
  else
    return -1.0f;
}

void SpectrumPanel::DoZoomIn() {
  m_zoomLevel++;
  m_hasSelection = false;
  UpdateLayout();
  Refresh();
}

void SpectrumPanel::DoZoomOut() {
  if (m_zoomLevel > 0) {
    m_hasSelection = false;
    m_zoomLevel--;
    double halfVisibleRange = ((double) m_sampleRate / (2.0f * pow(2, m_zoomLevel))) / 2.0f;
    if (m_currentMidHz < halfVisibleRange)
      m_currentMidHz = halfVisibleRange;
    if (m_currentMidHz > m_frequencyRange - halfVisibleRange)
      m_currentMidHz = m_frequencyRange - halfVisibleRange;
    UpdateLayout();
    Refresh();
  }
}

void SpectrumPanel::DoZoomAll() {
  if (m_zoomLevel > 0 || m_hasCustomZoom) {
    m_hasCustomZoom = false;
    m_hasSelection = false;
    m_zoomLevel = 0;
    m_currentMidHz = m_sampleRate / 4.0f;
    UpdateLayout();
    Refresh();
  }
}

void SpectrumPanel::DoZoomSelection() {
  if (m_hasSelection) {
    if (m_selectionEndBin < m_selectionStartBin) {
      m_currentLeftmostHz = ConvertBinIndexToHz(m_selectionEndBin);
      if (m_currentLeftmostHz < 0)
        m_currentLeftmostHz = 0;
      m_currentRightmostHz = ConvertBinIndexToHz(m_selectionStartBin);
      if (m_currentRightmostHz > m_frequencyRange)
        m_currentRightmostHz = m_frequencyRange;
    } else {
      m_currentLeftmostHz = ConvertBinIndexToHz(m_selectionStartBin);
      if (m_currentLeftmostHz < 0)
        m_currentLeftmostHz = 0;
      m_currentRightmostHz = ConvertBinIndexToHz(m_selectionEndBin);
      if (m_currentRightmostHz > m_frequencyRange)
        m_currentRightmostHz = m_frequencyRange;
    }

    m_visibleHzRange = m_currentRightmostHz - m_currentLeftmostHz;
    m_currentMidHz = m_currentLeftmostHz + (m_visibleHzRange / 2.0f);

    m_hasSelection = false;
    m_hasCustomZoom = true;
    Refresh();
  }
}

int SpectrumPanel::GetZoomLevel() {
  return m_zoomLevel;
}

bool SpectrumPanel::HasSelection() {
  return m_hasSelection;
}

bool SpectrumPanel::HasPitchSelection() {
  return m_hasPitchSelected;
}

void SpectrumPanel::SetCurrentMidHz(int sliderPosition) {
  if (m_visibleHzRange < m_frequencyRange) {
    // it's possible to move the middle frequency around within limitations
    if ((double) sliderPosition < m_visibleHzRange / 2) {
      // slider was moved more to the left than possible
      m_currentMidHz = m_visibleHzRange / 2;
    } else if ((double) sliderPosition > m_frequencyRange - m_visibleHzRange / 2) {
      // slider was moved more to the right than possible
      m_currentMidHz = m_frequencyRange - m_visibleHzRange / 2;
    } else {
      // slider is within acceptable range
      m_currentMidHz = (double) sliderPosition;
    }

    UpdateLayout();
    Refresh();
  }
}

int SpectrumPanel::GetPossibleSliderPosition() {
  return (int) m_currentMidHz;
}

void SpectrumPanel::SetPitchInterpolation(bool useInterpolation) {
  m_usePitchInterpolation = useInterpolation;
  Refresh();
}

bool SpectrumPanel::GetUsePitchInterpolation() {
  return m_usePitchInterpolation;
}

bool SpectrumPanel::GetHasCustomZoom() {
  return m_hasCustomZoom;
}

void SpectrumPanel::UpdateLayout() {
  if (!m_hasCustomZoom)
    m_visibleHzRange = (double) m_sampleRate / (2.0f * pow(2, m_zoomLevel));

  m_currentLeftmostHz = m_currentMidHz - m_visibleHzRange / 2;
  if (m_currentLeftmostHz < 0)
    m_currentLeftmostHz = 0;
  m_currentRightmostHz = m_currentMidHz + m_visibleHzRange / 2;
  if (m_currentRightmostHz > m_frequencyRange)
    m_currentRightmostHz = m_frequencyRange;

  if (m_hasPitchSelected) {
    // the currently selected pitch must be invalidated
    m_hasPitchSelected = false;
    m_lastClickedFftAreaXpos = -1;
    m_lastClickedFftAreaYpos = -1;
  }
}

unsigned SpectrumPanel::ConvertHzToClosestBinIndex(double hertz) {
  if (hertz <= 0)
    return 0;
  else if (hertz < (double) m_sampleRate / 2.0f)
    return hertz * m_fftSize / m_sampleRate;
  else
    return (m_fftSize / 2) - 1;
}

double SpectrumPanel::ConvertBinIndexToHz(unsigned binIndex) {
  if (binIndex < m_fftSize / 2)
    return (double) binIndex * m_sampleRate / (double) m_fftSize;
  else
    return (double) m_sampleRate / 2.0f;
}

double SpectrumPanel::InterpolateHz(unsigned centerBinIndex) {
  if (centerBinIndex < 1)
    return 0;

  if (centerBinIndex > m_fftSize - 2)
    return (double) m_sampleRate / 2.0f;

  double pitchToReturn;

  if (centerBinIndex > 2 && centerBinIndex < m_fftSize / 2 - 4) {
    // Estimate frequency as weighted average
    double fS = (double) m_sampleRate / m_fftSize;
    double pNumenator = 0;
    double pDenominator = 0;
    for (int i = -3; i < 4; i++) {
      pNumenator += (m_fftData[centerBinIndex + i] * (centerBinIndex + i) * fS);
      pDenominator += m_fftData[centerBinIndex + i];
    }
    pitchToReturn = pNumenator / pDenominator;

    return pitchToReturn;
  } else {
    // Use cubic interpolation
    double centerPeakBin;
    centerPeakBin = (m_fftData[centerBinIndex + 1] - m_fftData[centerBinIndex - 1]) / (2 * ( 2 * m_fftData[centerBinIndex] - m_fftData[centerBinIndex - 1] - m_fftData[centerBinIndex + 1]));
    pitchToReturn = (centerBinIndex + centerPeakBin) * m_sampleRate / (double) m_fftSize;

    return pitchToReturn;
  }
}

void SpectrumPanel::OnPaintEvent(wxPaintEvent& WXUNUSED(event)) {
  wxPaintDC dc(this);
  RenderPanel(dc);
}

void SpectrumPanel::RenderPanel(wxDC& dc) {
  m_overlay.Reset();

  wxSize panelSize = this->GetSize();
  int middleWidth = panelSize.x - (200); // 100 px margins left and right for info
  int availableFftWidth = middleWidth - 2;
  int middleHeight = panelSize.y - (40); // 40 px margin top for pitch info
  int availableFftHeight = middleHeight - 76;
  int startFftLinesAtY = panelSize.y - 76;
  double dBperFftPixelHeight = (double) availableFftHeight / 120.0f;

  dc.SetBrush(wxBrush(wxColour(*wxWHITE)));
  dc.SetPen(wxPen(wxColour(*wxBLACK), 1, wxPENSTYLE_SOLID));
  dc.SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_LIGHT));

  // white background for middle
  dc.DrawRectangle(100, 40, middleWidth, middleHeight);
  m_fftArea = wxRect(101, 41, availableFftWidth, availableFftHeight);

  // info bars at bottom
  dc.SetBrush(wxBrush(wxColour(*wxLIGHT_GREY)));
  dc.SetPen(wxPen(wxColour(128, 128, 128), 1, wxPENSTYLE_SOLID));
  dc.DrawRectangle(101, panelSize.y - 25, availableFftWidth, 25);
  dc.DrawRectangle(101, panelSize.y - 50, availableFftWidth, 25);
  dc.DrawRectangle(101, panelSize.y - 75, availableFftWidth, 25);

  dc.SetPen(wxPen(wxColour(*wxBLACK), 1, wxPENSTYLE_SOLID));
  wxString totalFrequencyRange = wxString::Format(wxT("Total frequency range is %.2lf Hz"), m_frequencyRange);
  wxSize freqRangeExtent = dc.GetTextExtent(totalFrequencyRange);
  int midPointX = panelSize.x / 2;
  dc.DrawText(totalFrequencyRange, midPointX - (freqRangeExtent.x / 2), (panelSize.y - 13) - (freqRangeExtent.y / 2));

  wxString visibleRange = wxString::Format(wxT("Visible range is %.2lf Hz"), m_visibleHzRange);
  wxSize visibleTextExtent = dc.GetTextExtent(visibleRange);
  dc.DrawText(visibleRange, midPointX - (visibleTextExtent.x / 2), (panelSize.y - 38) - (visibleTextExtent.y / 2));

  wxString leftmostHz = wxString::Format(wxT("%.2lf"), m_currentLeftmostHz);
  wxSize leftHzTextExtent = dc.GetTextExtent(leftmostHz);
  dc.DrawText(leftmostHz, 103, (panelSize.y - 38) - (leftHzTextExtent.y / 2));

  wxString rightmostHz = wxString::Format(wxT("%.2lf"), m_currentRightmostHz);
  wxSize rightHzTextExtent = dc.GetTextExtent(rightmostHz);
  dc.DrawText(rightmostHz, 99 + availableFftWidth - (rightHzTextExtent.x), (panelSize.y - 38) - (leftHzTextExtent.y / 2));

  dc.SetPen(wxPen(wxColour(128, 128, 128), 1, wxPENSTYLE_DOT));
  dc.DrawLine(midPointX, panelSize.y - 51, midPointX, panelSize.y - 75);
  dc.SetPen(wxPen(wxColour(*wxBLACK), 1, wxPENSTYLE_SOLID));
  wxString midStr = wxT("Mid frequency:");
  wxSize midStrTextExtent = dc.GetTextExtent(midStr);
  wxString midHz = wxString::Format(wxT("%.2lf Hz"), m_currentMidHz);
  wxSize midHzTextExtent = dc.GetTextExtent(midHz);
  dc.DrawText(midStr, midPointX - 2 - (midStrTextExtent.x), (panelSize.y - 63) - (midStrTextExtent.y / 2));
  dc.DrawText(midHz, midPointX + 5, (panelSize.y - 63) - (midHzTextExtent.y / 2));

  wxString topDb = wxT("0 dB");
  wxSize topDbExtent = dc.GetTextExtent(topDb);
  dc.DrawLine(panelSize.x - 100, 41, panelSize.x - 94 + topDbExtent.x, 41);
  dc.DrawText(topDb, panelSize.x - 95, 46);

  wxString bottomDb = wxT("-120 dB");
  wxSize bottomDbExtent = dc.GetTextExtent(bottomDb);
  dc.DrawLine(panelSize.x - 100, startFftLinesAtY, panelSize.x - 94 + bottomDbExtent.x, startFftLinesAtY);
  dc.DrawText(bottomDb, panelSize.x - 95, startFftLinesAtY - (bottomDbExtent.y + 5));

  unsigned firstVisibleBinIndex = ConvertHzToClosestBinIndex(m_currentLeftmostHz);
  unsigned lastVisibleBinIndex = ConvertHzToClosestBinIndex(m_currentRightmostHz);
  unsigned nbrVisibleBins = lastVisibleBinIndex - firstVisibleBinIndex + 1;
  int binsPerPixel = nbrVisibleBins / availableFftWidth;
  int remainingBins = nbrVisibleBins % availableFftWidth;
  double addAnotherBinAfter = 0;
  if (remainingBins > 0)
    addAnotherBinAfter = (double) availableFftWidth / (double) remainingBins;
  int currentFftPixelOffset = 0;
  unsigned binNbr = 0;
  double maxValueForThisPixel = -145;
  unsigned startNextLineAtBinIndex = binsPerPixel;
  int extraBinsAdded = 0;
  double pixelsPerBin = (double) availableFftWidth / (double) (nbrVisibleBins - 1);

  if (binsPerPixel >= 1) {
    // there will be at least one bin per pixel
    if (!m_lastBinEachPixel.empty())
      m_lastBinEachPixel.clear();

    while (binNbr < nbrVisibleBins) {
      // compare current fft data value to previous max
      if (m_fftData[binNbr + firstVisibleBinIndex] > maxValueForThisPixel)
        maxValueForThisPixel = m_fftData[binNbr + firstVisibleBinIndex];

      if ((binNbr + 1) == startNextLineAtBinIndex || binNbr == (nbrVisibleBins - 1)) {
        // we should draw a line at this pixel width now and start a new count/comparison
        if (maxValueForThisPixel > -120) {
          double exactHeight = (120 + maxValueForThisPixel) * dBperFftPixelHeight;
          dc.DrawLine(101 + currentFftPixelOffset, startFftLinesAtY, 101 + currentFftPixelOffset, (startFftLinesAtY - (int) exactHeight) - 1);
          m_lastBinEachPixel.push_back(binNbr);
        }

        if (remainingBins == 0)
          startNextLineAtBinIndex += binsPerPixel;
        else if (currentFftPixelOffset < (int) (addAnotherBinAfter * (extraBinsAdded + 1)))
          startNextLineAtBinIndex += binsPerPixel;
        else {
          startNextLineAtBinIndex += (binsPerPixel + 1);
          extraBinsAdded++;
        }

        currentFftPixelOffset++;
        maxValueForThisPixel = -145;
      }

      binNbr++;
    }
  } else {
    // there will at more than one pixel available for every bin so the drawing method should change
    int lastXpos;
    int lastYpos;
    while (binNbr < nbrVisibleBins) {
      double valueForThisBin = m_fftData[binNbr + firstVisibleBinIndex];

      double exactHeight = (120 + valueForThisBin) * dBperFftPixelHeight;
      double exactXOffset = pixelsPerBin * binNbr;

      if (binNbr == 0) {
        lastXpos = 101;
        lastYpos = startFftLinesAtY - (int) exactHeight - 1;
        binNbr++;
        continue;
      }

      int currentXpos = 101 + exactXOffset;
      int currentYpos = startFftLinesAtY - (int) exactHeight - 1;

      dc.DrawLine(lastXpos, lastYpos, currentXpos, currentYpos);

      lastXpos = currentXpos;
      lastYpos = currentYpos;

      binNbr++;
    }
  }

  if (m_hasPitchSelected) {
    // draw dotted vertical crossline to mark where the selecting click was
    dc.SetPen(wxPen(wxColour(*wxRED), 1, wxPENSTYLE_DOT));
    dc.DrawLine(101 + m_lastClickedFftAreaXpos, startFftLinesAtY, 101 + m_lastClickedFftAreaXpos, 40);

    dc.SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    dc.SetBackgroundMode(wxTRANSPARENT);
    dc.SetTextForeground(wxColour(*wxRED));
    unsigned clickedBin = 0;

    if (binsPerPixel >= 1) {
      if (m_lastClickedFftAreaXpos < (int) m_lastBinEachPixel.size()) {
        // first we must convert the x coordinate to the bin range it represents
        unsigned lastBinForPixel = firstVisibleBinIndex + m_lastBinEachPixel[m_lastClickedFftAreaXpos];
        unsigned firstBinForPixel;
        if (m_lastClickedFftAreaXpos > 0) {
          firstBinForPixel = firstVisibleBinIndex + m_lastBinEachPixel[m_lastClickedFftAreaXpos - 1] + 1;
        } else {
          firstBinForPixel = firstVisibleBinIndex;
        }

        // get bin and highest value for that clicked pixel bin range
        double highestValue = -145;
        for (unsigned i = firstBinForPixel; i <= lastBinForPixel; i++) {
          if (m_fftData[i] > highestValue) {
            highestValue = m_fftData[i];
            clickedBin = i;
          }
        }

      } else {
        // for some reason the clicked x coordinate is outside allowed range
      }

    } else {
      // convert the clicked x coordinate to a bin number
      clickedBin = firstVisibleBinIndex + (m_lastClickedFftAreaXpos / pixelsPerBin);
    }

    // translate to pitch and draw
    if (m_usePitchInterpolation) {
      m_selectedPitch = InterpolateHz(clickedBin);
    } else {
      m_selectedPitch = ConvertBinIndexToHz(clickedBin);
    }
    wxString pitchStr = wxString::Format(wxT("%.2lf Hz"), m_selectedPitch);
    wxSize pitchExtent = dc.GetTextExtent(pitchStr);
    dc.DrawText(pitchStr, 101 + m_lastClickedFftAreaXpos - (pitchExtent.x / 2), (40 - pitchExtent.y) / 2);

    // draw dotted horizontal crossline where dB level actually is
    int dBlineY = startFftLinesAtY - (int) ((120 + m_fftData[clickedBin]) * dBperFftPixelHeight) - 1;
    dc.DrawLine(101, dBlineY, 101 + availableFftWidth, dBlineY);
    // draw the dB level to the left for the bin used for pitch detection
    double fftValue = m_fftData[clickedBin];
    wxString dbStr = wxString::Format(wxT("%.2lf dB"), fftValue);
    wxSize dbExtent = dc.GetTextExtent(dbStr);
    dc.DrawText(dbStr, 95 - dbExtent.x, dBlineY - (dbExtent.y / 2));

  } else if (m_hasSelection) {

    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.SetPen(wxPen(wxColour(*wxRED), 1, wxPENSTYLE_DOT_DASH));
    int selectionWidth = -1;
    int leftX = m_startSelectionX;
    if (m_currentSelectionX > m_startSelectionX) {
      selectionWidth = m_currentSelectionX - m_startSelectionX + 1;
    } else {
      selectionWidth = m_startSelectionX - m_currentSelectionX + 1;
      leftX = m_currentSelectionX;
    }
    wxRect tempSelectionOutline(
      leftX,
      m_fftArea.GetY(),
      selectionWidth,
      m_fftArea.GetHeight()
    );
    dc.DrawRectangle(tempSelectionOutline);

    // store existing selection borders as bins for future selection zooms
    if (binsPerPixel >= 1) {
      int start = leftX - m_fftArea.GetX();
      int end = leftX - m_fftArea.GetX() + selectionWidth;
      if (start > 0)
        m_selectionStartBin = firstVisibleBinIndex + m_lastBinEachPixel[start - 1] + 1;
      else
        m_selectionStartBin = firstVisibleBinIndex;
      
      if (end < (int) m_lastBinEachPixel.size())
        m_selectionEndBin = firstVisibleBinIndex + m_lastBinEachPixel[end];
      else
        m_selectionEndBin = lastVisibleBinIndex;

    } else {
      m_selectionStartBin = firstVisibleBinIndex + ((leftX - m_fftArea.GetX()) / pixelsPerBin);
      m_selectionEndBin = firstVisibleBinIndex + ((leftX - m_fftArea.GetX() + selectionWidth) / pixelsPerBin);
    }

  }
}

void SpectrumPanel::OnLeftClick(wxMouseEvent& event) {
  wxCoord xPos = event.GetX();
  wxCoord yPos = event.GetY();
  SpectrumDialog *myParent = (SpectrumDialog*) GetParent();
  if (m_fftArea.Contains(xPos, yPos)) {
    if (m_hasSelection)
      m_hasSelection = false;

    m_hasPitchSelected = true;
    m_lastClickedFftAreaXpos = xPos - m_fftArea.GetX();
    m_lastClickedFftAreaYpos = yPos;
    m_startSelectionX = xPos;
    Refresh();
    myParent->PitchSelectionHasChanged();
  } else if (m_hasPitchSelected) {
    if (m_hasSelection)
      m_hasSelection = false;

    m_hasPitchSelected = false;
    m_lastClickedFftAreaXpos = -1;
    m_lastClickedFftAreaYpos = -1;
    Refresh();
    myParent->PitchSelectionHasChanged();
  } else {
    event.Skip();
  }
}

void SpectrumPanel::OnMouseMotion(wxMouseEvent& event) {
  if (m_lastClickedFftAreaXpos > -1 && event.Dragging() && (event.GetX() != m_startSelectionX)) {
    m_isSelecting = true;
    m_currentSelectionX = event.GetX();
    if (m_currentSelectionX < m_fftArea.GetX())
      m_currentSelectionX = m_fftArea.GetX();
    else if (m_currentSelectionX > m_fftArea.GetX() + m_fftArea.GetWidth())
      m_currentSelectionX = m_fftArea.GetX() + m_fftArea.GetWidth();

    wxClientDC dc(this);
    wxDCOverlay overlaydc(m_overlay, &dc);
    overlaydc.Clear();
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.SetPen(wxPen(wxColour(*wxRED), 1, wxPENSTYLE_DOT));
    int selectionWidth = -1;
    int leftX = m_startSelectionX;
    if (m_currentSelectionX > m_startSelectionX) {
      selectionWidth = m_currentSelectionX - m_startSelectionX + 1;
    } else {
      selectionWidth = m_startSelectionX - m_currentSelectionX + 1;
      leftX = m_currentSelectionX;
    }
    wxRect tempSelectionOutline(
      leftX,
      m_fftArea.GetY(),
      selectionWidth,
      m_fftArea.GetHeight()
    );
    dc.DrawRectangle(tempSelectionOutline);
  }
}

void SpectrumPanel::OnLeftRelease(wxMouseEvent& event) {
  if (m_isSelecting && (event.GetX() != m_startSelectionX)) {
    m_isSelecting = false;
    m_currentSelectionX = event.GetX();
    if (m_currentSelectionX < m_fftArea.GetX())
      m_currentSelectionX = m_fftArea.GetX();
    else if (m_currentSelectionX > m_fftArea.GetX() + m_fftArea.GetWidth())
      m_currentSelectionX = m_fftArea.GetX() + m_fftArea.GetWidth();
    m_hasSelection = true;
    m_hasPitchSelected = false;
    m_lastClickedFftAreaXpos = -1;
    m_lastClickedFftAreaYpos = -1;
    SpectrumDialog *myParent = (SpectrumDialog*) GetParent();
    myParent->PitchSelectionHasChanged();
    Refresh();
  } else if (event.GetX() == m_startSelectionX) {
    m_hasSelection = false;
    m_hasPitchSelected = true;
  } else {
    event.Skip();
  }
}

void SpectrumPanel::OnPanelSize(wxSizeEvent& WXUNUSED(event)) {
  wxSize panelSize = this->GetSize();
  int middleWidth = panelSize.x - (200); // 100 px margins left and right for info
  int availableFftWidth = middleWidth - 2;
  int middleHeight = panelSize.y - (40); // 40 px margin top for pitch info
  int availableFftHeight = middleHeight - 76;
  wxRect newFftArea(101, 41, availableFftWidth, availableFftHeight);
  if (!newFftArea.Contains(m_lastClickedFftAreaXpos, m_lastClickedFftAreaYpos)) {
    m_hasPitchSelected = false;
    m_lastClickedFftAreaXpos = -1;
    m_lastClickedFftAreaYpos = -1;
    SpectrumDialog *myParent = (SpectrumDialog*) GetParent();
    myParent->PitchSelectionHasChanged();
  }
}

