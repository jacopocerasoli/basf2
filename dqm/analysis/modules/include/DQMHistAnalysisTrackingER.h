/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <dqm/analysis/modules/DQMHistAnalysis.h>

#include <TCanvas.h>

namespace Belle2 {

  /** Analysis of ER Tracking DQM plots */
  class DQMHistAnalysisTrackingERModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    /** Constructor */
    DQMHistAnalysisTrackingERModule();
    /**Destructor */
    ~DQMHistAnalysisTrackingERModule() {};

    /** Module function initialize */
    void initialize() override;
    /** Module function event */
    void event() override;

  private:

    float m_onTimeHalfWidth = 50; /**< [ns], cluster is on time if within ± onTimeHalfWidth */

    /** Monitoring Object to be produced by this module, which contain defined canvases and monitoring variables */
    MonitoringObject* m_monObj = nullptr;

  };
} // end namespace Belle2

