/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>

#include <framework/datastore/StoreArray.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <mdst/dataobjects/Track.h>

/**
 * Very simp.
 */
namespace Belle2 {
  /// Module to copy RecoTracks.
  class FlippedRecoTracksMergerModule : public Module {

  public:
    /// Constructor of the module. Setting up parameters and description.
    FlippedRecoTracksMergerModule();

    /// Declare required StoreArray
    void initialize() override;

    /// Event processing, copies store array
    void event() override;

  private:
    /// Name of the input StoreArray
    std::string m_inputStoreArrayName;

    /// Store Array of the input tracks
    StoreArray<RecoTrack> m_inputRecoTracks;
    /// Store Array of the input tracks (for relations)
    //StoreArray<Track> m_tracks;

    /// Parameter: the 2nd mva cut
    float m_2nd_mva_cut = 0.9;
  };
}

