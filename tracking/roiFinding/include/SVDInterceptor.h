/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <tracking/roiFinding/ROIGeometry.h>

namespace Belle2 {
  class SVDIntercept;
  class RecoTrack;

  /** This Class implements the interceptor of the SVD tracks on the PXD layers.
   */
  class SVDInterceptor {

  public:

    /** Constructor */
    SVDInterceptor(double toleranceZ, double tolerancePhi);

    /** Destructor.
     */
    virtual ~SVDInterceptor();

    /** Fill the list of PXD intecepts corresponding to the list of track candidates.
     */
    void fillInterceptList(StoreArray<SVDIntercept>* listToBeFilled,
                           const StoreArray<RecoTrack>& trackList,
                           RelationArray* recoTrackToSVDIntercepts);


  private:

    const float m_svdLayerRadius[4] = {3.8, 8.0, 11.5, 14.0}; /**< mean SVD layer radius for both layers */

    ROIGeometry m_theROIGeometry; /**< the geometry of the Region Of Interest */

    /** Append the SVDIntercept infos related to the track theTrack to the listToBeFilled.
     */
    void appendIntercepts(StoreArray<SVDIntercept>* interceptList, std::list<ROIDetPlane> planeList, RecoTrack* recoTrack,
                          int recoTrackIndex, RelationArray* recoTrackToSVDIntercepts);

  };
}
