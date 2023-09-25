#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
This is a log-type unit test of the ROE made out of MCParticles.
It runs over a mdst file which contains two muons, converted gamma, K_S0 and Lambda0.
It uses a muon as a signal side, so ROE should contain only 3 V0 objects + a muon.
The mdst file was generated by scripts in the external repository:
https://gitlab.desy.de/belle2/software/examples-data-creation/roe-unittest-mc
There is no larger equivalent of this test.
"""

import b2test_utils
from basf2 import set_random_seed, create_path, process

# make logging more reproducible by replacing some strings
b2test_utils.configure_logging_for_tests()
set_random_seed("1337")
testinput = [b2test_utils.require_file('analysis/tests/pgun-roe-mdst.root')]
fsps = ['mu+', 'K_S0', 'Lambda0', 'gamma']
###############################################################################
# a new ParticleLoader for each fsp
testpath = create_path()
testpath.add_module('RootInput', inputFileNames=testinput)
for fsp in fsps:
    testpath.add_module('ParticleLoader', decayStrings=[fsp + ':MC'],
                        addDaughters=True, skipNonPrimaryDaughters=True, useMCParticles=True)
    testpath.add_module('ParticleListManipulator', outputListName=fsp,
                        inputListNames=[fsp + ':MC'], cut='mcPrimary > 0')
signal_side_name = 'B0'
testpath.add_module('ParticleCombiner',
                    decayString=signal_side_name+' -> mu+ mu-',
                    cut='')
testpath.add_module('ParticlePrinter',
                    listName='mu+')

testpath.add_module('RestOfEventBuilder', particleList=signal_side_name,
                    particleListsInput=fsps, fromMC=True)
###############################################################################
roe_path = create_path()
roe_path.add_module('RestOfEventPrinter',
                    unpackComposites=False,
                    fullPrint=False)
testpath.for_each('RestOfEvent', 'RestOfEvents', path=roe_path)
###############################################################################
process(testpath, 2)
