#!/usr/bin/env python3
# -*- coding: utf-8 -*-


import basf2 as b2
import modularAnalysis as ma
from variables import variables as vm
import stdPhotons

from ROOT import Belle2

# Necessary to run argparse
from ROOT import PyConfig

PyConfig.IgnoreCommandLineOptions = True  # noqa

import random
import argparse
from pathlib import Path

from grafei import GraFEIModule
from grafei import FlagBDecayModule
from grafei.modules.IsMostLikelyTempVarsModule import IsMostLikelyTempVars


def _parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-c", "--config", type=str, default=None, help="graFEI config file"
    )
    parser.add_argument(
        "-w", "--weight", type=str, default=None, help="graFEI weight file"
    )
    parser.add_argument(
        "-g",
        "--globaltag",
        type=str,
        default="graFEI_mixed_LCAMassProb_v1",
        help="Globaltag containing graFEI model",
    )
    parser.add_argument(
        "-l",
        "--lcas",
        type=list,
        default=[[0]],
        help="Choose LCAS matrix of signal side",
    )
    parser.add_argument(
        "-m",
        "--masses",
        type=list,
        default=["k"],
        help="Choose mass hypotheses of signal side",
    )
    return parser.parse_args()


if __name__ == "__main__":
    # Random seeds
    b2.set_random_seed(42)
    random.seed(42)

    args = _parse_args()

    store_mc_truth = True

    b2.conditions.prepend_globaltag(args.globaltag)
    b2.conditions.prepend_globaltag(ma.getAnalysisGlobaltag())

    input_file = Path(Belle2.Environment.Instance().getInputFilesOverride()[0])

    path = b2.create_path()
    ma.inputMdst(str(input_file), path=path)

    #################################################################################################
    # GraFEI requirements and reconstruction
    # 1) Cuts on charged tracks and photons
    # 2) Cut on isMostLikely with _noSVD(_noTOP) variables and custom priors
    # 3) Keep only "good events" i.e. validTree + signal side matches chosen LCAS and mass hypotheses
    #################################################################################################

    # 1) Cuts on charged tracks and photons
    charged_cuts = [
        "nCDCHits>20",
        "thetaInCDCAcceptance",
        "abs(dz)<1.0",
        "dr<0.5",
        "p<5",
        "pt>0.2",
    ]

    photon_cuts = [
        "beamBackgroundSuppression>0.4",
        "fakePhotonSuppression>0.3",
        "abs(clusterTiming)<100",
        "abs(formula(clusterTiming/clusterErrorTiming))<2.0",
        "[[clusterReg==1 and E>0.09] or [clusterReg==2 and E>0.09] or [clusterReg==3 and E>0.14]]",
    ]

    evt_cut = "n_gamma_in_evt<20 and n_charged_in_evt<20"

    # Fill particle list with optimized cuts
    charged_lists = [f"{c}:final" for c in ["p+", "e+", "pi+", "mu+", "K+"]]

    ma.fillParticleLists(
        [(c, " and ".join(charged_cuts)) for c in charged_lists],
        writeOut=True,
        path=path,
    )

    # 2) Cut on isMostLikely with _noSVD(_noTOP) variables and custom priors
    # TODO: try to use NN PID when ready
    priors = [0.068, 0.050, 0.7326, 0.1315, 0.0183, 0.00006]
    most_likely_module = IsMostLikelyTempVars(charged_lists, priors)
    path.add_module(most_likely_module)

    for charged_list in charged_lists:
        ma.applyCuts(charged_list, "extraInfo(IsMostLikelyTempVars)==1", path=path)

    stdPhotons.stdPhotons(
        listtype="tight",
        beamBackgroundMVAWeight="MC15ri",
        fakePhotonMVAWeight="MC15ri",
        path=path,
    )

    ma.cutAndCopyList(
        "gamma:final",
        "gamma:tight",
        " and ".join(photon_cuts),
        writeOut=True,
        path=path,
    )

    # Add requirements on total number of photons and charged in event
    vm.addAlias("n_gamma_in_evt", "nParticlesInList(gamma:final)")
    vm.addAlias("n_p_in_evt", "nParticlesInList(p+:final)")
    vm.addAlias("n_e_in_evt", "nParticlesInList(e+:final)")
    vm.addAlias("n_mu_in_evt", "nParticlesInList(mu+:final)")
    vm.addAlias("n_pi_in_evt", "nParticlesInList(pi+:final)")
    vm.addAlias("n_K_in_evt", "nParticlesInList(K+:final)")
    vm.addAlias(
        "n_charged_in_evt",
        "formula(n_p_in_evt+n_e_in_evt+n_mu_in_evt+n_pi_in_evt+n_K_in_evt)",
    )

    ma.applyEventCuts(evt_cut, path=path)

    particle_lists = charged_lists + ["gamma:final"]
    particle_types = [x.split(":")[0] for x in particle_lists]
    charged_types = [x.split(":")[0] for x in charged_lists]

    ma.combineAllParticles(
        [f"{part}:final" for part in particle_types], "Upsilon(4S):final", path=path
    )

    if store_mc_truth:
        ma.fillParticleListFromMC("Upsilon(4S):MC", "", path=path)

        b_parent_var = "BParentGenID"

        # Flag each particle according to the B meson and decay it came from
        for i, particle_list in enumerate(particle_lists):
            # Match MC particles for all lists
            ma.matchMCTruth(particle_list, path=path)

            # Add extraInfo to each particle indicating parent B genID and
            # whether it belongs to a semileptonic decay
            flag_decay_module = FlagBDecayModule(
                particle_list,
                b_parent_var=b_parent_var,
            )
            path.add_module(flag_decay_module)

    graFEI = GraFEIModule(
        "Upsilon(4S):final",
        cfg_path=args.config,
        param_file=args.weight,
        sig_side_lcas=args.lcas,
        sig_side_masses=args.masses,
    )
    path.add_module(graFEI)

    # Define signal-side B
    # Here we consider all charged basf2 mass hypotheses
    # since not necessarily they coincide with those
    # predicted by graFEI which we require to be kaons
    for part in charged_types:
        ma.reconstructDecay(
            f"B+:{part[:-1]} -> {part}:final",
            "daughter(0, extraInfo(graFEI_sigSide)) == 1",
            path=path,
        )
    ma.copyLists("B+:Bsgn", [f"B+:{part[:-1]}" for part in charged_types], path=path)

    # Define tag-side B
    for part in particle_types:
        ma.cutAndCopyList(
            f"{part}:Btag",
            f"{part}:final",
            cut="extraInfo(graFEI_sigSide) == 0",
            writeOut=True,
            path=path,
        )
    ma.combineAllParticles([f"{part}:Btag" for part in particle_types], "B+:Btag", path=path)

    # 3) Keep only "good events" i.e. validTree + signal side matches chosen LCAS and mass hypotheses
    ma.reconstructDecay(
        "Upsilon(4S):graFEI -> B+:Bsgn B-:Btag", "", path=path
    )

    if store_mc_truth:
        ma.matchMCTruth("B+:Bsgn", path=path)
        ma.matchMCTruth("B-:Btag", path=path)
        ma.matchMCTruth("Upsilon(4S):graFEI", path=path)

    ################################################################################
    # Make ntuples
    ################################################################################

    input_file_path = Belle2.Environment.Instance().getInputFilesOverride()[0]

    # Variables
    momentum_vars = [
        "p",
        "px",
        "py",
        "pz",
        "pt",
        "E",
    ]
    default_vars = [
        "PDG",
        "M",
        "Mbc",
        "deltaE",
        "phi",
        "theta",
        "cosTheta",
        # "cosTBz",
        # "cosTBTO",
    ]
    default_vars += momentum_vars

    tm_vars = [
        # "isSignal",
        # "isSignalAcceptMissingNeutrino",
        "mcErrors",
        "genMotherPDG",
        "mcPDG",
    ]

    # graFEI variables
    graFEI_vars = [
        "graFEI_probEdgeProd",
        "graFEI_probEdgeMean",
        "graFEI_probEdgeGeom",
        # "graFEI_validTree", # Always 1
        # "graFEI_goodEvent", # Always 1
        "graFEI_nFSP",
        "graFEI_nCharged_preFit",
        "graFEI_nElectrons_preFit",
        "graFEI_nMuons_preFit",
        "graFEI_nPions_preFit",
        "graFEI_nKaons_preFit",
        "graFEI_nProtons_preFit",
        "graFEI_nLeptons_preFit",
        "graFEI_nPhotons_preFit",
        "graFEI_nOthers_preFit",
        "graFEI_nCharged_postFit",
        "graFEI_nElectrons_postFit",
        "graFEI_nMuons_postFit",
        "graFEI_nPions_postFit",
        "graFEI_nKaons_postFit",
        "graFEI_nProtons_postFit",
        "graFEI_nLeptons_postFit",
        "graFEI_nPhotons_postFit",
        "graFEI_nOthers_postFit",
        "graFEI_nPredictedUnmatched",
        "graFEI_nPredictedUnmatched_noPhotons",
    ]

    graFEI_tm_vars = [
        "graFEI_truth_perfectLCA",
        "graFEI_truth_perfectMasses",
        "graFEI_truth_perfectEvent",
        "graFEI_truth_isSemileptonic",
        "graFEI_truth_nFSP",
        "graFEI_truth_nPhotons",
        "graFEI_truth_nElectrons",
        "graFEI_truth_nMuons",
        "graFEI_truth_nPions",
        "graFEI_truth_nKaons",
        "graFEI_truth_nProtons",
        "graFEI_truth_nOthers",
    ]

    if store_mc_truth:
        default_vars += tm_vars
        graFEI_vars += graFEI_tm_vars

    ma.variablesToEventExtraInfo(
        "Upsilon(4S):final",
        dict((f"extraInfo({var})", var) for var in graFEI_vars),
        path=path,
    )

    # Aliases
    for var in default_vars:
        vm.addAlias(f"Bsgn_{var}", f"daughter(0, {var})")
        vm.addAlias(f"Bsgn_d0_{var}", f"daughter(0, daughter(0, {var}))")
        vm.addAlias(f"Btag_{var}", f"daughter(1, {var})")
        vm.addAlias(f"Ups_{var}", var)
    for var in momentum_vars:
        vm.addAlias(
            f"Bsgn_d0_CMS_{var}", f"useCMSFrame(daughter(0, daughter(0, {var})))"
        )
        vm.addAlias(f"Btag_CMS_{var}", f"useCMSFrame(daughter(1, {var}))")
    for var in graFEI_vars:
        vm.addAlias(var, f"eventExtraInfo({var})")

    all_vars = (
        graFEI_vars
        + [f"Bsgn_{var}" for var in tm_vars]
        + [f"Bsgn_d0_{var}" for var in default_vars]
        + [f"Bsgn_d0_CMS_{var}" for var in momentum_vars]
        + [f"Btag_{var}" for var in default_vars]
        + [f"Btag_CMS_{var}" for var in momentum_vars]
        + [f"Ups_{var}" for var in default_vars]
    )

    ma.variablesToNtuple(
        "Upsilon(4S):graFEI",
        sorted(list(set(all_vars))),
        filename=f'graFEI_UpsReco_{input_file_path[input_file_path.rfind("/")+1:]}',
        treename="tree",
        path=path,
    )

    # Process
    b2.process(path)

    # print out the summary
    print(b2.statistics)
