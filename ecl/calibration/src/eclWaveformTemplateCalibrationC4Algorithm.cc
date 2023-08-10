/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <ecl/calibration/eclWaveformTemplateCalibrationC4Algorithm.h>

/* ECL headers. */
#include <ecl/dataobjects/ECLElementNumbers.h>
#include <ecl/dbobjects/ECLDigitWaveformParameters.h>

/* ROOT headers. */
#include <TFile.h>
#include <TGraph.h>
#include <TTree.h>
#include <TF1.h>

#include <stdlib.h>

using namespace std;
using namespace Belle2;
using namespace ECL;
using namespace Calibration;

/**-----------------------------------------------------------------------------------------------*/
eclWaveformTemplateCalibrationC4Algorithm::eclWaveformTemplateCalibrationC4Algorithm():
  CalibrationAlgorithm("DummyCollector")
{
  setDescription(
    "Collects results from C3 to produce final payload, which contains new waveform templates"
  );
}

CalibrationAlgorithm::EResult eclWaveformTemplateCalibrationC4Algorithm::calibrate()
{

  //save to db
  ECLDigitWaveformParameters* PhotonHadronDiodeParameters = new ECLDigitWaveformParameters();

  std::vector<float> cellIDs;
  std::vector<float> photonNorms;
  std::vector<float> hadronNorms;
  std::vector<float> diodeNorms;

  B2INFO("eclWaveformTemplateCalibrationC4Algorithm m_firstCellID, m_lastCellID " << m_firstCellID << " " << m_lastCellID);

  DBObjPtr<ECLDigitWaveformParameters> tempexistingPhotonWaveformParameters(Form("PhotonParameters_CellID%d_CellID%d", m_firstCellID,
      m_lastCellID));
  DBObjPtr<ECLDigitWaveformParameters> tempexistingHadronDiodeWaveformParameters(Form("HadronDiodeParameters_CellID%d_CellID%d",
      m_firstCellID, m_lastCellID));

  //------------------------------------------------------------------------
  // Get the input run list (should be only 1) for us to use to update the DBObjectPtrs
  auto runs = getRunList();
  // Take the first run
  ExpRun chosenRun = runs.front();
  B2INFO("merging using the ExpRun (" << chosenRun.second << "," << chosenRun.first << ")");
  // After here your DBObjPtrs are correct
  updateDBObjPtrs(1, chosenRun.second, chosenRun.first);
  int experimentNumber = chosenRun.first;

  for (int j = m_firstCellID; j <= m_lastCellID; j++) {
    B2INFO("Check Norm Parms CellID " << j);
    B2INFO("P " << j << " " << tempexistingPhotonWaveformParameters->getPhotonParameters(j)[0]);
    B2INFO("H " << j << " " << tempexistingHadronDiodeWaveformParameters->getHadronParameters(j)[0]);
    B2INFO("D " << j << " " << tempexistingHadronDiodeWaveformParameters->getDiodeParameters(j)[0]);

    cellIDs.push_back(j);
    photonNorms.push_back(tempexistingPhotonWaveformParameters->getPhotonParameters(j)[0]);
    hadronNorms.push_back(tempexistingPhotonWaveformParameters->getHadronParameters(j)[0]);
    diodeNorms.push_back(tempexistingPhotonWaveformParameters->getDiodeParameters(j)[0]);

    float tempPhotonWaveformParameters[11];
    float tempHadronWaveformParameters[11];
    float tempDiodeWaveformParameters[11];

    for (int k = 0; k < 11; k++) {
      tempPhotonWaveformParameters[k] = tempexistingPhotonWaveformParameters->getPhotonParameters(j)[k];
      tempHadronWaveformParameters[k] = tempexistingHadronDiodeWaveformParameters->getHadronParameters(j)[k];
      tempDiodeWaveformParameters[k] = tempexistingHadronDiodeWaveformParameters->getDiodeParameters(j)[k];
    }
    PhotonHadronDiodeParameters->setTemplateParameters(j, tempPhotonWaveformParameters, tempHadronWaveformParameters,
                                                       tempDiodeWaveformParameters);
  }

  auto gphotonNorms = new TGraph(cellIDs.size(), cellIDs.data(), photonNorms.data());
  gphotonNorms->SetName("gphotonNorms");
  auto ghadronNorms = new TGraph(cellIDs.size(), cellIDs.data(), hadronNorms.data());
  ghadronNorms->SetName("ghadronNorms");
  auto gdiodeNorms = new TGraph(cellIDs.size(), cellIDs.data(), diodeNorms.data());
  gdiodeNorms->SetName("gdiodeNorms");

  TString fName = m_outputName;
  TFile* histfile = new TFile(fName, "recreate");
  histfile->cd();
  gphotonNorms->Write();
  ghadronNorms->Write();
  gdiodeNorms->Write();
  histfile->Close();
  delete histfile;


  B2INFO("eclWaveformTemplateCalibrationC4Algorithm: Successful, now writing DB PAyload, chosenRun.second = " << experimentNumber);
  saveCalibration(PhotonHadronDiodeParameters, "ECLDigitWaveformParameters", IntervalOfValidity(experimentNumber, -1,
                  experimentNumber, -1));

  return c_OK;
}

