/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

//Calibration
#include <calibration/CalibrationCollectorModule.h>

//Framework
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>

#include <ecl/dbobjects/ECLCrystalCalib.h>

//Root
#include <TH2F.h>

namespace Belle2 {

  class ECLDigit;
  class ECLDsp;

  /** Calibration collector module that uses delayed Bhabha to compute coveriance matrix */
  class eclWaveformTemplateCalibrationC1CollectorModule : public CalibrationCollectorModule {

  public:

    /** Constructor.
     */
    eclWaveformTemplateCalibrationC1CollectorModule();

    /** Define histograms and read payloads from DB */
    void prepare() override;

    /** Select events and crystals and accumulate histograms */
    void collect() override;

    void closeRun() override;

  private:

    StoreArray<ECLDigit> m_eclDigits; /**< Required input array of ECLDigits */
    StoreArray<ECLDsp> m_eclDsps; /**< Required input array of ECLDSPs */
    StoreObjPtr<EventMetaData> m_evtMetaData; /**< dataStore EventMetaData */

    int m_baselineLimit; /**< Number of ADC points used to define baseline. */

    TH2F* varXvsCrysID; /**< Histogram to store collector output. */

    std::vector<float> m_ADCtoEnergy; /**< Crystal calibration constants. */

    double m_MinEnergyThreshold; /**< Minimum energy threshold of online fit result for Fitting Waveforms */
    double m_MaxEnergyThreshold; /**< Maximum energy threshold of online fit result for Fitting Waveforms */

    /** Crystal electronics. */
    DBObjPtr<ECLCrystalCalib> m_CrystalElectronics{"ECLCrystalElectronics"};

    /** Crystal energy. */
    DBObjPtr<ECLCrystalCalib> m_CrystalEnergy{"ECLCrystalEnergy"};

  };
} // end Belle2 namespace
