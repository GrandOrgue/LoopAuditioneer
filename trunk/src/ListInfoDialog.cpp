/* 
 * ListInfoDialog.cpp is a part of LoopAuditioneer software
 * Copyright (C) 2020-2021 Lars Palo 
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

#include "ListInfoDialog.h"
#include <wx/statline.h>
#include <wx/datectrl.h>

IMPLEMENT_CLASS(ListInfoDialog, wxDialog )

// Event table
BEGIN_EVENT_TABLE(ListInfoDialog, wxDialog)
  EVT_DATE_CHANGED(ID_CREATION_DATE, ListInfoDialog::OnDateChange)
END_EVENT_TABLE()

ListInfoDialog::ListInfoDialog(FileHandling *fh) {
  Init(fh);
}

ListInfoDialog::ListInfoDialog(
    FileHandling *fh,
    wxWindow* parent,
    wxWindowID id,
    const wxString& caption,
    const wxPoint& pos,
    const wxSize& size,
    long style
  ) {
  Init(fh);
  Create(parent, id, caption, pos, size, style);
}

void ListInfoDialog::Init(FileHandling *fh) {
  m_artist = fh->m_info.artist;
  m_copyright = fh->m_info.copyright;
  m_software = fh->m_info.software;
  m_comment = fh->m_info.comment;
  m_creation_date = fh->m_info.creation_date;
}

bool ListInfoDialog::Create(
  wxWindow* parent,
  wxWindowID id,
  const wxString& caption,
  const wxPoint& pos,
  const wxSize& size,
  long style
) {
  if (!wxDialog::Create(parent, id, caption, pos, size, style))
    return false;

  CreateControls();

  GetSizer()->Fit(this);
  GetSizer()->SetSizeHints(this);
  Centre();

  return true;
}

void ListInfoDialog::CreateControls() {
  // Create a top level sizer
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  this->SetSizer(topSizer);

  // Second box sizer to get nice margins
  wxBoxSizer *boxSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(boxSizer, 0, wxEXPAND|wxALL, 5);

  // Information message at top
  wxStaticText *infoMessage = new wxStaticText(
    this,
    wxID_STATIC,
    wxT("Information set in LIST INFO of wave file."),
    wxDefaultPosition,
    wxDefaultSize,
    0
  );
  boxSizer->Add(infoMessage, 0, wxALIGN_LEFT|wxALL, 5);
  
  // A horizontal line after information messagew
  wxStaticLine *dividerline = new wxStaticLine(
    this,
    wxID_STATIC,
    wxDefaultPosition,
    wxDefaultSize,
    wxLI_HORIZONTAL
  );
  boxSizer->Add(dividerline, 0, wxGROW|wxALL, 5);

  // Label for artist
  wxStaticText *artistLabel = new wxStaticText(
    this,
    wxID_STATIC,
    wxT("Artist: "),
    wxDefaultPosition,
    wxSize(100,-1),
    0
  );
  boxSizer->Add(artistLabel, 0, wxALL, 2);

  // Textcontrol for artist
  wxTextCtrl *artistCtrl = new wxTextCtrl(
    this,
    ID_ARTIST,
    wxEmptyString,
    wxDefaultPosition,
    wxDefaultSize,
    0
  );
  boxSizer->Add(artistCtrl, 0, wxEXPAND|wxALL, 5);

  // Label for copyright
  wxStaticText *copyLabel = new wxStaticText(
    this,
    wxID_STATIC,
    wxT("Copyright: "),
    wxDefaultPosition,
    wxSize(100,-1),
    0
  );
  boxSizer->Add(copyLabel, 0, wxALL, 2);

  // Textcontrol for copyright
  wxTextCtrl *copyCtrl = new wxTextCtrl(
    this,
    ID_COPY,
    wxEmptyString,
    wxDefaultPosition,
    wxDefaultSize,
    0
  );
  boxSizer->Add(copyCtrl, 0, wxEXPAND|wxALL, 5);
  
  // Label for software
  wxStaticText *softLabel = new wxStaticText(
    this,
    wxID_STATIC,
    wxT("Software: "),
    wxDefaultPosition,
    wxSize(100,-1),
    0
  );
  boxSizer->Add(softLabel, 0, wxALL, 2);

  // Textcontrol for software
  wxTextCtrl *softCtrl = new wxTextCtrl(
    this,
    ID_SOFT,
    wxEmptyString,
    wxDefaultPosition,
    wxDefaultSize,
    wxTE_READONLY
  );
  boxSizer->Add(softCtrl, 0, wxEXPAND|wxALL, 5);
  
  // Label for comment
  wxStaticText *commentLabel = new wxStaticText(
    this,
    wxID_STATIC,
    wxT("Comment: "),
    wxDefaultPosition,
    wxSize(100,-1),
    0
  );
  boxSizer->Add(commentLabel, 0, wxALL, 2);

  // Textcontrol for comment
  wxTextCtrl *commentCtrl = new wxTextCtrl(
    this,
    ID_COMMENT,
    wxEmptyString,
    wxDefaultPosition,
    wxDefaultSize,
    0
  );
  boxSizer->Add(commentCtrl, 0, wxEXPAND|wxALL, 5);
  
  // Label for creation date
  wxStaticText *dateLabel = new wxStaticText(
    this,
    wxID_STATIC,
    wxT("Creation date: "),
    wxDefaultPosition,
    wxSize(100,-1),
    0
  );
  boxSizer->Add(dateLabel, 0, wxALL, 2);

  // Date picker for creation date
  wxDatePickerCtrl *dateCtrl = new wxDatePickerCtrl(
    this,
    ID_CREATION_DATE,
    wxDefaultDateTime,
    wxDefaultPosition,
    wxDefaultSize
  );
  boxSizer->Add(dateCtrl, 0, wxEXPAND|wxALL, 5);
  
  // A horizontal line before the buttons
  wxStaticLine *bottomline = new wxStaticLine(
    this,
    wxID_STATIC,
    wxDefaultPosition,
    wxDefaultSize,
    wxLI_HORIZONTAL
  );
  boxSizer->Add(bottomline, 0, wxGROW|wxALL, 5);

  // Horizontal sizer for buttons
  wxBoxSizer* buttonRow = new wxBoxSizer(wxHORIZONTAL);
  boxSizer->Add(buttonRow, 0, wxEXPAND|wxALL, 5);

  // The OK button
  wxButton *okButton = new wxButton(
    this,
    wxID_OK,
    wxT("&Ok"),
    wxDefaultPosition,
    wxDefaultSize,
    0
  );
  buttonRow->Add(okButton, 0, wxALIGN_CENTER|wxALL, 5);

  // The Cancel button
  wxButton *cancelButton = new wxButton(
    this,
    wxID_CANCEL,
    wxT("&Cancel"),
    wxDefaultPosition,
    wxDefaultSize,
    0
  );
  buttonRow->Add(cancelButton, 0, wxALIGN_CENTER|wxALL, 5);

}

wxString ListInfoDialog::getArtist() {  return m_artist;}
wxString ListInfoDialog::getCopyright() {  return m_copyright;}
wxString ListInfoDialog::getSoftware() {  return wxT("LoopAudtioneer");}
wxString ListInfoDialog::getComment() {  return m_comment;}
wxDateTime ListInfoDialog::getCreationDate() {  return m_creation_date;}

// Override of transfer data to the window
bool ListInfoDialog::TransferDataToWindow() {
  wxTextCtrl *artistCtrl = (wxTextCtrl*) FindWindow(ID_ARTIST);
  wxTextCtrl *copyCtrl = (wxTextCtrl*) FindWindow(ID_COPY);
  wxTextCtrl *softCtrl = (wxTextCtrl*) FindWindow(ID_SOFT);
  wxTextCtrl *commentCtrl = (wxTextCtrl*) FindWindow(ID_COMMENT);
  wxDatePickerCtrl *dateCtrl = (wxDatePickerCtrl*) FindWindow(ID_CREATION_DATE);
  
  artistCtrl->SetValue(m_artist);
  copyCtrl->SetValue(m_copyright);
  softCtrl->SetValue(m_software);
  commentCtrl->SetValue(m_comment);
  dateCtrl->SetValue(m_creation_date);
  
  return true;
}

// Override of transfer data from the window
bool ListInfoDialog::TransferDataFromWindow() {
  wxTextCtrl *artistCtrl = (wxTextCtrl*) FindWindow(ID_ARTIST);
  wxTextCtrl *copyCtrl = (wxTextCtrl*) FindWindow(ID_COPY);
  wxTextCtrl *softCtrl = (wxTextCtrl*) FindWindow(ID_SOFT);
  wxTextCtrl *commentCtrl = (wxTextCtrl*) FindWindow(ID_COMMENT);
  wxDatePickerCtrl *dateCtrl = (wxDatePickerCtrl*) FindWindow(ID_CREATION_DATE);
  
  m_artist = artistCtrl->GetValue();
  m_copyright = copyCtrl->GetValue();
  m_software = softCtrl->GetValue();
  m_comment = commentCtrl->GetValue();
  m_creation_date = dateCtrl->GetValue();
  
  return true;
}

void ListInfoDialog::OnDateChange(wxDateEvent& event) {
  // update m_creation_date
  if (event.GetDate().IsValid())
    m_creation_date = event.GetDate();
}
