/*
 * SpectrumPanel.h is a part of LoopAuditioneer software
 * Copyright (C) 2026 Lars Palo and contributors (see AUTHORS file)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * You can contact the author on larspalo(at)yahoo.se
 */

#ifndef SPECTRUMPANEL_H
#define SPECTRUMPANEL_H

#include <wx/wx.h>
#include "wx/overlay.h"
#include <vector>

class SpectrumPanel : public wxPanel {
public:
  SpectrumPanel(double *fftData, unsigned fftSize, wxString fileName, unsigned samplerate, wxWindow *parent);
  ~SpectrumPanel();

  double GetSelectedPitch();
  void DoZoomIn();
  void DoZoomOut();
  void DoZoomAll();
  void DoZoomSelection();
  int GetZoomLevel();
  bool HasSelection();
  bool HasPitchSelection();
  void SetCurrentMidHz(int sliderPosition);
  int GetPossibleSliderPosition();
  void SetPitchInterpolation(bool useInterpolation);
  bool GetUsePitchInterpolation();
  bool GetHasCustomZoom();

private:
	DECLARE_EVENT_TABLE()

  double *m_fftData; // the array containing the FFT as power spectrum in dB
  unsigned m_fftSize;
  wxString m_fileName;
  unsigned m_sampleRate;
  bool m_hasSelection;
  wxOverlay m_overlay;
  bool m_isSelecting;
  wxCoord m_startSelectionX;
  wxCoord m_currentSelectionX;
  int m_zoomLevel;
  double m_frequencyRange;
  double m_visibleHzRange;
  double m_currentLeftmostHz;
  double m_currentRightmostHz;
  double m_currentMidHz;
  bool m_hasPitchSelected;
  wxRect m_fftArea;
  double m_selectedPitch;
  int m_lastClickedFftAreaXpos;
  int m_lastClickedFftAreaYpos;
  std::vector<unsigned> m_lastBinEachPixel;
  bool m_usePitchInterpolation;
  unsigned m_selectionStartBin;
  unsigned m_selectionEndBin;
  bool m_hasCustomZoom;

  void UpdateLayout();
  unsigned ConvertHzToClosestBinIndex(double hertz);
  double ConvertBinIndexToHz(unsigned binIndex);
  double InterpolateHz(unsigned centerBinIndex);

  void OnPaintEvent(wxPaintEvent& event);
  void RenderPanel(wxDC& dc);
  void OnLeftClick(wxMouseEvent& event);
  void OnMouseMotion(wxMouseEvent& event);
  void OnLeftRelease(wxMouseEvent& event);
  void OnPanelSize(wxSizeEvent& event);

};

#endif

