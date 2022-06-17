#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

##########################################################################
# This tutorial shows how to kick off an analysis using a trained set of #
# PID calibration weights. Only the computation of overall accuracy and  #
# the pi and K efficiencies is shown. This is intended to be used after  #
# B2A913-PrepPIDTrainingSample.py and training a set of weights.         #
##########################################################################

import pidDataUtils as pdu
import numpy as np

# Read in the dataset (specifically the held-out test set from our training split.)
standard = pdu.read_npz('data/slim_dstar/test.npz')  # read PID info from npz into a DataFrame
weighted = standard.copy(deep=True)                  # make a copy of the df
print(f'{len(standard)} events')

# Load the weights
weights = np.load('models/net_wgt.npy')

# Prepare DataFrames for analysis
standard = pdu.produce_analysis_df(standard)                   # Standard PID: _no_ weights
weighted = pdu.produce_analysis_df(weighted, weights=weights)  # Weighted PID: uses the calibration weights
print(f'{len(standard)} events after cuts')

print('\nValues of the weights')
print(weights)


def compute_accuracy(df, mask=None):
    if mask:
        _df = df.loc[mask]
    return (_df['pid'] == _df['labels']).values.sum() / len(_df)


print('\n          no wgt  wgt')
standard_acc = compute_accuracy(standard)
weighted_acc = compute_accuracy(weighted)
print(f'Accuracy: {standard_acc:.3f}   {weighted_acc:.3f}')
for label in [2, 3]:
    lbl = "pion" if label == 2 else "kaon"
    _standard_eff = compute_accuracy(standard, mask=standard["labels"] == label)
    _weighted_eff = compute_accuracy(weighted, mask=weighted["labels"] == label)
    print(f'{lbl} eff: {_standard_eff:.3f}   {_weighted_eff:.3f}')

# I have an external package, 'pidplots', that interfaces with these DataFrames
# and provides many methods for quickly making plots of the PID performance.
