/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include "EvtGenBase/EvtParticle.hh"
#include "EvtGenBase/EvtDecayAmp.hh"
#include <string>

class EvtParticle;

namespace Belle2 {

  /** The evtgen model to produce B-> Kstar nu nubar decay sample.
    * From factors are based on [arXiv:1503.05534v3].
    */
  class EvtKstarnunu_REV : public  EvtDecayAmp {

  public:

    /**
     * Constructor.
     */
    EvtKstarnunu_REV() {}

    /**
     * Destructor.
     */
    virtual ~EvtKstarnunu_REV();

    /**
     * The function which returns the name of the model.
     */
    std::string getName();

    /**
     * The function which makes a copy of the model.
     */
    EvtDecayBase* clone();

    /**
     * The function for an initialization.
     */
    void init();

    /**
     * The function to calculate a quark decay amplitude.
     */
    void decay(EvtParticle* p);

    /**
     * The function to sets a maximum probability.
     */
    void initProbMax();

  };

} // Belle 2 Namespace
