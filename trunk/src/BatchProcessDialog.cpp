/*
 * BatchProcessDialog.cpp is a part of LoopAuditioneer software
 * Copyright (C) 2011-2013 Lars Palo
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
#include "StopHarmonicDialog.h"
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
  m_batchProcessesAvailable.Add(wxT("List existing pitch info in file(s)"));
  m_batchProcessesAvailable.Add(wxT("Set pitch info from file name nr."));
  m_batchProcessesAvailable.Add(wxT("Copy pitch info from corresponding file(s)"));
  m_batchProcessesAvailable.Add(wxT("Create Pipe999PitchTuning= lines from file(s)"));
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
        m_statusProgress->AppendText(m_sourceField->GetValue());
        m_statusProgress->AppendText(wxT("\n"));
        m_statusProgress->AppendText(wxT("\n"));
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
        m_statusProgress->AppendText(m_sourceField->GetValue());
        m_statusProgress->AppendText(wxT("\n"));
        m_statusProgress->AppendText(wxT("\n"));
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
            double pitch = fh.GetTDPitch(audioData);
            int midi_note;
            double midi_note_pitch;
            double cent_deviation;
            unsigned int midi_pitch_fraction;
            if (pitch != 0) {
              midi_note = (69 + 12 * (log10(pitch / 440.0) / log10(2)));
              midi_note_pitch = 440.0 * pow(2, ((double)(midi_note - 69) / 12.0));
              cent_deviation = 1200 * (log10(pitch / midi_note_pitch) / log10(2));
              midi_pitch_fraction = ((double)UINT_MAX * (cent_deviation / 100.0));
            } else {
              midi_note = 0;
              midi_note_pitch = 0;
              cent_deviation = 0;
              midi_pitch_fraction = 0;
            }

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
            double pitch = fh.GetTDPitch(audioData);
            int midi_note;
            double midi_note_pitch;
            double cent_deviation;
            double deviationToRaise;
            double deviationToLower;
            if (pitch != 0) {
              midi_note = (69 + 12 * (log10(pitch / 440.0) / log10(2)));
              midi_note_pitch = 440.0 * pow(2, ((double)(midi_note - 69) / 12.0));
              cent_deviation = 1200 * (log10(pitch / midi_note_pitch) / log10(2));
              deviationToRaise = 100.0 - cent_deviation;
              deviationToLower = -cent_deviation;
            } else {
              midi_note = 0;
              midi_note_pitch = 0;
              cent_deviation = 0;
              deviationToRaise = 0;
              deviationToLower = 0;
            }

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

    case 9:
      // This is for listing existing pitch information in file(s)
      if (filesToProcess.IsEmpty() == false) {
        m_statusProgress->AppendText(m_sourceField->GetValue());
        m_statusProgress->AppendText(wxT("\n"));
        m_statusProgress->AppendText(wxT("\n"));
        for (unsigned i = 0; i < filesToProcess.GetCount(); i++) {
          m_statusProgress->AppendText(filesToProcess.Item(i));
          m_statusProgress->AppendText(wxT("\n"));
          FileHandling fh(filesToProcess.Item(i), m_sourceField->GetValue());
          if (fh.FileCouldBeOpened()) {
            // get pitch info and calculate resulting pitch frequency
            double cents = (double) fh.m_loops->GetMIDIPitchFraction() / (double)UINT_MAX * 100.0;
            int midiNote = (int) fh.m_loops->GetMIDIUnityNote();
            double midi_note_pitch = 440.0 * pow(2, ((double)(midiNote - 69) / 12.0));
            double resultingPitch = midi_note_pitch * pow(2, (cents / 1200.0));
            double deviationToRaise = 100 - cents;
            double deviationToLower = -cents;
           
            m_statusProgress->AppendText(wxString::Format(wxT("\tExisting MIDINote = %i\n"), midiNote));
            m_statusProgress->AppendText(wxString::Format(wxT("\tMIDIPitchFraction (in cents) = %.2f\n"), cents));
            m_statusProgress->AppendText(wxString::Format(wxT("\tResulting Frequency = %.2f\n"), resultingPitch));
            m_statusProgress->AppendText(wxString::Format(wxT("\tTo raise in ODF: Pipe999PitchTuning=%.2f\n"), deviationToRaise));
            m_statusProgress->AppendText(wxString::Format(wxT("\tTo lower in ODF Pipe999PitchTuning=%.2f\n"), deviationToLower));

          } else {
            m_statusProgress->AppendText(wxT("\tCouldn't open file!\n"));
          }
        }
        m_statusProgress->AppendText(wxT("Batch process complete!\n\n"));
      } else {
        m_statusProgress->AppendText(wxT("No wav files to process!\n"));
      }

    break;

    case 10:
      // This is for setting pitch info from file name
      if (filesToProcess.IsEmpty() == false) {
        // Create a dialog to select foot to use
        StopHarmonicDialog harmDlg(this);
        if (harmDlg.ShowModal() == wxID_OK) {
          int harmonicNr = harmDlg.GetSelectedHarmonic();
          m_statusProgress->AppendText(m_sourceField->GetValue());
          m_statusProgress->AppendText(wxT("\n"));
          m_statusProgress->AppendText(wxT("\n"));
          for (unsigned i = 0; i < filesToProcess.GetCount(); i++) {
            FileHandling fh(filesToProcess.Item(i), m_sourceField->GetValue());
            if (fh.FileCouldBeOpened()) {
              wxString currentFileName = filesToProcess.Item(i);
              wxString midiNrStr = currentFileName.Mid(0, 3);
              int midiNr = wxAtoi(midiNrStr);
              if (midiNr != 0) {
                // Calculate pitch from detected MIDI note
                double initialPitch = 440.0 * pow(2, ((double)(midiNr - 69) / 12.0));

                // Correct pitch from harmonic number
                double actualPitch = initialPitch * (8.0 / (64.0 / (double) harmonicNr));

                // Calculate new dwMIDINote and dwMIDIPitchFraction
                int midi_note = (69 + 12 * (log10(actualPitch / 440.0) / log10(2)));
                double midi_note_pitch = 440.0 * pow(2, ((double)(midi_note - 69) / 12.0));
                double cent_deviation = 1200 * (log10(actualPitch / midi_note_pitch) / log10(2));
                unsigned int midi_pitch_fraction = ((double)UINT_MAX * (cent_deviation / 100.0));

                m_statusProgress->AppendText(currentFileName);
                m_statusProgress->AppendText(wxT("\n"));
                m_statusProgress->AppendText(wxString::Format(wxT("\tExtracted MIDI number = %i \n"), midiNr));
                m_statusProgress->AppendText(wxString::Format(wxT("\tInitial pitch = %.2f \n"), initialPitch));
                m_statusProgress->AppendText(wxString::Format(wxT("\tRe-calculated pitch = %.2f \n"), actualPitch));
                m_statusProgress->AppendText(wxString::Format(wxT("\tMIDINote = %i \n"), midi_note));
                m_statusProgress->AppendText(wxString::Format(wxT("\tPitchFraction = %.2f \n"), cent_deviation));

                // set midi note and pitch fraction to loopmarkers
                fh.m_loops->SetMIDIUnityNote((char) midi_note);
                fh.m_loops->SetMIDIPitchFraction(midi_pitch_fraction);

                // save file
                fh.SaveAudioFile(filesToProcess.Item(i), m_targetField->GetValue());
                m_statusProgress->AppendText(wxT("\tDone!\n"));
              
              } else {
                m_statusProgress->AppendText(wxT("\tCouldn't determine MIDI note from name!\n"));
              }
            } else {
              m_statusProgress->AppendText(wxT("\tCouldn't open file!\n"));
            }
          }
          m_statusProgress->AppendText(wxT("Batch process complete!\n\n"));
        } else {
          m_statusProgress->AppendText(wxT("\tProcess aborted!\n"));
        }
      } else {
        m_statusProgress->AppendText(wxT("No wav files to process!\n"));
      }

    break;

    case 11:
      // This is for copying pitch information from corresponding file(s)
      if (filesToProcess.IsEmpty() == false) {
        m_statusProgress->AppendText(wxT("Reading source from "));
        m_statusProgress->AppendText(m_sourceField->GetValue());
        m_statusProgress->AppendText(wxT("\n"));
        m_statusProgress->AppendText(wxT("\n"));
        for (unsigned i = 0; i < filesToProcess.GetCount(); i++) {
          m_statusProgress->AppendText(filesToProcess.Item(i));
          m_statusProgress->AppendText(wxT("\n"));
          FileHandling sourceFile(filesToProcess.Item(i), m_sourceField->GetValue());
          if (sourceFile.FileCouldBeOpened()) {
            // get pitch info from the source file
            unsigned int pitchFraction = sourceFile.m_loops->GetMIDIPitchFraction();
            int midiNote = (int) sourceFile.m_loops->GetMIDIUnityNote();
            double cents = (double) pitchFraction / (double)UINT_MAX * 100.0;

            // try to open a corresponding target file
            FileHandling targetFile(filesToProcess.Item(i), m_targetField->GetValue());
            if (targetFile.FileCouldBeOpened()) {
              // set midi note and pitch fraction to target file
              targetFile.m_loops->SetMIDIUnityNote((char) midiNote);
              targetFile.m_loops->SetMIDIPitchFraction(pitchFraction);

              // save target file
              targetFile.SaveAudioFile(filesToProcess.Item(i), m_targetField->GetValue());
              m_statusProgress->AppendText(wxString::Format(wxT("\tMIDINote = %i \n"), midiNote));
              m_statusProgress->AppendText(wxString::Format(wxT("\tMIDIPitchFraction (in cents) = %.2f \n"), cents));
              m_statusProgress->AppendText(wxT("\tDone!\n"));
            } else {
              m_statusProgress->AppendText(wxT("\tCouldn't open target file with such name!\n"));
            }
          } else {
            m_statusProgress->AppendText(wxT("\tCouldn't open source file!\n"));
          }
        }
        m_statusProgress->AppendText(wxT("Batch process complete!\n\n"));
      } else {
        m_statusProgress->AppendText(wxT("No wav files to process!\n"));
      }

    break;

    case 12:
      // This is for writing out the Pipe999PitchTuning lines for GO ODFs
      if (filesToProcess.IsEmpty() == false) {
        for (unsigned i = 0; i < filesToProcess.GetCount(); i++) {
          FileHandling fh(filesToProcess.Item(i), m_sourceField->GetValue());
          if (fh.FileCouldBeOpened()) {
            // get pitch info and calculate resulting pitch frequency
            double cents = (double) fh.m_loops->GetMIDIPitchFraction() / (double)UINT_MAX * 100.0;
            int midiNote = (int) fh.m_loops->GetMIDIUnityNote();
            double midi_note_pitch = 440.0 * pow(2, ((double)(midiNote - 69) / 12.0));
            double resultingPitch = midi_note_pitch * pow(2, (cents / 1200.0));
            double deviationToRaise = 100 - cents;
            double deviationToLower = -cents;

            if (fabs(deviationToRaise) < fabs(deviationToLower)) {
              m_statusProgress->AppendText(
                wxString::Format(wxT("Pipe%03uPitchTuning=%.2f\n"), i + 1, deviationToRaise)
              );
            } else {
              m_statusProgress->AppendText(
                wxString::Format(wxT("Pipe%03uPitchTuning=%.2f\n"), i + 1, deviationToLower)
              );
            }
          } else {
            m_statusProgress->AppendText(wxT("\tCouldn't open file!\n"));
          }
        }
        m_statusProgress->AppendText(wxT("\nBatch process complete!\n\n"));
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

