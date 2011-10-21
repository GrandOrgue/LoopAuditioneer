/* 
 * LoopAuditioneerDef.h contains global constants
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

#ifndef LOOPAUDITIONEERDEF_H
#define LOOPAUDITIONEERDEF_H

#include <wx/wx.h>

enum {
  ID_LISTBOX = wxID_HIGHEST + 1,
  FILE_SELECT = wxID_HIGHEST + 2,
  M_GRID = wxID_HIGHEST + 3,
  OPEN_SELECTED = wxID_HIGHEST + 4,
  START_PLAYBACK = wxID_HIGHEST + 5,
  CUE_GRID = wxID_HIGHEST + 6,
  TIMER_ID = wxID_HIGHEST + 7,
  ADD_CUE = wxID_HIGHEST + 8,
  ADD_LOOP = wxID_HIGHEST + 9,
  BATCH_PROCESS = wxID_HIGHEST + 10,
  AUTOSEARCH_LOOPS = wxID_HIGHEST + 11,
  AUTOLOOP_SETTINGS = wxID_HIGHEST + 12
};

const wxString appName = wxT("LoopAuditioneer");
const wxString appVersion = wxT("0.6.0.0");

#endif
