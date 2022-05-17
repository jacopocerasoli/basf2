/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/modules/EventMerging/FixMergedObjectsModule.h>

#include <framework/gearbox/Const.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register module
//-----------------------------------------------------------------

REG_MODULE(FixMergedObjects);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

FixMergedObjectsModule::FixMergedObjectsModule() : Module()
{
  setDescription("Fix indices of mdst objects (Tracks, V0s, MCParticles) after DataStores were merged using an independent path.");

  setPropertyFlags(c_ParallelProcessingCertified);
}

void FixMergedObjectsModule::initialize()
{
  m_mergedArrayIndices.isRequired("MergedArrayIndices");
  m_tracks.isOptional();
  m_v0s.isOptional();
  m_mcParticles.isOptional();
}

void FixMergedObjectsModule::event()
{
  // This is quite easy, it is all just constant offsets (corresponding to length of StoreArray before Merge)

  if (m_tracks.isValid() && m_mergedArrayIndices->hasExtraInfo("Tracks")) {
    for (int t_idx = m_mergedArrayIndices->getExtraInfo("Tracks"); t_idx < m_tracks.getEntries(); t_idx++) {
      for (unsigned int i = 0; i < Const::ChargedStable::c_SetSize; i++) {
        // hypothesis not fitted
        if (m_tracks[t_idx]->m_trackFitIndices[i] == -1) {
          continue;
        }
        // declared friends class, so this can be done
        m_tracks[t_idx]->m_trackFitIndices[i] += m_mergedArrayIndices->getExtraInfo("TrackFitResults");
      }
    }
  }

  if (m_v0s.isValid() && m_mergedArrayIndices->hasExtraInfo("V0s")) {
    for (int v_idx = m_mergedArrayIndices->getExtraInfo("V0s"); v_idx < m_v0s.getEntries(); v_idx++) {
      // declared friends class, so this can be done
      m_v0s[v_idx]->m_trackIndexPositive += m_mergedArrayIndices->getExtraInfo("Tracks");
      m_v0s[v_idx]->m_trackIndexNegative += m_mergedArrayIndices->getExtraInfo("Tracks");
      m_v0s[v_idx]->m_trackFitResultIndexPositive += m_mergedArrayIndices->getExtraInfo("TrackFitResults");
      m_v0s[v_idx]->m_trackFitResultIndexNegative += m_mergedArrayIndices->getExtraInfo("TrackFitResults");
    }
  }

  if (m_mcParticles.isValid() && m_mergedArrayIndices->hasExtraInfo("MCParticles")) {
    for (int p_idx = m_mergedArrayIndices->getExtraInfo("MCParticles"); p_idx < m_mcParticles.getEntries(); p_idx++) {
      // declared friends class, so this can be done
      m_mcParticles[p_idx]->m_index += m_mergedArrayIndices->getExtraInfo("MCParticles");
      m_mcParticles[p_idx]->m_mother += m_mergedArrayIndices->getExtraInfo("MCParticles");
      m_mcParticles[p_idx]->m_firstDaughter += m_mergedArrayIndices->getExtraInfo("MCParticles");
      m_mcParticles[p_idx]->m_lastDaughter += m_mergedArrayIndices->getExtraInfo("MCParticles");
    }
  }
}

