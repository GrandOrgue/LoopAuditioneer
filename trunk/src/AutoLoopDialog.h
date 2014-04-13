/* 
 * AutoLoopDialog.h provide a GUI for setting parameters for AutoLooping
 * Copyright (C) 2011-2014 Lars Palo 
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

#ifndef AUTOLOOPDIALOG_H
#define AUTOLOOPDIALOG_H

#include <wx/wx.h>

// Identifiers
enum {
  ID_THRESHOLD = wxID_HIGHEST + 300,
  ID_DURATION = wxID_HIGHEST + 301,
  ID_BETWEEN = wxID_HIGHEST + 302,
  ID_QUALITY = wxID_HIGHEST + 303,
  ID_CANDIDATES = wxID_HIGHEST + 304,
  ID_NR_LOOPS = wxID_HIGHEST + 305,
  ID_LOOP_MULTIPLE = wxID_HIGHEST + 306,
  ID_SEARCH_CHECK = wxID_HIGHEST + 307,
  ID_SUSTAINSTART = wxID_HIGHEST + 308,
  ID_SUSTAINEND = wxID_HIGHEST + 309,
  ID_BRUTE_FORCE_CHECK = wxID_HIGHEST + 310
};

class AutoLoopDialog : public wxDialog {
  DECLARE_CLASS(AutoLoopDialog)
  DECLARE_EVENT_TABLE()

public:
  // Constructors
  AutoLoopDialog();
  AutoLoopDialog(
    wxWindow* parent,
    wxWindowID id = wxID_ANY,
    const wxString& caption = wxT("Parameters for auto searching loops"),
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
  );

  // Initialize our variables
  void Init();

  // Creation
  bool Create( 
    wxWindow* parent,
    wxWindowID id = wxID_ANY,
    const wxString& caption = wxT("Parameters for auto searching loops"),
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
  );

  // Creates the controls and sizers
  void CreateControls();

  // Accessing functions
  void SetThreshold(double th);
  void SetDuration(double d);
  void SetBetween(double b);
  void SetQuality(double q);
  void SetCandidates(int c);
  void SetNrLoops(int l);
  void SetMultiple(int m);
  void SetAutosearch(bool search);
  void SetStart(int start);
  void SetEnd(int end);
  void SetBruteForce(bool b);
  double GetThreshold();
  double GetDuration();
  double GetBetween();
  double GetQuality();
  int GetCandidates();
  int GetNrLoops();
  int GetMultiple();
  bool GetAutosearch();
  int GetStart();
  int GetEnd();
  bool GetBruteForce();

  // Overrides
  bool TransferDataToWindow();
  bool TransferDataFromWindow();

  // Event processing methods (for label updates)
  void OnAutosearchCheck(wxCommandEvent& event);
  void OnBruteForceCheck(wxCommandEvent& event);
  void OnStartSliderMove(wxCommandEvent& event);
  void OnEndSliderMove(wxCommandEvent& event);
  void OnThresholdSlider(wxCommandEvent& event);
  void OnDurationSlider(wxCommandEvent& event);
  void OnBetweenSlider(wxCommandEvent& event);
  void OnQuality(wxCommandEvent& event);

private:
  double m_threshold;  // 0.03 (3 %)
  double m_minDuration;      // 1.0 seconds
  double m_betweenLoops; // 0.3 seconds
  double m_quality;        // value (6) /32767 (0.00006) for float)
  int m_candidates;           // 50000
  int m_numberOfLoops;           // 6
  int m_loopMultiple;        // 10
  bool m_autoSearchSustain; // true
  int m_startPercentage; // 20
  int m_endPercentage; // 70
  bool m_searchBruteForce;

  // GUI controls
  wxStaticText *m_thresholdLabel;
  wxStaticText *m_durationLabel;
  wxStaticText *m_startLabel;
  wxStaticText *m_endLabel;
  wxStaticText *m_qualityLabel;
  wxStaticText *m_distanceLabel;
};

#endif
