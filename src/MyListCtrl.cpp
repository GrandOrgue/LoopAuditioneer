/* 
 * MyListCtrl.cpp is a part of LoopAuditioneer software
 * Copyright (C) 2014-2024 Lars Palo 
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

#include "MyListCtrl.h"
#include "MyFrame.h"

// Event table
BEGIN_EVENT_TABLE(MyListCtrl, wxListCtrl)
  EVT_KEY_DOWN(MyListCtrl::OnKeyDown)
  EVT_CHAR(MyListCtrl::OnKeyDown)
END_EVENT_TABLE()

MyListCtrl::MyListCtrl() {
}

MyListCtrl::MyListCtrl(
  wxWindow* parent,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size,
  long style
) {
  Create(parent, id, pos, size, style);
}

MyListCtrl::~MyListCtrl() {
}

bool MyListCtrl::Create(
  wxWindow* parent,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size,
  long style
) {
  if (!wxListCtrl::Create(parent, id, pos, size, style))
    return false;

  return true;
}

void MyListCtrl::OnKeyDown(wxKeyEvent& event) {
  MyFrame *myParent = (MyFrame *) GetParent();
  myParent->OnKeyboardInput(event);
}
