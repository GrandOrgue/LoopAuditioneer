/* 
 * MyFrame.h is a part of LoopAuditioneer software
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

#ifndef MYFRAME_H
#define MYFRAME_H

#include <wx/wx.h>
#include <wx/toolbar.h>
#include <wx/listbox.h>
#include "MyPanel.h"
#include "FileHandling.h"
#include "MySound.h"

class MyFrame : public wxFrame {
public:
  // Constructor
  MyFrame(const wxString& title);
  ~MyFrame();

  // Event Handlers
  void OnQuit(wxCommandEvent& event);
  void OnAbout(wxCommandEvent& event);
  void OnSelectDir(wxCommandEvent& event);
  void OnDblClick(wxCommandEvent& event);
  void OnSelection(wxCommandEvent& event);
  void OnGridCellClick(wxGridEvent& event);
  void OnCueGridCellClick(wxGridEvent& event);
  void OnSaveFile(wxCommandEvent& event);
  void OnStartPlay(wxCommandEvent& event);
  void OnStopPlay(wxCommandEvent& event);
  void DoStopPlay();

  wxListBox *m_fileListBox;
  MyPanel *m_panel;
  wxToolBar *toolBar;

  void EmptyListOfFileNames();
  void AddFileName(wxString fileName);
  void OpenAudioFile();
  void CloseOpenAudioFile();

  static int AudioCallback(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                    double streamTime, RtAudioStreamStatus status, void *userData );

  static void SetLoopPlayback(bool looping);

private:
  // This class handles events
  DECLARE_EVENT_TABLE()

  wxString workingDir;
  wxArrayString fileNames;
  wxString fileToOpen;
  FileHandling *m_audiofile;
  MySound *m_sound;

  void PopulateListOfFileNames();

  static bool loopPlay;
};

#endif

