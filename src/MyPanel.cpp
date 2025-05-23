/* 
 * MyPanel.cpp is a part of LoopAuditioneer
 * Copyright (C) 2011-2025 Lars Palo and contributors (see AUTHORS file)
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

#include "MyPanel.h"
#include "LoopAuditioneerDef.h"
#include "MyFrame.h"

BEGIN_EVENT_TABLE(MyPanel , wxScrolledWindow)
  EVT_KEY_DOWN(MyPanel::OnKeyDown)
  EVT_CHAR(MyPanel::OnKeyDown)
END_EVENT_TABLE()

MyPanel::MyPanel(wxFrame *parent) : wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,  wxFULL_REPAINT_ON_RESIZE | wxVSCROLL) {
  vbox = new wxBoxSizer(wxVERTICAL);
  fileNameLabel = new wxStaticText(this, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
  m_grid = new wxGrid(this, M_GRID, wxDefaultPosition, wxDefaultSize);
  m_grid->CreateGrid(0, 6);
  m_grid->SetRowLabelSize(90);
  m_grid->SetColLabelSize(30);
  m_grid->SetColMinimalAcceptableWidth(90);
  m_grid->SetDefaultRowSize(30);
  m_grid->SetDefaultColSize(90);
  m_grid->DisableDragGridSize();
  m_grid->DisableDragColSize();
  m_grid->DisableDragRowSize();
  m_grid->SetRowLabelAlignment(wxALIGN_CENTRE, wxALIGN_CENTRE);
  m_grid->SetLabelFont(wxFont(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
  m_grid->SetColLabelValue(0, wxT("Start"));
  m_grid->SetColLabelValue(1, wxT("End"));
  m_grid->SetColLabelValue(2, wxT("Samples"));
  m_grid->SetColLabelValue(3, wxT("Duration"));
  m_grid->SetColLabelValue(4, wxT("Max <>"));
  m_grid->SetColLabelValue(5, wxT("Save"));
  m_grid->SetSelectionMode(wxGrid::wxGridSelectRows);
  m_grid->SetColFormatBool(5);
  m_grid->SetDefaultCellAlignment(wxALIGN_CENTRE, wxALIGN_CENTRE);
  m_grid->SetCellHighlightPenWidth(0);
  m_cueGrid = new wxGrid(this, CUE_GRID, wxDefaultPosition, wxDefaultSize);
  m_cueGrid->CreateGrid(0,3);
  m_cueGrid->SetRowLabelSize(90);
  m_cueGrid->SetColLabelSize(30);
  m_cueGrid->SetColMinimalAcceptableWidth(90);
  m_cueGrid->SetDefaultRowSize(30);
  m_cueGrid->SetDefaultColSize(90);
  m_cueGrid->DisableDragGridSize();
  m_cueGrid->DisableDragColSize();
  m_cueGrid->DisableDragRowSize();
  m_cueGrid->SetRowLabelAlignment(wxALIGN_CENTRE, wxALIGN_CENTRE);
  m_cueGrid->SetLabelFont(wxFont(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
  m_cueGrid->SetColLabelValue(0, wxT("ID"));
  m_cueGrid->SetColLabelValue(1, wxT("Position"));
  m_cueGrid->SetColLabelValue(2, wxT("Save"));
  m_cueGrid->SetSelectionMode(wxGrid::wxGridSelectRows);
  m_cueGrid->SetColFormatBool(2);
  m_cueGrid->SetDefaultCellAlignment(wxALIGN_CENTRE, wxALIGN_CENTRE);
  m_cueGrid->SetCellHighlightPenWidth(0);
  vbox->Add(fileNameLabel, 0, wxALIGN_CENTER|wxTOP|wxBOTTOM, 5);
  vbox->Add(m_grid, 0, wxALIGN_CENTER | wxTOP| wxBOTTOM, 5);
  vbox->Add(m_cueGrid, 0, wxALIGN_CENTER | wxTOP | wxBOTTOM, 5);
  SetSizer(vbox);
  vbox->SetSizeHints(this);
  FitInside();
  SetScrollRate(5, 5);
}

MyPanel::~MyPanel() {

}

void MyPanel::SetFileNameLabel(wxFileName fullPath) {
  if (fullPath.IsOk() && fullPath.FileExists()) {
    wxString theFileName = fullPath.GetFullName();
    if (fullPath.GetDirCount()) {
      wxArrayString dirStrings = fullPath.GetDirs();
      wxString partToDisplay = dirStrings.Last();
      partToDisplay += wxFILE_SEP_PATH;
      partToDisplay += theFileName;
      fileNameLabel->SetLabel(wxT("Current open file: ") + partToDisplay);
    } else {
      fileNameLabel->SetLabel(wxT("Current open file: ") + theFileName);
    }
  } else {
    fileNameLabel->SetLabel(wxT("Current open file: "));
  }
}

void MyPanel::FillRowWithLoopData(int loopStart, int loopEnd, int sampleRate, bool toSave, int index, double quality) {
  m_grid->AppendRows(); // adds a new row at table bottom

  // fix values as strings
  wxString start = wxString::Format(wxT("%i"), loopStart);
  wxString end = wxString::Format(wxT("%i"), loopEnd);
  int numbersOfSamples = loopEnd - loopStart;
  wxString sampleNr = wxString::Format(wxT("%i"), numbersOfSamples);
  double duration = numbersOfSamples / (sampleRate * 1.0);
  wxString dur = wxString::Format(wxT("%.3f"), duration);
  wxString boolValue;
  if (toSave)
    boolValue = wxT("1");
  else
    boolValue = wxT("0");
  wxString qual = wxString::Format(wxT("%.6f"), quality);

  // set row label to Loop #
  wxString rowLabel = wxString::Format(wxT("Loop %i"), (index + 1));
  m_grid->SetRowLabelValue(index, rowLabel);

  // insert values into table row
  m_grid->SetCellValue(index , 0, start);
  m_grid->SetReadOnly(index, 0);
  m_grid->SetCellValue(index , 1, end);
  m_grid->SetReadOnly(index, 1);
  m_grid->SetCellValue(index , 2, sampleNr);
  m_grid->SetReadOnly(index, 2);
  m_grid->SetCellValue(index , 3, dur);
  m_grid->SetReadOnly(index, 3);
  m_grid->SetCellValue(index , 4, qual);
  m_grid->SetReadOnly(index, 4);
  m_grid->SetCellAlignment(index, 5, wxALIGN_CENTRE, wxALIGN_CENTRE);
  m_grid->SetCellValue(index, 5, boolValue);

  m_grid->AutoSizeColumns(false);
  GetSizer()->Layout();
}

void MyPanel::FillRowWithCueData(unsigned int id, unsigned int position, bool save, int index) {
  m_cueGrid->AppendRows(); // adds a new row at table bottom

  // fix values as strings
  wxString theID = wxString::Format(wxT("%i"), id);
  wxString pos = wxString::Format(wxT("%i"), position);
  wxString boolValue;
  if (save)
    boolValue = wxT("1");
  else
    boolValue = wxT("0");

  // set row label to Loop #
  wxString rowLabel = wxString::Format(wxT("Cue %i"), (index + 1));
  m_cueGrid->SetRowLabelValue(index, rowLabel);

  // insert values into table row
  m_cueGrid->SetCellValue(index , 0, theID);
  m_cueGrid->SetReadOnly(index, 0);
  m_cueGrid->SetCellValue(index , 1, pos);
  m_cueGrid->SetReadOnly(index, 1);
  m_cueGrid->SetCellAlignment(index, 2, wxALIGN_CENTRE, wxALIGN_CENTRE);
  m_cueGrid->SetCellValue(index, 2, boolValue);

  m_cueGrid->AutoSizeColumns(false);
  GetSizer()->Layout();
}

void MyPanel::EmptyTable() {
  int gridRows = m_grid->GetNumberRows();
  if (gridRows > 0)
    m_grid->DeleteRows(0, gridRows, true);

  int cueGridRows = m_cueGrid->GetNumberRows();
  if (cueGridRows > 0)
    m_cueGrid->DeleteRows(0, cueGridRows, true);

  m_grid->AutoSizeColumns(false);
  m_cueGrid->AutoSizeColumns(false);
  GetSizer()->Layout();
}

void MyPanel::ChangeCueData(unsigned int offset, int index) {
  wxString pos = wxString::Format(wxT("%i"), offset);

  m_cueGrid->SetCellValue(index , 1, pos);
  m_cueGrid->SetReadOnly(index, 1);

  m_cueGrid->AutoSizeColumns(false);
}

void MyPanel::ChangeLoopData(int loopStart, int loopEnd, int sampleRate, int index, double quality) {
  // fix values as strings
  wxString start = wxString::Format(wxT("%i"), loopStart);
  wxString end = wxString::Format(wxT("%i"), loopEnd);
  int numbersOfSamples = loopEnd - loopStart;
  wxString sampleNr = wxString::Format(wxT("%i"), numbersOfSamples);
  double duration = numbersOfSamples / (sampleRate * 1.0);
  wxString dur = wxString::Format(wxT("%.3f"), duration);
  wxString qual = wxString::Format(wxT("%.6f"), quality);

  // insert values into table row
  m_grid->SetCellValue(index , 0, start);
  m_grid->SetReadOnly(index, 0);
  m_grid->SetCellValue(index , 1, end);
  m_grid->SetReadOnly(index, 1);
  m_grid->SetCellValue(index , 2, sampleNr);
  m_grid->SetReadOnly(index, 2);
  m_grid->SetCellValue(index , 3, dur);
  m_grid->SetReadOnly(index, 3);
  m_grid->SetCellValue(index , 4, qual);
  m_grid->SetReadOnly(index, 4);

  m_grid->AutoSizeColumns(false);
}

void MyPanel::OnKeyDown(wxKeyEvent& event) {
  MyFrame *myParent = (MyFrame *) GetParent();
  myParent->OnKeyboardInput(event);
}

void MyPanel::UpdateLoopQuality(int index, double quality) {
  wxString qual = wxString::Format(wxT("%.6f"), quality);
  m_grid->SetCellValue(index , 4, qual);
  m_grid->SetReadOnly(index, 4);
  m_grid->AutoSizeColumns(false);
}
