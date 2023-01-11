/* 
 * BatchProcessDialog.h is a part of LoopAuditioneer software
 * Copyright (C) 2011-2023 Lars Palo 
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

#ifndef BATCHPROCESSDIALOG_H
#define BATCHPROCESSDIALOG_H

#include <wx/wx.h>
#include "AutoLoopDialog.h"

// Identifiers
enum {
  ID_ADD_SOURCE = wxID_HIGHEST + 200,
  ID_ADD_TARGET = wxID_HIGHEST + 201,
  ID_PROCESSBOX = wxID_HIGHEST + 202,
  ID_SOURCE_TEXT = wxID_HIGHEST + 203,
  ID_TARGET_TEXT = wxID_HIGHEST + 204,
  ID_STATUS_TEXT = wxID_HIGHEST + 205,
  ID_RUN_BATCH = wxID_HIGHEST + 206
};

class BatchProcessDialog : public wxDialog {
  DECLARE_CLASS(BatchProcessDialog)
  DECLARE_EVENT_TABLE()

public:
  // Constructors
  BatchProcessDialog(AutoLoopDialog* autoloopSettings);
  BatchProcessDialog(
    AutoLoopDialog* autoloopSettings,
    wxWindow* parent,
    wxWindowID id = wxID_ANY,
    const wxString& caption = wxT("Batch processing"),
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
  );

  // Initialize our variables
  void Init(AutoLoopDialog* autoloopSettings);

  // Creation
  bool Create( 
    wxWindow* parent,
    wxWindowID id = wxID_ANY,
    const wxString& caption = wxT("Batch processing"),
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
  );

  // Creates the controls and sizers
  void CreateControls();

  void ClearStatusProgress();
  wxString GetLastSource();
  wxString GetLastTarget();
  void SetLastSource(wxString source);
  void SetLastTarget(wxString target);
  void SetCurrentWorkingDir(wxString str);
  bool NeedToRefreshFileList();

private:
  wxArrayString m_batchProcessesAvailable;
  wxButton *m_selectSource;
  wxButton *m_selectTarget;
  wxButton *m_runButton;
  wxChoice *m_processChoiceBox;
  wxTextCtrl *m_sourceField;
  wxTextCtrl *m_targetField;
  wxTextCtrl *m_statusProgress;
  wxString m_lastSource;
  wxString m_lastTarget;
  AutoLoopDialog *m_loopSettings;
  wxString m_currentWorkingDir;
  bool m_mustRefreshMainDir;
  wxString m_infoArtist;
  wxString m_infoCopyright;
  wxString m_infoComment;

  // Event methods
  void OnAddSource(wxCommandEvent& event);
  void OnAddTarget(wxCommandEvent& event);
  void OnChoiceSelected(wxCommandEvent& event);
  void OnRunBatch(wxCommandEvent& event);

  wxString MyDoubleToString(double dbl, int precision);

  void ReadyToRockAndRoll();
};

#endif
