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


//CalibrationAlgorithm("DummyCollector")
/**-----------------------------------------------------------------------------------------------*/
eclWaveformTemplateCalibrationC4Algorithm::eclWaveformTemplateCalibrationC4Algorithm():
  CalibrationAlgorithm("eclWaveformTemplateCalibrationC2Collector")
{
  setDescription(
    "Perform energy calibration of ecl crystals by fitting a Novosibirsk function to energy deposited by photons in e+e- --> gamma gamma"
  );
}

CalibrationAlgorithm::EResult eclWaveformTemplateCalibrationC4Algorithm::calibrate()
{

  //save to db
  ECLDigitWaveformParameters* PhotonHadronDiodeParameters = new ECLDigitWaveformParameters();

  std::vector<bool> checkID(8736, false);

  int batchsize = 100;

  int experimentNumber = -999;

  for (int i = 0; i < 88; i++) {
    //for (int i = 0; i < 2; i++) {

    int firstCellID = (i * batchsize) + 1;
    int lastCellID = ((i + 1) * batchsize);
    if (lastCellID > 8736) lastCellID = 8736;

    B2INFO("eclWaveformTemplateCalibrationC4Algorithm m_firstCellID, m_lastCellID " << m_firstCellID << " " << m_lastCellID);

    DBObjPtr<ECLDigitWaveformParameters> tempexistingPhotonWaveformParameters(Form("PhotonParameters_CellID%d_CellID%d", firstCellID,
        lastCellID));
    DBObjPtr<ECLDigitWaveformParameters> tempexistingHadronDiodeWaveformParameters(Form("HadronDiodeParameters_CellID%d_CellID%d",
        firstCellID, lastCellID));

    //------------------------------------------------------------------------
    // Get the input run list (should be only 1) for us to use to update the DBObjectPtrs
    auto runs = getRunList();
    // Take the first run
    ExpRun chosenRun = runs.front();
    B2INFO("merging using the ExpRun (" << chosenRun.second << "," << chosenRun.first << ")");
    // After here your DBObjPtrs are correct
    updateDBObjPtrs(1, chosenRun.second, chosenRun.first);
    experimentNumber = chosenRun.first;
    //updateDBObjPtrs(1, 0,20);

    for (int j = firstCellID; j <= lastCellID; j++) {
      B2INFO("Check Norm Parms CellID " << j);
      B2INFO("P " << j << " " << tempexistingPhotonWaveformParameters->getPhotonParameters(j)[0]);
      B2INFO("H " << j << " " << tempexistingHadronDiodeWaveformParameters->getHadronParameters(j)[0]);
      B2INFO("D " << j << " " << tempexistingHadronDiodeWaveformParameters->getDiodeParameters(j)[0]);
      float tempPhotonWaveformParameters[11];
      float tempHadronWaveformParameters[11];
      float tempDiodeWaveformParameters[11];

      for (int k = 0; k < 11; k++) {
        tempPhotonWaveformParameters[k] = tempexistingPhotonWaveformParameters->getPhotonParameters(j)[k];
        tempHadronWaveformParameters[k] = tempexistingHadronDiodeWaveformParameters->getHadronParameters(j)[k];
        tempDiodeWaveformParameters[k] = tempexistingHadronDiodeWaveformParameters->getDiodeParameters(j)[k];
      }
      checkID[j - 1] = true;
      PhotonHadronDiodeParameters->setTemplateParameters(j, tempPhotonWaveformParameters, tempHadronWaveformParameters,
                                                         tempDiodeWaveformParameters);
    }

  }

  bool Pass = true;
  for (int k = 0; k < checkID.size(); k++) {

    if (checkID[k] == false) {
      B2INFO("eclWaveformTemplateCalibrationC4Algorithm:  ERROR checkID false at entry " << k);
      Pass = false;
    }

  }

  if (Pass == true) {

    B2INFO("eclWaveformTemplateCalibrationC4Algorithm: Successful, now writing DB PAyload, chosenRun.second = " << experimentNumber);
    saveCalibration(PhotonHadronDiodeParameters, "ECLDigitWaveformParameters", IntervalOfValidity(experimentNumber, -1,
                    experimentNumber, -1));
  }

  return c_OK;
}

