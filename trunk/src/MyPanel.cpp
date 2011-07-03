/* 
 * MyPanel.cpp is a part of LoopAuditioneer
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

#include "MyPanel.h"
#include "LoopAuditioneer.h"

MyPanel::MyPanel(wxPanel *parent) : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,  wxTAB_TRAVERSAL) {
  vbox = new wxBoxSizer(wxVERTICAL);
  fileNameLabel = new wxStaticText(this, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
  m_grid = new wxGrid(this, M_GRID, wxDefaultPosition, wxDefaultSize);
  m_grid->CreateGrid(0, 5);
  m_grid->SetRowLabelSize(wxGRID_AUTOSIZE);
  m_grid->SetColLabelSize(25);
  m_grid->SetColMinimalAcceptableWidth(50);
  m_grid->DisableDragGridSize();
  m_grid->DisableDragColSize();
  m_grid->DisableDragRowSize();
  m_grid->SetRowLabelAlignment(wxALIGN_CENTER, wxALIGN_LEFT);
  m_grid->SetLabelFont(wxFont(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
  m_grid->SetColLabelValue(0, wxT("Start"));
  m_grid->SetColLabelValue(1, wxT("End"));
  m_grid->SetColLabelValue(2, wxT("Samples"));
  m_grid->SetColLabelValue(3, wxT("Duration"));
  m_grid->SetColLabelValue(4, wxT("Save"));
  m_grid->SetSelectionMode(wxGrid::wxGridSelectRows);
  m_grid->SetColFormatBool(4);
  m_grid->SetDefaultCellAlignment(wxALIGN_CENTRE, wxALIGN_CENTRE);
  m_grid->SetCellHighlightPenWidth(0);
  m_cueGrid = new wxGrid(this, CUE_GRID, wxDefaultPosition, wxDefaultSize);
  m_cueGrid->CreateGrid(0,3);
  m_cueGrid->SetRowLabelSize(wxGRID_AUTOSIZE);
  m_cueGrid->SetColLabelSize(25);
  m_cueGrid->SetColMinimalAcceptableWidth(50);
  m_cueGrid->DisableDragGridSize();
  m_cueGrid->DisableDragColSize();
  m_cueGrid->DisableDragRowSize();
  m_cueGrid->SetRowLabelAlignment(wxALIGN_CENTER, wxALIGN_LEFT);
  m_cueGrid->SetLabelFont(wxFont(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
  m_cueGrid->SetColLabelValue(0, wxT("ID"));
  m_cueGrid->SetColLabelValue(1, wxT("Position"));
  m_cueGrid->SetColLabelValue(2, wxT("Save"));
  m_cueGrid->SetSelectionMode(wxGrid::wxGridSelectRows);
  m_cueGrid->SetColFormatBool(2);
  m_cueGrid->SetDefaultCellAlignment(wxALIGN_CENTRE, wxALIGN_CENTRE);
  m_cueGrid->SetCellHighlightPenWidth(0);
  vbox->Add(fileNameLabel, 0, wxALIGN_LEFT, 0);
  vbox->Add(m_grid, 0, wxALIGN_CENTER | wxTOP| wxBOTTOM, 5);
  vbox->Add(m_cueGrid, 0, wxALIGN_CENTER | wxTOP | wxBOTTOM, 5);
  vbox->SetSizeHints(this);
  SetSizer(vbox);
}

MyPanel::~MyPanel() {

}

void MyPanel::SetFileNameLabel(wxString name) {
  fileNameLabel->SetLabel(wxT("Current open file: ") + name);
}

void MyPanel::FillRowWithLoopData(int loopStart, int loopEnd, int sampleRate, bool toSave, int index) {
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
  m_grid->SetCellValue(index, 4, boolValue);

  m_grid->ForceRefresh(); // not really working...
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
 
  m_cueGrid->SetCellValue(index, 2, boolValue);

  m_cueGrid->ForceRefresh(); // not really working...
}

void MyPanel::EmptyTable() {
  int gridRows = m_grid->GetNumberRows();
  m_grid->DeleteRows(0, gridRows, true);

  int cueGridRows = m_cueGrid->GetNumberRows();
  m_cueGrid->DeleteRows(0, cueGridRows, true);
}

