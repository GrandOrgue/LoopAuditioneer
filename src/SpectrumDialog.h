/* 
 * SpectrumDialog.h is a part of LoopAuditioneer software
 * Copyright (C) 2024 Lars Palo and contributors (see AUTHORS file) 
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * You can contact the author on larspalo(at)yahoo.se
 */

#ifndef SPECTRUMDIALOG_H
#define SPECTRUMDIALOG_H

#include <wx/wx.h>
#include "SpectrumPanel.h"

// Identifiers
enum {
  ID_ZOOM_ALL_BTN = wxID_HIGHEST + 410,
  ID_ZOOM_IN_BTN = wxID_HIGHEST + 411,
  ID_ZOOM_OUT_BTN = wxID_HIGHEST + 412,
  ID_ZOOM_SEL_BTN = wxID_HIGHEST + 413,
  ID_ZOOM_SLIDER = wxID_HIGHEST + 414,
  ID_PITCH_INTERPOLATION_CHECK = wxID_HIGHEST + 415,
};

class SpectrumDialog : public wxDialog {
  DECLARE_CLASS(SpectrumDialog)
  DECLARE_EVENT_TABLE()

public:
  // Constructors
  SpectrumDialog(double *fftData, unsigned fftSize, wxString fileName, unsigned samplerate);
  SpectrumDialog(
    double *fftData,
    unsigned fftSize,
    wxString fileName,
    unsigned samplerate,
    wxWindow* parent,
    wxWindowID id = wxID_ANY,
    const wxString& title = wxT("Spectrum display"),
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxCLIP_CHILDREN|wxFULL_REPAINT_ON_RESIZE
  );

  ~SpectrumDialog();

  // Initialize our variables
  void Init(double *fftData, unsigned fftSize, wxString fileName, unsigned samplerate);

  // Creation
  bool Create(
    wxWindow* parent,
    wxWindowID id = wxID_ANY,
    const wxString& caption = wxT("Spectrum display"),
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxCLIP_CHILDREN|wxFULL_REPAINT_ON_RESIZE
  );

  // Creates the controls and sizers
  void CreateControls();

  double GetSelectedPitch();
  void PitchSelectionHasChanged();
  void SetInterpolatePitchOption(bool interpolate);

private:
  double *m_fftData; // the array containing the FFT as power spectrum in dB
  unsigned m_fftSize;
  wxString m_fileName;
  unsigned m_sampleRate;
  SpectrumPanel *m_drawingPanel;
  wxButton *m_zoomAllBtn;
  wxButton *m_zoomInBtn;
  wxButton *m_zoomOutBtn;
  wxButton *m_zoomSelectionBtn;
  wxSlider *m_zoomSlider;
  wxCheckBox *m_interpolatePitchCheck;

  void DecideOkButtonState();
  void DecideZoomButtonState();
  void OnZoomAllButton(wxCommandEvent& event);
  void OnZoomInButton(wxCommandEvent& event);
  void OnZoomOutButton(wxCommandEvent& event);
  void OnZoomSelection(wxCommandEvent& event);
  void OnZoomSlider(wxCommandEvent& event);
  void OnPitchInterpolationCheck(wxCommandEvent& event);

};

#endif
