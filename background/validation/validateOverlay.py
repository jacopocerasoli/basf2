#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
  <output>overlayPlots.root</output>
  <contact>marko.staric@ijs.si</contact>
  <description>Runs BG overlay and makes validation histograms</description>
</header>
"""

import basf2 as b2
from simulation import add_simulation
import os
import glob
from ROOT import Belle2
from ROOT import TH1F, TFile, TNamed

b2.set_random_seed(123452)


class Histogrammer(b2.Module):

    '''
    Make validation histograms for BG overlay.
    '''

    def initialize(self):
        ''' Initialize the Module: book histograms and set descriptions and checks'''

        #: list of histograms
        self.hist = []
        self.hist.append(TH1F('PXDDigits', 'PXDDigits (no data reduction)',
                              200, 20000, 40000))
        self.hist.append(TH1F('SVDShaperDigits', 'SVDShaperDigits', 100, 0, 8000))
        self.hist.append(TH1F('SVDShaperDigitsZS5', 'ZS5 SVDShaperDigits', 100, 0, 8000))
        self.hist.append(TH1F('SVDShaperDigitsZS5L3', 'ZS5 L3 Occupancy (%)', 100, 0, 20))
        self.hist.append(TH1F('CDCHits', 'CDCHits', 100, 0, 6000))
        self.hist.append(TH1F('TOPDigits', 'TOPDigits', 100, 0, 2000))
        self.hist.append(TH1F('ARICHDigits', 'ARICHDigits', 100, 0, 300))
        self.hist.append(TH1F('ECLDigits', 'ECLDigits, m_Amp > 500 (roughly 25 MeV)',
                              100, 0, 200))
        self.hist.append(TH1F('KLMDigits', 'KLMDigits', 150, 0, 150))

        #: number of L3 strips
        self.nSVDL3 = 768 * 7 * 2

        for h in self.hist:
            if h.GetName() == 'SVDShaperDigitsZS5L3':
                h.SetXTitle('ZS5 L3 occupancy (%)')
            else:
                h.SetXTitle('number of digits in event')
            h.SetYTitle('entries per bin')
            descr = TNamed('Description', 'Number of background ' + h.GetName() +
                           ' per event (with BG overlay only and no event generator)')
            h.GetListOfFunctions().Add(descr)
            check = TNamed('Check', 'Distribution must agree with its reference')
            h.GetListOfFunctions().Add(check)
            contact = TNamed('Contact', 'marko.staric@ijs.si')
            h.GetListOfFunctions().Add(contact)
            options = TNamed('MetaOptions', 'shifter')
            h.GetListOfFunctions().Add(options)

    def event(self):
        ''' Event processor: fill histograms '''

        for h in self.hist:
            digits = Belle2.PyStoreArray(h.GetName())
            svdDigits = Belle2.PyStoreArray('SVDShaperDigitsZS5')
            if h.GetName() == 'ECLDigits':
                n = 0
                for digit in digits:
                    if digit.getAmp() > 500:  # roughly 25 MeV
                        n += 1
                h.Fill(n)
            elif h.GetName() == 'SVDShaperDigitsZS5L3':
                nL3 = 0
                for svdDigit in svdDigits:
                    if svdDigit.getSensorID().getLayerNumber() == 3:  # only check SVD L3
                        nL3 += 1
                h.Fill(nL3/self.nSVDL3 * 100)
            else:
                h.Fill(digits.getEntries())

    def terminate(self):
        """ Write histograms to file."""

        tfile = TFile('overlayPlots.root', 'recreate')
        for h in self.hist:
            h.Write()
        tfile.Close()


bg = None
if 'BELLE2_BACKGROUND_DIR' in os.environ:
    bg = glob.glob(os.environ['BELLE2_BACKGROUND_DIR'] + '/*.root')
    if bg is None:
        b2.B2FATAL('No beam background samples found in folder ' +
                   os.environ['BELLE2_BACKGROUND_DIR'])
    b2.B2INFO('Using background samples from ' + os.environ['BELLE2_BACKGROUND_DIR'])
else:
    b2.B2FATAL('variable BELLE2_BACKGROUND_DIR is not set')

# Create path
main = b2.create_path()

# Set number of events to generate
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1000], 'runList': [1]})
main.add_module(eventinfosetter)

# Simulation
add_simulation(main, bkgfiles=bg, bkgOverlay=True, usePXDDataReduction=False, forceSetPXDDataReduction=True,
               simulateT0jitter=False)

# Add SVD offline zero-suppression (ZS5)
main.add_module(
    'SVDZeroSuppressionEmulator',
    SNthreshold=5,
    ShaperDigits='SVDShaperDigits',
    ShaperDigitsIN='SVDShaperDigitsZS5',
    FADCmode=True)

# Make histograms
main.add_module(Histogrammer())

# Show progress of processing
main.add_module('Progress')

# Process events
b2.process(main)

# Print call statistics
print(b2.statistics)
