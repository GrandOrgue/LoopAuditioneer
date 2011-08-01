/* 
 * FileHandling.cpp is a part of LoopAuditioneer software
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

#include "FileHandling.h"

FileHandling::FileHandling(wxString fileName, wxString path) : m_loops(NULL), m_cues(NULL), shortAudioData(NULL), intAudioData(NULL), doubleAudioData(NULL) {
  m_loops = new LoopMarkers();
  m_cues = new CueMarkers();
  wxString filePath;
  filePath = path.Append(wxT("/"));
  filePath += fileName;

  // Here we get all the info about the file to be able to later open new file in write mode if changes should be saved
  SndfileHandle sfHandle;

  sfHandle = SndfileHandle(((const char*)filePath.mb_str())); // Open file only for read first to get all info

  m_format = sfHandle.format();
  m_samplerate = sfHandle.samplerate();
  m_channels = sfHandle.channels();
  m_minorFormat = sfHandle.format() & SF_FORMAT_SUBMASK;

  // Try to get loop info from the file
  if (sfHandle.command(4304, &instr, sizeof(instr)) == SF_TRUE) {
    // There are loops!

    m_loops->SetMIDIUnityNote(instr.basenote);
    m_loops->SetMIDIPitchFraction(0);

    // Stort all the loops into m_loops loopsIn vector
    for (int i = 0; i < instr.loop_count; i++) {
      LOOPDATA temp;
      temp.dwType = instr.loops[i].mode;
      temp.dwStart = instr.loops[i].start;
      temp.dwEnd = instr.loops[i].end - 1; // -1 to compensate for libsndfile behaviour
      temp.dwPlayCount = instr.loops[i].count;
      temp.shouldBeSaved = true;
      m_loops->AddLoop(temp);
    }
  }

  // Try to get cue info from file
  if (sfHandle.command(4302, &cues, sizeof(cues)) == SF_TRUE) {
    // There are cues!

    // Check if the cue is a real cue or a label, only keep real cues!
    for (int i = 0; i < cues.cue_count; i++) {
      bool toAdd = true;

      for (int j = 0; j < instr.loop_count; j++) {
        if (cues.cue_points[i].dwSampleOffset == instr.loops[j].start)
          toAdd = false;
      }

      if (toAdd) {
        CUEPOINT tmp;
        tmp.dwName = cues.cue_points[i].dwName;
        tmp.dwPosition = cues.cue_points[i].dwPosition;
        tmp.fccChunk = cues.cue_points[i].fccChunk;
        tmp.dwChunkStart = cues.cue_points[i].dwChunkStart;
        tmp.dwBlockStart = cues.cue_points[i].dwBlockStart;
        tmp.dwSampleOffset = cues.cue_points[i].dwSampleOffset;
        tmp.keepThisCue = true;

        m_cues->AddCue(tmp);
      }
    }
  }

  // Decide what format to store audio data as and copy it
  if ((m_minorFormat == SF_FORMAT_DOUBLE) || (m_minorFormat == SF_FORMAT_FLOAT)) {
    ArrayLength = sfHandle.frames() * sfHandle.channels();
    doubleAudioData = new double[ArrayLength];
    sfHandle.read(doubleAudioData, ArrayLength);
  } else if ((m_minorFormat == SF_FORMAT_PCM_16) || (m_minorFormat == SF_FORMAT_PCM_S8)) {
    ArrayLength = sfHandle.frames() * sfHandle.channels();
    shortAudioData = new short[ArrayLength];
    sfHandle.read(shortAudioData, ArrayLength);
  } else {
    ArrayLength = sfHandle.frames() * sfHandle.channels();
    intAudioData = new int[ArrayLength];
    sfHandle.read(intAudioData, ArrayLength);
  }
}

FileHandling::~FileHandling() {
  delete m_loops;

  delete m_cues;

  delete[] shortAudioData;

  delete[] intAudioData;

  delete[] doubleAudioData;
}

void FileHandling::SaveAudioFile(wxString fileName, wxString path) {
  wxString filePath;
  filePath = path.Append(wxT("/"));
  filePath += fileName;

  // This we open the file write
  sfh = SndfileHandle(((const char*)filePath.mb_str()), SFM_WRITE, m_format, m_channels, m_samplerate);

  // Deal with the loops first
  m_loops->ExportLoops();
  instr.loop_count = m_loops->loopsOut.size();
  for (int i = 0; i < instr.loop_count; i++) {
    instr.loops[i].mode = m_loops->loopsOut[i].dwType;
    instr.loops[i].start = m_loops->loopsOut[i].dwStart;
    instr.loops[i].end = m_loops->loopsOut[i].dwEnd;
    instr.loops[i].count = m_loops->loopsOut[i].dwPlayCount;
  }
  sfh.command(4305, &instr, sizeof(instr)); // this writes the loops metadata

  // Then take care of the cues
  m_cues->ExportCues();
  cues.cue_count = m_cues->exportedCues.size();
  for (int i = 0; i < cues.cue_count; i++) {
    cues.cue_points[i].dwName =  m_cues->exportedCues[i].dwName;
    cues.cue_points[i].dwPosition =  m_cues->exportedCues[i].dwPosition;
    cues.cue_points[i].fccChunk =  m_cues->exportedCues[i].fccChunk;
    cues.cue_points[i].dwChunkStart =  m_cues->exportedCues[i].dwChunkStart;
    cues.cue_points[i].dwBlockStart =  m_cues->exportedCues[i].dwBlockStart;
    cues.cue_points[i].dwSampleOffset =  m_cues->exportedCues[i].dwSampleOffset;
  }
  sfh.command(4303, &cues, sizeof(cues));

  // Finally write the data back
  if ((m_minorFormat == SF_FORMAT_DOUBLE) || (m_minorFormat == SF_FORMAT_FLOAT)) {
    sfh.write(doubleAudioData, ArrayLength);
  } else if ((m_minorFormat == SF_FORMAT_PCM_16) || (m_minorFormat == SF_FORMAT_PCM_S8)) {
    sfh.write(shortAudioData, ArrayLength);
  } else {
    sfh.write(intAudioData, ArrayLength);
  }
  // File will be finally closed when the instance is deleted!
}

int FileHandling::GetSampleRate() {
  return m_samplerate;
}

int FileHandling::GetAudioFormat() {
  return m_minorFormat;
}

