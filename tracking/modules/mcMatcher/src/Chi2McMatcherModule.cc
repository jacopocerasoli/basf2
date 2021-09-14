/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2021 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Florian Schweiger                                        *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <tracking/modules/mcMatcher/Chi2McMatcherModule.h>

/* --------------- WARNING ---------------------------------------------- *
If you have more complex parameter types in your class then simple int,
double or std::vector of those you might need to uncomment the following
include directive to avoid an undefined reference on compilation.
* ---------------------------------------------------------------------- */

// datastore types
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/MCParticle.h>
#include <framework/gearbox/Const.h>
#include <Eigen/Dense>
#include <iostream>

// ROOT
#include <TVectorD.h>
#include <TMatrixD.h>

#include <tracking/mcMatcher/TrackMatchLookUp.h>

using namespace Belle2;

REG_MODULE(Chi2McMatcher)



// Constructor
Chi2McMatcherModule::Chi2McMatcherModule() : Module()
{
  // Set module properties
  setDescription(R"DOC(Monte Carlo matcher using the helix parameters for matching by chi2-method)DOC");

  // set module parameters
  std::vector<double> defaultCutOffs(6);
  defaultCutOffs[0] = 128024;
  defaultCutOffs[1] = 95;
  defaultCutOffs[2] = 173;
  defaultCutOffs[3] = 424;
  defaultCutOffs[4] = 90;
  defaultCutOffs[5] = 424;//in first approximation take proton
  addParam("CutOffs",
           m_param_CutOffs,
           "Defines the Cut Off values for each charged particle. pdg order [11,13,211,2212,321,1000010020]",
           defaultCutOffs);
  addParam("linalg",

           param_linalg,
           "time_measurement_variable",
           std::string("Eigen"));
}

void Chi2McMatcherModule::initialize()
{
// Check if there are MC Particles
  StoreArray<MCParticle> storeMCParticles;
  StoreArray<Track> storeTrack;
  storeMCParticles.isRequired();
  storeTrack.isRequired();
  storeTrack.registerRelationTo(storeMCParticles);

  // Variables to save data
  fileHeader = {"D0_track", "Phi0_track", "Omega_track", "Z0_track", "TanLambda_track",
                "D0_mc_chi2", "Phi0_mc_chi2", "Omega_mc_chi2", "Z0_mc_chi2", "TanLambda_mc_chi2",
                "chi2_value",
                "hitRelation", "chi2Relation", "bothRelation", "bothRelationAndSameMC", "notbothRelation", "noRelation" // (bool)
               };
}

