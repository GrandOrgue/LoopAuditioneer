/*
 * BatchProcessDialog.cpp is a part of LoopAuditioneer software
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

#include "BatchProcessDialog.h"
#include "StopHarmonicDialog.h"
#include "AutoLooping.h"
#include "FileHandling.h"
#include "CutNFadeDialog.h"
#include "CrossfadeDialog.h"
#include "ListInfoDialog.h"
#include <wx/statline.h>
#include <wx/listctrl.h>
#include <wx/filename.h>
#include <wx/dir.h>
#include <wx/datetime.h>
#include <vector>
#include <climits>
#include <cmath>
#include <cstring>

IMPLEMENT_CLASS(BatchProcessDialog, wxDialog )

BEGIN_EVENT_TABLE(BatchProcessDialog, wxDialog)
  EVT_BUTTON(ID_ADD_SOURCE, BatchProcessDialog::OnAddSource)
  EVT_BUTTON(ID_ADD_TARGET, BatchProcessDialog::OnAddTarget)
  EVT_BUTTON(ID_RUN_BATCH, BatchProcessDialog::OnRunBatch)
  EVT_CHOICE(ID_PROCESSBOX, BatchProcessDialog::OnChoiceSelected)
END_EVENT_TABLE()

BatchProcessDialog::BatchProcessDialog(AutoLoopDialog* autoloopSettings) {
  Init(autoloopSettings);
}

BatchProcessDialog::BatchProcessDialog(
    AutoLoopDialog* autoloopSettings,
    wxWindow* parent,
    wxWindowID id,
    const wxString& caption,
    const wxPoint& pos,
    const wxSize& size,
    long style
  ) {
  Init(autoloopSettings);
  Create(parent, id, caption, pos, size, style);
}

void BatchProcessDialog::Init(AutoLoopDialog* autoloopSettings) {
  m_batchProcessesAvailable.Add(wxEmptyString);
  m_batchProcessesAvailable.Add(wxT("Kill all loops"));
  m_batchProcessesAvailable.Add(wxT("Kill all cues"));
  m_batchProcessesAvailable.Add(wxT("Kill loops & cues"));
  m_batchProcessesAvailable.Add(wxT("Search for loops"));
  m_batchProcessesAvailable.Add(wxT("Store FFT detected pitch info"));
  m_batchProcessesAvailable.Add(wxT("List FFT pitch deviations"));
  m_batchProcessesAvailable.Add(wxT("Store HPS detected pitch info"));
  m_batchProcessesAvailable.Add(wxT("List HPS pitch deviations"));
  m_batchProcessesAvailable.Add(wxT("Store time domain detected pitch info"));
  m_batchProcessesAvailable.Add(wxT("List time domain pitch deviations"));
  m_batchProcessesAvailable.Add(wxT("List existing pitch info in file(s)"));
  m_batchProcessesAvailable.Add(wxT("Set pitch info from file name nr."));
  m_batchProcessesAvailable.Add(wxT("Copy pitch info from corresponding file(s)"));
  m_batchProcessesAvailable.Add(wxT("Write PitchTuning lines from embedded pitch"));
  m_batchProcessesAvailable.Add(wxT("Remove sound between last loop and cue"));
  m_batchProcessesAvailable.Add(wxT("Cut & Fade in/out"));
  m_batchProcessesAvailable.Add(wxT("Crossfade all loops"));
  m_batchProcessesAvailable.Add(wxT("Set LIST INFO strings"));

  m_lastSource = wxEmptyString;
  m_lastTarget = wxEmptyString;
  m_mustRefreshMainDir = false;

  m_loopSettings = autoloopSettings;
  m_currentWorkingDir = wxEmptyString;
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
    wxT("&Exit batch mode"),
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

void BatchProcessDialog::OnAddSource(wxCommandEvent& WXUNUSED(event)) {
  wxDirDialog *selectDirDialog = new wxDirDialog(
    this,
    wxT("Choose a source folder to batch process"),
    m_lastSource,
    wxDD_DIR_MUST_EXIST | wxDD_CHANGE_DIR
  );

  if (selectDirDialog->ShowModal() == wxID_OK) {
    wxString pathToAdd = selectDirDialog->GetPath();
    if (pathToAdd != wxEmptyString) {
      m_sourceField->ChangeValue(pathToAdd);
      m_lastSource = pathToAdd;
      m_targetField->ChangeValue(pathToAdd);
      m_lastTarget = pathToAdd;
    }
  }

  ReadyToRockAndRoll();
}

void BatchProcessDialog::OnAddTarget(wxCommandEvent& WXUNUSED(event)) {
  wxDirDialog *selectDirDialog = new wxDirDialog(
    this,
    wxT("Choose target folder for batch process"),
    m_lastTarget,
    wxDD_CHANGE_DIR
  );

  if (selectDirDialog->ShowModal() == wxID_OK) {
    wxString pathToAdd = selectDirDialog->GetPath();
    if (pathToAdd != wxEmptyString) {
      m_targetField->ChangeValue(pathToAdd);
      m_lastTarget = pathToAdd;
    }
  }

  ReadyToRockAndRoll();
}

void BatchProcessDialog::OnChoiceSelected(wxCommandEvent& WXUNUSED(event)) {
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

void BatchProcessDialog::OnRunBatch(wxCommandEvent& WXUNUSED(event)) {
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

  // sort the files and remove doubles for windows... if there are any!
  if (!filesToProcess.IsEmpty()) {
    filesToProcess.Sort();
    size_t lineCounter = 0;
    while (lineCounter < filesToProcess.GetCount() - 1) {
      if (filesToProcess[lineCounter] == filesToProcess[lineCounter + 1])
        filesToProcess.RemoveAt(lineCounter + 1);
      else
        lineCounter++;
    }
  }

  // Depending on selected process do
  switch(m_processChoiceBox->GetSelection()) {
    case 0:
      // This should be impossible as Run button should be disabled!
      m_statusProgress->AppendText(wxT("No process selected!\n"));
    break;

    case 1:
      // This removes all loops from the wav files!
      if (!filesToProcess.IsEmpty()) {
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
      if (!filesToProcess.IsEmpty()) {
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
      if (!filesToProcess.IsEmpty()) {
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
      // This is for autosearching for loops
      // First make sure to update AutoLooping object
      autoloop->SetThreshold(m_loopSettings->GetThreshold());
      autoloop->SetDuration(m_loopSettings->GetDuration());
      autoloop->SetBetween(m_loopSettings->GetBetween());
      autoloop->SetQuality(m_loopSettings->GetQuality());
      autoloop->SetCandidates(m_loopSettings->GetCandidates());
      autoloop->SetLoops(m_loopSettings->GetNrLoops());
      autoloop->SetMultiple(m_loopSettings->GetMultiple());
      autoloop->SetBruteForce(m_loopSettings->GetBruteForce());

      if (!filesToProcess.IsEmpty()) {
        for (unsigned i = 0; i < filesToProcess.GetCount(); i++) {
          m_statusProgress->AppendText(filesToProcess.Item(i));
          m_statusProgress->AppendText(wxT("\n"));
          FileHandling fh(filesToProcess.Item(i), m_sourceField->GetValue());
          if (fh.FileCouldBeOpened()) {
            // set sustainsection from slider data in autoloop settings to audiofile
            fh.SetSliderSustainsection(m_loopSettings->GetStart(), m_loopSettings->GetEnd());
            // adjust choice of sustainsection
            fh.SetAutoSustainSearch(m_loopSettings->GetAutosearch());
            int nbLoops = fh.m_loops->GetNumberOfLoops();
            m_statusProgress->AppendText(wxString::Format(wxT("\tFile opened, it already contains %i loop(s)\n"), nbLoops));
            if (nbLoops < 16) {
              // first get all loops already in file
              std::vector<std::pair<unsigned, unsigned> > loopsAlreadyInFile;
              for (int j = 0; j < nbLoops; j++) {
                LOOPDATA aLoop;
                fh.m_loops->GetLoopData(j, aLoop);
                loopsAlreadyInFile.push_back(std::make_pair(aLoop.dwStart, aLoop.dwEnd));
              }

              // now we need to search for loops and for that we need data as doubles
              unsigned nbrOfSmpls = fh.ArrayLength / fh.m_channels;
              double *audioData = new double[nbrOfSmpls];
              fh.SeparateStrongestChannel(audioData);
              // retrieve the used sustainsection
              std::pair <unsigned, unsigned> sustainSection = fh.GetSustainsection();
              // vector to receive found loops
              std::vector<std::pair<std::pair<unsigned, unsigned>, double> > addLoops;
              // call to search for loops
              bool foundLoops = autoloop->AutoFindLoops(
                audioData,
                fh.GetSampleRate(),
                addLoops,
                sustainSection.first,
                sustainSection.second,
                loopsAlreadyInFile
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
                m_statusProgress->AppendText(wxString::Format(wxT("\t%zu loop(s) found.\n"), addLoops.size()));
                if (nbLoops + addLoops.size() > 16)
                  m_statusProgress->AppendText(wxString::Format(wxT("\tOnly %i first loops could be saved.\n"), 16 - nbLoops));
              } else {
                // no loops found!
                m_statusProgress->AppendText(wxT("\tNo loops found!\n"));
              }

              fh.SaveAudioFile(filesToProcess.Item(i), m_targetField->GetValue());
              m_statusProgress->AppendText(wxT("\tDone!\n"));
            } else {
              // file already contained max number of loops
              m_statusProgress->AppendText(wxT("\tCannot save any more loops! Done!\n"));
            }
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
      if (!filesToProcess.IsEmpty()) {
        m_statusProgress->AppendText(m_sourceField->GetValue());
        m_statusProgress->AppendText(wxT("\n"));
        m_statusProgress->AppendText(wxT("\n"));
        for (unsigned i = 0; i < filesToProcess.GetCount(); i++) {
          m_statusProgress->AppendText(filesToProcess.Item(i));
          m_statusProgress->AppendText(wxT("\n"));
          FileHandling fh(filesToProcess.Item(i), m_sourceField->GetValue());
          if (fh.FileCouldBeOpened()) {
            m_statusProgress->AppendText(wxT("\tFile opened.\n"));

            // autosearch pitch and calculate midi note and pitch fraction
            double fftPitches[2];
            for (int j = 0; j < 2; j++)
              fftPitches[j] = 0;
            fh.GetFFTPitch(fftPitches);
            int midi_note = (69 + 12 * (log10(fftPitches[0] / 440.0) / log10(2)));
            double midi_note_pitch = 440.0 * pow(2, ((double)(midi_note - 69) / 12.0));
            double cent_deviation = 1200 * (log10(fftPitches[0] / midi_note_pitch) / log10(2));
            unsigned int midi_pitch_fraction = ((double)UINT_MAX * (cent_deviation / 100.0));

            // set midi note and pitch fraction to loopmarkers
            fh.m_loops->SetMIDIUnityNote((char) midi_note);
            fh.m_loops->SetMIDIPitchFraction(midi_pitch_fraction);
            
            fh.SaveAudioFile(filesToProcess.Item(i), m_targetField->GetValue());
            m_statusProgress->AppendText(wxString::Format(wxT("\tDetected pitch = %.2f Hz\n"), fftPitches[0]));
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

    case 6:
      // This is for autosearching pitch information with FFT and list it and lines to specify it in an ODF
      if (!filesToProcess.IsEmpty()) {
        m_statusProgress->AppendText(m_sourceField->GetValue());
        m_statusProgress->AppendText(wxT("\n"));
        m_statusProgress->AppendText(wxT("\n"));

        int lastMidiNr = 0;
        int pipeNr = 0;
        for (unsigned i = 0; i < filesToProcess.GetCount(); i++) {
          // get midi number from file name
          wxString currentFileName = filesToProcess.Item(i);
          wxString midiNrStr = currentFileName.Mid(0, 3);
          int midiNr = wxAtoi(midiNrStr);
          if (midiNr == 0 || midiNr == lastMidiNr)
            continue;
          else {
            lastMidiNr = midiNr;
            pipeNr++;
          }
          m_statusProgress->AppendText(filesToProcess.Item(i));
          m_statusProgress->AppendText(wxT("\n"));
          FileHandling fh(filesToProcess.Item(i), m_sourceField->GetValue());
          if (fh.FileCouldBeOpened()) {

            // autosearch pitch and calculate midi note and pitch fraction
            double fftPitches[2];
            for (int j = 0; j < 2; j++)
              fftPitches[j] = 0;
            fh.GetFFTPitch(fftPitches);
            int midi_note = (69 + 12 * (log10(fftPitches[0] / 440.0) / log10(2)));
            double midi_note_pitch = 440.0 * pow(2, ((double)(midi_note - 69) / 12.0));
            double cent_deviation = 1200 * (log10(fftPitches[0] / midi_note_pitch) / log10(2));

            m_statusProgress->AppendText(wxString::Format(wxT("\tFFT detected pitch = %.2f Hz\n"), fftPitches[0]));
            m_statusProgress->AppendText(wxString::Format(wxT("\tPipe%03dMIDIKeyNumber=%d"), pipeNr, midi_note));
            m_statusProgress->AppendText(wxT("\n"));
            m_statusProgress->AppendText(wxString::Format(wxT("\tPipe%03dPitchFraction="), pipeNr));
            m_statusProgress->AppendText(MyDoubleToString(cent_deviation, 6));
            m_statusProgress->AppendText(wxT("\n"));

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
      // This is for autosearching pitch information with HPS and store it in smpl chunk
      if (!filesToProcess.IsEmpty()) {
        m_statusProgress->AppendText(m_sourceField->GetValue());
        m_statusProgress->AppendText(wxT("\n"));
        m_statusProgress->AppendText(wxT("\n"));
        for (unsigned i = 0; i < filesToProcess.GetCount(); i++) {
          m_statusProgress->AppendText(filesToProcess.Item(i));
          m_statusProgress->AppendText(wxT("\n"));
          FileHandling fh(filesToProcess.Item(i), m_sourceField->GetValue());
          if (fh.FileCouldBeOpened()) {
            m_statusProgress->AppendText(wxT("\tFile opened.\n"));

            // autosearch pitch and calculate midi note and pitch fraction
            double fftPitches[2];
            for (int j = 0; j < 2; j++)
              fftPitches[j] = 0;
            fh.GetFFTPitch(fftPitches);
            int midi_note = (69 + 12 * (log10(fftPitches[1] / 440.0) / log10(2)));
            double midi_note_pitch = 440.0 * pow(2, ((double)(midi_note - 69) / 12.0));
            double cent_deviation = 1200 * (log10(fftPitches[1] / midi_note_pitch) / log10(2));
            unsigned int midi_pitch_fraction = ((double)UINT_MAX * (cent_deviation / 100.0));

            // set midi note and pitch fraction to loopmarkers
            fh.m_loops->SetMIDIUnityNote((char) midi_note);
            fh.m_loops->SetMIDIPitchFraction(midi_pitch_fraction);
            
            fh.SaveAudioFile(filesToProcess.Item(i), m_targetField->GetValue());
            m_statusProgress->AppendText(wxString::Format(wxT("\tDetected pitch = %.2f Hz\n"), fftPitches[1]));
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

    case 8:
      // This is for autosearching pitch information with HPS and list deviations in cent
      if (!filesToProcess.IsEmpty()) {
        m_statusProgress->AppendText(m_sourceField->GetValue());
        m_statusProgress->AppendText(wxT("\n"));
        m_statusProgress->AppendText(wxT("\n"));

        int lastMidiNr = 0;
        int pipeNr = 0;
        for (unsigned i = 0; i < filesToProcess.GetCount(); i++) {
          // get midi number from file name
          wxString currentFileName = filesToProcess.Item(i);
          wxString midiNrStr = currentFileName.Mid(0, 3);
          int midiNr = wxAtoi(midiNrStr);
          if (midiNr == 0 || midiNr == lastMidiNr)
            continue;
          else {
            lastMidiNr = midiNr;
            pipeNr++;
          }
          m_statusProgress->AppendText(filesToProcess.Item(i));
          m_statusProgress->AppendText(wxT("\n"));
          FileHandling fh(filesToProcess.Item(i), m_sourceField->GetValue());
          if (fh.FileCouldBeOpened()) {

            // autosearch pitch and calculate midi note and pitch fraction
            double fftPitches[2];
            for (int j = 0; j < 2; j++)
              fftPitches[j] = 0;
            fh.GetFFTPitch(fftPitches);
            int midi_note = (69 + 12 * (log10(fftPitches[1] / 440.0) / log10(2)));
            double midi_note_pitch = 440.0 * pow(2, ((double)(midi_note - 69) / 12.0));
            double cent_deviation = 1200 * (log10(fftPitches[1] / midi_note_pitch) / log10(2));

            m_statusProgress->AppendText(wxString::Format(wxT("\tHPS detected pitch = %.2f Hz\n"), fftPitches[1]));
            m_statusProgress->AppendText(wxString::Format(wxT("\tPipe%03dMIDIKeyNumber=%d"), pipeNr, midi_note));
            m_statusProgress->AppendText(wxT("\n"));
            m_statusProgress->AppendText(wxString::Format(wxT("\tPipe%03dPitchFraction="), pipeNr));
            m_statusProgress->AppendText(MyDoubleToString(cent_deviation, 6));
            m_statusProgress->AppendText(wxT("\n"));

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
      // This is for autosearching pitch information in timedomain and store it in smpl chunk
      if (!filesToProcess.IsEmpty()) {
        m_statusProgress->AppendText(m_sourceField->GetValue());
        m_statusProgress->AppendText(wxT("\n"));
        m_statusProgress->AppendText(wxT("\n"));
        for (unsigned i = 0; i < filesToProcess.GetCount(); i++) {
          m_statusProgress->AppendText(filesToProcess.Item(i));
          m_statusProgress->AppendText(wxT("\n"));
          FileHandling fh(filesToProcess.Item(i), m_sourceField->GetValue());
          if (fh.FileCouldBeOpened()) {
            m_statusProgress->AppendText(wxT("\tFile opened.\n"));

            // autosearch pitch and calculate midi note and pitch fraction
            double pitch = fh.GetTDPitch();
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
      // This is for autosearching pitch information in timedomain and list deviations in cent
      if (!filesToProcess.IsEmpty()) {
        m_statusProgress->AppendText(m_sourceField->GetValue());
        m_statusProgress->AppendText(wxT("\n"));
        m_statusProgress->AppendText(wxT("\n"));

        int lastMidiNr = 0;
        int pipeNr = 0;
        for (unsigned i = 0; i < filesToProcess.GetCount(); i++) {
          // get midi number from file name
          wxString currentFileName = filesToProcess.Item(i);
          wxString midiNrStr = currentFileName.Mid(0, 3);
          int midiNr = wxAtoi(midiNrStr);
          if (midiNr == 0 || midiNr == lastMidiNr)
            continue;
          else {
            lastMidiNr = midiNr;
            pipeNr++;
          }
          m_statusProgress->AppendText(filesToProcess.Item(i));
          m_statusProgress->AppendText(wxT("\n"));
          FileHandling fh(filesToProcess.Item(i), m_sourceField->GetValue());
          if (fh.FileCouldBeOpened()) {

            // autosearch pitch and calculate midi note and pitch fraction
            double pitch = fh.GetTDPitch();
            int midi_note;
            double midi_note_pitch;
            double cent_deviation;
            if (pitch != 0) {
              midi_note = (69 + 12 * (log10(pitch / 440.0) / log10(2)));
              midi_note_pitch = 440.0 * pow(2, ((double)(midi_note - 69) / 12.0));
              cent_deviation = 1200 * (log10(pitch / midi_note_pitch) / log10(2));
            } else {
              midi_note = 0;
              midi_note_pitch = 0;
              cent_deviation = 0;
            }

            m_statusProgress->AppendText(wxString::Format(wxT("\tDetected pitch in time domain = %.2f Hz\n"), pitch));
            m_statusProgress->AppendText(wxString::Format(wxT("\tPipe%03dMIDIKeyNumber=%d"), pipeNr, midi_note));
            m_statusProgress->AppendText(wxT("\n"));
            m_statusProgress->AppendText(wxString::Format(wxT("\tPipe%03dPitchFraction="), pipeNr));
            m_statusProgress->AppendText(MyDoubleToString(cent_deviation, 6));
            m_statusProgress->AppendText(wxT("\n"));

          } else {
            m_statusProgress->AppendText(wxT("\tCouldn't open file!\n"));
          }
        }
        m_statusProgress->AppendText(wxT("Batch process complete!\n\n"));
      } else {
        m_statusProgress->AppendText(wxT("No wav files to process!\n"));
      }

    break;

    case 11:
      // This is for listing existing pitch information in file(s)
      if (!filesToProcess.IsEmpty()) {
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
            m_statusProgress->AppendText(wxT("\tTo raise in ODF: Pipe999PitchTuning="));
            m_statusProgress->AppendText(MyDoubleToString(deviationToRaise, 6));
            m_statusProgress->AppendText(wxT("\n"));
            m_statusProgress->AppendText(wxT("\tTo lower in ODF Pipe999PitchTuning="));
            m_statusProgress->AppendText(MyDoubleToString(deviationToLower, 6));
            m_statusProgress->AppendText(wxT("\n"));

          } else {
            m_statusProgress->AppendText(wxT("\tCouldn't open file!\n"));
          }
        }
        m_statusProgress->AppendText(wxT("Batch process complete!\n\n"));
      } else {
        m_statusProgress->AppendText(wxT("No wav files to process!\n"));
      }

    break;

    case 12:
      // This is for setting pitch info from file name
      if (!filesToProcess.IsEmpty()) {
        // Create a dialog to select foot to use
        StopHarmonicDialog harmDlg(this);
        if (harmDlg.ShowModal() == wxID_OK) {
          int harmonicNr = harmDlg.GetSelectedHarmonic();
          double organPitch = harmDlg.GetSelectedPitch();
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
                double initialPitch = organPitch * pow(2, ((double)(midiNr - 69) / 12.0));

                // Correct pitch from harmonic number
                double actualPitch = initialPitch * (8.0 / (64.0 / (double) harmonicNr));

                // Calculate new dwMIDINote and dwMIDIPitchFraction
                int midi_note = (69 + 12 * (log10(actualPitch / organPitch) / log10(2)));
                double midi_note_pitch = organPitch * pow(2, ((double)(midi_note - 69) / 12.0));
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

    case 13:
      // This is for copying pitch information from corresponding file(s)
      if (!filesToProcess.IsEmpty()) {
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

    case 14:
      // This is for writing out the Pipe999PitchTuning lines for GO ODFs from embedded pitch
      if (!filesToProcess.IsEmpty()) {
        // Create a dialog to select harmonic number for the rank/stop to use
        StopHarmonicDialog harmDlg(this);
        if (harmDlg.ShowModal() == wxID_OK) {
          int harmonicNr = harmDlg.GetSelectedHarmonic();
          double organPitch = harmDlg.GetSelectedPitch();
          int lastMidiNr = 0;
          int pipeNr = 0;
          for (unsigned i = 0; i < filesToProcess.GetCount(); i++) {
            FileHandling fh(filesToProcess.Item(i), m_sourceField->GetValue());
            // get midi number from file name
            wxString currentFileName = filesToProcess.Item(i);
            wxString midiNrStr = currentFileName.Mid(0, 3);
            int midiNr = wxAtoi(midiNrStr);
            if (midiNr == 0 || midiNr == lastMidiNr)
              continue;
            else {
              lastMidiNr = midiNr;
              pipeNr++;
            }
            if (fh.FileCouldBeOpened()) {
              // Calculate pitch for detected MIDI note
              double midiPitch = organPitch * pow(2, ((double)(midiNr - 69) / 12.0));
              // Adjust pitch from harmonic number
              double actualPitch = midiPitch * (8.0 / (64.0 / (double) harmonicNr));
              // Compare with embedded pitch
              double centsEmbedded = (double) fh.m_loops->GetMIDIPitchFraction() / (double)UINT_MAX * 100.0;
              int midiNoteEmbedded = (int) fh.m_loops->GetMIDIUnityNote();
              double embeddedPitch = organPitch * pow(2, ((double)(midiNoteEmbedded - 69) / 12.0)) * pow(2, (centsEmbedded / 1200.0));
              double cent_deviation = 1200 * (log10(actualPitch / embeddedPitch) / log10(2));
              if (cent_deviation < -1200 || cent_deviation > 1200) {
                // Warn that this is not allowed
                m_statusProgress->AppendText(wxString::Format(wxT("PitchTuning value for %s is outside allowed range!\n"), currentFileName));
              } else {
                // Write the pitch tuning line
                m_statusProgress->AppendText(
                  wxString::Format(wxT("Pipe%03dPitchTuning="), pipeNr)
                );
                m_statusProgress->AppendText(MyDoubleToString(cent_deviation, 6));
                m_statusProgress->AppendText(wxT("\n"));
              }
            } else {
              m_statusProgress->AppendText(wxT("\tCouldn't open file!\n"));
            }
          }
        }
        m_statusProgress->AppendText(wxT("\nBatch process complete!\n\n"));
      } else {
        m_statusProgress->AppendText(wxT("No wav files to process!\n"));
      }

    break;

    case 15:
      // This is for deleting sound after last loop and before cue marker (if existing)
      if (!filesToProcess.IsEmpty()) {
        for (unsigned i = 0; i < filesToProcess.GetCount(); i++) {
          FileHandling fh(filesToProcess.Item(i), m_sourceField->GetValue());
          if (fh.FileCouldBeOpened()) {
            fh.TrimExcessData();

            // save file
            fh.SaveAudioFile(filesToProcess.Item(i), m_targetField->GetValue());
            m_statusProgress->AppendText(wxT("\tDone trimming "));
            m_statusProgress->AppendText(filesToProcess.Item(i));
            m_statusProgress->AppendText(wxT("\n"));
          } else {
            m_statusProgress->AppendText(wxT("\tCouldn't open file!\n"));
          }
        }
        m_statusProgress->AppendText(wxT("\nBatch process complete!\n\n"));
      } else {
        m_statusProgress->AppendText(wxT("No wav files to process!\n"));
      }

    break;

    case 16:
      // This is for cutting and fading in/out
      if (!filesToProcess.IsEmpty()) {

        // create the cut & fade dialog
        CutNFadeDialog cfDlg(this);
        // show the cut & fade dialog to get parameters
        if (cfDlg.ShowModal() == wxID_OK) {
           // update values
           cfDlg.TransferDataFromWindow();

          for (unsigned i = 0; i < filesToProcess.GetCount(); i++) {
            FileHandling fh(filesToProcess.Item(i), m_sourceField->GetValue());
            if (fh.FileCouldBeOpened()) {
              // make eventual cuts of audio data
              // from beginning
              if (cfDlg.GetCutStart() > 0) {
                bool success = fh.TrimStart(cfDlg.GetCutStart());

                if (!success)
                  m_statusProgress->AppendText(wxT("\nCouldn't trim from start!\n"));
              }

              // from end
              if (cfDlg.GetCutEnd() > 0) {
                bool success = fh.TrimEnd(cfDlg.GetCutEnd());

                if (!success)
                  m_statusProgress->AppendText(wxT("\nCouldn't trim from end!\n"));
              }

              // perform fade(s) as needed
              if (cfDlg.GetFadeStart() > 0)
                fh.PerformFade(cfDlg.GetFadeStart(), 0);

              if (cfDlg.GetFadeEnd() > 0)
                fh.PerformFade(cfDlg.GetFadeEnd(), 1);

              // save file
              fh.SaveAudioFile(filesToProcess.Item(i), m_targetField->GetValue());
              m_statusProgress->AppendText(wxT("\tDone trimming & fading "));
              m_statusProgress->AppendText(filesToProcess.Item(i));
              m_statusProgress->AppendText(wxT("\n"));
            } else {
              m_statusProgress->AppendText(wxT("\tCouldn't open file!\n"));
            }
          }
          m_statusProgress->AppendText(wxT("\nBatch process complete!\n\n"));
        } else {
          m_statusProgress->AppendText(wxT("\nBatch process aborted!\n"));
        }
      } else {
        m_statusProgress->AppendText(wxT("No wav files to process!\n"));
      }

    break;

    case 17:
      // This is for crossfading all existing loops
      if (!filesToProcess.IsEmpty()) {

        // create the crossfade dialog
        CrossfadeDialog cDlg(this);
        // show the crossfade dialog to get parameters
        if (cDlg.ShowModal() == wxID_OK) {
          // get values
          double crossfadeTime = cDlg.GetFadeduration();
          int crossfadetype = cDlg.GetFadetype();

          for (unsigned i = 0; i < filesToProcess.GetCount(); i++) {
            FileHandling fh(filesToProcess.Item(i), m_sourceField->GetValue());
            if (fh.FileCouldBeOpened()) {
              if (fh.m_loops->GetNumberOfLoops() > 0) {

                if (fh.m_loops->GetNumberOfLoops() > 1) {
                  // crossfading should be done in order of loop end point appearance
                  int *crossfadeOrder = new int[fh.m_loops->GetNumberOfLoops()];
                  for (int j = 0; j < fh.m_loops->GetNumberOfLoops(); j++)
                    crossfadeOrder[j] = j;

                  for (int j = 0; j < fh.m_loops->GetNumberOfLoops() - 1; j++) {
                    LOOPDATA l1;
                    fh.m_loops->GetLoopData(crossfadeOrder[j], l1);
                    unsigned lowestEndValue = l1.dwEnd;
                    
                    for (int k = j + 1; k < fh.m_loops->GetNumberOfLoops(); k++) {
                      LOOPDATA l2;
                      fh.m_loops->GetLoopData(crossfadeOrder[k], l2);

                      if (l2.dwEnd < lowestEndValue) {
                        lowestEndValue = l2.dwEnd;
                        int tempIdx = crossfadeOrder[j];
                        crossfadeOrder[j] = k;
                        crossfadeOrder[k] = tempIdx;
                      }
                    }
                  }

                  // now we know in which order the crossfades should be made
                  // but for every crossfade we must check if either another loop
                  // has a start or end point that might be affected by the crossfade
                  // and if so adjust the crossfade length
                  for (int j = 0; j < fh.m_loops->GetNumberOfLoops(); j++) {
                    LOOPDATA l1;
                    fh.m_loops->GetLoopData(crossfadeOrder[j], l1);
                    double actualFadeTime = crossfadeTime;
                    for (int k = 0; k < fh.m_loops->GetNumberOfLoops(); k++) {
                      if (k == j)
                        break;

                      LOOPDATA l2;
                      fh.m_loops->GetLoopData(crossfadeOrder[k], l2);

                      if (fabs((double) l2.dwEnd - (double) l1.dwEnd) / (double) fh.GetSampleRate() < actualFadeTime) {
                        actualFadeTime = fabs((double) l2.dwEnd - (double) l1.dwEnd) / (double) fh.GetSampleRate();
                        actualFadeTime -= 2.0 / (double) fh.GetSampleRate();
                      }

                      if (fabs((double) l2.dwStart - (double) l1.dwEnd) / (double) fh.GetSampleRate() < actualFadeTime) {
                        actualFadeTime = fabs((double) l2.dwStart - (double) l1.dwEnd) / (double) fh.GetSampleRate();
                        actualFadeTime -= 2.0 / (double) fh.GetSampleRate();
                      }
                    }
                    
                    if (actualFadeTime > 0) {
                      m_statusProgress->AppendText(wxString::Format(wxT("\t\tCrossfading loop %i with fadetime %.3f ms.\n"), crossfadeOrder[j] + 1, actualFadeTime));
                      // perform crossfading on the current loop with selected method
                      fh.PerformCrossfade(crossfadeOrder[j], actualFadeTime, crossfadetype);
                    } else {
                      m_statusProgress->AppendText(wxString::Format(wxT("\tCouldn't crossfade loop %i!\n"), crossfadeOrder[j]));
                    }
                  }
                  delete[] crossfadeOrder;
                } else {
                  // just one loop to crossfade
                  fh.PerformCrossfade(0, crossfadeTime, crossfadetype);
                }

                // save file
                fh.SaveAudioFile(filesToProcess.Item(i), m_targetField->GetValue());
                m_statusProgress->AppendText(wxT("\tDone crossfading "));
                m_statusProgress->AppendText(filesToProcess.Item(i));
                m_statusProgress->AppendText(wxT("\n"));
              } else {
                m_statusProgress->AppendText(wxT("\tNo loops to crossfade!\n"));
              }
            } else {
              m_statusProgress->AppendText(wxT("\tCouldn't open file!\n"));
            }
          }
          m_statusProgress->AppendText(wxT("\nBatch process complete!\n\n"));
        } else {
          m_statusProgress->AppendText(wxT("\nBatch process aborted!\n"));
        }
      } else {
        m_statusProgress->AppendText(wxT("No wav files to process!\n"));
      }

    break;
    
    case 18:
      // This is for setting LIST INFO strings
      if (!filesToProcess.IsEmpty()) {

        wxString art;
        wxString copyr;
        wxString comm;
        wxDateTime cr_dt;
        // we must actually open the first file to be able to create list info dialog
        FileHandling *first = new FileHandling(filesToProcess.Item(0), m_sourceField->GetValue());
        if (first->FileCouldBeOpened()) {
          // create the list info dialog
          ListInfoDialog infoDlg(first, this);
          // show the ListInfoDialog to get variables filled
          if (infoDlg.ShowModal() == wxID_OK) {
            art = infoDlg.getArtist();
            copyr = infoDlg.getCopyright();
            comm = infoDlg.getComment();
            cr_dt = infoDlg.getCreationDate();
          } else {
            // we must abort
            m_statusProgress->AppendText(wxT("\nProcess cancelled!\n"));
            break;
          }
        } else {
          // we must abort
          m_statusProgress->AppendText(wxT("\nThe first file couldn't be opened!\n"));
          break;
        }
        // we're done with the first opening
        delete first;
        
        // now we'll fill all the files with the set LIST INFO strings
        for (unsigned i = 0; i < filesToProcess.GetCount(); i++) {
          FileHandling fh(filesToProcess.Item(i), m_sourceField->GetValue());
          if (fh.FileCouldBeOpened()) {
            // set info strings
            fh.m_info.artist = art;
            fh.m_info.copyright = copyr;
            fh.m_info.comment = comm;
            fh.m_info.creation_date = cr_dt;

            // save file
            fh.SaveAudioFile(filesToProcess.Item(i), m_targetField->GetValue());
            m_statusProgress->AppendText(wxT("\tDone writing LIST INFO strings to "));
            m_statusProgress->AppendText(filesToProcess.Item(i));
            m_statusProgress->AppendText(wxT("\n"));
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
  delete autoloop;

  if (m_currentWorkingDir.IsSameAs(m_targetField->GetValue()))
    m_mustRefreshMainDir = true;
}

wxString BatchProcessDialog::MyDoubleToString(double dbl, int precision) {
  wxString formatString;
  formatString << wxT("%.") << precision << wxT("f");
  wxString returnString = wxString::Format(formatString, dbl);

  if (returnString.Find(wxT(',')) != wxNOT_FOUND) {
    returnString.Replace(wxT(","), wxT("."));
  }

  return returnString;
}

void BatchProcessDialog::ClearStatusProgress() {
  m_statusProgress->Clear();
}

wxString BatchProcessDialog::GetLastSource() {
  return m_lastSource;
}

wxString BatchProcessDialog::GetLastTarget() {
  return m_lastTarget;
}

void BatchProcessDialog::SetLastSource(wxString source) {
  if (wxDir::Exists(source)) {
    m_lastSource = source;
    m_sourceField->ChangeValue(m_lastSource);
  } else {
    m_lastSource = wxEmptyString;
    m_sourceField->ChangeValue(m_lastSource);
  }
}

void BatchProcessDialog::SetLastTarget(wxString target) {
  if (wxDir::Exists(target)) {
    m_lastTarget = target;
    m_targetField->ChangeValue(m_lastTarget);
  } else {
    m_lastTarget = wxEmptyString;
    m_targetField->ChangeValue(m_lastTarget);
  }
}

bool BatchProcessDialog::NeedToRefreshFileList() {
  if (m_mustRefreshMainDir)
    return true;
  else
    return false;
}

void BatchProcessDialog::SetCurrentWorkingDir(wxString str) {
  m_currentWorkingDir = str;
}
