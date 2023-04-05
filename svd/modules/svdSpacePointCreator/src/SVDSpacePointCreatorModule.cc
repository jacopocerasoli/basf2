/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#include <svd/modules/svdSpacePointCreator/SVDSpacePointCreatorModule.h>
#include <svd/modules/svdSpacePointCreator/SpacePointHelperFunctions.h>

#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>

#include <svd/dataobjects/SVDEventInfo.h>

using namespace std;
using namespace Belle2;


REG_MODULE(SVDSpacePointCreator);

SVDSpacePointCreatorModule::SVDSpacePointCreatorModule() :
  Module()
{
  InitializeCounters();

  setDescription("Imports Clusters of the SVD detector and converts them to spacePoints.");
  setPropertyFlags(c_ParallelProcessingCertified);

  // 1. Collections.
  addParam("SVDClusters", m_svdClustersName,
           "SVDCluster collection name", string(""));
  addParam("SpacePoints", m_spacePointsName,
           "SpacePoints collection name", string("SVDSpacePoints"));
  addParam("EventLevelTrackingInfoName", m_eventLevelTrackingInfoName,
           "EventLevelTrackingInfo collection name", string(""));
  addParam("EventInfo", m_svdEventInfoName,
           "SVDEventInfo collection name.", string("SVDEventInfo"));

  // 2.Modification parameters:
  addParam("NameOfInstance", m_nameOfInstance,
           "allows the user to set an identifier for this module. Usefull if one wants to use several instances of that module", string(""));
  addParam("OnlySingleClusterSpacePoints", m_onlySingleClusterSpacePoints,
           "standard is false. If activated, the module will not try to find combinations of U and V clusters for the SVD any more",
           bool(false));

  addParam("MinClusterTime", m_minClusterTime, "clusters with time below this value are not considered to make spacePoints.",
           float(-20));
  addParam("inputPDF", m_inputPDF,
           "Path containing pdf root file", std::string("/data/svd/spacePointQICalibration.root"));
  addParam("useQualityEstimator", m_useQualityEstimator,
           "Standard is true. If turned off spacepoints will not be assigned a quality in their pairing.", bool(false));

  addParam("useLegacyNaming", m_useLegacyNaming,
           "Use old PDF name convention?", bool(true));

  addParam("numMaxSpacePoints", m_numMaxSpacePoints,
           "Maximum number of SpacePoints allowed in an event, above this threshold no SpacePoint will be created",
           unsigned(m_numMaxSpacePoints));

  addParam("useSVDGroupInfo", m_useSVDGroupInfo,
           "Use SVD group info to reject combinations from clusters belonging to different groups",
           bool(true));
  addParam("useSVDGroupInfoIn6Sample", m_useSVDGroupInfoIn6Sample,
           "Use SVD group info to reject combinations from clusters belonging to different groups in 6-sample DAQ mode",
           bool(true));
  addParam("useSVDGroupInfoIn3Sample", m_useSVDGroupInfoIn3Sample,
           "Use SVD group info to reject combinations from clusters belonging to different groups in 3-sample DAQ mode",
           bool(true));

  addParam("useDB", m_useDB, "if False, use configuration module parameters", bool(true));

}



void SVDSpacePointCreatorModule::beginRun()
{
  if (m_useDB) {
    if (!m_recoConfig.isValid())
      B2FATAL("no valid configuration found for SVD reconstruction");
    else
      B2DEBUG(20, "SVDRecoConfiguration: from now on we are using " << m_recoConfig->get_uniqueID());

    m_useSVDGroupInfoIn6Sample = m_recoConfig->getUseOfSVDGroupInfoInSPCreator(6);
    m_useSVDGroupInfoIn3Sample = m_recoConfig->getUseOfSVDGroupInfoInSPCreator(3);
  }

  if (m_useSVDGroupInfo) {
    if (m_useSVDGroupInfoIn6Sample)
      B2INFO("SVDSpacePointCreator : SVDCluster groupId is used for 6-sample DAQ mode.");
    else
      B2INFO("SVDSpacePointCreator : SVDCluster groupId is not used for 6-sample DAQ mode.");
    if (m_useSVDGroupInfoIn3Sample)
      B2INFO("SVDSpacePointCreator : SVDCluster groupId is used for 3-sample DAQ mode.");
    else
      B2INFO("SVDSpacePointCreator : SVDCluster groupId is not used for 3-sample DAQ mode.");
  } else
    B2INFO("SVDSpacePointCreator : SVDCluster groupId is not used while forming cluster combinations.");
}


