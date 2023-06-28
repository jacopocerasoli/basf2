/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <ecl/calibration/eclAutocovarianceCalibrationC2Algorithm.h>

/* ECL headers. */
#include <ecl/dataobjects/ECLElementNumbers.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>

/* ROOT headers. */
#include <TFile.h>
#include <TGraph.h>
#include <TH1D.h>
#include <TDirectory.h>

using namespace Belle2;
using namespace ECL;

/**-----------------------------------------------------------------------------------------------*/
eclAutocovarianceCalibrationC2Algorithm::eclAutocovarianceCalibrationC2Algorithm():
  CalibrationAlgorithm("eclAutocovarianceCalibrationC2Collector")
{
  setDescription(
    "Determine baseline for waveforms to be used in computing the covariance matrix"
  );
}

CalibrationAlgorithm::EResult eclAutocovarianceCalibrationC2Algorithm::calibrate()
{
  ///**-----------------------------------------------------------------------------------------------*/
  ///** Histograms containing the data collected by eclAutocovarianceCalibrationC2CollectorModule */
  auto m_BaselineVsCrysID = getObjectPtr<TH1D>("BaselineVsCrysID");
  auto m_CounterVsCrysID = getObjectPtr<TH1D>("CounterVsCrysID");

  std::vector<float> cryIDs;
  std::vector<float> baselines;
  std::vector<float> baselinesError;

  for (int crysID = 0; crysID < ECLElementNumbers::c_NCrystals; crysID++) {

    double totalCounts = m_CounterVsCrysID->GetBinContent(m_CounterVsCrysID->GetBin(crysID + 1));

    if (totalCounts < m_TotalCountsThreshold) {
      B2INFO("eclAutocovarianceCalibrationC2Algorithm: warning total entries for cell ID " << crysID + 1 << " is only: " << totalCounts <<
             " Requirement is m_TotalCountsThreshold: " << m_TotalCountsThreshold);
      /** We require all crystals to have a minimum number of waveforms available.  If c_NotEnoughData is returned then the next run will be appended.  */
      return c_NotEnoughData;
    }

    double baseline = m_BaselineVsCrysID->GetBinContent(m_BaselineVsCrysID->GetBin(crysID + 1));;
    baseline /= float(m_numberofADCPoints);
    baseline /= totalCounts;

    B2INFO("crysID " << crysID << " baseline: " << baseline);

    cryIDs.push_back(crysID);
    baselines.push_back(baseline);
    baselinesError.push_back(0);

    B2INFO("eclAutocovarianceCalibrationC2Algorithm crysID baseline totalCounts  " << crysID << " " << baseline << " " << totalCounts);

  }

  auto gBaselineVsCrysID = new TGraph(cryIDs.size(), cryIDs.data(), baselines.data());
  gBaselineVsCrysID->SetName("gBaselineVsCrysID");

  /** Write out the baseline results */
  TString fName = m_outputName;
  TDirectory::TContext context;
  TFile* histfile = new TFile(fName, "recreate");
  histfile->cd();
  m_BaselineVsCrysID->Write();
  gBaselineVsCrysID->Write();
  histfile->Close();

  /** Saving baseline results to db for access in stage C3 */
  ECLCrystalCalib* PPThreshold = new ECLCrystalCalib();
  PPThreshold->setCalibVector(baselines, baselinesError);
  saveCalibration(PPThreshold, "ECLAutocovarianceCalibrationC2Baseline");

  return c_OK;
}
