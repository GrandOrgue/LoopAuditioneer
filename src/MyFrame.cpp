/*
 * MyFrame.cpp is a part of LoopAuditioneer software
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

#include "MyFrame.h"
#include <wx/dirdlg.h>
#include <wx/dir.h>
#include "LoopAuditioneer.h"
#include "LoopAuditioneerDef.h"
#include <wx/gdicmn.h>
#include <wx/aboutdlg.h>
#include "LoopParametersDialog.h"
#include "BatchProcessDialog.h"

bool MyFrame::loopPlay = true; // default to loop play

// Event table
BEGIN_EVENT_TABLE(MyFrame, wxFrame)
  EVT_MENU(wxID_ABOUT, MyFrame::OnAbout)
  EVT_MENU(wxID_EXIT, MyFrame::OnQuit)
  EVT_MENU(FILE_SELECT, MyFrame::OnSelectDir)
  EVT_MENU(wxID_SAVE, MyFrame::OnSaveFile)
  EVT_MENU(wxID_SAVEAS, MyFrame::OnSaveFileAs)
  EVT_LISTBOX_DCLICK(ID_LISTBOX, MyFrame::OnDblClick)
  EVT_LISTBOX(ID_LISTBOX, MyFrame::OnSelection)
  EVT_TOOL(OPEN_SELECTED, MyFrame::OnDblClick)
  EVT_GRID_CMD_CELL_LEFT_CLICK(M_GRID, MyFrame::OnGridCellClick)
  EVT_GRID_CMD_CELL_LEFT_CLICK(CUE_GRID, MyFrame::OnCueGridCellClick)
  EVT_GRID_CMD_LABEL_LEFT_CLICK(M_GRID, MyFrame::OnGridCellClick)
  EVT_GRID_CMD_LABEL_LEFT_CLICK(CUE_GRID, MyFrame::OnCueGridCellClick)
  EVT_GRID_CMD_CELL_RIGHT_CLICK(M_GRID, MyFrame::OnLoopGridRightClick)
  EVT_GRID_CMD_LABEL_RIGHT_CLICK(M_GRID, MyFrame::OnLoopGridRightClick)
  EVT_TOOL(START_PLAYBACK, MyFrame::OnStartPlay)
  EVT_TOOL(wxID_STOP, MyFrame::OnStopPlay)
  EVT_TOOL(ADD_LOOP, MyFrame::OnAddLoop)
  EVT_TOOL(AUTOSEARCH_LOOPS, MyFrame::OnAutoLoop)
  EVT_TOOL(AUTOLOOP_SETTINGS, MyFrame::OnAutoLoopSettings)
  EVT_TOOL(BATCH_PROCESS, MyFrame::OnBatchProcess)
  EVT_TIMER(TIMER_ID, MyFrame::UpdatePlayPosition)
END_EVENT_TABLE()

void MyFrame::OnAbout(wxCommandEvent& event) {
  wxAboutDialogInfo info;
  info.SetName(appName);
  info.SetVersion(appVersion);
  info.SetDescription(wxT("This program allows users to listen to and evaluate loops and cues embedded in wav files."));
  info.SetCopyright(wxT("Copyright (C) 2011 Lars Palo <larspalo AT yahoo DOT se>\nReleased under GNU GPLv3 licence"));
  info.SetWebSite(wxT("http://sourceforge.net/projects/loopauditioneer/"));

  wxAboutBox(info);
}

void MyFrame::OnSelectDir(wxCommandEvent& event) {
  wxString defaultPath = wxT("/home/$USER");
  if (workingDir != wxEmptyString)
    defaultPath = workingDir;
  wxDirDialog dialog(::wxGetApp().frame, wxT("Choose a folder with samples"), defaultPath);
  if (dialog.ShowModal() == wxID_OK) {
    workingDir = dialog.GetPath();
    EmptyListOfFileNames();
    PopulateListOfFileNames();
    m_fileListBox->Set(fileNames);
    SetStatusText(workingDir, 1);
  }
}

void MyFrame::OnQuit(wxCommandEvent& event) {
  delete m_autoloopSettings;
  delete m_autoloop;
  // Destroy the frame
  Close();
}

void MyFrame::OnDblClick(wxCommandEvent& event) {
  int sel = m_fileListBox->GetSelection();
  if (sel != -1) {
    fileToOpen = m_fileListBox->GetString(sel);
    OpenAudioFile();
  }
}

void MyFrame::OnSelection(wxCommandEvent& event) {
  if (toolBar->GetToolEnabled(OPEN_SELECTED) == false)
    toolBar->EnableTool(OPEN_SELECTED, true);
  if (fileMenu->IsEnabled(OPEN_SELECTED) == false)
    fileMenu->Enable(OPEN_SELECTED, true);
}

void MyFrame::OpenAudioFile() {
  CloseOpenAudioFile();
  m_audiofile = new FileHandling(fileToOpen, workingDir);

  if (m_audiofile->FileCouldBeOpened()) {
    wxString filePath;
    filePath = workingDir.Append(wxT("/"));
    filePath += fileToOpen;
    m_waveform = new WaveformDrawer(this, filePath);
    vbox->Add(m_waveform, 1, wxEXPAND);
    Fit();

    m_sound->SetSampleRate(m_audiofile->GetSampleRate());
    m_sound->SetAudioFormat(m_audiofile->GetAudioFormat());
    m_sound->SetChannels(m_audiofile->m_channels);
    m_sound->OpenAudioStream();
    m_panel->SetFileNameLabel(fileToOpen);

    // populate the wxGrid in m_panel with the loop data and add it to the waveform drawer
    int sRate = m_audiofile->GetSampleRate();
    for (int i = 0; i < m_audiofile->m_loops->GetNumberOfLoops(); i++) {
      LOOPDATA tempData;
      m_audiofile->m_loops->GetLoopData(i, tempData);
      m_panel->FillRowWithLoopData(tempData.dwStart, tempData.dwEnd, sRate, tempData.shouldBeSaved, i);
      m_waveform->AddLoopPosition(tempData.dwStart, tempData.dwEnd);
    }

    // populate the wxGrid m_cueGrid in m_panel with the cue data and add it to the waveform drawer
    for (unsigned int i = 0; i < m_audiofile->m_cues->GetNumberOfCues(); i++) {
      CUEPOINT tempCue;
      m_audiofile->m_cues->GetCuePoint(i, tempCue);
      m_panel->FillRowWithCueData(tempCue.dwName, tempCue.dwSampleOffset, tempCue.keepThisCue, i);
      m_waveform->AddCuePosition(tempCue.dwSampleOffset);
    }

    UpdateAllViews();

    // enable save as...
    toolBar->EnableTool(wxID_SAVEAS, true);
    fileMenu->Enable(wxID_SAVEAS, true);

    toolBar->EnableTool(ADD_LOOP, true);
    toolMenu->Enable(ADD_LOOP, true);

    toolBar->EnableTool(AUTOSEARCH_LOOPS, true);
    toolMenu->Enable(AUTOSEARCH_LOOPS, true);

  } else {
    // libsndfile couldn't open the file or no audio data in file
    wxString message = wxT("Sorry, libsndfile couldn't open selected file!");
    wxMessageDialog *dialog = new wxMessageDialog(
      NULL,
      message,
      wxT("Error opening file"),
      wxOK | wxICON_ERROR
    );
    dialog->ShowModal();

    delete m_audiofile;
    m_audiofile = 0;

    m_panel->SetFileNameLabel(wxEmptyString);

  } // end of if file open was successful checking
}

void MyFrame::CloseOpenAudioFile() {
  toolBar->EnableTool(START_PLAYBACK, false);
  transportMenu->Enable(START_PLAYBACK, false);

  // disable save
  toolBar->EnableTool(wxID_SAVE, false);
  fileMenu->Enable(wxID_SAVE, false);

  // disable save as...
  toolBar->EnableTool(wxID_SAVEAS, false);
  fileMenu->Enable(wxID_SAVEAS, false);

  // disable add loop
  toolBar->EnableTool(ADD_LOOP, false);
  toolMenu->Enable(ADD_LOOP, false);
  toolBar->EnableTool(AUTOSEARCH_LOOPS, false);
  toolMenu->Enable(AUTOSEARCH_LOOPS, false);

  m_sound->CloseAudioStream();

  if (m_audiofile)
    m_panel->EmptyTable();

  delete m_audiofile;
  delete m_waveform;
  m_waveform = 0;

  UpdateAllViews();
}

void MyFrame::OnGridCellClick(wxGridEvent& event) {
  if (event.GetRow() >= 0) {
    m_panel->m_grid->ClearSelection();
    m_panel->m_cueGrid->ClearSelection();
    m_panel->m_grid->SelectRow(event.GetRow());

    // set the currently selected loops positions
    LOOPDATA currentLoop;
    m_audiofile->m_loops->GetLoopData(event.GetRow(), currentLoop);
    m_sound->SetLoopPosition(0, currentLoop.dwStart, currentLoop.dwEnd, m_audiofile->m_channels);

    if (event.GetCol() == 4) {
      // User clicked in the save column
      if (m_panel->m_grid->GetCellValue(event.GetRow(), event.GetCol()) == wxT("1")) {
        // Equals to a true value to begin with
        m_panel->m_grid->SetCellValue(event.GetRow(), event.GetCol(), wxT("0"));
        m_audiofile->m_loops->SetSaveOption(false, event.GetRow());
        toolBar->EnableTool(wxID_SAVE, true);
        fileMenu->Enable(wxID_SAVE, true);
      } else {
        // The value was false
        m_panel->m_grid->SetCellValue(event.GetRow(), event.GetCol(), wxT("1"));
        m_audiofile->m_loops->SetSaveOption(true, event.GetRow());
        toolBar->EnableTool(wxID_SAVE, true);
        fileMenu->Enable(wxID_SAVE, true);
      }
    }
    m_panel->m_grid->SetGridCursor(0, 4); // this is to hide the highlight box
    SetLoopPlayback(true); // set the playback to be for loops
    if (toolBar->GetToolEnabled(wxID_STOP) == false) {
      toolBar->EnableTool(START_PLAYBACK, true);
      transportMenu->Enable(START_PLAYBACK, true);
    }
  }
}

void MyFrame::OnCueGridCellClick(wxGridEvent& event) {
  if (event.GetRow() >= 0) {
    m_panel->m_grid->ClearSelection();
    m_panel->m_cueGrid->ClearSelection();
    m_panel->m_cueGrid->SelectRow(event.GetRow());

    // set the current position for the selected cue
    CUEPOINT currentCue;
    m_audiofile->m_cues->GetCuePoint(event.GetRow(), currentCue);
    m_sound->SetLoopPosition(currentCue.dwSampleOffset, currentCue.dwSampleOffset, currentCue.dwSampleOffset, m_audiofile->m_channels);

    SetLoopPlayback(false); // set the playback to not be for loops

    if (event.GetCol() == 2) {
      // User clicked in the save column
      if (m_panel->m_cueGrid->GetCellValue(event.GetRow(), event.GetCol()) == wxT("1")) {
        // Equals to a true value to begin with
        m_panel->m_cueGrid->SetCellValue(event.GetRow(), event.GetCol(), wxT("0"));
        m_audiofile->m_cues->SetSaveOption(false, event.GetRow());
        toolBar->EnableTool(wxID_SAVE, true);
        fileMenu->Enable(wxID_SAVE, true);
      } else {
        // The value was false
        m_panel->m_cueGrid->SetCellValue(event.GetRow(), event.GetCol(), wxT("1"));
        m_audiofile->m_cues->SetSaveOption(true, event.GetRow());
        toolBar->EnableTool(wxID_SAVE, true);
        fileMenu->Enable(wxID_SAVE, true);
      }
    }
    m_panel->m_cueGrid->SetGridCursor(event.GetRow(), 2); // this is to hide the highlight box and easily find selected row
    if (toolBar->GetToolEnabled(wxID_STOP) == false) {
      toolBar->EnableTool(START_PLAYBACK, true);
      transportMenu->Enable(START_PLAYBACK, true);
    }
  }
}

void MyFrame::OnSaveFile(wxCommandEvent& event) {
  m_audiofile->SaveAudioFile(fileToOpen, workingDir);
  toolBar->EnableTool(wxID_SAVE, false);
  fileMenu->Enable(wxID_SAVE, false);
}

void MyFrame::OnSaveFileAs(wxCommandEvent& event) {
  wxFileDialog *saveFileAsDialog = new wxFileDialog(this, wxT("Save file as..."), workingDir, fileToOpen, wxT("WAV files (*.wav)|*.wav"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

  if (saveFileAsDialog->ShowModal() == wxID_OK) {
    wxString fileName = saveFileAsDialog->GetFilename();
    wxString folderName = saveFileAsDialog->GetDirectory();
    m_audiofile->SaveAudioFile(fileName, folderName);

    // switch to the newly created file
    fileToOpen = fileName;
    workingDir = folderName;
    OpenAudioFile();

    // empty and repopulate file list in case user saved into the current directory
    EmptyListOfFileNames();
    PopulateListOfFileNames();
    m_fileListBox->Set(fileNames);
    SetStatusText(workingDir, 1);
  }
}

void MyFrame::OnStartPlay(wxCommandEvent& event) {
  m_timer.Start(50);
  // if it's a loop make sure start position is set to start of data
  if (m_panel->m_grid->IsSelection()) {
    m_sound->SetStartPosition(0, m_audiofile->m_channels);
    m_waveform->SetPlayPosition(0);
  }

  // if it's a cue make sure start position is set to the selected cue's dwPosition
  if (m_panel->m_cueGrid->IsSelection()) {
    CUEPOINT currentCue;
    m_audiofile->m_cues->GetCuePoint(m_panel->m_cueGrid->GetGridCursorRow(), currentCue);
    m_sound->SetStartPosition(currentCue.dwSampleOffset, m_audiofile->m_channels);
    m_waveform->SetPlayPosition(currentCue.dwSampleOffset);
  }

  toolBar->EnableTool(START_PLAYBACK, false);
  toolBar->EnableTool(wxID_STOP, true);
  transportMenu->Enable(START_PLAYBACK, false);
  transportMenu->Enable(wxID_STOP, true);
  m_sound->StartAudioStream();
}

void MyFrame::OnStopPlay(wxCommandEvent& event) {
  m_timer.Stop();
  DoStopPlay();
  m_waveform->SetPlayPosition(0);
  m_waveform->paintNow();
}

void MyFrame::DoStopPlay() {
  m_sound->StopAudioStream();

  toolBar->EnableTool(wxID_STOP, false);
  toolBar->EnableTool(START_PLAYBACK, true);

  transportMenu->Enable(START_PLAYBACK, true);
  transportMenu->Enable(wxID_STOP, false);
}

MyFrame::MyFrame(const wxString& title) : wxFrame(NULL, wxID_ANY, title), m_timer(this, TIMER_ID) {
  m_audiofile = NULL;
  m_waveform = NULL;
  m_sound = new MySound();
  m_autoloopSettings = new AutoLoopDialog(this);
  m_autoloop = new AutoLooping();

  workingDir = wxEmptyString;

  // Create a file menu
  fileMenu = new wxMenu();

  // Add file menu items
  fileMenu->Append(FILE_SELECT, wxT("&Choose folder"), wxT("Select working folder"));
  fileMenu->Append(OPEN_SELECTED, wxT("&Open file"), wxT("Open selected file"));
  fileMenu->Append(wxID_SAVE, wxT("&Save"), wxT("Save current file"));
  fileMenu->Append(wxID_SAVEAS, wxT("Save &as..."), wxT("Save current file with new name"));
  fileMenu->AppendSeparator();
  fileMenu->Append(AUTOLOOP_SETTINGS, wxT("&Autoloop settings"), wxT("Adjust settings for loop searching"));
  fileMenu->AppendSeparator();
  fileMenu->Append(wxID_EXIT, wxT("&Exit\tAlt-X"), wxT("Quit this program"));

  fileMenu->Enable(OPEN_SELECTED, false);
  fileMenu->Enable(wxID_SAVE, false);
  fileMenu->Enable(wxID_SAVEAS, false);

  // Create a transport menu
  transportMenu = new wxMenu();

  // Add transport menu items
  transportMenu->Append(START_PLAYBACK, wxT("&Play"), wxT("Start playback"));
  transportMenu->Append(wxID_STOP, wxT("&Stop"), wxT("Stop playback"));

  transportMenu->Enable(START_PLAYBACK, false);
  transportMenu->Enable(wxID_STOP, false);

  // Create a tool menu
  toolMenu = new wxMenu();

  // Add items to the tool menu
  toolMenu->Append(ADD_LOOP, wxT("&New loop"), wxT("Create a new loop"));
  toolMenu->Append(AUTOSEARCH_LOOPS, wxT("&Autoloop"), wxT("Search for loop(s)"));
  toolMenu->Append(BATCH_PROCESS, wxT("&Batch processing"), wxT("Batch processing of files"));

  toolMenu->Enable(ADD_LOOP, false);
  toolMenu->Enable(AUTOSEARCH_LOOPS, false);

  // Create a help menu
  helpMenu = new wxMenu();

  // Add help menu items
  helpMenu->Append(wxID_ABOUT, wxT("&About...\tF1"), wxT("Show about dialog"));

  // Create a menu bar and append the menus to it
  menuBar = new wxMenuBar();
  menuBar->Append(fileMenu, wxT("&File"));
  menuBar->Append(transportMenu, wxT("&Transport"));
  menuBar->Append(toolMenu, wxT("T&ools"));
  menuBar->Append(helpMenu, wxT("&Help"));

  // Attach menu bar to frame
  SetMenuBar(menuBar);

  // Create Status bar
  CreateStatusBar(2);
  SetStatusText(wxT("Ready"), 0);

  // Create toolbar
  toolBar = CreateToolBar(wxNO_BORDER | wxTB_HORIZONTAL | wxTB_FLAT);
  toolBar->SetToolBitmapSize(wxSize(24, 24));
  wxImage::AddHandler(new wxPNGHandler);
  wxBitmap selectFolder(wxT("../icons/24x24/Open_folder.png"), wxBITMAP_TYPE_PNG);
  wxBitmap openSelectedFile(wxT("../icons/24x24/Open_file.png"), wxBITMAP_TYPE_PNG);
  wxBitmap saveFile(wxT("../icons/24x24/Save.png"), wxBITMAP_TYPE_PNG);
  wxBitmap saveFileAs(wxT("../icons/24x24/Save_as.png"), wxBITMAP_TYPE_PNG);
  wxBitmap startPlayback(wxT("../icons/24x24/Right.png"), wxBITMAP_TYPE_PNG);
  wxBitmap stopPlayback(wxT("../icons/24x24/Stop.png"), wxBITMAP_TYPE_PNG);
  wxBitmap loopCreation(wxT("../icons/24x24/Refresh.png"), wxBITMAP_TYPE_PNG);
  wxBitmap autoLoop(wxT("../icons/24x24/Search.png"), wxBITMAP_TYPE_PNG);
  wxBitmap autoLoopSettings(wxT("../icons/24x24/Yin-Yang.png"), wxBITMAP_TYPE_PNG);
  wxBitmap batchProcess(wxT("../icons/24x24/Gear.png"), wxBITMAP_TYPE_PNG);
  toolBar->AddTool(FILE_SELECT, selectFolder, wxT("Select working folder"), wxT("Select working folder"));
  toolBar->AddTool(OPEN_SELECTED, openSelectedFile, wxT("Open selected file"), wxT("Open selected file"));
  toolBar->AddTool(wxID_SAVE, saveFile, wxT("Save file"), wxT("Save file"));
  toolBar->AddTool(wxID_SAVEAS, saveFileAs, wxT("Save as..."), wxT("Save file with new name"));
  toolBar->AddTool(START_PLAYBACK, startPlayback, wxT("Play"), wxT("Play"));
  toolBar->AddTool(wxID_STOP, stopPlayback, wxT("Stop"), wxT("Stop"));
  toolBar->AddTool(ADD_LOOP, loopCreation, wxT("New loop"), wxT("Create a new loop"));
  toolBar->AddTool(AUTOSEARCH_LOOPS, autoLoop, wxT("Autoloop"), wxT("Search for loop(s)"));
  toolBar->AddTool(AUTOLOOP_SETTINGS, autoLoopSettings, wxT("Autoloop settings"), wxT("Change settings for auto loopsearching"));
  toolBar->AddTool(BATCH_PROCESS, batchProcess, wxT("Batch processing"), wxT("Batch processing of files/folders"));
  toolBar->Realize();
  toolBar->EnableTool(OPEN_SELECTED, false);
  toolBar->EnableTool(wxID_SAVE, false);
  toolBar->EnableTool(wxID_SAVEAS, false);
  toolBar->EnableTool(START_PLAYBACK, false);
  toolBar->EnableTool(wxID_STOP, false);
  toolBar->EnableTool(ADD_LOOP, false);
  toolBar->EnableTool(AUTOSEARCH_LOOPS, false);
  this->SetToolBar(toolBar);

  // Create panels for frame content

  vbox = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);

  m_fileListBox = new wxListBox(
    this,
    ID_LISTBOX,
    wxDefaultPosition,
    wxSize(200,200),
    fileNames,
    wxLB_SINGLE | wxLB_SORT
  );

  hbox->Add(m_fileListBox, 1, wxEXPAND | wxLEFT | wxTOP | wxBOTTOM, 10);

  m_panel = new MyPanel(this);

  hbox->Add(m_panel, 3, wxEXPAND | wxALL, 10);

  vbox->Add(hbox, 1, wxEXPAND);
  vbox->SetSizeHints(this);
  SetSizer(vbox);
  // Fit();
  // Center();

  SetBackgroundColour(wxT("#f4f2ef"));
}

MyFrame::~MyFrame() {
  if (m_audiofile) {
    delete m_audiofile;
    m_audiofile = 0;
  }
  if (m_sound) {
    delete m_sound;
    m_sound = 0;
  }
  if (m_waveform) {
    delete m_waveform;
    m_sound = 0;
  }
  if (m_autoloopSettings)
    delete m_autoloopSettings;
  if (m_autoloop)
    delete m_autoloop;
}

void MyFrame::EmptyListOfFileNames() {
  fileNames.Empty();
}

void MyFrame::AddFileName(wxString fileName) {
  fileNames.Add(fileName);
}

void MyFrame::PopulateListOfFileNames() {
  wxDir dir(workingDir);

  if (!dir.IsOpened()) {
    wxMessageDialog *dial = new wxMessageDialog(NULL, wxT("Couldn't open folder"), wxT("Error"), wxOK | wxICON_ERROR);
    dial->ShowModal();
    return;
  }

  wxString fileName;

  bool search = dir.GetFirst(&fileName, wxT("*.wav"), wxDIR_FILES);
  while (search) {
    AddFileName(fileName);
    search = dir.GetNext(&fileName);
  }

  search = dir.GetFirst(&fileName, wxT("*.WAV"), wxDIR_FILES);
  while (search) {
    AddFileName(fileName);
    search = dir.GetNext(&fileName);
  }

  // Remove double entries as Windows is case insensitive...
  fileNames.Sort();
  size_t lineCounter = 0;
  while (lineCounter < fileNames.GetCount() - 1) {
    if (fileNames[lineCounter] == fileNames[lineCounter + 1])
      fileNames.RemoveAt(lineCounter + 1);
    else
      lineCounter++;
  }
}

int MyFrame::AudioCallback(void *outputBuffer,
                           void *inputBuffer,
                           unsigned int nBufferFrames,
                           double streamTime,
                           RtAudioStreamStatus status,
                           void *userData ) {
  int nChannels = ::wxGetApp().frame->m_audiofile->m_channels;

  // First get what type of AudioData should we work with
  if (::wxGetApp().frame->m_audiofile->shortAudioData) {
    short *buffer = static_cast<short*>(outputBuffer);

    // keep track of position, see pos in MySound.h
    unsigned int *position = (unsigned int *) userData;

    // Loop that feeds the outputBuffer with data
    if (position[0] < ::wxGetApp().frame->m_audiofile->ArrayLength) {
      for (unsigned int i = 0; i < nBufferFrames * nChannels; i++) {
        *buffer++ = ::wxGetApp().frame->m_audiofile->shortAudioData[(position[0])];
        position[0] += 1;

        if (loopPlay) {
          // Check to control loop playback, see MySound.h pos member and MyFrame.h loopPlay member
          if (position[0] > position[2])
            position[0] = position[1];
        }

        // stop if end of file data is reached and reset current position to start of the cue
        if (position[0] > ::wxGetApp().frame->m_audiofile->ArrayLength - 1) {
          wxCommandEvent evt(wxEVT_COMMAND_TOOL_CLICKED, wxID_STOP);
          ::wxGetApp().frame->AddPendingEvent(evt);

          return 0;
        }
      }
    } else {
      // we end up here until buffer is drained?
    }

    return 0;
  } else if (::wxGetApp().frame->m_audiofile->intAudioData) {
    int *buffer = static_cast<int*>(outputBuffer);

    // keep track of position, see pos in MySound.h
    unsigned int *position = (unsigned int *) userData;

    // Loop that feeds the outputBuffer with data
    if (position[0] < ::wxGetApp().frame->m_audiofile->ArrayLength) {
      for (unsigned int i = 0; i < nBufferFrames * nChannels; i++) {
        *buffer++ = ::wxGetApp().frame->m_audiofile->intAudioData[(position[0])];
        position[0] += 1;

        if (loopPlay) {
          // Check to control loop playback, see MySound.h pos member and MyFrame.h loopPlay member
          if (position[0] > position[2])
          position[0] = position[1];
        }

        // stop if end of file data is reached and reset current position to start of the cue
        if (position[0] > ::wxGetApp().frame->m_audiofile->ArrayLength - 1) {
          wxCommandEvent evt(wxEVT_COMMAND_TOOL_CLICKED, wxID_STOP);
          ::wxGetApp().frame->AddPendingEvent(evt);

          return 0;
        }
      }
    } else {
      // we end up here until buffer is drained?
    }

    return 0;
  } else if (::wxGetApp().frame->m_audiofile->doubleAudioData) {
    double *buffer = static_cast<double*>(outputBuffer);

    // keep track of position, see pos in MySound.h
    unsigned int *position = (unsigned int *) userData;

    // Loop that feeds the outputBuffer with data
    if (position[0] < ::wxGetApp().frame->m_audiofile->ArrayLength) {
      for (unsigned int i = 0; i < nBufferFrames * nChannels; i++) {
        *buffer++ = ::wxGetApp().frame->m_audiofile->doubleAudioData[(position[0])];
        position[0] += 1;

        if (loopPlay) {
          // Check to control loop playback, see MySound.h pos member and MyFrame.h loopPlay member
          if (position[0] > position[2])
            position[0] = position[1];
        }

        // stop if end of file data is reached and reset current position to start of the cue
        if (position[0] > ::wxGetApp().frame->m_audiofile->ArrayLength - 1) {
          wxCommandEvent evt(wxEVT_COMMAND_TOOL_CLICKED, wxID_STOP);
          ::wxGetApp().frame->AddPendingEvent(evt);

          return 0;
        }
      }
    }  else {
      // we end up here until buffer is drained?
    }

    return 0;
  } else {
    // There is no audio data to play!
    return 2;
  }

}

void MyFrame::SetLoopPlayback(bool looping) {
  if (looping)
    loopPlay = true;
  else
    loopPlay = false;
}

void MyFrame::UpdatePlayPosition(wxTimerEvent& evt) {
  if (m_waveform) {
    m_waveform->SetPlayPosition(m_sound->pos[0] / m_audiofile->m_channels);
    m_waveform->paintNow();
  }
}

void MyFrame::AddNewCue(unsigned int offset) {
  CUEPOINT newCue;
  newCue.dwName = m_audiofile->m_cues->GetNumberOfCues(); // this should be the new cues index
  newCue.dwPosition = 0;
  newCue.fccChunk = 1635017060; // value for data chunk
  newCue.dwChunkStart = 0;
  newCue.dwBlockStart = 0;
  newCue.dwSampleOffset = offset;
  newCue.keepThisCue = true;

  m_audiofile->m_cues->AddCue(newCue); // add the cue to the file cue vector
  m_panel->FillRowWithCueData(newCue.dwName, newCue.dwSampleOffset, newCue.keepThisCue, newCue.dwName);
  m_waveform->AddCuePosition(newCue.dwSampleOffset);

  UpdateAllViews();

  toolBar->EnableTool(wxID_SAVE, true);
  fileMenu->Enable(wxID_SAVE, true);
}

void MyFrame::ChangeCuePosition(unsigned int offset, int index) {
  m_audiofile->m_cues->ChangePosition(offset, index);
  m_panel->ChangeCueData(offset, index);

  UpdateAllViews();

  toolBar->EnableTool(wxID_SAVE, true);
  fileMenu->Enable(wxID_SAVE, true);
}

void MyFrame::OnAddLoop(wxCommandEvent& event) {
  LoopParametersDialog loopDialog(0, m_audiofile->ArrayLength / m_audiofile->m_channels, m_audiofile->ArrayLength / m_audiofile->m_channels, this);

  if (loopDialog.ShowModal() == wxID_OK) {
    unsigned int loopStartSample = loopDialog.GetLoopStart();
    unsigned int loopEndSample = loopDialog.GetLoopEnd();

    // Add the new loop to the loop vector
    LOOPDATA newLoop;
    newLoop.dwType = SF_LOOP_FORWARD;
    newLoop.dwStart = loopStartSample;
    newLoop.dwEnd = loopEndSample;
    newLoop.dwPlayCount = 0;
    newLoop.shouldBeSaved = true;
    m_audiofile->m_loops->AddLoop(newLoop);

    // Add the new loop to the grid
    int sRate = m_audiofile->GetSampleRate();
    m_panel->FillRowWithLoopData(newLoop.dwStart, newLoop.dwEnd, sRate, newLoop.shouldBeSaved, m_audiofile->m_loops->GetNumberOfLoops() - 1);

    // Add the new loop to waveform drawer
    m_waveform->AddLoopPosition(newLoop.dwStart, newLoop.dwEnd);

    UpdateAllViews();
  }
}

void MyFrame::OnLoopGridRightClick(wxGridEvent& event) {
  if (event.GetRow() >= 0) {
    m_panel->m_grid->ClearSelection();
    m_panel->m_cueGrid->ClearSelection();
    m_panel->m_grid->SelectRow(event.GetRow());

    // set the currently selected loops positions
    LOOPDATA currentLoop;
    int index = event.GetRow();
    m_audiofile->m_loops->GetLoopData(index, currentLoop);

    m_panel->m_grid->SetGridCursor(0, 4); // this is to hide the highlight box
    SetLoopPlayback(true); // set the playback to be for loops
    if (toolBar->GetToolEnabled(wxID_STOP) == false) {
      toolBar->EnableTool(START_PLAYBACK, true);
      transportMenu->Enable(START_PLAYBACK, true);
    }

    LoopParametersDialog loopDialog(currentLoop.dwStart, currentLoop.dwEnd, m_audiofile->ArrayLength / m_audiofile->m_channels, this);

    if (loopDialog.ShowModal() == wxID_OK) {
      unsigned int loopStartSample = loopDialog.GetLoopStart();
      unsigned int loopEndSample = loopDialog.GetLoopEnd();

      // Store the changes to the loop in the loop vector
      m_audiofile->m_loops->SetLoopPositions(loopStartSample, loopEndSample, index);
      m_audiofile->m_loops->GetLoopData(index, currentLoop);

      // Change loop to the grid
      int sRate = m_audiofile->GetSampleRate();
      m_panel->ChangeLoopData(currentLoop.dwStart, currentLoop.dwEnd, sRate, index);

      // Change loop in waveform drawer
      m_waveform->ChangeLoopPositions(currentLoop.dwStart, currentLoop.dwEnd, index);

      // Set loops positions for playback
      m_sound->SetLoopPosition(0, currentLoop.dwStart, currentLoop.dwEnd, m_audiofile->m_channels);

      UpdateAllViews();
    }
  }
}

void MyFrame::UpdateAllViews() {
  // force updates of wxGrids in m_panel by jiggling the size of the frame! Ugly hack necessary for Windows!
  wxSize size = GetSize();
  size.IncBy(1, 1);
  SetSize(size);
  size.DecBy(1, 1);
  SetSize(size);

  if (m_waveform) {
    m_waveform->Refresh();
    m_waveform->Update();
  }
}

void MyFrame::OnBatchProcess(wxCommandEvent& event) {
  BatchProcessDialog batchDialog(
    this,
    wxID_ANY,
    wxT("Batch processing"),
    wxDefaultPosition,
    wxDefaultSize
  );
  batchDialog.ShowModal();
}

void MyFrame::OnAutoLoop(wxCommandEvent& event) {
  // prepare a vector to receive the loops
  std::vector<std::pair<std::pair<unsigned, unsigned>, double> > loops;

  // get the audio data as doubles from m_waveform
  double *audioData = new double[m_audiofile->ArrayLength];
  bool gotData = m_waveform->GetDoubleAudioData(audioData, m_audiofile->ArrayLength);

  if (gotData) {
    // this is the call to search for loops
    bool foundSomeLoops = m_autoloop->AutoFindLoops(
      audioData,
      m_audiofile->ArrayLength,
      m_audiofile->m_channels,
      m_audiofile->GetSampleRate(),
      loops,
      m_autoloopSettings->GetAutosearch(),
      m_autoloopSettings->GetStart(),
      m_autoloopSettings->GetEnd()
    );

    if (foundSomeLoops) {
      for (int i = 0; i < loops.size(); i++) {
        // Add the new loop to the loop vector
        LOOPDATA newLoop;
        newLoop.dwType = SF_LOOP_FORWARD;
        newLoop.dwStart = loops[i].first.first;
        newLoop.dwEnd = loops[i].first.second;
        newLoop.dwPlayCount = 0;
        newLoop.shouldBeSaved = true;
        m_audiofile->m_loops->AddLoop(newLoop);

        // Add the new loop to the grid
        m_panel->FillRowWithLoopData(
          newLoop.dwStart,
          newLoop.dwEnd,
          m_audiofile->GetSampleRate(),
          newLoop.shouldBeSaved,
          m_audiofile->m_loops->GetNumberOfLoops() - 1
        );

        // Add the new loop to waveform drawer
        m_waveform->AddLoopPosition(newLoop.dwStart, newLoop.dwEnd);
      }
      UpdateAllViews();
    } else {
      // no loops found!
      wxString message = wxT("Sorry, didn't find any loops!");
      wxMessageDialog *dialog = new wxMessageDialog(
        NULL,
        message,
        wxT("No loops found!"),
        wxOK | wxICON_ERROR
      );
      dialog->ShowModal();
    }
  } else {
    // couldn't get audio data as doubles!
    wxString message = wxT("Sorry, couldn't get the audio data!");
    wxMessageDialog *dialog = new wxMessageDialog(
      NULL,
      message,
      wxT("Error getting audio data"),
      wxOK | wxICON_ERROR
    );
    dialog->ShowModal();
  }
  delete[] audioData;
}

void MyFrame::OnAutoLoopSettings(wxCommandEvent& event) {
  if (m_autoloopSettings->ShowModal() == wxID_OK) {
    // Update AutoLooping object
    m_autoloop->SetThreshold(m_autoloopSettings->GetThreshold());
    m_autoloop->SetDuration(m_autoloopSettings->GetDuration());
    m_autoloop->SetBetween(m_autoloopSettings->GetBetween());
    m_autoloop->SetQuality(m_autoloopSettings->GetQuality());
    m_autoloop->SetCandidates(m_autoloopSettings->GetCandidates());
    m_autoloop->SetLoops(m_autoloopSettings->GetNrLoops());
    m_autoloop->SetMultiple(m_autoloopSettings->GetMultiple());
  }
}

