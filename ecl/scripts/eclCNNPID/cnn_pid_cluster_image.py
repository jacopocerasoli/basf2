#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import torch
import numpy as np
from torch.utils.data import Dataset
from sklearn.preprocessing import LabelEncoder


class ClusterImage(Dataset):
    """ Prepare an image with necessary inputs for ConvNet.

    It gets 7x7(=49) pixels of energy + 1 thetaId + 1 PhiId + 1 Pt.
    Then prepare proper format for each input.

    Regarding energy pixels, the specified threshold of 1 MeV is applied
    which means pixels with < 1 MeV will become 0.
    Clipping is also applied on pixels with the condition that
    pixels with energy more than 1.0 GeV will be replaced with 1.0 GeV.
    """

    def __init__(
        self,
        params
    ):
        np_energy = params['energy_image'].astype(dtype=np.float32)

        np_shape = (1, 1, params['image_length'], params['image_length'])

        np_energy_reshaped = np_energy.reshape(np_shape)
        self.energy = torch.from_numpy(np_energy_reshaped)

        self.energy[self.energy < params['threshold']] = 0.

        self.energy = np.clip(self.energy, 0, 1.0)

        theta_input = params['thetaId']
        theta_input = LabelEncoder().fit_transform(theta_input.ravel())
        self.theta_input = torch.from_numpy(theta_input)

        phi_input = params['phiId']
        phi_input = LabelEncoder().fit_transform(phi_input.ravel())
        self.phi_input = torch.from_numpy(phi_input)

        pt = params['pt'].astype(dtype=np.float32)
        self.pt = torch.from_numpy(pt)

    def __len__(self):

        return(self.pt.shape[0])

    def __getitem__(self, idx):

        return(self.energy[idx],
               self.theta_input[idx],
               self.phi_input[idx],
               self.pt[idx])
