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

#include "LoopAuditioneer.h"
#include "LoopAuditioneerDef.h"

IMPLEMENT_APP(LoopAuditioneerApp)

// This initializes the application
bool LoopAuditioneerApp::OnInit() {
  // Create the frame window
  wxString fullAppName = wxEmptyString;
  fullAppName.Append(appName);
  fullAppName.Append(wxT(" "));
  fullAppName.Append(appVersion);
  frame = new MyFrame(fullAppName);

  // load icons
  m_icons = wxIconBundle(wxIcon(wxT("icons/LoopyIcon-16.png"), wxBITMAP_TYPE_PNG));
  m_icons.AddIcon(wxIcon(wxT("icons/LoopyIcon-24.png"), wxBITMAP_TYPE_PNG));
  m_icons.AddIcon(wxIcon(wxT("icons/LoopyIcon-32.png"), wxBITMAP_TYPE_PNG));
  m_icons.AddIcon(wxIcon(wxT("icons/LoopyIcon-48.png"), wxBITMAP_TYPE_PNG));

  frame->SetIcons(m_icons);

  // Show the frame
  frame->Show(true);

  // Start the event loop
  return true;
}

