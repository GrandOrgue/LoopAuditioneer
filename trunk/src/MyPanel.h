/* 
 * MyPanel.h is a part of LoopAuditioneer software
 * Copyright (C) 2011-2020 Lars Palo 
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

#ifndef MYPANEL_H
#define MYPANEL_H

#include <wx/wx.h>
#include <wx/stattext.h>
#include <wx/grid.h>
#include <wx/filename.h>

class MyPanel : public wxScrolledWindow {
public:
  MyPanel(wxFrame *parent);
  ~MyPanel();

  wxStaticText *fileNameLabel;
  wxBoxSizer *vbox;
  wxGrid *m_grid;
  wxGrid *m_cueGrid;
  void SetFileNameLabel(wxFileName fullPath);
  void EmptyTable();
  void FillRowWithLoopData(int loopStart, int loopEnd, int sampleRate, bool toSave, int index);
  void FillRowWithCueData(unsigned int id, unsigned int position, bool save, int index);
  void ChangeCueData(unsigned int offset, int index);
  void ChangeLoopData(int loopStart, int loopEnd, int sampleRate, int index);
  void OnKeyDown(wxKeyEvent& event);

private:
  DECLARE_EVENT_TABLE()
};

#endif
