/* 
 * ListInfoDialog.h is a part of LoopAuditioneer software
 * Copyright (C) 2020 Lars Palo 
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

#ifndef LISTINFODIALOG_H
#define LISTINFODIALOG_H

#include <wx/wx.h>
#include "FileHandling.h"
#include <wx/datetime.h>
#include <wx/dateevt.h>

// Identifiers
enum {
  ID_ARTIST = wxID_HIGHEST + 560,
  ID_COPY = wxID_HIGHEST + 561,
  ID_SOFT = wxID_HIGHEST + 562,
  ID_COMMENT = wxID_HIGHEST + 563,
  ID_CREATION_DATE = wxID_HIGHEST + 564
};

class ListInfoDialog : public wxDialog {
  DECLARE_CLASS(ListInfoDialog)
  DECLARE_EVENT_TABLE()

public:
  // Constructors
  ListInfoDialog(FileHandling *fh);
  ListInfoDialog(
    FileHandling *fh,
    wxWindow* parent,
    wxWindowID id = wxID_ANY,
    const wxString& caption = wxT("LIST INFO text"),
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
  );

  // Initialize our variables
  void Init(FileHandling *fh);

  // Creation
  bool Create(
    wxWindow* parent,
    wxWindowID id = wxID_ANY,
    const wxString& caption = wxT("LIST INFO text"),
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
  );

  // Creates the controls and sizers
  void CreateControls();

  // Accessors
  wxString getArtist();
  wxString getCopyright();
  wxString getSoftware();
  wxString getComment();
  wxDateTime getCreationDate();
  
    // Overrides
  bool TransferDataToWindow();
  bool TransferDataFromWindow();

private:
  wxString m_artist;
  wxString m_copyright;
  wxString m_software;
  wxString m_comment;
  wxDateTime m_creation_date;
  
  void OnDateChange(wxDateEvent& event);

};

#endif

