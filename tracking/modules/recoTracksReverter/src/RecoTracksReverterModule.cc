/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/recoTracksReverter/RecoTracksReverterModule.h>

using namespace Belle2;

REG_MODULE(RecoTracksReverter);

RecoTracksReverterModule::RecoTracksReverterModule() :
  Module()
{
  setDescription("Revert the RecoTracks (without their fit information)");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("inputStoreArrayName", m_inputStoreArrayName,
           "Name of the input StoreArray");
  addParam("outputStoreArrayName", m_outputStoreArrayName,
           "Name of the output StoreArray");
  addParam("mvaFlipCut", m_mvaFlipCut,
           "mva Flip cut",  m_mvaFlipCut);
}

void RecoTracksReverterModule::initialize()
{
  m_inputRecoTracks.isRequired(m_inputStoreArrayName);

  m_outputRecoTracks.registerInDataStore(m_outputStoreArrayName, DataStore::c_ErrorIfAlreadyRegistered);
  RecoTrack::registerRequiredRelations(m_outputRecoTracks);

  m_outputRecoTracks.registerRelationTo(m_inputRecoTracks);

}

void RecoTracksReverterModule::event()
{
  for (const RecoTrack& recoTrack : m_inputRecoTracks) {

    if (not recoTrack.wasFitSuccessful()) {
      continue;
    }
    if (recoTrack.getFlipQualityIndicator() > m_mvaFlipCut) {
      Track* b2track = recoTrack.getRelatedFrom<Belle2::Track>();
      if (b2track) {

        const auto& measuredStateOnPlane = recoTrack.getMeasuredStateOnPlaneFromLastHit();
        TVector3 currentPosition = measuredStateOnPlane.getPos();
        TVector3 currentMomentum = measuredStateOnPlane.getMom();
        double currentCharge = measuredStateOnPlane.getCharge();

        RecoTrack* newRecoTrack = m_outputRecoTracks.appendNew(currentPosition, -currentMomentum, -currentCharge,
                                                               recoTrack.getStoreArrayNameOfCDCHits(), recoTrack.getStoreArrayNameOfSVDHits(), recoTrack.getStoreArrayNameOfPXDHits(),
                                                               recoTrack.getStoreArrayNameOfBKLMHits(), recoTrack.getStoreArrayNameOfEKLMHits(),
                                                               recoTrack.getStoreArrayNameOfRecoHitInformation());
        newRecoTrack->addHitsFromRecoTrack(&recoTrack, newRecoTrack->getNumberOfTotalHits(), true);
        newRecoTrack->addRelationTo(&recoTrack);
      }
    }

  }
}

