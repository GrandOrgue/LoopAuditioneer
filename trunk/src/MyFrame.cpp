/*
 * MyFrame.cpp is a part of LoopAuditioneer software
 * Copyright (C) 2011-2015 Lars Palo
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
#include <climits>
#include "PitchDialog.h"
#include "LoopOverlay.h"
#include <wx/busyinfo.h>
#include "sndfile.hh"

bool MyFrame::loopPlay = true; // default to loop play
int MyFrame::volumeMultiplier = 1; // default value

// Event table
BEGIN_EVENT_TABLE(MyFrame, wxFrame)
  EVT_CLOSE(MyFrame::OnClose)
  EVT_MENU(wxID_ABOUT, MyFrame::OnAbout)
  EVT_MENU(wxID_EXIT, MyFrame::OnQuit)
  EVT_MENU(FILE_SELECT, MyFrame::OnSelectDir)
  EVT_MENU(wxID_SAVE, MyFrame::OnSaveFile)
  EVT_MENU(wxID_SAVEAS, MyFrame::OnSaveFileAs)
  EVT_MENU(EDIT_LOOP, MyFrame::OnEditLoop)
  EVT_MENU(VIEW_LOOPPOINTS, MyFrame::OnViewLoop)
  EVT_MENU(LOOP_ONLY, MyFrame::OnLoopPlayback)
  EVT_MENU(SAVE_AND_OPEN_NEXT, MyFrame::OnSaveOpenNext)
  EVT_MENU(wxID_HELP, MyFrame::OnHelp)
  EVT_LIST_ITEM_ACTIVATED(ID_LISTCTRL, MyFrame::OnDblClick)
  EVT_LIST_ITEM_SELECTED(ID_LISTCTRL, MyFrame::OnSelection)
  EVT_TOOL(OPEN_SELECTED, MyFrame::OnOpenSelected)
  EVT_GRID_CMD_CELL_LEFT_CLICK(M_GRID, MyFrame::OnGridCellClick)
  EVT_GRID_CMD_CELL_LEFT_CLICK(CUE_GRID, MyFrame::OnCueGridCellClick)
  EVT_GRID_CMD_LABEL_LEFT_CLICK(M_GRID, MyFrame::OnGridCellClick)
  EVT_GRID_CMD_LABEL_LEFT_CLICK(CUE_GRID, MyFrame::OnCueGridCellClick)
  EVT_GRID_CMD_CELL_RIGHT_CLICK(M_GRID, MyFrame::OnLoopGridRightClick)
  EVT_GRID_CMD_LABEL_RIGHT_CLICK(M_GRID, MyFrame::OnLoopGridRightClick)
  EVT_GRID_CMD_SELECT_CELL(M_GRID, MyFrame::OnGridCellSelect)
  EVT_GRID_CMD_SELECT_CELL(CUE_GRID, MyFrame::OnCueGridCellSelect)
  EVT_TOOL(START_PLAYBACK, MyFrame::OnStartPlay)
  EVT_TOOL(wxID_STOP, MyFrame::OnStopPlay)
  EVT_TOOL(ADD_LOOP, MyFrame::OnAddLoop)
  EVT_TOOL(AUTOSEARCH_LOOPS, MyFrame::OnAutoLoop)
  EVT_TOOL(AUTOLOOP_SETTINGS, MyFrame::OnAutoLoopSettings)
  EVT_TOOL(BATCH_PROCESS, MyFrame::OnBatchProcess)
  EVT_TOOL(PITCH_SETTINGS, MyFrame::OnPitchSettings)
  EVT_TOOL(ZOOM_IN_AMP, MyFrame::OnZoomInAmplitude)
  EVT_TOOL(ZOOM_OUT_AMP, MyFrame::OnZoomOutAmplitude)
  EVT_TIMER(TIMER_ID, MyFrame::UpdatePlayPosition)
  EVT_SLIDER(ID_VOLUME_SLIDER, MyFrame::OnVolumeSlider)
  EVT_TOOL(X_FADE, MyFrame::OnCrossfade)
  EVT_TOOL(CUT_N_FADE, MyFrame::OnCutFade)
  EVT_KEY_DOWN(MyFrame::OnKeyboardInput)
END_EVENT_TABLE()

void MyFrame::OnAbout(wxCommandEvent& event) {
  wxAboutDialogInfo info;
  info.SetName(appName);
  info.SetVersion(appVersion);
  info.SetDescription(wxT("This program allows users to view, create, edit and listen to loops and cues embedded in wav files."));
  info.SetCopyright(wxT("Copyright (C) 2011-2015 Lars Palo <larspalo AT yahoo DOT se>\nReleased under GNU GPLv3 licence"));
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
    PopulateListCtrl();
    SetStatusText(workingDir, 2);
    if (!fileNames.IsEmpty()) {
      m_fileListCtrl->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
      m_panel->SetFocus();
    }
  }
}

void MyFrame::OnClose(wxCloseEvent & event) {
  config->Write(wxT("General/LastWorkingDir"), workingDir);
  wxSlider *volumeSl = (wxSlider*) FindWindow(ID_VOLUME_SLIDER);
  int vol = volumeSl->GetValue();
  config->Write(wxT("General/LastVolume"), vol);
  config->Write(wxT("General/LoopOnlyPlayback"), m_loopOnly);
  config->Write(wxT("BatchProcess/LastSource"), m_batchProcess->GetLastSource());
  config->Write(wxT("BatchProcess/LastTarget"), m_batchProcess->GetLastTarget());
  config->Write(wxT("LoopSettings/AutoSearchSustain"), m_autoloopSettings->GetAutosearch());
  config->Write(wxT("LoopSettings/BruteForce"), m_autoloopSettings->GetBruteForce());
  config->Write(wxT("LoopSettings/SustainStart"), m_autoloopSettings->GetStart());
  config->Write(wxT("LoopSettings/SustainEnd"), m_autoloopSettings->GetEnd());
  config->Write(wxT("LoopSettings/Threshold"), m_autoloopSettings->GetThreshold());
  config->Write(wxT("LoopSettings/MinDuration"), m_autoloopSettings->GetDuration());
  config->Write(wxT("LoopSettings/MinDistance"), m_autoloopSettings->GetBetween());
  config->Write(wxT("LoopSettings/Quality"), m_autoloopSettings->GetQuality());
  config->Write(wxT("LoopSettings/Candidates"), m_autoloopSettings->GetCandidates());
  config->Write(wxT("LoopSettings/LoopsToReturn"), m_autoloopSettings->GetNrLoops());
  config->Write(wxT("LoopSettings/LoopPoolMultiple"), m_autoloopSettings->GetMultiple());
  config->Flush();

  Destroy();
}

void MyFrame::OnQuit(wxCommandEvent& event) {
  // Close the frame by calling OnClose() via event generation
  Close();
}

void MyFrame::OnDblClick(wxListEvent& event) {
  int sel = event.GetIndex();
  if (sel != -1) {
    wxListItem item;
    item.SetId(sel);
    item.SetMask(wxLIST_MASK_TEXT);
    item.SetColumn(0);
    m_fileListCtrl->GetItem(item);
    fileToOpen = item.GetText();
    OpenAudioFile();
    currentOpenFileIdx = sel;
  }
}

void MyFrame::OnOpenSelected(wxCommandEvent& event) {
  if (currentSelectedIdx != -1) {
    wxListItem item;
    item.SetId(currentSelectedIdx);
    item.SetMask(wxLIST_MASK_TEXT);
    item.SetColumn(0);
    m_fileListCtrl->GetItem(item);
    fileToOpen = item.GetText();
    OpenAudioFile();
    currentOpenFileIdx = currentSelectedIdx;
    m_fileListCtrl->EnsureVisible(currentSelectedIdx);
  }
}

void MyFrame::OnSelection(wxListEvent& event) {
  currentSelectedIdx = event.GetIndex();
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
    filePath = workingDir;
    filePath += wxFILE_SEP_PATH;
    filePath += fileToOpen;
    m_waveform = new WaveformDrawer(this, filePath);
    lowerBox->Clear();
    lowerBox->Add(m_waveform, 1, wxEXPAND, 0);
    vbox->Layout();

    m_sound->SetSampleRate(m_audiofile->GetSampleRate());
    m_sound->SetAudioFormat(m_audiofile->GetAudioFormat());
    m_sound->SetChannels(m_audiofile->m_channels);
    m_sound->OpenAudioStream();
    m_panel->SetFileNameLabel(fileToOpen);

    SetStatusText(wxString::Format(wxT("Zoom level: x %i"), m_waveform->GetAmplitudeZoomLevel()), 1);

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

    toolBar->EnableTool(PITCH_SETTINGS, true);
    toolMenu->Enable(PITCH_SETTINGS, true);

    toolBar->EnableTool(ZOOM_IN_AMP, true);
    toolBar->EnableTool(ZOOM_OUT_AMP, true);
    viewMenu->Enable(ZOOM_IN_AMP, true);
    viewMenu->Enable(ZOOM_OUT_AMP, true);
    toolBar->EnableTool(CUT_N_FADE, true);
    toolMenu->Enable(CUT_N_FADE, true);

    // select first loop if such is present
    if (m_panel->m_grid->GetNumberRows() > 0) {
      m_panel->m_grid->SelectRow(0, false);
      toolBar->EnableTool(wxID_STOP, false);
      toolBar->EnableTool(START_PLAYBACK, true);
      transportMenu->Enable(START_PLAYBACK, true);
      transportMenu->Enable(wxID_STOP, false);
      toolBar->EnableTool(X_FADE, true);
      toolMenu->Enable(X_FADE, true);
    }
  } else {
    // libsndfile couldn't open the file or no audio data in file
    wxString message = wxT("Sorry, libsndfile couldn't open selected file:\n");
    message += workingDir;
    message += wxFILE_SEP_PATH;
    message += fileToOpen;
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

    SetStatusText(wxEmptyString, 1);

  } // end of if file open was successful checking
}

void MyFrame::CloseOpenAudioFile() {
  toolBar->EnableTool(START_PLAYBACK, false);
  transportMenu->Enable(START_PLAYBACK, false);

  // disable save
  toolBar->EnableTool(wxID_SAVE, false);
  fileMenu->Enable(wxID_SAVE, false);
  fileMenu->Enable(SAVE_AND_OPEN_NEXT, false);

  // disable save as...
  toolBar->EnableTool(wxID_SAVEAS, false);
  fileMenu->Enable(wxID_SAVEAS, false);

  // disable tools
  toolBar->EnableTool(ADD_LOOP, false);
  toolMenu->Enable(ADD_LOOP, false);
  toolBar->EnableTool(AUTOSEARCH_LOOPS, false);
  toolMenu->Enable(AUTOSEARCH_LOOPS, false);
  toolBar->EnableTool(PITCH_SETTINGS, false);
  toolMenu->Enable(PITCH_SETTINGS, false);
  toolBar->EnableTool(ZOOM_IN_AMP, false);
  toolBar->EnableTool(ZOOM_OUT_AMP, false);
  viewMenu->Enable(ZOOM_IN_AMP, false);
  viewMenu->Enable(ZOOM_OUT_AMP, false);
  toolBar->EnableTool(X_FADE, false);
  toolMenu->Enable(X_FADE, false);
  toolBar->EnableTool(CUT_N_FADE, false);
  toolMenu->Enable(CUT_N_FADE, false);

  m_sound->CloseAudioStream();

  if (m_audiofile)
    m_panel->EmptyTable();

  delete m_audiofile;
  delete m_waveform;
  m_waveform = 0;

  SetStatusText(wxEmptyString, 1);

  UpdateAllViews();
}

void MyFrame::OnGridCellClick(wxGridEvent& event) {
  if (event.GetRow() >= 0) {
    if (m_panel->m_grid->IsSelection())
      m_panel->m_grid->ClearSelection();
    if (m_panel->m_cueGrid->IsSelection())
      m_panel->m_cueGrid->ClearSelection();
    m_panel->m_grid->SelectRow(event.GetRow());

    // set/update the currently selected loops positions
    LOOPDATA currentLoop;
    m_audiofile->m_loops->GetLoopData(event.GetRow(), currentLoop);
    if (!m_loopOnly)
      m_sound->SetLoopPosition(0, currentLoop.dwStart, currentLoop.dwEnd, m_audiofile->m_channels);
    else {
      if (((double) (currentLoop.dwEnd - currentLoop.dwStart)) / (double) m_audiofile->GetSampleRate() > 0.5) {
        unsigned pos = currentLoop.dwEnd - (m_audiofile->GetSampleRate() / 2);
        m_sound->SetLoopPosition(pos, currentLoop.dwStart, currentLoop.dwEnd, m_audiofile->m_channels);
      } else {
        m_sound->SetLoopPosition(currentLoop.dwStart, currentLoop.dwStart, currentLoop.dwEnd, m_audiofile->m_channels);
      }
    }

    if (event.GetCol() == 4) {
      // User clicked in the save column
      if (m_panel->m_grid->GetCellValue(event.GetRow(), event.GetCol()) == wxT("1")) {
        // Equals to a true value to begin with
        m_panel->m_grid->SetCellValue(event.GetRow(), event.GetCol(), wxT("0"));
        m_audiofile->m_loops->SetSaveOption(false, event.GetRow());
        toolBar->EnableTool(wxID_SAVE, true);
        fileMenu->Enable(wxID_SAVE, true);
        fileMenu->Enable(SAVE_AND_OPEN_NEXT, true);
      } else {
        // The value was false
        m_panel->m_grid->SetCellValue(event.GetRow(), event.GetCol(), wxT("1"));
        m_audiofile->m_loops->SetSaveOption(true, event.GetRow());
        toolBar->EnableTool(wxID_SAVE, true);
        fileMenu->Enable(wxID_SAVE, true);
        fileMenu->Enable(SAVE_AND_OPEN_NEXT, true);
      }
    }
    m_panel->m_grid->SetGridCursor(event.GetRow(), 4); // this is to move scrolling
    SetLoopPlayback(true); // set the playback to be for loops
    if (toolBar->GetToolEnabled(wxID_STOP) == false) {
      toolBar->EnableTool(START_PLAYBACK, true);
      transportMenu->Enable(START_PLAYBACK, true);
    }
    toolBar->EnableTool(X_FADE, true);
    toolMenu->Enable(X_FADE, true);
  }
}

void MyFrame::OnCueGridCellClick(wxGridEvent& event) {
  if (event.GetRow() >= 0) {
    if (m_panel->m_grid->IsSelection())
      m_panel->m_grid->ClearSelection();
    if (m_panel->m_cueGrid->IsSelection())
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
        fileMenu->Enable(SAVE_AND_OPEN_NEXT, true);
      } else {
        // The value was false
        m_panel->m_cueGrid->SetCellValue(event.GetRow(), event.GetCol(), wxT("1"));
        m_audiofile->m_cues->SetSaveOption(true, event.GetRow());
        toolBar->EnableTool(wxID_SAVE, true);
        fileMenu->Enable(wxID_SAVE, true);
        fileMenu->Enable(SAVE_AND_OPEN_NEXT, true);
      }
    }
    m_panel->m_cueGrid->SetGridCursor(event.GetRow(), 2); // this is to hide the highlight box and easily find selected row
    if (toolBar->GetToolEnabled(wxID_STOP) == false) {
      toolBar->EnableTool(START_PLAYBACK, true);
      transportMenu->Enable(START_PLAYBACK, true);
    }
    toolBar->EnableTool(X_FADE, false);
    toolMenu->Enable(X_FADE, false);
  }
}

void MyFrame::UpdateCurrentFileInfo() {
  if (!m_audiofile->m_loops->loopsOut.empty()) {
    wxString loopNbr = wxString::Format(wxT("%i"), m_audiofile->m_loops->loopsOut.size());
    m_fileListCtrl->SetItem(currentOpenFileIdx, 1, loopNbr);
  } else {
    m_fileListCtrl->SetItem(currentOpenFileIdx, 1, wxT("0"));
  }

  if (!m_audiofile->m_cues->exportedCues.empty()) {
    wxString cueNbr = wxString::Format(wxT("%i"), m_audiofile->m_cues->exportedCues.size());
    m_fileListCtrl->SetItem(currentOpenFileIdx, 2, cueNbr);
  } else {
    m_fileListCtrl->SetItem(currentOpenFileIdx, 2, wxT("0"));
  }

  int midiNote = (int) m_audiofile->m_loops->GetMIDIUnityNote();
  wxString note = wxString::Format(wxT("%i"), midiNote);
  m_fileListCtrl->SetItem(currentOpenFileIdx, 3, note);

  double cents = (double) m_audiofile->m_loops->GetMIDIPitchFraction() / (double)UINT_MAX * 100.0;
  wxString fraction = wxString::Format(wxT("%.2f"), cents);
  m_fileListCtrl->SetItem(currentOpenFileIdx, 4, fraction);
}

void MyFrame::OnSaveFile(wxCommandEvent& event) {
  m_audiofile->SaveAudioFile(fileToOpen, workingDir);

  // make sure to update the listCtrl columns as things might have changed
  UpdateCurrentFileInfo();

  // open the file again to remove old leftovers in grids etc.
  OpenAudioFile();
}

void MyFrame::OnSaveOpenNext(wxCommandEvent& event) {
  m_audiofile->SaveAudioFile(fileToOpen, workingDir);

  // make sure to update the listCtrl columns as things might have changed
  UpdateCurrentFileInfo();

  // open next file if possible
  if (currentSelectedIdx < (fileNames.GetCount() - 1) && currentSelectedIdx != wxNOT_FOUND) {
    m_fileListCtrl->SetItemState(currentSelectedIdx + 1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    m_fileListCtrl->EnsureVisible(currentSelectedIdx);

    wxCommandEvent evt(wxEVT_TOOL, OPEN_SELECTED);
    ::wxGetApp().frame->AddPendingEvent(evt);
  } else {
    OpenAudioFile();
  }
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
    PopulateListCtrl();
    SetStatusText(workingDir, 1);

    // make sure to update current open file index and select it
    currentOpenFileIdx = fileNames.Index(fileToOpen);
    m_fileListCtrl->SetItemState(currentOpenFileIdx, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
  }
}

void MyFrame::OnStartPlay(wxCommandEvent& event) {
  m_timer.Start(50);
  // if it's a loop make sure start position is set to start of data
  // or to within the loop if that option is ticked
  if (m_panel->m_grid->IsSelection()) {
    // set the currently selected loops positions and prepare for playback
    LOOPDATA currentLoop;
    wxArrayInt selectedRows = m_panel->m_grid->GetSelectedRows();
    int firstSelected = selectedRows[0];
    m_audiofile->m_loops->GetLoopData(firstSelected, currentLoop);
    m_sound->SetLoopPosition(0, currentLoop.dwStart, currentLoop.dwEnd, m_audiofile->m_channels);
    if (!m_loopOnly) {
      m_sound->SetStartPosition(0, m_audiofile->m_channels);
      m_waveform->SetPlayPosition(0);
    } else {
      if (((double) (currentLoop.dwEnd - currentLoop.dwStart)) / (double) m_audiofile->GetSampleRate() > 0.5) {
        unsigned pos = currentLoop.dwEnd - (m_audiofile->GetSampleRate() / 2);
        m_sound->SetStartPosition(pos, m_audiofile->m_channels);
        m_waveform->SetPlayPosition(pos);
      } else {
        m_sound->SetStartPosition(currentLoop.dwStart, m_audiofile->m_channels);
        m_waveform->SetPlayPosition(currentLoop.dwStart);
      }
    }
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
  m_crossfades = new CrossfadeDialog(this);
  m_cutNFade = new CutNFadeDialog(this);
  m_batchProcess = new BatchProcessDialog(m_autoloopSettings, this);
  currentOpenFileIdx = -1;
  currentSelectedIdx = -1;
  fileToOpen = wxEmptyString;
  config = new wxFileConfig(wxT("LoopAuditioneer"));

  // Create a file menu
  fileMenu = new wxMenu();

  // Add file menu items
  fileMenu->Append(FILE_SELECT, wxT("&Choose folder\tCtrl+O"), wxT("Select working folder"));
  fileMenu->Append(OPEN_SELECTED, wxT("&Open file\tCtrl+F"), wxT("Open selected file"));
  fileMenu->Append(wxID_SAVE, wxT("&Save\tCtrl+S"), wxT("Save current file"));
  fileMenu->Append(wxID_SAVEAS, wxT("Save &as...\tShift+Ctrl+S"), wxT("Save current file with new name"));
  fileMenu->Append(SAVE_AND_OPEN_NEXT, wxT("Save, open next\tCtrl+Alt+S"), wxT("Save current file and open next"));
  fileMenu->AppendSeparator();
  fileMenu->Append(AUTOLOOP_SETTINGS, wxT("&Autoloop settings\tCtrl+A"), wxT("Adjust settings for loop searching"));
  fileMenu->AppendSeparator();
  fileMenu->Append(wxID_EXIT, wxT("&Exit\tCtrl+Q"), wxT("Quit this program"));

  fileMenu->Enable(OPEN_SELECTED, false);
  fileMenu->Enable(wxID_SAVE, false);
  fileMenu->Enable(wxID_SAVEAS, false);
  fileMenu->Enable(SAVE_AND_OPEN_NEXT, false);

  // Create a view menu
  viewMenu = new wxMenu();

  // Add view menu items
  viewMenu->Append(ZOOM_IN_AMP, wxT("&Zoom in\tCtrl++"), wxT("Zoom in on amplitude"));
  viewMenu->Append(ZOOM_OUT_AMP, wxT("&Zoom out\tCtrl+-"), wxT("Zoom out on amplitude"));

  viewMenu->Enable(ZOOM_IN_AMP, false);
  viewMenu->Enable(ZOOM_OUT_AMP, false);

  // Create a transport menu
  transportMenu = new wxMenu();

  // Add transport menu items
  transportMenu->Append(START_PLAYBACK, wxT("&Play"), wxT("Start playback"));
  transportMenu->Append(wxID_STOP, wxT("&Stop"), wxT("Stop playback"));
  transportMenu->AppendCheckItem(LOOP_ONLY, wxT("&Loop only\tCtrl+V"), wxT("Start playback inside of loop"));

  transportMenu->Enable(START_PLAYBACK, false);
  transportMenu->Enable(wxID_STOP, false);

  // Create a tool menu
  toolMenu = new wxMenu();

  // Add items to the tool menu
  toolMenu->Append(ADD_LOOP, wxT("&New loop\tCtrl+N"), wxT("Create a new loop"));
  toolMenu->Append(AUTOSEARCH_LOOPS, wxT("&Autoloop\tCtrl+Z"), wxT("Search for loop(s)"));
  toolMenu->Append(BATCH_PROCESS, wxT("&Batch processing\tCtrl+B"), wxT("Batch processing of files"));
  toolMenu->Append(PITCH_SETTINGS, wxT("&Pitch detection\tCtrl+D"), wxT("Find/set info about pitch"));
  toolMenu->Append(X_FADE, wxT("&Crossfade\tCtrl+X"), wxT("Crossfade the selected loop"));
  toolMenu->Append(CUT_N_FADE, wxT("Cut and &fade\tCtrl+C"), wxT("Cut & fade in/out"));

  toolMenu->Enable(ADD_LOOP, false);
  toolMenu->Enable(AUTOSEARCH_LOOPS, false);
  toolMenu->Enable(PITCH_SETTINGS, false);
  toolMenu->Enable(X_FADE, false);
  toolMenu->Enable(CUT_N_FADE, false);

  // Create a help menu
  helpMenu = new wxMenu();

  // Add help menu items
  helpMenu->Append(wxID_HELP, wxT("&LoopAuditioneer Help \tF1"), wxT("Show application help"));
  helpMenu->Append(wxID_ABOUT, wxT("&About..."), wxT("Show about dialog"));

  // Create a menu bar and append the menus to it
  menuBar = new wxMenuBar();
  menuBar->Append(fileMenu, wxT("&File"));
  menuBar->Append(viewMenu, wxT("&View"));
  menuBar->Append(transportMenu, wxT("&Transport"));
  menuBar->Append(toolMenu, wxT("T&ools"));
  menuBar->Append(helpMenu, wxT("&Help"));

  // Attach menu bar to frame
  SetMenuBar(menuBar);

  // create the popup menu for the loops
  m_loopPopupMenu = new wxMenu();
  m_loopPopupMenu->Append(EDIT_LOOP, wxT("&Edit loop"), wxT("Edit the currently selected loop"));
  m_loopPopupMenu->Append(X_FADE, wxT("&Crossfade loop"), wxT("Crossfade the currently selected loop"));
  m_loopPopupMenu->Append(VIEW_LOOPPOINTS, wxT("&View looppoints"), wxT("Closeup view of overlayed looppoints"));

  // Create Status bar
  CreateStatusBar(3);
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
  wxBitmap pitchInfo(wxT("../icons/24x24/Bell.png"), wxBITMAP_TYPE_PNG);
  wxBitmap zoomInAmp(wxT("../icons/24x24/Zoom_in.png"), wxBITMAP_TYPE_PNG);
  wxBitmap zoomOutAmp(wxT("../icons/24x24/Zoom_out.png"), wxBITMAP_TYPE_PNG);
  wxBitmap crossfade(wxT("../icons/24x24/Wizard.png"), wxBITMAP_TYPE_PNG);
  wxBitmap cutfade(wxT("../icons/24x24/Software.png"), wxBITMAP_TYPE_PNG);
  toolBar->AddTool(FILE_SELECT, wxT("Select working folder"), selectFolder, wxT("Select working folder"));
  toolBar->AddTool(OPEN_SELECTED, wxT("Open selected file"), openSelectedFile, wxT("Open selected file"));
  toolBar->AddTool(wxID_SAVE, wxT("Save file"), saveFile, wxT("Save file"));
  toolBar->AddTool(wxID_SAVEAS, wxT("Save as..."), saveFileAs, wxT("Save file with new name"));
  toolBar->AddSeparator();
  toolBar->AddTool(START_PLAYBACK, wxT("Play"), startPlayback, wxT("Play"));
  toolBar->AddTool(wxID_STOP, wxT("Stop"), stopPlayback, wxT("Stop"));
  toolBar->AddSeparator();
  toolBar->AddTool(ADD_LOOP, wxT("New loop"), loopCreation, wxT("Create a new loop"));
  toolBar->AddTool(AUTOSEARCH_LOOPS, wxT("Autoloop"), autoLoop, wxT("Search for loop(s)"));
  toolBar->AddTool(AUTOLOOP_SETTINGS, wxT("Autoloop settings"), autoLoopSettings, wxT("Change settings for auto loopsearching"));
  toolBar->AddTool(BATCH_PROCESS, wxT("Batch processing"), batchProcess, wxT("Batch processing of files/folders"));
  toolBar->AddTool(PITCH_SETTINGS, wxT("Pitch settings"), pitchInfo, wxT("Find/set information about pitch"));
  toolBar->AddTool(X_FADE, wxT("Crossfade"), crossfade, wxT("Crossfade a selected loop"));
  toolBar->AddTool(CUT_N_FADE, wxT("Cut & Fade"), cutfade, wxT("Cut & Fade in/out"));
  toolBar->AddSeparator();
  toolBar->AddTool(ZOOM_IN_AMP, wxT("Zoom in"), zoomInAmp, wxT("Zoom in on amplitude"));
  toolBar->AddTool(ZOOM_OUT_AMP, wxT("Zoom out"), zoomOutAmp, wxT("Zoom out on amplitude"));

  // Text label for volume slider
  wxStaticText *volumeLabel = new wxStaticText (
    toolBar,
    wxID_ANY,
    wxT(" Volume boost: "),
    wxDefaultPosition,
    wxDefaultSize
  );

  // Slider for volume (0 to 4)
  wxSlider *volumeSlider = new wxSlider ( 
    toolBar, 
    ID_VOLUME_SLIDER,
    0,
    0,
    4,
    wxDefaultPosition, 
    wxSize(100, -1), 
    wxSL_HORIZONTAL
  );

  toolBar->AddControl(volumeLabel);
  toolBar->AddControl(volumeSlider);
  toolBar->Realize();
  toolBar->EnableTool(OPEN_SELECTED, false);
  toolBar->EnableTool(wxID_SAVE, false);
  toolBar->EnableTool(wxID_SAVEAS, false);
  toolBar->EnableTool(START_PLAYBACK, false);
  toolBar->EnableTool(wxID_STOP, false);
  toolBar->EnableTool(ADD_LOOP, false);
  toolBar->EnableTool(AUTOSEARCH_LOOPS, false);
  toolBar->EnableTool(PITCH_SETTINGS, false);
  toolBar->EnableTool(ZOOM_IN_AMP, false);
  toolBar->EnableTool(ZOOM_OUT_AMP, false);
  toolBar->EnableTool(X_FADE, false);
  toolBar->EnableTool(CUT_N_FADE, false);
  this->SetToolBar(toolBar);

  // Create sizers for frame content
  vbox = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
  vbox->Add(hbox, 1, wxEXPAND, 0);

  m_fileListCtrl = new MyListCtrl(
    this,
    ID_LISTCTRL
  );
  m_fileListCtrl->SetMinSize(wxSize(385,150));
  hbox->Add(m_fileListCtrl, 1, wxEXPAND | wxLEFT | wxTOP | wxBOTTOM, 10);

  m_panel = new MyPanel(this);
  m_panel->SetMinSize(wxSize(600,150));
  hbox->Add(m_panel, 2, wxEXPAND | wxALL, 10);

  lowerBox = new wxBoxSizer(wxHORIZONTAL);
  lowerBox->AddStretchSpacer();
  vbox->Add(lowerBox, 1, wxEXPAND, 0);
  
  SetSizer(vbox);
  vbox->SetSizeHints(this);
  // Layout();
  SetMinSize(wxSize(1000,560));
  SetBackgroundColour(wxT("#f4f2ef"));

  if (config->Read(wxT("General/LastWorkingDir"), &workingDir)) {
    // if value was found it's now in the variable workingDir
  } else {
    // if it's not found default to an empty string
    workingDir = wxEmptyString;
  }

  int readInt;
  if (config->Read(wxT("General/LastVolume"), &readInt)) {
    wxSlider *volumeSl = (wxSlider*) FindWindow(ID_VOLUME_SLIDER);
    volumeSl->SetValue(readInt);
    volumeMultiplier = (int) (pow(2, (double) readInt));
  }

  bool b;
  if (config->Read(wxT("General/LoopOnlyPlayback"), &b)) {
    m_loopOnly = b;
    if (m_loopOnly)
      transportMenu->Check(LOOP_ONLY, true);
    else
      transportMenu->Check(LOOP_ONLY, false);
  }

  wxString str;
  if (config->Read(wxT("BatchProcess/LastSource"), &str))
    m_batchProcess->SetLastSource(str);

  if (config->Read(wxT("BatchProcess/LastTarget"), &str))
    m_batchProcess->SetLastTarget(str);

  if (config->Read(wxT("LoopSettings/AutoSearchSustain"), &b))
    m_autoloopSettings->SetAutosearch(b);

  if (config->Read(wxT("LoopSettings/BruteForce"), &b)) {
    m_autoloopSettings->SetBruteForce(b);
    m_autoloop->SetBruteForce(b);
  }

  if (config->Read(wxT("LoopSettings/SustainStart"), &readInt))
    m_autoloopSettings->SetStart(readInt);

  if (config->Read(wxT("LoopSettings/SustainEnd"), &readInt))
    m_autoloopSettings->SetEnd(readInt);

  double dbl;
  if (config->Read(wxT("LoopSettings/Threshold"), &dbl)) {
    m_autoloopSettings->SetThreshold(dbl);
    m_autoloop->SetThreshold(dbl);
  }

  if (config->Read(wxT("LoopSettings/MinDuration"), &dbl)) {
    m_autoloopSettings->SetDuration(dbl);
    m_autoloop->SetDuration(dbl);
  }

  if (config->Read(wxT("LoopSettings/MinDistance"), &dbl)) {
    m_autoloopSettings->SetBetween(dbl);
    m_autoloop->SetBetween(dbl);
  }

  if (config->Read(wxT("LoopSettings/Quality"), &dbl)) {
    m_autoloopSettings->SetQuality(dbl);
    m_autoloop->SetQuality(dbl);
  }

  if (config->Read(wxT("LoopSettings/Candidates"), &readInt)) {
    m_autoloopSettings->SetCandidates(readInt);
    m_autoloop->SetCandidates(readInt);
  }

  if (config->Read(wxT("LoopSettings/LoopsToReturn"), &readInt)) {
    m_autoloopSettings->SetNrLoops(readInt);
    m_autoloop->SetLoops(readInt);
  }

  if (config->Read(wxT("LoopSettings/LoopPoolMultiple"), &readInt)) {
    m_autoloopSettings->SetMultiple(readInt);
    m_autoloop->SetMultiple(readInt);
  }

  m_autoloopSettings->UpdateLabels();
  m_panel->SetFocus();
}

MyFrame::~MyFrame() {
  delete config;

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
  // But only do it if wxArrayString not is empty!
  if (fileNames.IsEmpty() == false) {
    fileNames.Sort();
    size_t lineCounter = 0;
    while (lineCounter < fileNames.GetCount() - 1) {
      if (fileNames[lineCounter] == fileNames[lineCounter + 1])
        fileNames.RemoveAt(lineCounter + 1);
      else
        lineCounter++;
    }
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
        *buffer++ = ::wxGetApp().frame->m_audiofile->shortAudioData[(position[0])] * volumeMultiplier;
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
        *buffer++ = ::wxGetApp().frame->m_audiofile->intAudioData[(position[0])] * volumeMultiplier;
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
        *buffer++ = ::wxGetApp().frame->m_audiofile->doubleAudioData[(position[0])] * volumeMultiplier;
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
  fileMenu->Enable(SAVE_AND_OPEN_NEXT, true);
}

void MyFrame::ChangeCuePosition(unsigned int offset, int index) {
  m_audiofile->m_cues->ChangePosition(offset, index);
  m_panel->ChangeCueData(offset, index);

  UpdateAllViews();

  toolBar->EnableTool(wxID_SAVE, true);
  fileMenu->Enable(wxID_SAVE, true);
  fileMenu->Enable(SAVE_AND_OPEN_NEXT, true);
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

    // Enable save icon and menu
    toolBar->EnableTool(wxID_SAVE, true);
    fileMenu->Enable(wxID_SAVE, true);
    fileMenu->Enable(SAVE_AND_OPEN_NEXT, true);

    UpdateAllViews();
  }
}

void MyFrame::OnLoopGridRightClick(wxGridEvent& event) {
  if (event.GetRow() >= 0) {
    if (m_panel->m_grid->IsSelection())
      m_panel->m_grid->ClearSelection();
    if (m_panel->m_cueGrid->IsSelection())
      m_panel->m_cueGrid->ClearSelection();
    m_panel->m_grid->SelectRow(event.GetRow());

    // Enable x-fade as now a loop is selected
    toolBar->EnableTool(X_FADE, true);
    toolMenu->Enable(X_FADE, true);

    m_panel->m_grid->SetGridCursor(event.GetRow(), 4); // this is to fix scrolling issues
    SetLoopPlayback(true); // set the playback to be for loops
    if (toolBar->GetToolEnabled(wxID_STOP) == false) {
      toolBar->EnableTool(START_PLAYBACK, true);
      transportMenu->Enable(START_PLAYBACK, true);
    }

    // Now we should have a context menu pop up where user can do something
    PopupMenu(m_loopPopupMenu, event.GetPosition());
  }
}

void MyFrame::UpdateAllViews() {
  // force updates of wxGrids in m_panel by jiggling the size of the frame!
  wxSize size = GetSize();
  size.IncBy(1, 1);
  SetSize(size);
  size.DecBy(1, 1);
  SetSize(size);

  if (m_waveform) {
    m_waveform->Refresh();
    m_waveform->Update();
  }

  m_panel->SetFocus();
}

void MyFrame::OnBatchProcess(wxCommandEvent& event) {
  m_batchProcess->ClearStatusProgress();
  m_batchProcess->SetCurrentWorkingDir(workingDir);
  m_batchProcess->ShowModal();
  if (workingDir != wxEmptyString && m_batchProcess->NeedToRefreshFileList()) {
    EmptyListOfFileNames();
    PopulateListOfFileNames();
    PopulateListCtrl();

    // also reload any open file to update any changes too
    if (fileToOpen != wxEmptyString)
      OpenAudioFile();
  }
}

void MyFrame::OnAutoLoop(wxCommandEvent& event) {
  // prepare a vector to receive the loops
  std::vector<std::pair<std::pair<unsigned, unsigned>, double> > loops;

  // get the audio data as doubles from m_waveform
  double *audioData = new double[m_audiofile->ArrayLength];
  bool gotData = m_waveform->GetDoubleAudioData(audioData, m_audiofile->ArrayLength);

  if (gotData) {
    bool foundSomeLoops = false;

    if (!foundSomeLoops) {
      // stop other windows and set a busyinfo to calm user if it takes some time
      wxWindowDisabler disableAll;

      wxBusyInfo searchInfo(wxT("Searching for loops, please wait..."), this);
      wxSafeYield();
    
      // this is the call to search for loops
      foundSomeLoops = m_autoloop->AutoFindLoops(
        audioData,
        m_audiofile->ArrayLength,
        m_audiofile->m_channels,
        m_audiofile->GetSampleRate(),
        loops,
        m_autoloopSettings->GetAutosearch(),
        m_autoloopSettings->GetStart(),
        m_autoloopSettings->GetEnd()
      );
    }

    if (foundSomeLoops) {
      for (unsigned i = 0; i < loops.size(); i++) {
        // Prepare loop data for insertion into loop vector
        LOOPDATA newLoop;
        newLoop.dwType = SF_LOOP_FORWARD;
        newLoop.dwStart = loops[i].first.first;
        newLoop.dwEnd = loops[i].first.second;
        newLoop.dwPlayCount = 0;
        newLoop.shouldBeSaved = true;

        // Add the loop to the audio files loop vector
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

      // Enable save icon and menu
      toolBar->EnableTool(wxID_SAVE, true);
      fileMenu->Enable(wxID_SAVE, true);
      fileMenu->Enable(SAVE_AND_OPEN_NEXT, true);

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
  int oldStart = m_autoloopSettings->GetStart();
  int oldEnd = m_autoloopSettings->GetEnd();
  bool oldAutoSustain = m_autoloopSettings->GetAutosearch();
  if (m_autoloopSettings->ShowModal() == wxID_OK) {
    // Update AutoLooping object
    m_autoloop->SetThreshold(m_autoloopSettings->GetThreshold());
    m_autoloop->SetDuration(m_autoloopSettings->GetDuration());
    m_autoloop->SetBetween(m_autoloopSettings->GetBetween());
    m_autoloop->SetQuality(m_autoloopSettings->GetQuality());
    m_autoloop->SetCandidates(m_autoloopSettings->GetCandidates());
    m_autoloop->SetLoops(m_autoloopSettings->GetNrLoops());
    m_autoloop->SetMultiple(m_autoloopSettings->GetMultiple());
    m_autoloop->SetBruteForce(m_autoloopSettings->GetBruteForce());
  } else {
    // Reset parameter values to actually used
    m_autoloopSettings->SetThreshold(m_autoloop->GetThreshold());
    m_autoloopSettings->SetDuration(m_autoloop->GetMinDuration());
    m_autoloopSettings->SetBetween(m_autoloop->GetMinDistance());
    m_autoloopSettings->SetQuality(m_autoloop->GetQuality());
    m_autoloopSettings->SetCandidates(m_autoloop->GetCandidates());
    m_autoloopSettings->SetNrLoops(m_autoloop->GetLoopsToReturn());
    m_autoloopSettings->SetMultiple(m_autoloop->GetLoopMultiple());
    m_autoloopSettings->SetAutosearch(oldAutoSustain);
    m_autoloopSettings->SetStart(oldStart);
    m_autoloopSettings->SetEnd(oldEnd);
    m_autoloopSettings->SetBruteForce(m_autoloop->GetBruteForce());
    m_autoloopSettings->UpdateLabels();
  }
}

void MyFrame::OnPitchSettings(wxCommandEvent& event) {
  // get the audio data as doubles from m_waveform
  double *audioData = new double[m_audiofile->ArrayLength];
  bool gotData = m_waveform->GetDoubleAudioData(audioData, m_audiofile->ArrayLength);
  if (!gotData) {
    delete[] audioData;
    return;
  }

  int midi_note;
  int hps_midi_note;
  int td_midi_note;
  double midi_note_pitch;
  double hps_midi_note_pitch;
  double td_midi_note_pitch;
  double cent_deviation;
  double hps_cent_deviation;
  double td_cent_deviation;
  unsigned int midi_pitch_fraction;
  unsigned int hps_midi_pitch_fraction;
  unsigned int td_midi_pitch_fraction;
  double fftPitches[2];
  for (int i = 0; i < 2; i++)
    fftPitches[i] = 0;
  bool got_fftpitch = m_audiofile->GetFFTPitch(audioData, fftPitches);
  double td_pitch = m_audiofile->GetTDPitch(audioData);
  double cent_from_file = (double) m_audiofile->m_loops->GetMIDIPitchFraction() / (double)UINT_MAX * 100.0;

  if (got_fftpitch) {
    // FFT detection
    midi_note = (69 + 12 * (log10(fftPitches[0] / 440.0) / log10(2)));
    midi_note_pitch = 440.0 * pow(2, ((double)(midi_note - 69) / 12.0));
    cent_deviation = 1200 * (log10(fftPitches[0] / midi_note_pitch) / log10(2));
    midi_pitch_fraction = ((double)UINT_MAX * (cent_deviation / 100.0));

    hps_midi_note = (69 + 12 * (log10(fftPitches[1] / 440.0) / log10(2)));
    hps_midi_note_pitch = 440.0 * pow(2, ((double)(hps_midi_note - 69) / 12.0));
    hps_cent_deviation = 1200 * (log10(fftPitches[1] / hps_midi_note_pitch) / log10(2));
    hps_midi_pitch_fraction = ((double)UINT_MAX * (hps_cent_deviation / 100.0));
  } else {
    midi_note = 0;
    midi_note_pitch = 0;
    cent_deviation = 0;
    midi_pitch_fraction = 0;

    hps_midi_note = 0;
    hps_midi_note_pitch = 0;
    hps_cent_deviation = 0;
    hps_midi_pitch_fraction = 0;
  }

  if (td_pitch != 0) {
    // TD detection
    td_midi_note = (69 + 12 * (log10(td_pitch / 440.0) / log10(2)));
    td_midi_note_pitch = 440.0 * pow(2, ((double)(td_midi_note - 69) / 12.0));
    td_cent_deviation = 1200 * (log10(td_pitch / td_midi_note_pitch) / log10(2));
    td_midi_pitch_fraction = ((double)UINT_MAX * (td_cent_deviation / 100.0));
  } else {
    td_midi_note = 0;
    td_midi_note_pitch = 0;
    td_cent_deviation = 0;
    td_midi_pitch_fraction = 0;
  }

  PitchDialog dialog(
    fftPitches[0],
    midi_note,
    cent_deviation,
    fftPitches[1],
    hps_midi_note,
    hps_cent_deviation,
    td_pitch,
    td_midi_note,
    td_cent_deviation,
    (int) m_audiofile->m_loops->GetMIDIUnityNote(),
    cent_from_file,
    this
  );

  if (dialog.ShowModal() == wxID_OK) {
    int selectedMethod = dialog.GetMethodUsed();
    if (selectedMethod == 0) {
      m_audiofile->m_loops->SetMIDIUnityNote((char) midi_note);
      m_audiofile->m_loops->SetMIDIPitchFraction(midi_pitch_fraction);
    } else if (selectedMethod == 1) {
      m_audiofile->m_loops->SetMIDIUnityNote((char) hps_midi_note);
      m_audiofile->m_loops->SetMIDIPitchFraction(hps_midi_pitch_fraction);
    } else if (selectedMethod == 2) {
      m_audiofile->m_loops->SetMIDIUnityNote((char) td_midi_note);
      m_audiofile->m_loops->SetMIDIPitchFraction(td_midi_pitch_fraction);
    } else if (selectedMethod == 3) {
      m_audiofile->m_loops->SetMIDIUnityNote((char) dialog.GetMIDINote());
      m_audiofile->m_loops->SetMIDIPitchFraction((unsigned)((double)UINT_MAX * (dialog.GetPitchFraction() / 100.0)));
    }
    // enable save icon and menu
    toolBar->EnableTool(wxID_SAVE, true);
    fileMenu->Enable(wxID_SAVE, true);
    fileMenu->Enable(SAVE_AND_OPEN_NEXT, true);
  }

  delete[] audioData;
}

void MyFrame::OnZoomInAmplitude(wxCommandEvent& event) {
  m_waveform->ZoomInAmplitude();
  SetStatusText(wxString::Format(wxT("Zoom level: x %i"), m_waveform->GetAmplitudeZoomLevel()), 1);
  UpdateAllViews();
}

void MyFrame::OnZoomOutAmplitude(wxCommandEvent& event) {
  m_waveform->ZoomOutAmplitude();
  SetStatusText(wxString::Format(wxT("Zoom level: x %i"), m_waveform->GetAmplitudeZoomLevel()), 1);
  UpdateAllViews();
}

void MyFrame::OnVolumeSlider(wxCommandEvent& event) {
  wxSlider *volumeSl = (wxSlider*) FindWindow(ID_VOLUME_SLIDER);
  int value = volumeSl->GetValue();

  volumeMultiplier = (int) (pow(2, (double) value));
}

void MyFrame::OnCrossfade(wxCommandEvent& event) {
  // get the audio data as doubles from m_waveform
  double *audioData = new double[m_audiofile->ArrayLength];
  bool gotData = m_waveform->GetDoubleAudioData(audioData, m_audiofile->ArrayLength);
  if (!gotData) {
    delete[] audioData;
    return;
  }

  wxArrayInt selectedRows = m_panel->m_grid->GetSelectedRows();
  int firstSelected;
  if (!selectedRows.IsEmpty())
    firstSelected = selectedRows[0];
  else {
    delete[] audioData;
    return;
  }

  wxString dialogTitle = wxString::Format(wxT("Crossfading parameters for Loop %i"), firstSelected + 1);
  m_crossfades->SetCaption(dialogTitle);

  // show the crossfade dialog to get parameters (method and time)
  if (m_crossfades->ShowModal() == wxID_OK) {
    double crossfadeTime = m_crossfades->GetFadeduration();
    int crossfadetype = m_crossfades->GetFadetype();

    // perform crossfading on the first selected loop with selected method
    m_audiofile->PerformCrossfade(audioData, firstSelected, crossfadeTime, crossfadetype);

    // update the waveform data
    m_waveform->UpdateWaveTracks(audioData, m_audiofile->m_channels, m_audiofile->ArrayLength);

    // change the current audiodata in m_audiofile
    if (m_audiofile->shortAudioData != NULL) {
      for (unsigned i = 0; i < m_audiofile->ArrayLength; i++)
        m_audiofile->shortAudioData[i] = lrint(audioData[i] * (1.0 * 0x7FFF));
    } else if (m_audiofile->intAudioData != NULL) {
      for (unsigned i = 0; i < m_audiofile->ArrayLength; i++)
        m_audiofile->intAudioData[i] = lrint(audioData[i] * (1.0 * 0x7FFFFFFF));
    } else if (m_audiofile->doubleAudioData != NULL) {
      for (unsigned i = 0; i < m_audiofile->ArrayLength; i++)
        m_audiofile->doubleAudioData[i] = audioData[i];
    }

    // Enable save icon and menu
    toolBar->EnableTool(wxID_SAVE, true);
    fileMenu->Enable(wxID_SAVE, true);
    fileMenu->Enable(SAVE_AND_OPEN_NEXT, true);

    // then we should make sure to update the views
    UpdateAllViews();
  } else {
    // user clicked cancel...
  }

  delete[] audioData;
}

void MyFrame::OnEditLoop(wxCommandEvent& event) {
  wxArrayInt selectedRows = m_panel->m_grid->GetSelectedRows();
  int firstSelected;
  if (!selectedRows.IsEmpty())
    firstSelected = selectedRows[0];
  else {
    return;
  }

  // set the currently selected loops positions
  LOOPDATA currentLoop;
  m_audiofile->m_loops->GetLoopData(firstSelected, currentLoop);

  wxString dialogTitle = wxString::Format(wxT("Edit Loop %i"), firstSelected + 1);

  LoopParametersDialog loopDialog(
    currentLoop.dwStart,
    currentLoop.dwEnd,
    m_audiofile->ArrayLength / m_audiofile->m_channels,
    this,
    wxID_ANY,
    dialogTitle
  );

  if (loopDialog.ShowModal() == wxID_OK) {
    unsigned int loopStartSample = loopDialog.GetLoopStart();
    unsigned int loopEndSample = loopDialog.GetLoopEnd();

    // Store the changes to the loop in the loop vector
    m_audiofile->m_loops->SetLoopPositions(loopStartSample, loopEndSample, firstSelected);
    m_audiofile->m_loops->GetLoopData(firstSelected, currentLoop);

    // Change loop to the grid
    int sRate = m_audiofile->GetSampleRate();
    m_panel->ChangeLoopData(currentLoop.dwStart, currentLoop.dwEnd, sRate, firstSelected);

    // Change loop in waveform drawer
    m_waveform->ChangeLoopPositions(currentLoop.dwStart, currentLoop.dwEnd, firstSelected);

    // Set loops positions for playback
    m_sound->SetLoopPosition(0, currentLoop.dwStart, currentLoop.dwEnd, m_audiofile->m_channels);

    // Enable save icon and menu
    toolBar->EnableTool(wxID_SAVE, true);
    fileMenu->Enable(wxID_SAVE, true);
    fileMenu->Enable(SAVE_AND_OPEN_NEXT, true);

    UpdateAllViews();
  }
}

void MyFrame::OnViewLoop(wxCommandEvent& event) {
  wxArrayInt selectedRows = m_panel->m_grid->GetSelectedRows();
  int firstSelected;
  if (!selectedRows.IsEmpty())
    firstSelected = selectedRows[0];
  else {
    return;
  }

  // set the currently selected loops positions
  LOOPDATA currentLoop;
  m_audiofile->m_loops->GetLoopData(firstSelected, currentLoop);

  // get the audio data as doubles from m_waveform
  double *audioData = new double[m_audiofile->ArrayLength];
  bool gotData = m_waveform->GetDoubleAudioData(audioData, m_audiofile->ArrayLength);
  if (!gotData) {
    delete[] audioData;
    return;
  }

  // check that the loop and data is safe to send to the loopoverlay
  if (currentLoop.dwStart > 21 &&
      currentLoop.dwEnd < (m_audiofile->ArrayLength / m_audiofile->m_channels) - 22) {

    wxString dialogTitle = wxString::Format(wxT("Waveform overlay of Loop %i"), firstSelected + 1);

    LoopOverlay lpo(
      audioData,
      currentLoop.dwStart,
      currentLoop.dwEnd,
      m_audiofile->m_channels,
      this,
      wxID_ANY,
      dialogTitle
    );

    lpo.ShowModal();
  }

  delete[] audioData;
}

void MyFrame::OnCutFade(wxCommandEvent& event) {
  // show the cut & fade dialog to get parameters
  if (m_cutNFade->ShowModal() == wxID_OK) {
    // update values
    m_cutNFade->TransferDataFromWindow();

    // make eventual cuts of audio data

    // from beginning
    if (m_cutNFade->GetCutStart() > 0) {
      bool success = m_audiofile->TrimStart(m_cutNFade->GetCutStart());

      if (!success) {
        wxString message = wxT("Couldn't cut audio data from beginning!");
        wxMessageDialog *dialog = new wxMessageDialog(
          NULL,
          message,
          wxT("Error cutting audio data"),
          wxOK | wxICON_ERROR
        );
        dialog->ShowModal();
      }
    }

    // from end
    if (m_cutNFade->GetCutEnd() > 0) {
      bool success = m_audiofile->TrimEnd(m_cutNFade->GetCutEnd());

      if (!success) {
        wxString message = wxT("Couldn't cut audio data from end!");
        wxMessageDialog *dialog = new wxMessageDialog(
          NULL,
          message,
          wxT("Error cutting audio data"),
          wxOK | wxICON_ERROR
        );
        dialog->ShowModal();
      }
    }

    double *audioData = new double[m_audiofile->ArrayLength];

    // convert audio data into doubles if needed
    if (m_audiofile->shortAudioData != NULL) {
      for (unsigned i = 0; i < m_audiofile->ArrayLength; i++)
        audioData[i] = (double) m_audiofile->shortAudioData[i] / (1.0 * 0x7FFF);
    } else if (m_audiofile->intAudioData != NULL) {
      for (unsigned i = 0; i < m_audiofile->ArrayLength; i++)
        audioData[i] = (double) m_audiofile->intAudioData[i] / (1.0 * 0x7FFFFFFF);
    } else if (m_audiofile->doubleAudioData != NULL) {
      for (unsigned i = 0; i < m_audiofile->ArrayLength; i++)
        audioData[i] = m_audiofile->doubleAudioData[i];
    }

    // perform fade(s) as needed
    if (m_cutNFade->GetFadeStart() > 0)
      m_audiofile->PerformFade(audioData, m_cutNFade->GetFadeStart(), 0);

    if (m_cutNFade->GetFadeEnd() > 0)
      m_audiofile->PerformFade(audioData, m_cutNFade->GetFadeEnd(), 1);

    // update the waveform data
    m_waveform->UpdateWaveTracks(audioData, m_audiofile->m_channels, m_audiofile->ArrayLength);

    // update the current audiodata in m_audiofile
    if (m_audiofile->shortAudioData != NULL) {
      for (unsigned i = 0; i < m_audiofile->ArrayLength; i++)
        m_audiofile->shortAudioData[i] = lrint(audioData[i] * (1.0 * 0x7FFF));
    } else if (m_audiofile->intAudioData != NULL) {
      for (unsigned i = 0; i < m_audiofile->ArrayLength; i++)
        m_audiofile->intAudioData[i] = lrint(audioData[i] * (1.0 * 0x7FFFFFFF));
    } else if (m_audiofile->doubleAudioData != NULL) {
      for (unsigned i = 0; i < m_audiofile->ArrayLength; i++)
        m_audiofile->doubleAudioData[i] = audioData[i];
    }

    // Enable save icon and menu
    toolBar->EnableTool(wxID_SAVE, true);
    fileMenu->Enable(wxID_SAVE, true);
    fileMenu->Enable(SAVE_AND_OPEN_NEXT, true);

    // populate the wxGrid in m_panel with the loop data and add it to the waveform drawer
    m_panel->EmptyTable();
    m_waveform->ClearMetadata();
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

    // then we should make sure to update the views
    UpdateAllViews();

    delete[] audioData;
  } else {
    // user clicked cancel...
  }
}

void MyFrame::OnLoopPlayback(wxCommandEvent& event) {
  if (event.IsChecked())
    m_loopOnly = true;
  else
    m_loopOnly = false;
}

void MyFrame::OnKeyboardInput(wxKeyEvent& event) {
  // spacebar should start or stop playback depending if playback or stop is activated
  if (event.GetKeyCode() == WXK_SPACE) {
    if (toolBar->GetToolEnabled(START_PLAYBACK)) {
      wxCommandEvent evt(wxEVT_COMMAND_TOOL_CLICKED, START_PLAYBACK);
      ::wxGetApp().frame->AddPendingEvent(evt);
      return;
    } else if (toolBar->GetToolEnabled(wxID_STOP)) {
      wxCommandEvent evt(wxEVT_COMMAND_TOOL_CLICKED, wxID_STOP);
      ::wxGetApp().frame->AddPendingEvent(evt);
      return;
    } else
      return;
  }

  // A move volume slider right (volume multiple up)
  if (event.GetKeyCode() == 65 && event.GetModifiers() == wxMOD_NONE) {
    wxSlider *volumeSl = (wxSlider*) FindWindow(ID_VOLUME_SLIDER);
    int value = volumeSl->GetValue();
    if (value < 4) {
      volumeSl->SetValue(value + 1);
      wxCommandEvent evt(wxEVT_COMMAND_SLIDER_UPDATED, ID_VOLUME_SLIDER);
      ::wxGetApp().frame->AddPendingEvent(evt);
    }
    return;
  }

  // Z move volume slider (volume multiple down)
  if (event.GetKeyCode() == 90 && event.GetModifiers() == wxMOD_NONE) {
    wxSlider *volumeSl = (wxSlider*) FindWindow(ID_VOLUME_SLIDER);
    int value = volumeSl->GetValue();
    if (value > 0) {
      volumeSl->SetValue(value - 1);
      wxCommandEvent evt(wxEVT_COMMAND_SLIDER_UPDATED, ID_VOLUME_SLIDER);
      ::wxGetApp().frame->AddPendingEvent(evt);
    }
    return;
  }

  // S moves selection up the filelist
  if (event.GetKeyCode() == 83 && event.GetModifiers() == wxMOD_NONE) {
    if (currentSelectedIdx > 0 && currentSelectedIdx != wxNOT_FOUND) {
      m_fileListCtrl->SetItemState(currentSelectedIdx - 1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
      m_fileListCtrl->EnsureVisible(currentSelectedIdx);
    }
    return;
  }

  // X moves selection down the filelist
  if (event.GetKeyCode() == 88 && event.GetModifiers() == wxMOD_NONE) {
    if (currentSelectedIdx < (fileNames.GetCount() - 1) && currentSelectedIdx != wxNOT_FOUND) {
      m_fileListCtrl->SetItemState(currentSelectedIdx + 1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
      m_fileListCtrl->EnsureVisible(currentSelectedIdx);
    }
    return;
  }

  // D moves selection up the looplist
  if (event.GetKeyCode() == 68 && event.GetModifiers() == wxMOD_NONE) {
    if (m_panel->m_grid->GetNumberRows() > 0) {
      wxArrayInt selection = m_panel->m_grid->GetSelectedRows();
      if (!selection.IsEmpty()) {
      int firstSelected = selection[0];
        if (firstSelected > 0) {
          m_panel->m_grid->SelectRow(firstSelected - 1, false);
          m_panel->m_grid->SetGridCursor(firstSelected - 1, 1);

          // set/update the currently selected loops positions
          LOOPDATA currentLoop;
          m_audiofile->m_loops->GetLoopData(firstSelected - 1, currentLoop);
          if (!m_loopOnly)
            m_sound->SetLoopPosition(0, currentLoop.dwStart, currentLoop.dwEnd, m_audiofile->m_channels);
          else {
            if (((double) (currentLoop.dwEnd - currentLoop.dwStart)) / (double) m_audiofile->GetSampleRate() > 0.5) {
              unsigned pos = currentLoop.dwEnd - (m_audiofile->GetSampleRate() / 2);
              m_sound->SetLoopPosition(pos, currentLoop.dwStart, currentLoop.dwEnd, m_audiofile->m_channels);
            } else {
              m_sound->SetLoopPosition(currentLoop.dwStart, currentLoop.dwStart, currentLoop.dwEnd, m_audiofile->m_channels);
            }
          }
        }
      } else {
        if (m_panel->m_cueGrid->IsSelection())
          m_panel->m_cueGrid->ClearSelection();
        m_panel->m_grid->SelectRow(0, false);
        m_panel->m_grid->SetGridCursor(0, 1);

        // set/update the currently selected loops positions
        LOOPDATA currentLoop;
        m_audiofile->m_loops->GetLoopData(0, currentLoop);
        if (!m_loopOnly)
          m_sound->SetLoopPosition(0, currentLoop.dwStart, currentLoop.dwEnd, m_audiofile->m_channels);
        else {
          if (((double) (currentLoop.dwEnd - currentLoop.dwStart)) / (double) m_audiofile->GetSampleRate() > 0.5) {
            unsigned pos = currentLoop.dwEnd - (m_audiofile->GetSampleRate() / 2);
            m_sound->SetLoopPosition(pos, currentLoop.dwStart, currentLoop.dwEnd, m_audiofile->m_channels);
          } else {
            m_sound->SetLoopPosition(currentLoop.dwStart, currentLoop.dwStart, currentLoop.dwEnd, m_audiofile->m_channels);
          }
        }
        SetLoopPlayback(true);
        toolBar->EnableTool(X_FADE, true);
        toolMenu->Enable(X_FADE, true);
      }
    }
    return;
  }

  // C moves selection down the looplist
  if (event.GetKeyCode() == 67 && event.GetModifiers() == wxMOD_NONE) {
    if (m_panel->m_grid->GetNumberRows() > 0) {
      wxArrayInt selection = m_panel->m_grid->GetSelectedRows();
      if (!selection.IsEmpty()) {
      int firstSelected = selection[0];
        if (firstSelected < (m_panel->m_grid->GetNumberRows() - 1)) {
          m_panel->m_grid->SelectRow(firstSelected + 1, false);
          m_panel->m_grid->SetGridCursor(firstSelected + 1, 1);

          // set/update the currently selected loops positions
          LOOPDATA currentLoop;
          m_audiofile->m_loops->GetLoopData(firstSelected + 1, currentLoop);
          if (!m_loopOnly)
            m_sound->SetLoopPosition(0, currentLoop.dwStart, currentLoop.dwEnd, m_audiofile->m_channels);
          else {
            if (((double) (currentLoop.dwEnd - currentLoop.dwStart)) / (double) m_audiofile->GetSampleRate() > 0.5) {
              unsigned pos = currentLoop.dwEnd - (m_audiofile->GetSampleRate() / 2);
              m_sound->SetLoopPosition(pos, currentLoop.dwStart, currentLoop.dwEnd, m_audiofile->m_channels);
            } else {
              m_sound->SetLoopPosition(currentLoop.dwStart, currentLoop.dwStart, currentLoop.dwEnd, m_audiofile->m_channels);
            }
          }
        }
      } else {
        if (m_panel->m_cueGrid->IsSelection())
          m_panel->m_cueGrid->ClearSelection();
        m_panel->m_grid->SelectRow(0, false);
        m_panel->m_grid->SetGridCursor(0, 1);

        // set/update the currently selected loops positions
        LOOPDATA currentLoop;
        m_audiofile->m_loops->GetLoopData(0, currentLoop);
        if (!m_loopOnly)
          m_sound->SetLoopPosition(0, currentLoop.dwStart, currentLoop.dwEnd, m_audiofile->m_channels);
        else {
          if (((double) (currentLoop.dwEnd - currentLoop.dwStart)) / (double) m_audiofile->GetSampleRate() > 0.5) {
            unsigned pos = currentLoop.dwEnd - (m_audiofile->GetSampleRate() / 2);
            m_sound->SetLoopPosition(pos, currentLoop.dwStart, currentLoop.dwEnd, m_audiofile->m_channels);
          } else {
            m_sound->SetLoopPosition(currentLoop.dwStart, currentLoop.dwStart, currentLoop.dwEnd, m_audiofile->m_channels);
          }
        }
        SetLoopPlayback(true);
        toolBar->EnableTool(X_FADE, true);
        toolMenu->Enable(X_FADE, true);
      }
    }
    return;
  }

  // F moves selection up the cuelist
  if (event.GetKeyCode() == 70 && event.GetModifiers() == wxMOD_NONE) {
    if (m_panel->m_cueGrid->GetNumberRows() > 0) {
      wxArrayInt selection = m_panel->m_cueGrid->GetSelectedRows();
      if (!selection.IsEmpty()) {
      int firstSelected = selection[0];
        if (firstSelected > 0) {
          m_panel->m_cueGrid->SelectRow(firstSelected - 1, false);
          m_panel->m_cueGrid->SetGridCursor(firstSelected - 1, 1);

          // set/update the currently selected cue position
          CUEPOINT currentCue;
          m_audiofile->m_cues->GetCuePoint(firstSelected - 1, currentCue);
          m_sound->SetLoopPosition(currentCue.dwSampleOffset, currentCue.dwSampleOffset, currentCue.dwSampleOffset, m_audiofile->m_channels);

          SetLoopPlayback(false); // set the playback to not be for loops
        }
      } else {
        if (m_panel->m_grid->IsSelection())
          m_panel->m_grid->ClearSelection();
        m_panel->m_cueGrid->SelectRow(0, false);
        m_panel->m_cueGrid->SetGridCursor(0, 1);

        // set/update the currently selected cue position
        CUEPOINT currentCue;
        m_audiofile->m_cues->GetCuePoint(0, currentCue);
        m_sound->SetLoopPosition(currentCue.dwSampleOffset, currentCue.dwSampleOffset, currentCue.dwSampleOffset, m_audiofile->m_channels);

        SetLoopPlayback(false); // set the playback to not be for loops
        toolBar->EnableTool(X_FADE, false);
        toolMenu->Enable(X_FADE, false);
      }
    }
    return;
  }

  // V moves selection down the cuelist
  if (event.GetKeyCode() == 86 && event.GetModifiers() == wxMOD_NONE) {
    if (m_panel->m_cueGrid->GetNumberRows() > 0) {
      wxArrayInt selection = m_panel->m_cueGrid->GetSelectedRows();
      if (!selection.IsEmpty()) {
      int firstSelected = selection[0];
        if (firstSelected < (m_panel->m_cueGrid->GetNumberRows() - 1)) {
          m_panel->m_cueGrid->SelectRow(firstSelected + 1, false);
          m_panel->m_cueGrid->SetGridCursor(firstSelected + 1, 1);

          // set/update the currently selected cue position
          CUEPOINT currentCue;
          m_audiofile->m_cues->GetCuePoint(firstSelected + 1, currentCue);
          m_sound->SetLoopPosition(currentCue.dwSampleOffset, currentCue.dwSampleOffset, currentCue.dwSampleOffset, m_audiofile->m_channels);

          SetLoopPlayback(false); // set the playback to not be for loops
        }
      } else {
        if (m_panel->m_grid->IsSelection())
          m_panel->m_grid->ClearSelection();
        m_panel->m_cueGrid->SelectRow(0, false);
        m_panel->m_cueGrid->SetGridCursor(0, 1);

        // set/update the currently selected cue position
        CUEPOINT currentCue;
        m_audiofile->m_cues->GetCuePoint(0, currentCue);
        m_sound->SetLoopPosition(currentCue.dwSampleOffset, currentCue.dwSampleOffset, currentCue.dwSampleOffset, m_audiofile->m_channels);

        SetLoopPlayback(false); // set the playback to not be for loops
        toolBar->EnableTool(X_FADE, false);
        toolMenu->Enable(X_FADE, false);
      }
    }
    return;
  }

  // W toggles currently selected loop/cue save option on/off
  if (event.GetKeyCode() == 87 && event.GetModifiers() == wxMOD_NONE) {
    if (m_panel->m_grid->IsSelection() && m_panel->m_grid->GetNumberRows() > 0) {
      wxArrayInt selection = m_panel->m_grid->GetSelectedRows();
      if (!selection.IsEmpty()) {
        int firstSelected = selection[0];
        if (m_panel->m_grid->GetCellValue(firstSelected, 4) == wxT("1")) {
          // Equals to a true value to begin with
          m_panel->m_grid->SetCellValue(firstSelected, 4, wxT("0"));
          m_audiofile->m_loops->SetSaveOption(false, firstSelected);
          toolBar->EnableTool(wxID_SAVE, true);
          fileMenu->Enable(wxID_SAVE, true);
          fileMenu->Enable(SAVE_AND_OPEN_NEXT, true);
        } else {
          // The value was false
          m_panel->m_grid->SetCellValue(firstSelected, 4, wxT("1"));
          m_audiofile->m_loops->SetSaveOption(true, firstSelected);
          toolBar->EnableTool(wxID_SAVE, true);
          fileMenu->Enable(wxID_SAVE, true);
          fileMenu->Enable(SAVE_AND_OPEN_NEXT, true);
        }
      }
    } else if (m_panel->m_cueGrid->IsSelection() && m_panel->m_cueGrid->GetNumberRows() > 0) {
      wxArrayInt selection = m_panel->m_cueGrid->GetSelectedRows();
      if (!selection.IsEmpty()) {
        int firstSelected = selection[0];
        if (m_panel->m_cueGrid->GetCellValue(firstSelected, 2) == wxT("1")) {
          // Equals to a true value to begin with
          m_panel->m_cueGrid->SetCellValue(firstSelected, 2, wxT("0"));
          m_audiofile->m_cues->SetSaveOption(false, firstSelected);
          toolBar->EnableTool(wxID_SAVE, true);
          fileMenu->Enable(wxID_SAVE, true);
          fileMenu->Enable(SAVE_AND_OPEN_NEXT, true);
        } else {
          // The value was false
          m_panel->m_cueGrid->SetCellValue(firstSelected, 2, wxT("1"));
          m_audiofile->m_cues->SetSaveOption(true, firstSelected);
          toolBar->EnableTool(wxID_SAVE, true);
          fileMenu->Enable(wxID_SAVE, true);
          fileMenu->Enable(SAVE_AND_OPEN_NEXT, true);
        }
      }
    }
    return;
  }

  // E toggles all loop(s) save option on/off
  if (event.GetKeyCode() == 69 && event.GetModifiers() == wxMOD_NONE) {
    if (m_audiofile != NULL) {
      for (int i = 0; i < m_audiofile->m_loops->GetNumberOfLoops(); i++) {
        if (m_panel->m_grid->GetCellValue(i, 4) == wxT("1")) {
          // Equals to a true value to begin with
          m_panel->m_grid->SetCellValue(i, 4, wxT("0"));
          m_audiofile->m_loops->SetSaveOption(false, i);
        } else {
          // The value was false
          m_panel->m_grid->SetCellValue(i, 4, wxT("1"));
          m_audiofile->m_loops->SetSaveOption(true, i);
        }
      }
      toolBar->EnableTool(wxID_SAVE, true);
      fileMenu->Enable(wxID_SAVE, true);
      fileMenu->Enable(SAVE_AND_OPEN_NEXT, true);
    }
    return;
  }

  // Q toggles all cue(s) save option on/off
  if (event.GetKeyCode() == 81 && event.GetModifiers() == wxMOD_NONE) {
    if (m_audiofile != NULL) {
      for (unsigned i = 0; i < m_audiofile->m_cues->GetNumberOfCues(); i++) {
        if (m_panel->m_cueGrid->GetCellValue(i, 2) == wxT("1")) {
          // Equals to a true value to begin with
          m_panel->m_cueGrid->SetCellValue(i, 2, wxT("0"));
          m_audiofile->m_cues->SetSaveOption(false, i);
        } else {
          // The value was false
          m_panel->m_cueGrid->SetCellValue(i, 2, wxT("1"));
          m_audiofile->m_cues->SetSaveOption(true, i);
        }
      }
      toolBar->EnableTool(wxID_SAVE, true);
      fileMenu->Enable(wxID_SAVE, true);
      fileMenu->Enable(SAVE_AND_OPEN_NEXT, true);
    }
    return;
  }

  event.Skip();
}

void MyFrame::OnGridCellSelect(wxGridEvent& event) {
  wxSize visibleSize = m_panel->GetClientSize();
  int rowNbr = event.GetRow();
  int adjustedRowNbr = rowNbr + 2;
  int rowNbrInView = visibleSize.GetHeight() / m_panel->m_grid->GetRowSize(rowNbr);

  if (adjustedRowNbr > rowNbrInView)
    m_panel->Scroll(0, (m_panel->m_grid->GetRowSize(rowNbr) / 5) * (adjustedRowNbr + 2 - rowNbrInView));
  else if (adjustedRowNbr < rowNbrInView)
    m_panel->Scroll(0, (m_panel->m_grid->GetRowSize(rowNbr) / -5) * (rowNbrInView - (adjustedRowNbr)));
}

void MyFrame::OnCueGridCellSelect(wxGridEvent& event) {
  if (m_panel->m_cueGrid->GetNumberRows() > 0) {
    int rowNbr = event.GetRow();
    wxSize visibleSize = m_panel->GetClientSize();
    wxSize loopGridSize = m_panel->m_grid->GetClientSize();
    int adjustedRowNbr = rowNbr + 2;
    int rowSize = m_panel->m_cueGrid->GetRowSize(rowNbr);
    int rowNbrInView = visibleSize.GetHeight() / rowSize;

    if (adjustedRowNbr > rowNbrInView)
      m_panel->Scroll(0, (rowSize / 5) * (adjustedRowNbr + 2 - rowNbrInView) + loopGridSize.GetHeight() / 5 + rowSize / 5 * 2);
    else if (adjustedRowNbr < rowNbrInView)
      m_panel->Scroll(0, (rowSize / -5) * (rowNbrInView - (adjustedRowNbr)) + loopGridSize.GetHeight() / 5 + rowSize / 5 * 2);
  }
}

void MyFrame::PopulateListCtrl() {
  m_fileListCtrl->ClearAll();

  wxListItem itemCol;
  itemCol.SetText(wxT("File name"));
  itemCol.SetImage(0);
  m_fileListCtrl->InsertColumn(0, itemCol);

  itemCol.SetText(wxT("Loops"));
  itemCol.SetAlign(wxLIST_FORMAT_CENTRE);
  m_fileListCtrl->InsertColumn(1, itemCol);

  itemCol.SetText(wxT("Cues"));
  m_fileListCtrl->InsertColumn(2, itemCol);

  itemCol.SetText(wxT("Note"));
  m_fileListCtrl->InsertColumn(3, itemCol);

  itemCol.SetText(wxT("Fraction"));
  m_fileListCtrl->InsertColumn(4, itemCol);

  m_fileListCtrl->Hide();

  if (!fileNames.IsEmpty()) {
    for (int i = 0; i < fileNames.GetCount(); i++) {
      m_fileListCtrl->InsertItem(i, fileNames[i]);

      SF_INSTRUMENT instr;
      SF_CUES cues;
      SndfileHandle sfh;
      wxString filePath;
      filePath = workingDir;
      filePath += wxFILE_SEP_PATH;
      filePath += fileNames[i];
      sfh = SndfileHandle(((const char*)filePath.mb_str()));

      if (sfh) {
        if (sfh.command(4304, &instr, sizeof(instr)) == SF_TRUE) {
          wxString loopNbr = wxString::Format(wxT("%i"), instr.loop_count);
          m_fileListCtrl->SetItem(i, 1, loopNbr);

          int midiNote = (int) instr.basenote;
          wxString note = wxString::Format(wxT("%i"), midiNote);
          m_fileListCtrl->SetItem(i, 3, note);

          double cents = (double) instr.dwMIDIPitchFraction / (double)UINT_MAX * 100.0;
          wxString fraction = wxString::Format(wxT("%.2f"), cents);
          m_fileListCtrl->SetItem(i, 4, fraction);
        } else {
          m_fileListCtrl->SetItem(i, 1, wxT("0"));
          m_fileListCtrl->SetItem(i, 3, wxT("0"));
          m_fileListCtrl->SetItem(i, 4, wxT("0"));
        }

        if (sfh.command(4302, &cues, sizeof(cues)) == SF_TRUE) {
          wxString cueNbr = wxString::Format(wxT("%i"), cues.cue_count);
          m_fileListCtrl->SetItem(i, 2, cueNbr);
        } else {
          m_fileListCtrl->SetItem(i, 2, wxT("0"));
        }
      }
    }
  }

  m_fileListCtrl->SetColumnWidth(0, wxLIST_AUTOSIZE);
  m_fileListCtrl->SetColumnWidth(1, wxLIST_AUTOSIZE_USEHEADER);
  m_fileListCtrl->SetColumnWidth(2, wxLIST_AUTOSIZE_USEHEADER);
  m_fileListCtrl->SetColumnWidth(3, wxLIST_AUTOSIZE_USEHEADER);
  m_fileListCtrl->SetColumnWidth(4, wxLIST_AUTOSIZE_USEHEADER);

  m_fileListCtrl->Show();
}

void MyFrame::OnHelp(wxCommandEvent& event) {
  ::wxGetApp().m_helpController->DisplayContents();
}