void Chi2McMatcherModule::event()
{

  // suppress the ROOT "matrix is singular" error message
  auto default_gErrorIgnoreLevel = gErrorIgnoreLevel;
  gErrorIgnoreLevel = 5000;

  StoreArray<MCParticle> MCParticles;
  StoreArray<Track> Tracks;

  // get StoreArray length
  int nTracks = Tracks.getEntries();
  int nMCParticles = MCParticles.getEntries();

  // check if there are Tracks and MCParticles to match to
  if (not nMCParticles or not nTracks) {
    // Cannot perfom matching
    return;
  }

  // compare all tracks with all mcParticles in event
  //
  for (int it = 0 ; it < nTracks; ++it) {
    auto track = Tracks[it];
    ++totalCount;
    int hitMatch = 0;
    int chi2Match = 0;
    double track_helix[5];// D0,Phi0,Omega,Z0,TanLambda
    double mc_helix[5];
    double classifiers[6] = {0, 0, 0, 0, 0, 0}; //"hitRelation","chi2Relation","bothRelation","bothRelationAndSameMC","notbothRelation","noRelation"
    auto hitMCParticle = track->getRelated<MCParticle>();
    // test for existing relations
    if (track->getRelated<MCParticle>()) {
      //B2DEBUG(100, "relations = " << track->getRelated<MCParticle>());
      //continue;
      ++hitRelationCounter;
      classifiers[0] = 1; //hitrelation
      hitMatch = 1;
    }
    // initialize minimal chi2 variables
    int ip_min = -1;
    double chi2Min = std::numeric_limits<double>::infinity();

    //int det0_count = 0;

    for (int ip = 0; ip < nMCParticles; ++ip) {
      //int det0 = 0;
      auto mcParticle = MCParticles[ip];
      // Check if current particle is a charged stable particle
      const Const::ParticleType mcParticleType(std::abs(mcParticle->getPDG()));
      if (not Const::chargedStableSet.contains(mcParticleType)) {
        continue;
      }

      // get trackfitresult of current track with clossest mass to current mcparticle Type
      auto trackFitResult = track->getTrackFitResultWithClosestMass(mcParticleType);
      TMatrixD Covariance5 = trackFitResult->getCovariance5();

      // check if matrix is invertable
      double det = Covariance5.Determinant();
      if (det == 0.0) {
        continue;
        /*
              B2DEBUG(1, "=============================================");
              det0 = 1;
              ++det0_count;
              TMatrixD cov(5, 5);
              cov = Covariance5;
              for (int i = 0; i < 5; ++i) {
                cov[2][i] = 0.000000000000001;
                cov[i][2] = 0.000000000000001;
              }
              B2DEBUG(1, "cov.Determinant() = " << cov.Determinant());
              if (cov.Determinant() == 0.0) {
                Covariance5.Print();
                B2DEBUG(1, "Not invertable");
                cov.Print();
                continue;
              }
              Covariance5 = cov;
        */
      }

      // generate helix for current mc particle
      double charge_sign = 1;
      if (mcParticle->getCharge() < 0) { charge_sign = -1;}
      auto b_field = BFieldManager::getField(mcParticle->getVertex()).Z() / Belle2::Unit::T;
      auto mcParticleHelix = Helix(mcParticle->getVertex(), mcParticle->getMomentum(), charge_sign, b_field);

      // initialize chi2Cur
      double chi2Cur = std::numeric_limits<double>::infinity();
      if (param_linalg == "Eigen") {
        // Eigen

        Eigen::VectorXd delta(5);

        delta(0) = trackFitResult->getD0()        - mcParticleHelix.getD0();
        delta(1) = trackFitResult->getPhi0()      - mcParticleHelix.getPhi0();
        delta(2) = trackFitResult->getOmega()     - mcParticleHelix.getOmega();
        delta(3) = trackFitResult->getZ0()        - mcParticleHelix.getZ0();
        delta(4) = trackFitResult->getTanLambda() - mcParticleHelix.getTanLambda();

        Eigen::MatrixXd covariance5Eigen(5, 5);
        for (int i = 0; i < 5; i++) {
          for (int j = 0; j < 5; j++) {
            covariance5Eigen(i, j) = Covariance5[i][j];
          }
        }
        chi2Cur = ((delta.transpose()) * (covariance5Eigen.inverse() * delta))(0, 0);
      } else if (param_linalg == "ROOT") {
        // ROOT

        TMatrixD delta(5, 1);
        delta[0][0] = trackFitResult->getD0()        - mcParticleHelix.getD0();
        delta[1][0] = trackFitResult->getPhi0()      - mcParticleHelix.getPhi0();
        delta[2][0] = trackFitResult->getOmega()     - mcParticleHelix.getOmega();
        delta[3][0] = trackFitResult->getZ0()        - mcParticleHelix.getZ0();
        delta[4][0] = trackFitResult->getTanLambda() - mcParticleHelix.getTanLambda();

        //TMatrixD covariance5_inv(TMatrixD::kInverted, Covariance5);
        Covariance5.InvertFast();

        TMatrixD deltaT = delta;
        deltaT.T();

        chi2Cur = ((deltaT) * (Covariance5 * delta))[0][0];
      }
      if (chi2Min > chi2Cur) {
        chi2Min = chi2Cur;
        ip_min = ip;
        track_helix[0] = trackFitResult->getD0();
        track_helix[1] = trackFitResult->getPhi0();
        track_helix[2] = trackFitResult->getOmega();
        track_helix[3] = trackFitResult->getZ0();
        track_helix[4] = trackFitResult->getTanLambda();
        mc_helix[0] = mcParticleHelix.getD0();
        mc_helix[1] = mcParticleHelix.getPhi0();
        mc_helix[2] = mcParticleHelix.getOmega();
        mc_helix[3] = mcParticleHelix.getZ0();
        mc_helix[4] = mcParticleHelix.getTanLambda();
      }
    }

    if (ip_min == -1) {
      continue;
    }
    // initialize Cut Off
    double cutOff = 0;

    // fill cut off with value
    int mcMinPDG = abs(MCParticles[ip_min]->getPDG());

    if (mcMinPDG == Belle2::Const::electron.getPDGCode()) {
      cutOff = m_param_CutOffs[0];
    } else if (mcMinPDG == Const::muon.getPDGCode()) {
      cutOff = m_param_CutOffs[1];
    } else if (mcMinPDG == Const::pion.getPDGCode()) {
      cutOff = m_param_CutOffs[2];
    } else if (mcMinPDG == Const::proton.getPDGCode()) {
      cutOff = m_param_CutOffs[3];
    } else if (mcMinPDG == Const::kaon.getPDGCode()) {
      cutOff = m_param_CutOffs[4];
    } else if (mcMinPDG == Const::deuteron.getPDGCode()) {
      cutOff = m_param_CutOffs[5];
    } else {
      B2WARNING("The pdg for minimal chi2 was not in chargedstable particle list: MinPDG = " << mcMinPDG);
      continue;
    }
    //TrackMatchLookUp trackMatchLookUp("Track");
    //TrackMatchLookUp::PRToMCMatchInfo prMatchInfo = trackMatchLookUp.getPRToMCMatchInfo(Tracks[it]);
    //B2DEBUG(1,"prMatchInfo = "<<prMatchInfo);
    //TrackMatchLookup trackMatchLookUp("Track");
    //auto matching_status = TrackMatchLookUp::extractPRToMCMatchInfo(Tracks[it]);
    //RecoTrack::MatchingStatus matchingStatus = track->getMatchingStatus();
    //B2DEBUG(1,"MatchingStatus = "<<track::MatchingStatus);
    B2DEBUG(100, "cutoff = " << cutOff);
    if (chi2Min < cutOff) {
      Tracks[it]->addRelationTo(MCParticles[ip_min]);
      ++chi2RelationCounter;
      chi2Match = 1;
      classifiers[1] = 1;
    }

    if ((chi2Match == 1) and (hitMatch == 1)) {
      ++bothRelationCounter;
      classifiers[2] = 1;
      if (hitMCParticle == MCParticles[ip_min]) {
        ++bothRelationAndSameMCCounter;
        classifiers[3] = 1;
      }
    } else if ((chi2Match == 1) or (hitMatch == 1)) {
      ++notBothRelationCounter;
      classifiers[4] = 1;
    } else {
      ++noRelationCounter;
      classifiers[5] = 1;
    }




    for (int i = 0; i < 5; i++) {
      fileContent.push_back(track_helix[i]);
    }
    for (int i = 0; i < 5; i++) {
      fileContent.push_back(mc_helix[i]);
    }
    fileContent.push_back(chi2Min);
    for (int i = 0; i < 5; i++) {
      fileContent.push_back(classifiers[i]);
    }
    /*
    B2DEBUG(1,"totalCount = "<<totalCount);
    B2DEBUG(1,"hitRelationCounter = "<<hitRelationCounter);
    B2DEBUG(1,"chi2RelationCounter = "<<chi2RelationCounter);
    B2DEBUG(1,"bothRelationCounter = "<<bothRelationCounter);
    B2DEBUG(1,"bothRelationAndSameMCCounter = "<<bothRelationAndSameMCCounter);
    B2DEBUG(1,"notbothRelationCounter = "<<notBothRelationCounter);
    B2DEBUG(1,"noRelationCounter = "<<noRelationCounter);
    */
  }
  // reset ROOT Error Level to default
  gErrorIgnoreLevel = default_gErrorIgnoreLevel;
}



void Chi2McMatcherModule::terminate()
{
  std::ofstream outfile;
  outfile.open("hitmatch_vs_chi2match.csv");
  if (!outfile) {  // file couldn't be opened
    std::cerr << "Error: file could not be opened" << std::endl;
    exit(1);
  }
  for (int i = 0; i < fileHeader.size(); i++) {
    outfile << fileHeader[i];
    if (i != (fileHeader.size() - 1)) {
      outfile << ",";
    }
  }
  outfile << std::endl;
  int count = 0;
  for (int index = 0; index < fileContent.size(); index++) {
    outfile << fileContent[index];
    count++;
    if (count == 16) {
      count = 0;
      outfile << std::endl;
      continue;
    }
    outfile << ",";
  }
  outfile.close();
}
