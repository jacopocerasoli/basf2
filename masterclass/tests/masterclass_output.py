#!/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
# from simulation import add_simulation
# from reconstruction import add_reconstruction


main = b2.Path()

# b2.set_random_seed(12345)
# eventinfosetter = b2.register_module('EventInfoSetter')
# eventinfosetter.param({'evtNumList': [10]})
# main.add_module(eventinfosetter)
# particlegun = b2.register_module('ParticleGun')
# particlegun.param('momentumGeneration', 'uniform')
# particlegun.param('momentumParams', [0.5, 4.0])
# particlegun.param('pdgCodes', [13, -13, 211, -211, 321, -321, 22])
# particlegun.param('nTracks', 10)
# particlegun.param('thetaGeneration', 'uniformCos')
# particlegun.param('thetaParams', [0.0, 180.0])
# main.add_module(particlegun)
# add_simulation(main)
# add_reconstruction(main)

# # b2.set_random_seed(12345)
# eventinfosetter = b2.register_module('EventInfoSetter')
# eventinfosetter.param({'evtNumList': [10]})
# main.add_module(eventinfosetter)
# decfile = b2.find_file('BztoKstautau.dec')
# b2.conditions.prepend_globaltag("mc_production_MC15ri_a")
# main.add_module('EvtGenInput', userDECFile=decfile)
# add_simulation(main)
# add_reconstruction(main)

inputMod = main.add_module('RootInput')
inputMod.param('inputFileName', '/group/belle2/dataprod/MC/MC15ri/charged/sub00/mdst_000587_prod00024816_task10020000589.root')
inputMod.param('entrySequences', ['0:10'])

masterclass = main.add_module('MasterClass')
masterclass.param('outputFileName', 'masterclass.root')

b2.process(main)
