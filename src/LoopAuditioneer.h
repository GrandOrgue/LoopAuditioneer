/* 
 * LoopAuditioneer is a tool for evaluating loops (and cues) in wav files
 * especially useful for samples intended for organ samplesets
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

#ifndef LOOPAUDITIONEER_H
#define LOOPAUDITIONEER_H

#include <wx/wx.h>
#include "MyFrame.h"

class LoopAuditioneerApp : public wxApp {
public:
  virtual bool OnInit();
  int OnExit();
  MyFrame *frame;
  wxIconBundle m_icons;
};

DECLARE_APP(LoopAuditioneerApp)

#endif

