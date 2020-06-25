/* 
 * MyListCtrl.h is a part of LoopAuditioneer software
 * Copyright (C) 2014-2020 Lars Palo 
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

#ifndef MYLISTCTRL_H
#define MYLISTCTRL_H

#include <wx/listctrl.h>

class MyListCtrl : public wxListCtrl {
public:
  // Constructors
  MyListCtrl();
  MyListCtrl(
    wxWindow* parent,
    wxWindowID id,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxLC_REPORT | wxLC_SINGLE_SEL
  );
  ~MyListCtrl();

  // Two step creation
  bool Create(
    wxWindow* parent,
    wxWindowID id,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxLC_REPORT | wxLC_SINGLE_SEL
  );

  // Event handler
  void OnKeyDown(wxKeyEvent& event);

private:
  // This class handles events
  DECLARE_EVENT_TABLE()
};

#endif
