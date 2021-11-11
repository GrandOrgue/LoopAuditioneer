/* 
 * LoopAuditioneer is a tool for evaluating loops (and cues) in wav files
 * especially useful for samples intended for organ samplesets
 * Copyright (C) 2011-2021 Lars Palo 
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
#include "wx/fs_zip.h"
#include "wx/image.h"
#include <wx/filename.h>
#include <wx/stdpaths.h>

IMPLEMENT_APP(LoopAuditioneerApp)

// This initializes the application
bool LoopAuditioneerApp::OnInit() {
  // Create the frame window
  wxString fullAppName = wxEmptyString;
  fullAppName.Append(appName);
  fullAppName.Append(wxT(" "));
  fullAppName.Append(wxT(MY_APP_VERSION));
  frame = new MyFrame(fullAppName);

  frame->SetWindowStyle(wxDEFAULT_FRAME_STYLE | wxWANTS_CHARS);
  
  wxFileName fn(wxStandardPaths::Get().GetExecutablePath());
  fn = fn.GetPath();
  wxString BaseDir = fn.GetPath();
  wxString ResourceDir = BaseDir + wxFILE_SEP_PATH + wxT("share");

  // the help controller
  wxImage::AddHandler(new wxJPEGHandler);
  wxFileSystem::AddHandler(new wxZipFSHandler);
  m_helpController = new wxHtmlHelpController();
  m_helpController->AddBook(wxFileName(ResourceDir + wxFILE_SEP_PATH + wxT("LoopAuditioneer/help/help.zip")));
  m_helpController->SetFrameParameters(wxT("%s"), wxDefaultSize, wxDefaultPosition); 	

  // load icons
  m_icons = wxIconBundle(wxIcon(ResourceDir + wxFILE_SEP_PATH + wxT("icons/hicolor/16x16/apps/LoopAuditioneer.png"), wxBITMAP_TYPE_PNG));
  m_icons.AddIcon(wxIcon(ResourceDir + wxFILE_SEP_PATH + wxT("icons/hicolor/24x24/apps/LoopAuditioneer.png"), wxBITMAP_TYPE_PNG));
  m_icons.AddIcon(wxIcon(ResourceDir + wxFILE_SEP_PATH + wxT("icons/hicolor/32x32/apps/LoopAuditioneer.png"), wxBITMAP_TYPE_PNG));
  m_icons.AddIcon(wxIcon(ResourceDir + wxFILE_SEP_PATH + wxT("icons/hicolor/48x48/apps/LoopAuditioneer.png"), wxBITMAP_TYPE_PNG));
  m_icons.AddIcon(wxIcon(ResourceDir + wxFILE_SEP_PATH + wxT("icons/hicolor/64x64/apps/LoopAuditioneer.png"), wxBITMAP_TYPE_PNG));
  m_icons.AddIcon(wxIcon(ResourceDir + wxFILE_SEP_PATH + wxT("icons/hicolor/128x128/apps/LoopAuditioneer.png"), wxBITMAP_TYPE_PNG));
  m_icons.AddIcon(wxIcon(ResourceDir + wxFILE_SEP_PATH + wxT("icons/hicolor/256x256/apps/LoopAuditioneer.png"), wxBITMAP_TYPE_PNG));
  m_icons.AddIcon(wxIcon(ResourceDir + wxFILE_SEP_PATH + wxT("icons/hicolor/512x512/apps/LoopAuditioneer.png"), wxBITMAP_TYPE_PNG));
  m_icons.AddIcon(wxIcon(ResourceDir + wxFILE_SEP_PATH + wxT("icons/hicolor/1024x1024/apps/LoopAuditioneer.png"), wxBITMAP_TYPE_PNG));

  frame->SetIcons(m_icons);

  // Show the frame
  frame->Show(true);

  // Start the event loop
  return true;
}

int LoopAuditioneerApp::OnExit() {
  delete m_helpController;
  return wxApp::OnExit();
}

