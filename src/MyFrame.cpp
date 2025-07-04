/*
 * MyFrame.cpp is a part of LoopAuditioneer software
 * Copyright (C) 2011-2025 Lars Palo and contributors (see AUTHORS file)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
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
#include <wx/settings.h>
#include <wx/filename.h>
#include "ListInfoDialog.h"
#include "AudioSettingsDialog.h"
#include "FreePixelIcons.h"

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
  EVT_TOOL(VIEW_LOOPPOINTS, MyFrame::OnViewLoop)
  EVT_MENU(LOOP_ONLY, MyFrame::OnLoopPlayback)
  EVT_MENU(SAVE_AND_OPEN_NEXT, MyFrame::OnSaveOpenNext)
  EVT_MENU(wxID_HELP, MyFrame::OnHelp)
  EVT_MENU(AUDIO_SETTINGS, MyFrame::OnAudioSettings)
  EVT_MENU(CLOSE_OPEN_PREV, MyFrame::OnCloseOpenPrev)
  EVT_MENU(CLOSE_OPEN_NEXT, MyFrame::OnCloseOpenNext)
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
  EVT_TOOL(VIEW_LOOPPOINTS, MyFrame::OnViewLoop)
  EVT_TOOL(CUT_N_FADE, MyFrame::OnCutFade)
  EVT_TOOL(LIST_INFO, MyFrame::OnListInfo)
  EVT_KEY_DOWN(MyFrame::OnKeyboardInput)
  EVT_SIZE(MyFrame::OnSize)
END_EVENT_TABLE()

void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event)) {
  wxAboutDialogInfo info;
  info.SetName(appName);
  info.SetVersion(wxT(MY_APP_VERSION));
  info.SetDescription(wxT("This program allows users to view, create, edit and listen to loops and cues embedded in wav files as well as perform other tasks necessary for preparing samples for usage in sample players."));
  info.SetCopyright(wxT("Copyright (C) 2011-2025 Lars Palo and contributors (see AUTHORS)\n<larspalo AT yahoo DOT se>\nReleased under GNU GPLv3 licence"));
  info.SetWebSite(wxT("https://github.com/GrandOrgue/LoopAuditioneer"));

  wxAboutBox(info);
}

void MyFrame::OnSelectDir(wxCommandEvent& WXUNUSED(event)) {
  wxString defaultPath = wxT("/home/$USER");
  if (workingDir != wxEmptyString)
    defaultPath = workingDir;
  wxDirDialog dialog(::wxGetApp().frame, wxT("Choose a folder with samples"), defaultPath);
  if (dialog.ShowModal() == wxID_OK) {
    // close any open files
    CloseOpenAudioFile();
    
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

void MyFrame::OnClose(wxCloseEvent& WXUNUSED(event)) {
  config->Write(wxT("General/LastWorkingDir"), workingDir);
  wxSlider *volumeSl = (wxSlider*) FindWindow(ID_VOLUME_SLIDER);
  int vol = volumeSl->GetValue();
  config->Write(wxT("General/LastVolume"), vol);
  config->Write(wxT("General/LoopOnlyPlayback"), m_loopOnly);
  GetCurrentFrameSizes();
  config->Write(wxT("General/FrameXPosition"), m_xPosition);
  config->Write(wxT("General/FrameYPosition"), m_yPosition);
  config->Write(wxT("General/FrameWidth"), m_frameWidth);
  config->Write(wxT("General/FrameHeight"), m_frameHeight);
  config->Write(wxT("General/FrameMaximized"), m_frameMaximized);
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
  config->Write(wxT("Audio/Api"), m_sound->GetApi());
  config->Write(wxT("Audio/Device"), m_sound->GetDevice());
  config->Write(wxT("Pitch/PitchMethod"), m_pitchMethod);
  config->Write(wxT("Pitch/SpectrumFftSize"), m_spectrumFftSize);
  config->Write(wxT("Pitch/SpectrumWindow"), m_spectrumWindow);
  config->Write(wxT("Pitch/SpectrumInterpolatePitch"), m_spectrumInterpolatePitch);
  
  config->Flush();

  Destroy();
}

void MyFrame::OnQuit(wxCommandEvent& WXUNUSED(event)) {
  // Close the frame by calling OnClose() via event generation
  Close(true);
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
    m_fileListCtrl->EnsureVisible(sel);
  }
}

void MyFrame::OnOpenSelected(wxCommandEvent& WXUNUSED(event)) {
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
  if (!toolBar->GetToolEnabled(OPEN_SELECTED))
    toolBar->EnableTool(OPEN_SELECTED, true);
  if (!fileMenu->IsEnabled(OPEN_SELECTED))
    fileMenu->Enable(OPEN_SELECTED, true);
  if (!fileMenu->IsEnabled(CLOSE_OPEN_PREV))
    fileMenu->Enable(CLOSE_OPEN_PREV, true);
  if (!fileMenu->IsEnabled(CLOSE_OPEN_NEXT))
    fileMenu->Enable(CLOSE_OPEN_NEXT, true);
}

void MyFrame::OpenAudioFile() {
  CloseOpenAudioFile();
  m_audiofile = new FileHandling(fileToOpen, workingDir);

  if (m_audiofile->FileCouldBeOpened()) {
    // set sustainsection from slider data in autoloop settings to audiofile
    m_audiofile->SetSliderSustainsection(m_autoloopSettings->GetStart(), m_autoloopSettings->GetEnd());
    // adjust choice of sustainsection
    m_audiofile->SetAutoSustainSearch(m_autoloopSettings->GetAutosearch());
    
    wxString filePath;
    filePath = workingDir;
    filePath += wxFILE_SEP_PATH;
    filePath += fileToOpen;
    m_waveform = new WaveformDrawer(this, m_audiofile);
    lowerBox->Clear();
    lowerBox->Add(m_waveform, 1, wxEXPAND, 0);
    vbox->Layout();

    m_sound->SetSampleRate(m_audiofile->GetSampleRate());
    m_sound->SetChannels(m_audiofile->m_channels);
    wxFileName fullFilePath(filePath);
    m_panel->SetFileNameLabel(fullFilePath);

    SetStatusText(wxString::Format(wxT("Zoom level: x %i"), m_waveform->GetAmplitudeZoomLevel()), 1);

    UpdateLoopsAndCuesDisplay();

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
    toolBar->EnableTool(LIST_INFO, true);
    toolMenu->Enable(LIST_INFO, true);

    // select first loop if such is present
    if (m_panel->m_grid->GetNumberRows() > 0) {
      m_panel->m_grid->SelectRow(0, false);
      m_waveform->SetLoopSelection(0);
      toolBar->EnableTool(wxID_STOP, false);
      toolBar->EnableTool(START_PLAYBACK, true);
      transportMenu->Enable(START_PLAYBACK, true);
      transportMenu->Enable(wxID_STOP, false);
      toolBar->EnableTool(X_FADE, true);
      toolMenu->Enable(X_FADE, true);
      toolBar->EnableTool(VIEW_LOOPPOINTS, true);
      toolMenu->Enable(VIEW_LOOPPOINTS, true);
      SetLoopPlayback(true);
    }

    if (m_sound->StreamNeedsResampling()) {
      // initialize samplerate converter
      m_resampler = new MyResampler(m_audiofile->m_channels);
      // fill resample data struct with values
      double src_ratio = (1.0 * m_sound->GetSampleRateToUse()) / (1.0 * m_audiofile->GetSampleRate());
      m_resampler->SetDataEndOfInput(0); // Set this later
      m_resampler->SetDataInputFrames(m_audiofile->ArrayLength / m_audiofile->m_channels);
      m_resampler->SetDataIn(m_audiofile->floatAudioData);
      m_resampler->SetDataSrcRatio(src_ratio);
      m_resampler->SimpleResample(m_audiofile->m_channels);
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
    wxFileName fullFileName(wxEmptyString, wxEmptyString);
    m_panel->SetFileNameLabel(fullFileName);

    SetStatusText(wxEmptyString, 1);

  } // end of if file open was successful checking
}

void MyFrame::CloseOpenAudioFile() {
  // if audio stream is running it must be stopped first
  if (m_sound->IsStreamActive()) {
    DoStopPlay();
  }

  // Cleanup samplerate converter
  if (m_resampler != NULL) {
    delete m_resampler;
    m_resampler = 0;
  }

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
  toolBar->EnableTool(VIEW_LOOPPOINTS, false);
  toolMenu->Enable(VIEW_LOOPPOINTS, false);
  toolBar->EnableTool(CUT_N_FADE, false);
  toolMenu->Enable(CUT_N_FADE, false);
  toolBar->EnableTool(LIST_INFO, false);
  toolMenu->Enable(LIST_INFO, false);

  m_sound->CloseAudioStream();

  if (m_audiofile)
    m_panel->EmptyTable();
  if (m_audiofile != NULL) {
    delete m_audiofile;
    m_audiofile = 0;
  }
  if (m_waveform != NULL) {
    delete m_waveform;
    m_waveform = 0;
  }

  SetStatusText(wxEmptyString, 1);
  m_panel->fileNameLabel->SetLabel(wxT("Current open file: "));
  lowerBox->Clear();
  lowerBox->AddStretchSpacer();
  vbox->Layout();

  UpdateAllViews();
}

void MyFrame::OnGridCellClick(wxGridEvent& event) {
  if (event.GetRow() >= 0) {
    if (m_panel->m_grid->IsSelection())
      m_panel->m_grid->ClearSelection();
    if (m_panel->m_cueGrid->IsSelection())
      m_panel->m_cueGrid->ClearSelection();
    m_panel->m_grid->SelectRow(event.GetRow());

    // notify waveform drawer of selected loop
    m_waveform->SetLoopSelection(event.GetRow());
    UpdateAllViews();

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

    if (event.GetCol() == 5) {
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
    toolBar->EnableTool(VIEW_LOOPPOINTS, true);
    toolMenu->Enable(VIEW_LOOPPOINTS, true);
  }
}

void MyFrame::OnCueGridCellClick(wxGridEvent& event) {
  if (event.GetRow() >= 0) {
    if (m_panel->m_grid->IsSelection())
      m_panel->m_grid->ClearSelection();
    if (m_panel->m_cueGrid->IsSelection())
      m_panel->m_cueGrid->ClearSelection();
    m_panel->m_cueGrid->SelectRow(event.GetRow());

    // notify waveform drawer of cue selection
    m_waveform->SetCueSelection(event.GetRow());
    UpdateAllViews();

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
    toolBar->EnableTool(VIEW_LOOPPOINTS, false);
    toolMenu->Enable(VIEW_LOOPPOINTS, false);
  }
}

void MyFrame::UpdateCurrentFileInfo() {
  if (!m_audiofile->m_loops->loopsOut.empty()) {
    wxString loopNbr = wxString::Format(wxT("%u"), (unsigned) m_audiofile->m_loops->loopsOut.size());
    m_fileListCtrl->SetItem(currentOpenFileIdx, 1, loopNbr);
  } else {
    m_fileListCtrl->SetItem(currentOpenFileIdx, 1, wxT("0"));
  }

  if (!m_audiofile->m_cues->exportedCues.empty()) {
    wxString cueNbr = wxString::Format(wxT("%u"), (unsigned) m_audiofile->m_cues->exportedCues.size());
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

void MyFrame::OnSaveFile(wxCommandEvent& WXUNUSED(event)) {
  m_audiofile->SaveAudioFile(fileToOpen, workingDir);

  // make sure to update the listCtrl columns as things might have changed
  UpdateCurrentFileInfo();

  // open the file again to remove old leftovers in grids etc.
  OpenAudioFile();

  m_fileListCtrl->EnsureVisible(currentSelectedIdx);
}

void MyFrame::OnSaveOpenNext(wxCommandEvent& WXUNUSED(event)) {
  m_audiofile->SaveAudioFile(fileToOpen, workingDir);

  // make sure to update the listCtrl columns as things might have changed
  UpdateCurrentFileInfo();

  // open next file if possible
  if (currentSelectedIdx != wxNOT_FOUND && (unsigned) currentSelectedIdx < (fileNames.GetCount() - 1)) {
    m_fileListCtrl->SetItemState(currentSelectedIdx + 1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    m_fileListCtrl->EnsureVisible(currentSelectedIdx);

    wxCommandEvent evt(wxEVT_TOOL, OPEN_SELECTED);
    ::wxGetApp().frame->AddPendingEvent(evt);
  } else {
    OpenAudioFile();
    m_fileListCtrl->EnsureVisible(currentSelectedIdx);
  }
}

void MyFrame::OnCloseOpenPrev(wxCommandEvent& WXUNUSED(event)) {
  // open previous file if possible
  if (currentSelectedIdx > 0 && currentSelectedIdx != wxNOT_FOUND) {
    m_fileListCtrl->SetItemState(currentSelectedIdx - 1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    m_fileListCtrl->EnsureVisible(currentSelectedIdx);

    wxCommandEvent evt(wxEVT_TOOL, OPEN_SELECTED);
    ::wxGetApp().frame->AddPendingEvent(evt);
  } else {
    OpenAudioFile();
    m_fileListCtrl->EnsureVisible(currentSelectedIdx);
  }
}

void MyFrame::OnCloseOpenNext(wxCommandEvent& WXUNUSED(event)) {
  // open next file if possible
  if (currentSelectedIdx != wxNOT_FOUND && (unsigned) currentSelectedIdx < (fileNames.GetCount() - 1)) {
    m_fileListCtrl->SetItemState(currentSelectedIdx + 1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    m_fileListCtrl->EnsureVisible(currentSelectedIdx);

    wxCommandEvent evt(wxEVT_TOOL, OPEN_SELECTED);
    ::wxGetApp().frame->AddPendingEvent(evt);
  } else {
    OpenAudioFile();
    m_fileListCtrl->EnsureVisible(currentSelectedIdx);
  }
}

void MyFrame::OnSaveFileAs(wxCommandEvent& WXUNUSED(event)) {
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

void MyFrame::OnStartPlay(wxCommandEvent& WXUNUSED(event)) {
  m_sound->OpenAudioStream();
  if (m_sound->IsStreamAvailable()) {
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
          if (m_sound->StreamNeedsResampling())
            m_sound->SetStartPosition(pos * m_resampler->GetRatioUsed(), m_audiofile->m_channels);
          else
            m_sound->SetStartPosition(pos, m_audiofile->m_channels);
          m_waveform->SetPlayPosition(pos);
        } else {
          if (m_sound->StreamNeedsResampling())
            m_sound->SetStartPosition(currentLoop.dwStart * m_resampler->GetRatioUsed(), m_audiofile->m_channels);
          else
            m_sound->SetStartPosition(currentLoop.dwStart, m_audiofile->m_channels);
          m_waveform->SetPlayPosition(currentLoop.dwStart);
        }
      }
    }

    // if it's a cue make sure start position is set to the selected cue's dwPosition
    if (m_panel->m_cueGrid->IsSelection()) {
      CUEPOINT currentCue;
      m_audiofile->m_cues->GetCuePoint(m_panel->m_cueGrid->GetGridCursorRow(), currentCue);
      if (m_sound->StreamNeedsResampling()) {
        m_sound->SetStartPosition(currentCue.dwSampleOffset * m_resampler->GetRatioUsed(), m_audiofile->m_channels);
      } else {
        m_sound->SetStartPosition(currentCue.dwSampleOffset, m_audiofile->m_channels);
      }
      m_waveform->SetPlayPosition(currentCue.dwSampleOffset);
    }

    toolBar->EnableTool(START_PLAYBACK, false);
    toolBar->EnableTool(wxID_STOP, true);
    transportMenu->Enable(START_PLAYBACK, false);
    transportMenu->Enable(wxID_STOP, true);
    m_sound->StartAudioStream();
  } else {
    toolBar->EnableTool(START_PLAYBACK, false);
    toolBar->EnableTool(wxID_STOP, false);
    transportMenu->Enable(START_PLAYBACK, false);
    transportMenu->Enable(wxID_STOP, false);
    wxMessageDialog *dialog = new wxMessageDialog(
      NULL,
      m_sound->GetLastError(),
      wxT("Please check audio settings!"),
      wxOK | wxICON_ERROR
    );
    dialog->ShowModal();
  }
}

void MyFrame::OnStopPlay(wxCommandEvent& WXUNUSED(event)) {
  DoStopPlay(); 
}

void MyFrame::DoStopPlay() {
  m_timer.Stop();
  m_sound->StopAudioStream();

  toolBar->EnableTool(wxID_STOP, false);
  toolBar->EnableTool(START_PLAYBACK, true);

  transportMenu->Enable(START_PLAYBACK, true);
  transportMenu->Enable(wxID_STOP, false);

  m_waveform->SetPlayPosition(0);
  m_waveform->paintNow();

  m_sound->CloseAudioStream();

}

MyFrame::MyFrame(const wxString& title) : wxFrame(NULL, wxID_ANY, title), m_timer(this, TIMER_ID) {
  m_audiofile = NULL;
  m_waveform = NULL;
  m_resampler = NULL;
  m_autoloopSettings = new AutoLoopDialog(this);
  m_autoloop = new AutoLooping();
  m_crossfades = new CrossfadeDialog(this);
  m_cutNFade = new CutNFadeDialog(this);
  m_batchProcess = new BatchProcessDialog(m_autoloopSettings, this);
  currentOpenFileIdx = -1;
  currentSelectedIdx = -1;
  fileToOpen = wxEmptyString;
  config = new wxFileConfig(wxT("LoopAuditioneer"));
  m_xPosition = 0;
  m_yPosition = 0;
  m_frameWidth = 1000;
  m_frameHeight = 560;
  m_frameMaximized = false;
  SetBackgroundStyle(wxBG_STYLE_SYSTEM);

  // Create a file menu
  fileMenu = new wxMenu();

  // Add file menu items
  fileMenu->Append(FILE_SELECT, wxT("&Choose folder\tCtrl+O"), wxT("Select working folder"));
  fileMenu->Append(OPEN_SELECTED, wxT("&Open file\tCtrl+F"), wxT("Open selected file"));
  fileMenu->Append(CLOSE_OPEN_PREV, wxT("Open previous file\tCtrl+Alt+U"), wxT("Open next upper file in filelist"));
  fileMenu->Append(CLOSE_OPEN_NEXT, wxT("Open next file\tCtrl+Alt+N"), wxT("Open next lower file in filelist"));
  fileMenu->Append(wxID_SAVE, wxT("&Save\tCtrl+S"), wxT("Save current file"));
  fileMenu->Append(wxID_SAVEAS, wxT("Save &as...\tShift+Ctrl+S"), wxT("Save current file with new name"));
  fileMenu->Append(SAVE_AND_OPEN_NEXT, wxT("Save, open next\tCtrl+Alt+S"), wxT("Save current file and open next"));
  fileMenu->AppendSeparator();
  fileMenu->Append(AUTOLOOP_SETTINGS, wxT("&Autoloop settings\tCtrl+A"), wxT("Adjust settings for loop searching"));
  fileMenu->Append(AUDIO_SETTINGS, wxT("Audio settings"), wxT("Adjust settings for audio api/device"));
  fileMenu->AppendSeparator();
  fileMenu->Append(wxID_EXIT, wxT("&Exit\tCtrl+Q"), wxT("Quit this program"));

  fileMenu->Enable(OPEN_SELECTED, false);
  fileMenu->Enable(CLOSE_OPEN_PREV, false);
  fileMenu->Enable(CLOSE_OPEN_NEXT, false);
  fileMenu->Enable(wxID_SAVE, false);
  fileMenu->Enable(wxID_SAVEAS, false);
  fileMenu->Enable(SAVE_AND_OPEN_NEXT, false);

  // Create a view menu
  viewMenu = new wxMenu();

  // Add view menu items
  viewMenu->Append(ZOOM_IN_AMP, wxT("Zoom &in\tCtrl++"), wxT("Zoom in on amplitude"));
  viewMenu->Append(ZOOM_OUT_AMP, wxT("Zoom &out\tCtrl+-"), wxT("Zoom out on amplitude"));

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
  toolMenu->Append(VIEW_LOOPPOINTS, wxT("View looppoints\tCtrl+W"), wxT("View waveform overlay at looppoints"));
  toolMenu->Append(CUT_N_FADE, wxT("Cut and &fade\tCtrl+C"), wxT("Cut & fade in/out"));
  toolMenu->Append(LIST_INFO, wxT("Info text\tCtrl+I"), wxT("View/set info text"));

  toolMenu->Enable(ADD_LOOP, false);
  toolMenu->Enable(AUTOSEARCH_LOOPS, false);
  toolMenu->Enable(PITCH_SETTINGS, false);
  toolMenu->Enable(X_FADE, false);
  toolMenu->Enable(VIEW_LOOPPOINTS, false);
  toolMenu->Enable(CUT_N_FADE, false);
  toolMenu->Enable(LIST_INFO, false);

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
  int statusWidths[3] = { -1, 150, -2 };
  SetStatusWidths(3, statusWidths);
  SetStatusText(wxT("Ready"), 0);

  // Create toolbar
  toolBar = CreateToolBar(wxNO_BORDER | wxTB_HORIZONTAL | wxTB_FLAT);
  toolBar->SetToolBitmapSize(wxSize(24, 24));
  wxImage::AddHandler(new wxPNGHandler);
  wxBitmap selectFolder = wxBITMAP_PNG_FROM_DATA(Open_folder);
  wxBitmap openSelectedFile = wxBITMAP_PNG_FROM_DATA(Open_file);
  wxBitmap saveFile = wxBITMAP_PNG_FROM_DATA(Save);
  wxBitmap saveFileAs = wxBITMAP_PNG_FROM_DATA(Save_as);
  wxBitmap startPlayback = wxBITMAP_PNG_FROM_DATA(Right);
  wxBitmap stopPlayback = wxBITMAP_PNG_FROM_DATA(Stop);
  wxBitmap loopCreation = wxBITMAP_PNG_FROM_DATA(Refresh);
  wxBitmap autoLoop = wxBITMAP_PNG_FROM_DATA(Search);
  wxBitmap autoLoopSettings = wxBITMAP_PNG_FROM_DATA(Yin_Yang);
  wxBitmap batchProcess = wxBITMAP_PNG_FROM_DATA(Gear);
  wxBitmap pitchInfo = wxBITMAP_PNG_FROM_DATA(Bell);
  wxBitmap zoomInAmp = wxBITMAP_PNG_FROM_DATA(Zoom_in);
  wxBitmap zoomOutAmp = wxBITMAP_PNG_FROM_DATA(Zoom_out);
  wxBitmap crossfade = wxBITMAP_PNG_FROM_DATA(Wizard);
  wxBitmap cutfade = wxBITMAP_PNG_FROM_DATA(Software);
  wxBitmap viewloop = wxBITMAP_PNG_FROM_DATA(Diagram);
  wxBitmap listInfo = wxBITMAP_PNG_FROM_DATA(Text);
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
  toolBar->AddTool(VIEW_LOOPPOINTS, wxT("View looppoints"), viewloop, wxT("View waveform overlay at looppoints"));
  toolBar->AddTool(CUT_N_FADE, wxT("Cut & Fade"), cutfade, wxT("Cut & Fade in/out"));
  toolBar->AddTool(LIST_INFO, wxT("Info text"), listInfo, wxT("View/set info text"));
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
  toolBar->EnableTool(VIEW_LOOPPOINTS, false);
  toolBar->EnableTool(CUT_N_FADE, false);
  toolBar->EnableTool(LIST_INFO, false);
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
  hbox->Add(m_fileListCtrl, 1, wxEXPAND | wxLEFT | wxTOP, 10);

  m_panel = new MyPanel(this);
  m_panel->SetMinSize(wxSize(600,150));
  hbox->Add(m_panel, 2, wxEXPAND | wxTOP | wxLEFT | wxRIGHT, 10);

  lowerBox = new wxBoxSizer(wxHORIZONTAL);
  lowerBox->AddStretchSpacer();
  vbox->Add(lowerBox, 1, wxEXPAND, 0);

  SetSizer(vbox);
  vbox->SetSizeHints(this);

  SetMinSize(wxSize(1000,560));

  // adjust width of the list control columns
  int colWidth = m_fileListCtrl->GetClientSize().GetWidth() / 5;

  m_fileListCtrl->InsertColumn(0, wxT("File name"), wxLIST_FORMAT_LEFT, colWidth);
  m_fileListCtrl->InsertColumn(1, wxT("Loops"), wxLIST_FORMAT_CENTRE, colWidth);
  m_fileListCtrl->InsertColumn(2, wxT("Cues"), wxLIST_FORMAT_CENTRE, colWidth);
  m_fileListCtrl->InsertColumn(3, wxT("Note"), wxLIST_FORMAT_CENTRE, colWidth);
  m_fileListCtrl->InsertColumn(4, wxT("Fraction"), wxLIST_FORMAT_CENTRE, colWidth);

  // create sound output
  wxString apiStr;
  int deviceId;
  if (config->Read(wxT("Audio/Api"), &apiStr)) {
    // if value was found it's now in apiStr
  } else {
    apiStr = wxEmptyString;
  }
  if (config->Read(wxT("Audio/Device"), &deviceId)) {
    // if value was found it's now in deviceId
  } else {
    // we set it too high so that the default device will be used instead
    deviceId = INT_MAX;
  }
  m_sound = new MySound(apiStr, (unsigned) deviceId);

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

  if (config->Read(wxT("General/FrameXPosition"), &readInt))
    m_xPosition = readInt;

  if (config->Read(wxT("General/FrameYPosition"), &readInt))
    m_yPosition = readInt;

  if (config->Read(wxT("General/FrameWidth"), &readInt))
    m_frameWidth = readInt;

  if (config->Read(wxT("General/FrameHeight"), &readInt))
    m_frameHeight = readInt;

  if (config->Read(wxT("General/FrameMaximized"), &b))
    m_frameMaximized = b;

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
    if (dbl <= 0.1 && dbl >= 0.0001) {
      m_autoloopSettings->SetQuality(dbl);
      m_autoloop->SetQuality(dbl);
    }
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

  if (config->Read(wxT("Pitch/PitchMethod"), &readInt)) {
    SetPitchMethod(readInt);
  } else {
    SetPitchMethod(0);
  }

  if (config->Read(wxT("Pitch/SpectrumFftSize"), &readInt)) {
    SetSpectrumFftSize(readInt);
  } else {
    SetSpectrumFftSize(5);
  }

  if (config->Read(wxT("Pitch/SpectrumWindow"), &readInt)) {
    SetSpectrumWindow(readInt);
  } else {
    SetSpectrumWindow(9);
  }

  if (config->Read(wxT("Pitch/SpectrumInterpolatePitch"), &b)) {
    SetSpectrumInterpolatePitch(b);
  } else {
    SetSpectrumInterpolatePitch(false);
  }

  if (m_frameMaximized)
    Maximize();
  else if (m_xPosition != 0 && m_yPosition != 0)
    SetSize(m_xPosition, m_yPosition, m_frameWidth, m_frameHeight);

  m_autoloopSettings->UpdateLabels();
  m_panel->SetFocus();
}

MyFrame::~MyFrame() {
  delete config;

  if (m_resampler) {
    delete m_resampler;
    m_resampler = 0;
  }
  if (m_audiofile) {
    delete m_audiofile;
    m_audiofile = 0;
  }
  if (m_sound) {
    m_sound->StopAudioStream();
    m_sound->CloseAudioStream();
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
  if (!fileNames.IsEmpty()) {
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
                           unsigned nBufferFrames,
                           double streamTime,
                           RtAudioStreamStatus status,
                           void *userData ) {
  (void)inputBuffer;
  (void)streamTime;
  (void)status;
  unsigned nChannels = (unsigned) ::wxGetApp().frame->m_audiofile->m_channels;
  unsigned useChannels = ::wxGetApp().frame->m_sound->GetChannelsUsed();
  unsigned excessChannels = 0;
  if (nChannels > useChannels)
    excessChannels = nChannels - useChannels;

  float *buffer = static_cast<float*>(outputBuffer);

  // keep track of position, see pos in MySound.h
  unsigned *position = (unsigned *) userData;

  if (::wxGetApp().frame->m_sound->StreamNeedsResampling()) {
    // audio stream uses the resampled audio data
    if (position[0] < ::wxGetApp().frame->m_resampler->m_resampledDataLength) {
      for (unsigned i = 0; i < nBufferFrames; i++) {
        for (unsigned j = 0; j < useChannels; j++) {
          *buffer++ = ::wxGetApp().frame->m_resampler->resampledAudioData[(position[0])] * volumeMultiplier;
          position[0] += 1;
        }

        if (excessChannels)
          position[0] += excessChannels;

        if (loopPlay) {
          // Check to control loop playback, see MySound.h pos member and MyFrame.h loopPlay member
          if (position[0] > lround(position[2] / nChannels * ::wxGetApp().frame->m_resampler->GetRatioUsed()) * nChannels )
            position[0] = lround(position[1] / nChannels * ::wxGetApp().frame->m_resampler->GetRatioUsed()) * nChannels;
        }

        // stop if end of file data is reached and reset current position to start of the cue
        if (position[0] > ::wxGetApp().frame->m_resampler->m_resampledDataLength - 1) {
          wxCommandEvent evt(wxEVT_COMMAND_TOOL_CLICKED, wxID_STOP);
          ::wxGetApp().frame->AddPendingEvent(evt);

          return 0;
        }
      }
    } else {
      // we end up here until buffer is drained?
    }
  } else {
    // Loop that feeds the outputBuffer with data when no resampling is needed
    if (position[0] < ::wxGetApp().frame->m_audiofile->ArrayLength) {
      for (unsigned i = 0; i < nBufferFrames; i++) {
        for (unsigned j = 0; j < useChannels; j++) {
          *buffer++ = ::wxGetApp().frame->m_audiofile->floatAudioData[(position[0])] * volumeMultiplier;
          position[0] += 1;
        }

        if (excessChannels)
          position[0] += excessChannels;

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
  }

  return 0;

}

void MyFrame::SetLoopPlayback(bool looping) {
  if (looping)
    loopPlay = true;
  else
    loopPlay = false;
}

void MyFrame::SetPitchMethod(int method) {
  if (method >= 0 && method < 5)
    m_pitchMethod = method;
  else
    m_pitchMethod = 0;
}

void MyFrame::SetSpectrumFftSize(int size) {
  if (size >= 0 && size < 7)
    m_spectrumFftSize = size;
  else
    m_spectrumFftSize = 5;
}

void MyFrame::SetSpectrumWindow(int type) {
  if (type >= 0 && type < 10)
    m_spectrumWindow = type;
  else
    m_spectrumWindow = 9;
}

void MyFrame::SetSpectrumInterpolatePitch(bool interpolate) {
  m_spectrumInterpolatePitch = interpolate;
}


void MyFrame::UpdatePlayPosition(wxTimerEvent& WXUNUSED(event)) {
  if (m_waveform && m_audiofile->m_channels != 0) {
    if (m_sound->StreamNeedsResampling())
      m_waveform->SetPlayPosition((m_sound->pos[0] / m_audiofile->m_channels) / m_resampler->GetRatioUsed());
    else
      m_waveform->SetPlayPosition(m_sound->pos[0] / m_audiofile->m_channels);
    m_waveform->paintNow();
  }
}

void MyFrame::AddNewCue(unsigned offset) {
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

void MyFrame::ChangeCuePosition(unsigned offset, int index) {
  m_audiofile->m_cues->ChangePosition(offset, index);
  m_panel->ChangeCueData(offset, index);

  UpdateAllViews();

  toolBar->EnableTool(wxID_SAVE, true);
  fileMenu->Enable(wxID_SAVE, true);
  fileMenu->Enable(SAVE_AND_OPEN_NEXT, true);
}

void MyFrame::OnAddLoop(wxCommandEvent& WXUNUSED(event)) {
  // set default start and end as sustainsection
  std::pair<unsigned, unsigned> currentSustain = m_audiofile->GetSustainsection();
  unsigned start = currentSustain.first;
  unsigned end = currentSustain.second;
  // get audio data to analyze
  unsigned nbrOfSmpls = m_audiofile->ArrayLength / m_audiofile->m_channels;
  double *audioData = new double[nbrOfSmpls];
  m_audiofile->SeparateStrongestChannel(audioData);
  // adjust to reasonable loop points
  // search for closest (going towards positive) zero crossing
  unsigned startIdx = start;
  for (unsigned i = start; i < end; i++) {
    if (signbit(audioData[i])) {
      if (!signbit(audioData[i + 1]) != !signbit(audioData[i])) {
        // which is closer to zero
        if (fabs(audioData[i]) < fabs(audioData[i + 1])) {
          startIdx = i;
          break;
        } else {
          startIdx = i + 1;
          break;
        }
      }
    }
  }
  // find a suitable match from end
  unsigned endIdx = end;
  for (unsigned i = end; i > startIdx; i--) {
    if (!signbit(audioData[i])) {
      if (signbit(audioData[i - 1]) != signbit(audioData[i])) {
        endIdx = i - 1;
          break;
      }
    }
  }
  // clean up
  delete[] audioData;
  LoopParametersDialog loopDialog(startIdx, endIdx, m_audiofile->ArrayLength / m_audiofile->m_channels, this);

  if (loopDialog.ShowModal() == wxID_OK) {
    unsigned loopStartSample = loopDialog.GetLoopStart();
    unsigned loopEndSample = loopDialog.GetLoopEnd();

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
    m_panel->FillRowWithLoopData(newLoop.dwStart, newLoop.dwEnd, sRate, newLoop.shouldBeSaved, m_audiofile->m_loops->GetNumberOfLoops() - 1, m_audiofile->GetLoopQuality(m_audiofile->m_loops->GetNumberOfLoops() - 1));

    // Add the new loop to waveform drawer
    m_waveform->AddLoopPosition(newLoop.dwStart, newLoop.dwEnd);

    // Enable save icon and menu
    toolBar->EnableTool(wxID_SAVE, true);
    fileMenu->Enable(wxID_SAVE, true);
    fileMenu->Enable(SAVE_AND_OPEN_NEXT, true);

    // Make sure the new loop in the grid is selected
    if (m_panel->m_grid->GetNumberRows() > 0) {
      m_panel->m_grid->SelectRow((m_audiofile->m_loops->GetNumberOfLoops() - 1), false);
      toolBar->EnableTool(wxID_STOP, false);
      toolBar->EnableTool(START_PLAYBACK, true);
      transportMenu->Enable(START_PLAYBACK, true);
      transportMenu->Enable(wxID_STOP, false);
      toolBar->EnableTool(X_FADE, true);
      toolMenu->Enable(X_FADE, true);
      toolBar->EnableTool(VIEW_LOOPPOINTS, true);
      toolMenu->Enable(VIEW_LOOPPOINTS, true);
      m_panel->m_grid->SetGridCursor((m_audiofile->m_loops->GetNumberOfLoops() - 1), 4);
      SetLoopPlayback(true);
      // notify waveform of selected loop
      m_waveform->SetLoopSelection((m_audiofile->m_loops->GetNumberOfLoops() - 1));
    }
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
    toolBar->EnableTool(VIEW_LOOPPOINTS, true);
    toolMenu->Enable(VIEW_LOOPPOINTS, true);

    m_panel->m_grid->SetGridCursor(event.GetRow(), 4); // this is to fix scrolling issues
    SetLoopPlayback(true); // set the playback to be for loops
    if (toolBar->GetToolEnabled(wxID_STOP) == false) {
      toolBar->EnableTool(START_PLAYBACK, true);
      transportMenu->Enable(START_PLAYBACK, true);
    }

    // Now we should have a context menu pop up where user can do something. event.GetPosition not correct to window
    wxPoint pt = ScreenToClient(::wxGetMousePosition());
    PopupMenu(m_loopPopupMenu, pt);
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
  
  Refresh();
}

void MyFrame::OnBatchProcess(wxCommandEvent& WXUNUSED(event)) {
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

void MyFrame::OnAutoLoop(wxCommandEvent& WXUNUSED(event)) {
  // prepare a vector to receive the loops
  std::vector<std::pair<std::pair<unsigned, unsigned>, double> > loops;

  bool foundSomeLoops = false;

  if (!foundSomeLoops) {
    // stop other windows and set a busyinfo to calm user if it takes some time
    wxWindowDisabler disableAll;
    wxBusyInfo searchInfo(wxT("Searching for loops, please wait..."), this);
    wxSafeYield();

    // time to search for loops
    foundSomeLoops = m_autoloop->AutoFindLoops(m_audiofile, loops);
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
        m_audiofile->m_loops->GetNumberOfLoops() - 1,
        m_audiofile->GetLoopQuality(m_audiofile->m_loops->GetNumberOfLoops() - 1)
      );

      // Add the new loop to waveform drawer
      m_waveform->AddLoopPosition(newLoop.dwStart, newLoop.dwEnd);
    }

    // Enable save icon and menu
    toolBar->EnableTool(wxID_SAVE, true);
    fileMenu->Enable(wxID_SAVE, true);
    fileMenu->Enable(SAVE_AND_OPEN_NEXT, true);

    // Make sure a loop in the grid is selected if no selection exist
    if (!m_panel->m_grid->IsSelection() || !m_panel->m_cueGrid->IsSelection()) {
      if (m_panel->m_grid->GetNumberRows() > 0) {
        m_panel->m_grid->SelectRow(0, false);
        toolBar->EnableTool(wxID_STOP, false);
        toolBar->EnableTool(START_PLAYBACK, true);
        transportMenu->Enable(START_PLAYBACK, true);
        transportMenu->Enable(wxID_STOP, false);
        toolBar->EnableTool(X_FADE, true);
        toolMenu->Enable(X_FADE, true);
        toolBar->EnableTool(VIEW_LOOPPOINTS, true);
        toolMenu->Enable(VIEW_LOOPPOINTS, true);
        m_panel->m_grid->SetGridCursor(0, 4);
        SetLoopPlayback(true);
        // notify waveform drawer of selected loop
        m_waveform->SetLoopSelection(0);
      }
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
}

void MyFrame::OnAutoLoopSettings(wxCommandEvent& WXUNUSED(event)) {
  if (m_audiofile && (!m_audiofile->GetAutoSustainSearch())) {
    // update dialog with settings from file if they are changed there
    std::pair<unsigned, unsigned> currentSustain = m_audiofile->GetSustainsection();
    int start = ((double) currentSustain.first / (double) (m_audiofile->ArrayLength / m_audiofile->m_channels)) * 100 + 0.5;
    int end = ((double) currentSustain.second / (double) (m_audiofile->ArrayLength / m_audiofile->m_channels)) * 100 + 0.5;
    m_autoloopSettings->SetStart(start);
    m_autoloopSettings->SetEnd(end);
  }
  int oldStart = m_autoloopSettings->GetStart();
  int oldEnd = m_autoloopSettings->GetEnd();
  bool oldAutoSustain = m_autoloopSettings->GetAutosearch();
  m_autoloopSettings->TransferDataToWindow();
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
    
    // Only update audiofile if it exist! It should be updated when loaded anyway!
    if (m_audiofile) {
      // update audiofile slider sustainsection if changed
      m_audiofile->SetSliderSustainsection(m_autoloopSettings->GetStart(), m_autoloopSettings->GetEnd());
      // update chosen sustainsection selection
      m_audiofile->SetAutoSustainSearch(m_autoloopSettings->GetAutosearch());
    }
    
    // force a redraw as sustainsection might have changed
    UpdateAllViews();
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

void MyFrame::OnPitchSettings(wxCommandEvent& WXUNUSED(event)) {
  if (!m_audiofile)
    return;

  PitchDialog dialog(m_audiofile, this);
  dialog.SetPreferredPitchMethod(m_pitchMethod);
  dialog.SetPreferredFftSize(m_spectrumFftSize);
  dialog.SetPreferredWindow(m_spectrumWindow);
  dialog.SetPreferredInterpolatePitch(m_spectrumInterpolatePitch);

  if (dialog.ShowModal() == wxID_OK) {
    dialog.TransferSelectedPitchToFile();
    // enable save icon and menu
    toolBar->EnableTool(wxID_SAVE, true);
    fileMenu->Enable(wxID_SAVE, true);
    fileMenu->Enable(SAVE_AND_OPEN_NEXT, true);
  }
  m_pitchMethod = dialog.GetMethodUsed();
  m_spectrumFftSize = dialog.GetFftSize();
  m_spectrumWindow = dialog.GetWindowType();
  m_spectrumInterpolatePitch = dialog.GetInterpolatePitch();
}

void MyFrame::OnZoomInAmplitude(wxCommandEvent& WXUNUSED(event)) {
  m_waveform->ZoomInAmplitude();
  SetStatusText(wxString::Format(wxT("Zoom level: x %i"), m_waveform->GetAmplitudeZoomLevel()), 1);
  UpdateAllViews();
}

void MyFrame::OnZoomOutAmplitude(wxCommandEvent& WXUNUSED(event)) {
  m_waveform->ZoomOutAmplitude();
  SetStatusText(wxString::Format(wxT("Zoom level: x %i"), m_waveform->GetAmplitudeZoomLevel()), 1);
  UpdateAllViews();
}

void MyFrame::OnVolumeSlider(wxCommandEvent& WXUNUSED(event)) {
  wxSlider *volumeSl = (wxSlider*) FindWindow(ID_VOLUME_SLIDER);
  int value = volumeSl->GetValue();

  volumeMultiplier = (int) (pow(2, (double) value));
  m_panel->SetFocus();
}

void MyFrame::OnCrossfade(wxCommandEvent& WXUNUSED(event)) {
  wxArrayInt selectedRows = m_panel->m_grid->GetSelectedRows();
  int firstSelected;
  if (!selectedRows.IsEmpty())
    firstSelected = selectedRows[0];
  else {
    // there's no selected loop to crossfade
    return;
  }

  wxString dialogTitle = wxString::Format(wxT("Crossfading parameters for Loop %i"), firstSelected + 1);
  m_crossfades->SetCaption(dialogTitle);

  // show the crossfade dialog to get parameters (method and time)
  if (m_crossfades->ShowModal() == wxID_OK) {
    double crossfadeTime = m_crossfades->GetFadeduration();
    int crossfadetype = m_crossfades->GetFadetype();

    // perform crossfading on the first selected loop with selected method
    m_audiofile->PerformCrossfade(firstSelected, crossfadeTime, crossfadetype);
    
    // if resampled audio is used it must be updated too!
    if (m_sound->StreamNeedsResampling()) {
      m_resampler->ResetState();
      // fill resample data struct with values
      double src_ratio = (1.0 * m_sound->GetSampleRateToUse()) / (1.0 * m_audiofile->GetSampleRate());
      m_resampler->SetDataEndOfInput(0); // Set this later
      m_resampler->SetDataInputFrames(m_audiofile->ArrayLength / m_audiofile->m_channels);
      m_resampler->SetDataIn(m_audiofile->floatAudioData);
      m_resampler->SetDataSrcRatio(src_ratio);
      m_resampler->SimpleResample(m_audiofile->m_channels);
    }

    // Enable save icon and menu
    toolBar->EnableTool(wxID_SAVE, true);
    fileMenu->Enable(wxID_SAVE, true);
    fileMenu->Enable(SAVE_AND_OPEN_NEXT, true);

    // Update loop quality
    m_panel->UpdateLoopQuality(firstSelected, m_audiofile->GetLoopQuality(firstSelected));

    // then we should make sure to update the views
    UpdateAllViews();
  } else {
    // user clicked cancel...
  }
}

void MyFrame::OnEditLoop(wxCommandEvent& WXUNUSED(event)) {
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
    unsigned loopStartSample = loopDialog.GetLoopStart();
    unsigned loopEndSample = loopDialog.GetLoopEnd();

    // Store the changes to the loop in the loop vector
    m_audiofile->m_loops->SetLoopPositions(loopStartSample, loopEndSample, firstSelected);
    m_audiofile->m_loops->GetLoopData(firstSelected, currentLoop);

    // Change loop to the grid
    int sRate = m_audiofile->GetSampleRate();
    m_panel->ChangeLoopData(currentLoop.dwStart, currentLoop.dwEnd, sRate, firstSelected, m_audiofile->GetLoopQuality(firstSelected));

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

void MyFrame::OnViewLoop(wxCommandEvent& WXUNUSED(event)) {
  wxArrayInt selectedRows = m_panel->m_grid->GetSelectedRows();
  int firstSelected;
  if (!selectedRows.IsEmpty())
    firstSelected = selectedRows[0];
  else {
    return;
  }

  wxString dialogTitle = wxT("Waveform overlay of loops");

  LoopOverlay lpo(
    m_audiofile,
    firstSelected,
    this,
    wxID_ANY,
    dialogTitle
  );

  lpo.ShowModal();

  // Update menus and views if loops are changed
  if (lpo.GetHasChanged()) {
    // Enable save icon and menu
    toolBar->EnableTool(wxID_SAVE, true);
    fileMenu->Enable(wxID_SAVE, true);
    fileMenu->Enable(SAVE_AND_OPEN_NEXT, true);

    UpdateLoopsAndCuesDisplay();
    UpdateAllViews();
    m_panel->m_grid->SelectRow(firstSelected);
    if (!m_panel->m_grid->IsVisible(firstSelected, 0))
      m_panel->m_grid->MakeCellVisible(firstSelected, 0);
  }
}

void MyFrame::OnCutFade(wxCommandEvent& WXUNUSED(event)) {
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

    // perform fade(s) as needed
    if (m_cutNFade->GetFadeStart() > 0)
      m_audiofile->PerformFade(m_cutNFade->GetFadeStart(), 0);

    if (m_cutNFade->GetFadeEnd() > 0)
      m_audiofile->PerformFade(m_cutNFade->GetFadeEnd(), 1);

    // Enable save icon and menu
    toolBar->EnableTool(wxID_SAVE, true);
    fileMenu->Enable(wxID_SAVE, true);
    fileMenu->Enable(SAVE_AND_OPEN_NEXT, true);

    UpdateLoopsAndCuesDisplay();

    // if resampled audio is used it must be updated too!
    if (m_sound->StreamNeedsResampling()) {
      m_resampler->ResetState();
      // fill resample data struct with values
      double src_ratio = (1.0 * m_sound->GetSampleRateToUse()) / (1.0 * m_audiofile->GetSampleRate());
      m_resampler->SetDataEndOfInput(0); // Set this later
      m_resampler->SetDataInputFrames(m_audiofile->ArrayLength / m_audiofile->m_channels);
      m_resampler->SetDataIn(m_audiofile->floatAudioData);
      m_resampler->SetDataSrcRatio(src_ratio);
      m_resampler->SimpleResample(m_audiofile->m_channels);
    }

    // then we should make sure to update the views
    UpdateAllViews();
  } else {
    // user clicked cancel...
  }
}

void MyFrame::OnListInfo(wxCommandEvent& WXUNUSED(event)) {
  ListInfoDialog infoDlg(m_audiofile, this);
  // force update of values
  infoDlg.Init(m_audiofile);
  infoDlg.TransferDataToWindow();
  
  if (infoDlg.ShowModal() == wxID_OK) {
    infoDlg.TransferDataFromWindow();
    m_audiofile->m_info.artist = infoDlg.getArtist();
    m_audiofile->m_info.copyright = infoDlg.getCopyright();
    m_audiofile->m_info.software = infoDlg.getSoftware();
    m_audiofile->m_info.comment = infoDlg.getComment();
    m_audiofile->m_info.creation_date = infoDlg.getCreationDate();
    
    // Enable save icon and menu
    toolBar->EnableTool(wxID_SAVE, true);
    fileMenu->Enable(wxID_SAVE, true);
    fileMenu->Enable(SAVE_AND_OPEN_NEXT, true);
  } else {
    // user clicked cancel...
  }
}

void MyFrame::OnAudioSettings(wxCommandEvent& WXUNUSED(event)) {
  if (m_sound->IsStreamActive()) {
    // if audio is playing we must reset things
    DoStopPlay();
  }
  AudioSettingsDialog audioDlg(m_sound, this);

  if (audioDlg.ShowModal() == wxID_OK) {
    audioDlg.TransferDataFromWindow();
    m_sound->SetApiToUse(RtAudio::getCompiledApiByName(std::string(audioDlg.GetSoundApi().mb_str())));
    m_sound->SetAudioDevice(audioDlg.GetSoundDeviceId());
    if (m_audiofile) {
      // if a file already is open we must adjust device parameters to it
      m_sound->SetSampleRate(m_audiofile->GetSampleRate());
      m_sound->SetChannels(m_audiofile->m_channels);
      m_sound->OpenAudioStream();
    }
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
      wxCommandEvent evt(wxEVT_TOOL, START_PLAYBACK);
      ::wxGetApp().frame->AddPendingEvent(evt);
      return;
    } else if (toolBar->GetToolEnabled(wxID_STOP)) {
      wxCommandEvent evt(wxEVT_TOOL, wxID_STOP);
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
    if (currentSelectedIdx != wxNOT_FOUND && (unsigned) currentSelectedIdx < (fileNames.GetCount() - 1)) {
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
        // notify waveform drawer of selected loop
        m_waveform->SetLoopSelection(firstSelected - 1);
        UpdateAllViews();
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
        toolBar->EnableTool(VIEW_LOOPPOINTS, true);
        toolMenu->Enable(VIEW_LOOPPOINTS, true);
        // notify waveform drawer of selected loop
        m_waveform->SetLoopSelection(0);
        UpdateAllViews();
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
        // notify waveform drawer of selected loop
        m_waveform->SetLoopSelection(firstSelected + 1);
        UpdateAllViews();
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
        toolBar->EnableTool(VIEW_LOOPPOINTS, true);
        toolMenu->Enable(VIEW_LOOPPOINTS, true);
        // notify waveform drawer of selected loop
        m_waveform->SetLoopSelection(0);
        UpdateAllViews();
      }
    }
    return;
  } else if (event.GetKeyCode() == 67 && event.GetModifiers() == wxMOD_CONTROL) {
    // workaround for wxGrid catching
    wxCommandEvent evt(wxEVT_MENU, CUT_N_FADE);
    wxPostEvent(this, evt);
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

          // notify waveform drawer of selected cue
          m_waveform->SetCueSelection(firstSelected - 1);
          UpdateAllViews();
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
        toolBar->EnableTool(VIEW_LOOPPOINTS, false);
        toolMenu->Enable(VIEW_LOOPPOINTS, false);

        // notify waveform drawer of selected cue
        m_waveform->SetCueSelection(0);
        UpdateAllViews();
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

          // notify waveform drawer of selected cue
          m_waveform->SetCueSelection(firstSelected + 1);
          UpdateAllViews();
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
        toolBar->EnableTool(VIEW_LOOPPOINTS, false);
        toolMenu->Enable(VIEW_LOOPPOINTS, false);

        // notify waveform drawer of selected cue
        m_waveform->SetCueSelection(0);
        UpdateAllViews();
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
        if (m_panel->m_grid->GetCellValue(firstSelected, 5) == wxT("1")) {
          // Equals to a true value to begin with
          m_panel->m_grid->SetCellValue(firstSelected, 5, wxT("0"));
          m_audiofile->m_loops->SetSaveOption(false, firstSelected);
          toolBar->EnableTool(wxID_SAVE, true);
          fileMenu->Enable(wxID_SAVE, true);
          fileMenu->Enable(SAVE_AND_OPEN_NEXT, true);
        } else {
          // The value was false
          m_panel->m_grid->SetCellValue(firstSelected, 5, wxT("1"));
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
        if (m_panel->m_grid->GetCellValue(i, 5) == wxT("1")) {
          // Equals to a true value to begin with
          m_panel->m_grid->SetCellValue(i, 5, wxT("0"));
          m_audiofile->m_loops->SetSaveOption(false, i);
        } else {
          // The value was false
          m_panel->m_grid->SetCellValue(i, 5, wxT("1"));
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
  m_fileListCtrl->DeleteAllItems();

  m_fileListCtrl->Hide();

  if (!fileNames.IsEmpty()) {
    for (unsigned i = 0; i < fileNames.GetCount(); i++) {
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

  int availableWidth;

  if (m_fileListCtrl->GetSize() != m_fileListCtrl->GetVirtualSize())
    availableWidth = m_fileListCtrl->GetClientSize().GetWidth() - wxSystemSettings::GetMetric(wxSYS_VSCROLL_X);
  else
    availableWidth = m_fileListCtrl->GetClientSize().GetWidth();

  // adjust width of the list control columns
  if (m_fileListCtrl->GetColumnWidth(0) +
    m_fileListCtrl->GetColumnWidth(1) +
    m_fileListCtrl->GetColumnWidth(2) +
    m_fileListCtrl->GetColumnWidth(3) +
    m_fileListCtrl->GetColumnWidth(4) < availableWidth
  ) {
    int sizeLeft = availableWidth - (
      m_fileListCtrl->GetColumnWidth(1) +
      m_fileListCtrl->GetColumnWidth(2) +
      m_fileListCtrl->GetColumnWidth(3) +
      m_fileListCtrl->GetColumnWidth(4)
    );
    if (sizeLeft > m_fileListCtrl->GetColumnWidth(0))
      m_fileListCtrl->SetColumnWidth(0, sizeLeft);
  }

  m_fileListCtrl->Show();
}

void MyFrame::OnHelp(wxCommandEvent& WXUNUSED(event)) {
  ::wxGetApp().m_helpController->DisplayContents();
}

void MyFrame::GetCurrentFrameSizes() {
  if (IsMaximized())
    m_frameMaximized = true;
  else {
    GetPosition(&m_xPosition, &m_yPosition);
    GetSize(&m_frameWidth, &m_frameHeight);
    m_frameMaximized = false;
  }
}

void MyFrame::OnSize(wxSizeEvent& event) {
  GetCurrentFrameSizes();
  event.Skip();
}

void MyFrame::UpdateLoopsAndCuesDisplay() {
  // populate the wxGrid in m_panel with the loop data and add it to the waveform drawer
  m_panel->EmptyTable();
  m_waveform->ClearMetadata();
  int sRate = m_audiofile->GetSampleRate();
  for (int i = 0; i < m_audiofile->m_loops->GetNumberOfLoops(); i++) {
    LOOPDATA tempData;
    m_audiofile->m_loops->GetLoopData(i, tempData);
    m_panel->FillRowWithLoopData(tempData.dwStart, tempData.dwEnd, sRate, tempData.shouldBeSaved, i, m_audiofile->GetLoopQuality(i));
    m_waveform->AddLoopPosition(tempData.dwStart, tempData.dwEnd);
  }

  // populate the wxGrid m_cueGrid in m_panel with the cue data and add it to the waveform drawer
  for (unsigned i = 0; i < m_audiofile->m_cues->GetNumberOfCues(); i++) {
    CUEPOINT tempCue;
    m_audiofile->m_cues->GetCuePoint(i, tempCue);
    m_panel->FillRowWithCueData(tempCue.dwName, tempCue.dwSampleOffset, tempCue.keepThisCue, i);
    m_waveform->AddCuePosition(tempCue.dwSampleOffset);
  }
}

void MyFrame::UpdateAutoloopSliderSustainsection(int start, int end) {
  if (m_audiofile && (!m_audiofile->GetAutoSustainSearch())) {
    m_autoloopSettings->SetStart(start);
    m_autoloopSettings->SetEnd(end);
  }
}
