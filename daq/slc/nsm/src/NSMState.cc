/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/slc/nsm/NSMState.h"

using namespace Belle2;

const NSMState NSMState::ONLINE_S(1, "ONLINE");

const NSMState& NSMState::operator=(const NSMState& state)
{
  Enum::operator=(state);
  return *this;
}
