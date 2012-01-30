/*
 * BatchProcessDialog.cpp is a part of LoopAuditioneer software
 * Copyright (C) 2011-2012 Lars Palo
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

#include "BatchProcessDialog.h"
#include "AutoLoopDialog.h"
#include "AutoLooping.h"
#include "FileHandling.h"
#include "sndfile.hh"
#include <wx/statline.h>
#include <wx/listctrl.h>
#include <wx/filename.h>
#include <wx/dir.h>
#include <vector>
#include <climits>
#include <cmath>

IMPLEMENT_CLASS(BatchProcessDialog, wxDialog )

BEGIN_EVENT_TABLE(BatchProcessDialog, wxDialog)
  EVT_BUTTON(ID_ADD_SOURCE, BatchProcessDialog::OnAddSource)
  EVT_BUTTON(ID_ADD_TARGET, BatchProcessDialog::OnAddTarget)
  EVT_BUTTON(ID_RUN_BATCH, BatchProcessDialog::OnRunBatch)
  EVT_CHOICE(ID_PROCESSBOX, BatchProcessDialog::OnChoiceSelected)
END_EVENT_TABLE()

BatchProcessDialog::BatchProcessDialog() {
  Init();
}

BatchProcessDialog::BatchProcessDialog(
    wxWindow* parent,
    wxWindowID id,
    const wxString& caption,
    const wxPoint& pos,
    const wxSize& size,
    long style
  ) {
  Init();
  Create(parent, id, caption, pos, size, style);
}

void BatchProcessDialog::Init() {
  m_batchProcessesAvailable.Add(wxEmptyString);
  m_batchProcessesAvailable.Add(wxT("Kill all loops"));
  m_batchProcessesAvailable.Add(wxT("Kill all cues"));
  m_batchProcessesAvailable.Add(wxT("Kill loops & cues"));
  m_batchProcessesAvailable.Add(wxT("Search for loops"));
  m_batchProcessesAvailable.Add(wxT("Store FFT detected pitch info"));
  m_batchProcessesAvailable.Add(wxT("List FFT pitch deviations"));
  m_batchProcessesAvailable.Add(wxT("Store time domain detected pitch info"));
  m_batchProcessesAvailable.Add(wxT("List time domain pitch deviations"));
}

bool BatchProcessDialog::Create(
  wxWindow* parent,
  wxWindowID id,
  const wxString& caption,
  const wxPoint& pos,
  const wxSize& size,
  long style
) {
  if (!wxDialog::Create(parent, id, caption, pos, size, style))
    return false;

  CreateControls();

  GetSizer()->Fit(this);
  GetSizer()->SetSizeHints(this);
  Centre();

  return true;
}

void BatchProcessDialog::CreateControls() {
  // Create a top level sizer
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  this->SetSizer(topSizer);

  // Second box sizer to get nice margins
  wxBoxSizer *boxSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(boxSizer, 0, wxEXPAND|wxALL, 5);

  // Horizontal sizer for sources
  wxBoxSizer *m_sourceRow = new wxBoxSizer(wxHORIZONTAL);
  boxSizer->Add(m_sourceRow, 0, wxEXPAND|wxALL, 5);

  // Label for source
  wxStaticText *m_sourceLabel = new wxStaticText(
    this,
    wxID_STATIC,
    wxT("Source: "),
    wxDefaultPosition,
    wxSize(100,-1),
    0
  );
  m_sourceRow->Add(m_sourceLabel, 0, wxALL, 2);

  // The source textctrl
  m_sourceField = new wxTextCtrl(
    this,
    ID_SOURCE_TEXT,
    wxEmptyString,
    wxDefaultPosition,
    wxDefaultSize,
    wxTE_READONLY
  );
  m_sourceRow->Add(m_sourceField, 1, wxEXPAND|wxALL, 2);

  // The select source button
  m_selectSource = new wxButton(
    this,
    ID_ADD_SOURCE,
    wxT("Select folder"),
    wxDefaultPosition,
    wxDefaultSize,
    0
  );
  m_sourceRow->Add(m_selectSource, 0, wxALL, 2);

  // Horizontal sizer for target
  wxBoxSizer *m_targetRow = new wxBoxSizer(wxHORIZONTAL);
  boxSizer->Add(m_targetRow, 0, wxEXPAND|wxALL, 5);

  // Label for target
  wxStaticText *m_targetLabel = new wxStaticText(
    this,
    wxID_STATIC,
    wxT("Target: "),
    wxDefaultPosition,
    wxSize(100,-1),
    0
  );
  m_targetRow->Add(m_targetLabel, 0, wxALL, 2);

  // The target textctrl
  m_targetField = new wxTextCtrl(
    this,
    ID_TARGET_TEXT,
    wxEmptyString,
    wxDefaultPosition,
    wxDefaultSize,
    wxTE_READONLY
  );
  m_targetRow->Add(m_targetField, 1, wxEXPAND|wxALL, 2);

  // The select target button
  m_selectTarget = new wxButton(
    this,
    ID_ADD_TARGET,
    wxT("Select folder"),
    wxDefaultPosition,
    wxDefaultSize,
    0
  );
  m_targetRow->Add(m_selectTarget, 0, wxALL, 2);

  // Horizontal sizer for processes
  wxBoxSizer *m_processRow = new wxBoxSizer(wxHORIZONTAL);
  boxSizer->Add(m_processRow, 0, wxEXPAND|wxALL, 5);

  // Label for dropdown
  wxStaticText *m_processLabel = new wxStaticText(
    this,
    wxID_STATIC,
    wxT("Process: "),
    wxDefaultPosition,
    wxSize(100,-1),
    0
  );
  m_processRow->Add(m_processLabel, 0, wxALL, 2);

  // Dropdown for selecting process
  m_processChoiceBox = new wxChoice(
    this,
    ID_PROCESSBOX,
    wxDefaultPosition,
    wxDefaultSize,
    m_batchProcessesAvailable
  );
  m_processRow->Add(m_processChoiceBox, 0, wxGROW|wxALL, 2);

  // Horizontal sizer for status text
  wxBoxSizer *m_statusRow = new wxBoxSizer(wxHORIZONTAL);
  boxSizer->Add(m_statusRow, 0, wxEXPAND|wxALL, 5);

  // Grouping of sustainsection options
  wxStaticBox *statusBox = new wxStaticBox(
    this,
    wxID_STATIC,
    wxT("Progress status"),
    wxDefaultPosition,
    wxDefaultSize
  );

  // Vertical sizer for static box
  wxStaticBoxSizer *staticStatusBox = new wxStaticBoxSizer(statusBox, wxVERTICAL);
  m_statusRow->Add(staticStatusBox, 1, wxEXPAND|wxALL, 0);

  // The status textctrl
  m_statusProgress = new wxTextCtrl(
    this,
    ID_STATUS_TEXT,
    wxEmptyString,
    wxDefaultPosition,
    wxSize(-1,150),
    wxTE_READONLY|wxTE_MULTILINE
  );
  staticStatusBox->Add(m_statusProgress, 1, wxEXPAND|wxALIGN_CENTER|wxALL, 5);

  // Horizontal sizer for buttons
  wxBoxSizer* m_buttonRow = new wxBoxSizer(wxHORIZONTAL);
  boxSizer->Add(m_buttonRow, 0, wxEXPAND|wxALL, 5);

  // The OK button
  m_runButton = new wxButton(
    this,
    ID_RUN_BATCH,
    wxT("&Run batch"),
    wxDefaultPosition,
    wxDefaultSize,
    0
  );
  m_runButton->Enable(false);
  m_buttonRow->Add(m_runButton, 0, wxALL, 5);

  // The Cancel button
  wxButton *m_cancelButton = new wxButton(
    this,
    wxID_CANCEL,
    wxT("&Cancel"),
    wxDefaultPosition,
    wxDefaultSize,
    0
  );
  m_buttonRow->Add(m_cancelButton, 0, wxALL, 5);

  // A horizontal line after the buttons
  wxStaticLine *m_line = new wxStaticLine(
    this,
    wxID_STATIC,
    wxDefaultPosition,
    wxDefaultSize,
    wxLI_HORIZONTAL
  );
  boxSizer->Add(m_line, 0, wxGROW|wxALL, 5);

  // Information message at bottom
  wxStaticText *m_info = new wxStaticText(
    this,
    wxID_STATIC,
    wxT("Choose source and target folders to process and click OK to run batch on all wav files."),
    wxDefaultPosition,
    wxDefaultSize,
    0
  );
  boxSizer->Add(m_info, 0, wxALIGN_LEFT|wxALL, 5);

}

void BatchProcessDialog::OnAddSource(wxCommandEvent& event) {
  wxDirDialog *selectDirDialog = new wxDirDialog(
    this,
    wxT("Choose a source folder to batch process"),
    wxEmptyString,
    wxDD_DIR_MUST_EXIST | wxDD_CHANGE_DIR
  );

  if (selectDirDialog->ShowModal() == wxID_OK) {
    wxString pathToAdd = selectDirDialog->GetPath();
    if (pathToAdd != wxEmptyString) {
      m_sourceField->ChangeValue(pathToAdd);
    }
  }

  ReadyToRockAndRoll();
}

void BatchProcessDialog::OnAddTarget(wxCommandEvent& event) {
  wxDirDialog *selectDirDialog = new wxDirDialog(
    this,
    wxT("Choose target folder for batch process"),
    wxEmptyString,
    wxDD_CHANGE_DIR
  );

  if (selectDirDialog->ShowModal() == wxID_OK) {
    wxString pathToAdd = selectDirDialog->GetPath();
    if (pathToAdd != wxEmptyString) {
      m_targetField->ChangeValue(pathToAdd);
    }
  }

  ReadyToRockAndRoll();
}

void BatchProcessDialog::OnChoiceSelected(wxCommandEvent& event) {
  ReadyToRockAndRoll();
}

void BatchProcessDialog::ReadyToRockAndRoll() {
  // Check if OK button should be enabled
  if (m_sourceField->GetValue() != wxEmptyString) {
    if (m_targetField->GetValue() != wxEmptyString) {
      if (m_processChoiceBox->GetSelection() > 0) {
        m_runButton->Enable(true);
      } else {
          m_runButton->Enable(false);
      }
    } else {
        m_runButton->Enable(false);
    }
  } else {
    m_runButton->Enable(false);
  }
}

void BatchProcessDialog::OnRunBatch(wxCommandEvent& event) {
  AutoLoopDialog *autoloopSettings = new AutoLoopDialog(this);
  AutoLooping *autoloop = new AutoLooping();
  // Get all wav files in the source directory
  wxArrayString filesToProcess;

  wxDir dir(m_sourceField->GetValue());

  if (!dir.IsOpened()) {
    wxMessageDialog *dial = new wxMessageDialog(NULL, wxT("Couldn't open folder"), wxT("Error"), wxOK | wxICON_ERROR);
    dial->ShowModal();
    return;
  }

  wxString fileName;

  bool search = dir.GetFirst(&fileName, wxT("*.wav"), wxDIR_FILES);
  while (search) {
    filesToProcess.Add(fileName);
    search = dir.GetNext(&fileName);
  }

  search = dir.GetFirst(&fileName, wxT("*.WAV"), wxDIR_FILES);
  while (search) {
    filesToProcess.Add(fileName);
    search = dir.GetNext(&fileName);
  }

  // sort the files and remove doubles for windows...
  filesToProcess.Sort();
  size_t lineCounter = 0;
  while (lineCounter < filesToProcess.GetCount() - 1) {
    if (filesToProcess[lineCounter] == filesToProcess[lineCounter + 1])
      filesToProcess.RemoveAt(lineCounter + 1);
    else
      lineCounter++;
  }

  // Depending on selected process do
  switch(m_processChoiceBox->GetSelection()) {
    case 0:
      // This should be impossible as Run button should be disabled!
      m_statusProgress->AppendText(wxT("No process selected!\n"));
    break;

    case 1:
      // This removes all loops from the wav files!
      if (filesToProcess.IsEmpty() == false) {
        for (unsigned i = 0; i < filesToProcess.GetCount(); i++) {
          m_statusProgress->AppendText(filesToProcess.Item(i));
          m_statusProgress->AppendText(wxT("\n"));
          FileHandling fh(filesToProcess.Item(i), m_sourceField->GetValue());
          if (fh.FileCouldBeOpened()) {
            m_statusProgress->AppendText(wxT("\tFile opened.\n"));
            if (fh.m_loops->GetNumberOfLoops() > 0) {
              for (int j = 0; j < fh.m_loops->GetNumberOfLoops(); j++)
                fh.m_loops->SetSaveOption(false, j);
            }
            fh.SaveAudioFile(filesToProcess.Item(i), m_targetField->GetValue());
            m_statusProgress->AppendText(wxT("\tDone!\n"));
          } else {
            m_statusProgress->AppendText(wxT("\tCouldn't open file!\n"));
          }
        }
        m_statusProgress->AppendText(wxT("Batch process complete!\n\n"));
      } else {
        m_statusProgress->AppendText(wxT("No wav files to process!\n"));
      }
    break;

    case 2:
      // This removes all cues from the wav files!
      if (filesToProcess.IsEmpty() == false) {
        for (unsigned i = 0; i < filesToProcess.GetCount(); i++) {
          m_statusProgress->AppendText(filesToProcess.Item(i));
          m_statusProgress->AppendText(wxT("\n"));
          FileHandling fh(filesToProcess.Item(i), m_sourceField->GetValue());
          if (fh.FileCouldBeOpened()) {
            m_statusProgress->AppendText(wxT("\tFile opened.\n"));
            if (fh.m_cues->GetNumberOfCues() > 0) {
              for (unsigned j = 0; j < fh.m_cues->GetNumberOfCues(); j++)
                fh.m_cues->SetSaveOption(false, j);
            }
            fh.SaveAudioFile(filesToProcess.Item(i), m_targetField->GetValue());
            m_statusProgress->AppendText(wxT("\tDone!\n"));
          } else {
            m_statusProgress->AppendText(wxT("\tCouldn't open file!\n"));
          }
        }
        m_statusProgress->AppendText(wxT("Batch process complete!\n\n"));
      } else {
        m_statusProgress->AppendText(wxT("No wav files to process!\n"));
      }
    break;

    case 3:
      // This removes both loops and cues from the wav files!
      if (filesToProcess.IsEmpty() == false) {
        for (unsigned i = 0; i < filesToProcess.GetCount(); i++) {
          m_statusProgress->AppendText(filesToProcess.Item(i));
          m_statusProgress->AppendText(wxT("\n"));
          FileHandling fh(filesToProcess.Item(i), m_sourceField->GetValue());
          if (fh.FileCouldBeOpened()) {
            m_statusProgress->AppendText(wxT("\tFile opened.\n"));
            if (fh.m_loops->GetNumberOfLoops() > 0) {
              for (int j = 0; j < fh.m_loops->GetNumberOfLoops(); j++)
                fh.m_loops->SetSaveOption(false, j);
            }
            if (fh.m_cues->GetNumberOfCues() > 0) {
              for (unsigned j = 0; j < fh.m_cues->GetNumberOfCues(); j++)
                fh.m_cues->SetSaveOption(false, j);
            }
            fh.SaveAudioFile(filesToProcess.Item(i), m_targetField->GetValue());
            m_statusProgress->AppendText(wxT("\tDone!\n"));
          } else {
            m_statusProgress->AppendText(wxT("\tCouldn't open file!\n"));
          }
        }
        m_statusProgress->AppendText(wxT("Batch process complete!\n\n"));
      } else {
        m_statusProgress->AppendText(wxT("No wav files to process!\n"));
      }
    break;

    case 4:
      // This is for autosearching for loops, first show dialog
      if (autoloopSettings->ShowModal() == wxID_OK) {
        // Update AutoLooping object
        autoloop->SetThreshold(autoloopSettings->GetThreshold());
        autoloop->SetDuration(autoloopSettings->GetDuration());
        autoloop->SetBetween(autoloopSettings->GetBetween());
        autoloop->SetQuality(autoloopSettings->GetQuality());
        autoloop->SetCandidates(autoloopSettings->GetCandidates());
        autoloop->SetLoops(autoloopSettings->GetNrLoops());
        autoloop->SetMultiple(autoloopSettings->GetMultiple());
      }
      if (filesToProcess.IsEmpty() == false) {
        for (unsigned i = 0; i < filesToProcess.GetCount(); i++) {
          m_statusProgress->AppendText(filesToProcess.Item(i));
          m_statusProgress->AppendText(wxT("\n"));
          FileHandling fh(filesToProcess.Item(i), m_sourceField->GetValue());
          if (fh.FileCouldBeOpened()) {
            m_statusProgress->AppendText(wxT("\tFile opened.\n"));

            // now we need to search for loops and for that we need data as doubles
            wxString fullFilePath = m_sourceField->GetValue() + wxT("/") + filesToProcess.Item(i);
            m_statusProgress->AppendText(wxT("\tRead double data from ") + fullFilePath + wxT("\n"));
            SndfileHandle sfh;
            sfh = SndfileHandle(((const char*)fullFilePath.mb_str()));
            double *audioData = new double[sfh.frames() * sfh.channels()];
            sfh.read(audioData, sfh.frames() * sfh.channels());
            // vector to receive found loops
            std::vector<std::pair<std::pair<unsigned, unsigned>, double> > addLoops;
            // call to search for loops
            bool foundLoops = autoloop->AutoFindLoops(
              audioData,
              fh.ArrayLength,
              fh.m_channels,
              fh.GetSampleRate(),
              addLoops,
              autoloopSettings->GetAutosearch(),
              autoloopSettings->GetStart(),
              autoloopSettings->GetEnd()
            );
            // delete the now unneccessary array of double audio data
            delete[] audioData;

            if (foundLoops) {
              for (unsigned i = 0; i < addLoops.size(); i++) {
                // Add the new loop to the loop vector
                LOOPDATA newLoop;
                newLoop.dwType = SF_LOOP_FORWARD;
                newLoop.dwStart = addLoops[i].first.first;
                newLoop.dwEnd = addLoops[i].first.second;
                newLoop.dwPlayCount = 0;
                newLoop.shouldBeSaved = true;
                fh.m_loops->AddLoop(newLoop);
              }
              m_statusProgress->AppendText(wxString::Format(wxT("\t%i loops found.\n"), addLoops.size()));
            } else {
              // no loops found!
              m_statusProgress->AppendText(wxT("\tNo loops found!\n"));
            }

            fh.SaveAudioFile(filesToProcess.Item(i), m_targetField->GetValue());
            m_statusProgress->AppendText(wxT("\tDone!\n"));
          } else {
            m_statusProgress->AppendText(wxT("\tCouldn't open file!\n"));
          }
        }
        m_statusProgress->AppendText(wxT("Batch process complete!\n\n"));
      } else {
        m_statusProgress->AppendText(wxT("No wav files to process!\n"));
      }

    break;

    case 5:
      // This is for autosearching pitch information with FFT and store it in smpl chunk
      if (filesToProcess.IsEmpty() == false) {
        for (unsigned i = 0; i < filesToProcess.GetCount(); i++) {
          m_statusProgress->AppendText(filesToProcess.Item(i));
          m_statusProgress->AppendText(wxT("\n"));
          FileHandling fh(filesToProcess.Item(i), m_sourceField->GetValue());
          if (fh.FileCouldBeOpened()) {
            m_statusProgress->AppendText(wxT("\tFile opened.\n"));

            // now we need data as doubles
            wxString fullFilePath = m_sourceField->GetValue() + wxT("/") + filesToProcess.Item(i);
            SndfileHandle sfh;
            sfh = SndfileHandle(((const char*)fullFilePath.mb_str()));
            double *audioData = new double[sfh.frames() * sfh.channels()];
            sfh.read(audioData, sfh.frames() * sfh.channels());

            // autosearch pitch and calculate midi note and pitch fraction
            double pitch = fh.GetFFTPitch(audioData);
            int midi_note = (69 + 12 * (log10(pitch / 440.0) / log10(2)));
            double midi_note_pitch = 440.0 * pow(2, ((double)(midi_note - 69) / 12.0));
            double cent_deviation = 1200 * (log10(pitch / midi_note_pitch) / log10(2));
            unsigned int midi_pitch_fraction = ((double)UINT_MAX * (cent_deviation / 100.0));

            // set midi note and pitch fraction to loopmarkers
            fh.m_loops->SetMIDIUnityNote((char) midi_note);
            fh.m_loops->SetMIDIPitchFraction(midi_pitch_fraction);
            
            fh.SaveAudioFile(filesToProcess.Item(i), m_targetField->GetValue());
            m_statusProgress->AppendText(wxString::Format(wxT("\tDetected pitch = %.2f Hz\n"), pitch));
            m_statusProgress->AppendText(wxT("\tDone!\n"));

            // delete the now unneccessary array of double audio data
            delete[] audioData;
          } else {
            m_statusProgress->AppendText(wxT("\tCouldn't open file!\n"));
          }
        }
        m_statusProgress->AppendText(wxT("Batch process complete!\n\n"));
      } else {
        m_statusProgress->AppendText(wxT("No wav files to process!\n"));
      }

    break;

    case 6:
      // This is for autosearching pitch information with FFT and list deviations in cent
      if (filesToProcess.IsEmpty() == false) {
        m_statusProgress->AppendText(m_sourceField->GetValue());
        m_statusProgress->AppendText(wxT("\n"));
        m_statusProgress->AppendText(wxT("\n"));
        for (unsigned i = 0; i < filesToProcess.GetCount(); i++) {
          m_statusProgress->AppendText(filesToProcess.Item(i));
          m_statusProgress->AppendText(wxT("\n"));
          FileHandling fh(filesToProcess.Item(i), m_sourceField->GetValue());
          if (fh.FileCouldBeOpened()) {
            // now we need data as doubles
            wxString fullFilePath = m_sourceField->GetValue() + wxT("/") + filesToProcess.Item(i);
            SndfileHandle sfh;
            sfh = SndfileHandle(((const char*)fullFilePath.mb_str()));
            double *audioData = new double[sfh.frames() * sfh.channels()];
            sfh.read(audioData, sfh.frames() * sfh.channels());

            // autosearch pitch and calculate midi note and pitch fraction
            double pitch = fh.GetFFTPitch(audioData);
            int midi_note = (69 + 12 * (log10(pitch / 440.0) / log10(2)));
            double midi_note_pitch = 440.0 * pow(2, ((double)(midi_note - 69) / 12.0));
            double cent_deviation = 1200 * (log10(pitch / midi_note_pitch) / log10(2));
            double deviationToRaise = 100.0 - cent_deviation;
            double deviationToLower = -cent_deviation;

            m_statusProgress->AppendText(wxString::Format(wxT("\tFFT detected pitch = %.2f Hz\n"), pitch));
            m_statusProgress->AppendText(wxString::Format(wxT("\tCents to tune up = %.2f \n"), deviationToRaise));
            m_statusProgress->AppendText(wxString::Format(wxT("\tCents to tune down = %.2f \n"), deviationToLower));

            // delete the now unneccessary array of double audio data
            delete[] audioData;
          } else {
            m_statusProgress->AppendText(wxT("\tCouldn't open file!\n"));
          }
        }
        m_statusProgress->AppendText(wxT("Batch process complete!\n\n"));
      } else {
        m_statusProgress->AppendText(wxT("No wav files to process!\n"));
      }

    break;

    case 7:
      // This is for autosearching pitch information in timedomain and store it in smpl chunk
      if (filesToProcess.IsEmpty() == false) {
        for (unsigned i = 0; i < filesToProcess.GetCount(); i++) {
          m_statusProgress->AppendText(filesToProcess.Item(i));
          m_statusProgress->AppendText(wxT("\n"));
          FileHandling fh(filesToProcess.Item(i), m_sourceField->GetValue());
          if (fh.FileCouldBeOpened()) {
            m_statusProgress->AppendText(wxT("\tFile opened.\n"));

            // now we need data as doubles
            wxString fullFilePath = m_sourceField->GetValue() + wxT("/") + filesToProcess.Item(i);
            SndfileHandle sfh;
            sfh = SndfileHandle(((const char*)fullFilePath.mb_str()));
            double *audioData = new double[sfh.frames() * sfh.channels()];
            sfh.read(audioData, sfh.frames() * sfh.channels());

            // autosearch pitch and calculate midi note and pitch fraction
            double pitch = fh.GetFFTPitch(audioData);
            pitch = fh.GetTDPitch();
            int midi_note = (69 + 12 * (log10(pitch / 440.0) / log10(2)));
            double midi_note_pitch = 440.0 * pow(2, ((double)(midi_note - 69) / 12.0));
            double cent_deviation = 1200 * (log10(pitch / midi_note_pitch) / log10(2));
            unsigned int midi_pitch_fraction = ((double)UINT_MAX * (cent_deviation / 100.0));

            // set midi note and pitch fraction to loopmarkers
            fh.m_loops->SetMIDIUnityNote((char) midi_note);
            fh.m_loops->SetMIDIPitchFraction(midi_pitch_fraction);
            
            fh.SaveAudioFile(filesToProcess.Item(i), m_targetField->GetValue());
            m_statusProgress->AppendText(wxString::Format(wxT("\tDetected pitch = %.2f Hz\n"), pitch));
            m_statusProgress->AppendText(wxT("\tDone!\n"));

            // delete the now unneccessary array of double audio data
            delete[] audioData;
          } else {
            m_statusProgress->AppendText(wxT("\tCouldn't open file!\n"));
          }
        }
        m_statusProgress->AppendText(wxT("Batch process complete!\n\n"));
      } else {
        m_statusProgress->AppendText(wxT("No wav files to process!\n"));
      }

    break;

    case 8:
      // This is for autosearching pitch information in timedomain and list deviations in cent
      if (filesToProcess.IsEmpty() == false) {
        m_statusProgress->AppendText(m_sourceField->GetValue());
        m_statusProgress->AppendText(wxT("\n"));
        m_statusProgress->AppendText(wxT("\n"));
        for (unsigned i = 0; i < filesToProcess.GetCount(); i++) {
          m_statusProgress->AppendText(filesToProcess.Item(i));
          m_statusProgress->AppendText(wxT("\n"));
          FileHandling fh(filesToProcess.Item(i), m_sourceField->GetValue());
          if (fh.FileCouldBeOpened()) {
            // now we need data as doubles
            wxString fullFilePath = m_sourceField->GetValue() + wxT("/") + filesToProcess.Item(i);
            SndfileHandle sfh;
            sfh = SndfileHandle(((const char*)fullFilePath.mb_str()));
            double *audioData = new double[sfh.frames() * sfh.channels()];
            sfh.read(audioData, sfh.frames() * sfh.channels());

            // autosearch pitch and calculate midi note and pitch fraction
            double pitch = fh.GetFFTPitch(audioData);
            pitch = fh.GetTDPitch();
            int midi_note = (69 + 12 * (log10(pitch / 440.0) / log10(2)));
            double midi_note_pitch = 440.0 * pow(2, ((double)(midi_note - 69) / 12.0));
            double cent_deviation = 1200 * (log10(pitch / midi_note_pitch) / log10(2));
            double deviationToRaise = 100.0 - cent_deviation;
            double deviationToLower = -cent_deviation;

            m_statusProgress->AppendText(wxString::Format(wxT("\tDetected pitch in time domain = %.2f Hz\n"), pitch));
            m_statusProgress->AppendText(wxString::Format(wxT("\tCents to tune up = %.2f \n"), deviationToRaise));
            m_statusProgress->AppendText(wxString::Format(wxT("\tCents to tune down = %.2f \n"), deviationToLower));

            // delete the now unneccessary array of double audio data
            delete[] audioData;
          } else {
            m_statusProgress->AppendText(wxT("\tCouldn't open file!\n"));
          }
        }
        m_statusProgress->AppendText(wxT("Batch process complete!\n\n"));
      } else {
        m_statusProgress->AppendText(wxT("No wav files to process!\n"));
      }

    break;

    default:
      // This should be impossible as well!
      m_statusProgress->AppendText(wxT("No process selected!\n"));
  }
  delete autoloopSettings;
  delete autoloop;
}