void SVDSpacePointCreatorModule::initialize()
{
  // prepare all store- and relationArrays:
  m_spacePoints.registerInDataStore(m_spacePointsName, DataStore::c_DontWriteOut | DataStore::c_ErrorIfAlreadyRegistered);
  m_svdClusters.isRequired(m_svdClustersName);


  //Relations to cluster objects only if the ancestor relations exist:
  m_spacePoints.registerRelationTo(m_svdClusters, DataStore::c_Event, DataStore::c_DontWriteOut);

  B2DEBUG(20, "SVDSpacePointCreatorModule(" << m_nameOfInstance << ")::initialize: names set for containers:\n" <<
          "\nsvdClusters: " << m_svdClusters.getName() <<
          "\nspacePoints: " << m_spacePoints.getName());

  if (m_useQualityEstimator == true) {
    if (m_inputPDF.empty()) {
      B2ERROR("Input PDF filename not set");
    } else {
      std::string fullPath = FileSystem::findFile(m_inputPDF);
      if (fullPath.empty()) {
        B2ERROR("PDF file:" << m_inputPDF << "not located! Check filename input matches name of PDF file!");
      }
      m_inputPDF = fullPath;
    }

    m_calibrationFile = new TFile(m_inputPDF.c_str(), "READ");
    if (!m_calibrationFile->IsOpen())
      B2FATAL("Couldn't open pdf file:" << m_inputPDF);
  }

  // set some counters for output:
  InitializeCounters();
}



void SVDSpacePointCreatorModule::event()
{


  bool useSVDGroupInfo = m_useSVDGroupInfo;
  if (useSVDGroupInfo) {
    // first take Event Informations:
    StoreObjPtr<SVDEventInfo> temp_eventinfo(m_svdEventInfoName);
    if (!temp_eventinfo.isValid())
      m_svdEventInfoName = "SVDEventInfoSim";
    StoreObjPtr<SVDEventInfo> eventinfo(m_svdEventInfoName);
    if (!eventinfo) B2ERROR("No SVDEventInfo!");
    int numberOfAcquiredSamples = eventinfo->getNSamples();
    // then use the respective parameters
    if (numberOfAcquiredSamples == 6)
      useSVDGroupInfo = m_useSVDGroupInfoIn6Sample;
    else if (numberOfAcquiredSamples == 3)
      useSVDGroupInfo = m_useSVDGroupInfoIn3Sample;
  }

  if (m_onlySingleClusterSpacePoints == true) {
    provideSVDClusterSingles(m_svdClusters,
                             m_spacePoints); /// WARNING TODO: missing: possibility to allow storing of u- or v-type clusters only!
  } else {
    provideSVDClusterCombinations(m_svdClusters, m_spacePoints, m_HitTimeCut, m_useQualityEstimator, m_calibrationFile,
                                  m_useLegacyNaming, m_numMaxSpacePoints, m_eventLevelTrackingInfoName, useSVDGroupInfo);
  }


  B2DEBUG(21, "SVDSpacePointCreatorModule(" << m_nameOfInstance <<
          ")::event: spacePoints for single SVDClusters created! Size of arrays:\n" <<
          ", svdClusters: " << m_svdClusters.getEntries() <<
          ", spacePoints: " << m_spacePoints.getEntries());


  if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 10, PACKAGENAME()) == true) {
    for (int index = 0; index < m_spacePoints.getEntries(); index++) {
      const SpacePoint* sp = m_spacePoints[index];

      B2DEBUG(29, "SVDSpacePointCreatorModule(" << m_nameOfInstance << ")::event: spacePoint " << index <<
              " with type " << sp->getType() <<
              " and VxdID " << VxdID(sp->getVxdID()) <<
              " is tied to a cluster in: " << sp->getArrayName());
    }
  }

  m_TESTERSVDClusterCtr += m_svdClusters.getEntries();
  m_TESTERSpacePointCtr += m_spacePoints.getEntries();

}



void SVDSpacePointCreatorModule::terminate()
{
  B2DEBUG(20, "SVDSpacePointCreatorModule(" << m_nameOfInstance << ")::terminate: total number of occured instances:\n" <<
          ", svdClusters: " << m_TESTERSVDClusterCtr <<
          ", spacePoints: " << m_TESTERSpacePointCtr);
  if (m_useQualityEstimator == true) {
    m_calibrationFile->Delete();
  }
}


void SVDSpacePointCreatorModule::InitializeCounters()
{
  m_TESTERSVDClusterCtr = 0;
  m_TESTERSpacePointCtr = 0;
}
