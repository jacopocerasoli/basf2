/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

// ROOT headers.
#include <TObject.h>

// C++ headers.
#include <cstdint>

namespace Belle2 {
  /**
   * ECL/KLM clustering event level information:
   * - out of time ECLCalDigits;
   * - ECLClusters rejected before storing to mdst;
   * - multi-strip KLMDigits.
   */
  class EventLevelClusteringInfo : public TObject {
  public:

    /** Default constructor */
    EventLevelClusteringInfo() = default;

    /** ECL: getter for the number of ECLCalDigits that are out of time and above some MeV (scale ~linearly with background), forward endcap. */
    uint16_t getNECLCalDigitsOutOfTimeFWD() const
    {
      return m_nECLCalDigitsOutOfTimeFWD;
    }

    /** ECL: getter for the number of ECLCalDigits that are out of time and above some MeV (scale ~linearly with background), barrel. */
    uint16_t getNECLCalDigitsOutOfTimeBarrel() const
    {
      return m_nECLCalDigitsOutOfTimeBarrel;
    }

    /** ECL: getter for the number of ECLCalDigits that are out of time and above some MeV (scale ~linearly with background), backward endcap. */
    uint16_t getNECLCalDigitsOutOfTimeBWD() const
    {
      return m_nECLCalDigitsOutOfTimeBWD;
    }

    /** ECL: getter for the number of ECLCalDigits that are out of time and above some MeV (scale ~linearly with background), entire ECL. */
    uint16_t getNECLCalDigitsOutOfTime() const
    {
      return m_nECLCalDigitsOutOfTimeFWD + m_nECLCalDigitsOutOfTimeBarrel + m_nECLCalDigitsOutOfTimeBWD;
    }

    /** ECL: setter for the number of ECLCalDigits that are out of time and above some MeV, forward endcap. */
    void setNECLCalDigitsOutOfTimeFWD(uint16_t const nECLCalDigitsOutOfTimeFWD)
    {
      m_nECLCalDigitsOutOfTimeFWD = nECLCalDigitsOutOfTimeFWD;
    }

    /** ECL: setter for the number of ECLCalDigits that are out of time and above some MeV, barrel. */
    void setNECLCalDigitsOutOfTimeBarrel(uint16_t const nECLCalDigitsOutOfTimeBarrel)
    {
      m_nECLCalDigitsOutOfTimeBarrel = nECLCalDigitsOutOfTimeBarrel;
    }

    /** ECL: setter for the number of ECLCalDigits that are out of time and above some MeV, backward endcap. */
    void setNECLCalDigitsOutOfTimeBWD(uint16_t const nECLCalDigitsOutOfTimeBWD)
    {
      m_nECLCalDigitsOutOfTimeBWD = nECLCalDigitsOutOfTimeBWD;
    }

    /** ECL: getter for the number of photon ECLShowers that are not stored as ECLClusters, forward endcap. */
    uint8_t getNECLShowersRejectedFWD() const
    {
      return m_nECLShowersRejectedFWD;
    }

    /** ECL: getter for the number of photon ECLShowers that are not stored as ECLClusters, barrel. */
    uint8_t getNECLShowersRejectedBarrel() const
    {
      return m_nECLShowersRejectedBarrel;
    }

    /** ECL: getter for the number of photon ECLShowers that are not stored as ECLClusters, backward endcap. */
    uint8_t getNECLShowersRejectedBWD() const
    {
      return m_nECLShowersRejectedBWD;
    }

    /** ECL: setter for the number of photon ECLShowers that are not stored as ECLClusters, entire ECL. */
    uint8_t getNECLShowersRejected() const
    {
      return m_nECLShowersRejectedFWD + m_nECLShowersRejectedBarrel + m_nECLShowersRejectedBWD;
    }

    /** ECL: setter for the number of photon ECLShowers that are not stored as ECLClusters, forward endcap. */
    void setNECLShowersRejectedFWD(uint8_t const nECLShowersRejectedFWD)
    {
      m_nECLShowersRejectedFWD = nECLShowersRejectedFWD;
    }

    /** ECL: setter for the number of photon ECLShowers that are not stored as ECLClusters, barrel. */
    void setNECLShowersRejectedBarrel(uint8_t const nECLShowersRejectedBarrel)
    {
      m_nECLShowersRejectedBarrel = nECLShowersRejectedBarrel;
    }

    /** ECL: setter for the number of photon ECLShowers that are not stored as ECLClusters, backward endcap. */
    void setNECLShowersRejectedBWD(uint8_t const nECLShowersRejectedBWD)
    {
      m_nECLShowersRejectedBWD = nECLShowersRejectedBWD;
    }

    /** KLM: getter for the number of multi-strip KLMDigits, forward endcap. */
    uint16_t getNKLMDigitsMultiStripFWD() const
    {
      return m_nKLMDigitsMultiStripFWD;
    }

    /** KLM: getter for the number of multi-strip KLMDigits, barrel. */
    uint16_t getNKLMDigitsMultiStripBarrel() const
    {
      return m_nKLMDigitsMultiStripBarrel;
    }

    /** KLM: getter for the number of multi-strip KLMDigits, backward endcap. */
    uint16_t getNKLMDigitsMultiStripBWD() const
    {
      return m_nKLMDigitsMultiStripBWD;
    }

    /** KLM: getter for the number of multi-strip KLMDigits, entire KLM. */
    uint16_t getNKLMDigitsMultiStrip() const
    {
      return m_nKLMDigitsMultiStripFWD + m_nKLMDigitsMultiStripBarrel + m_nKLMDigitsMultiStripBWD;
    }

    /** KLM: getter for the number of multi-strip KLMDigits, forward endcap. */
    void setNKLMDigitsMultiStripFWD(uint16_t const nKLMDigitsMultiStripFWD)
    {
      m_nKLMDigitsMultiStripFWD = nKLMDigitsMultiStripFWD;
    }

    /** KLM: getter for the number of multi-strip KLMDigits, barrel. */
    void setNKLMDigitsMultiStripBarrel(uint16_t const nKLMDigitsMultiStripBarrel)
    {
      m_nKLMDigitsMultiStripBarrel = nKLMDigitsMultiStripBarrel;
    }

    /** KLM: getter for the number of multi-strip KLMDigits, backward endcap. */
    void setNKLMDigitsMultiStripBWD(uint16_t const nKLMDigitsMultiStripBWD)
    {
      m_nKLMDigitsMultiStripBWD = nKLMDigitsMultiStripBWD;
    }

  private:

    /** ECL: number of out of time, energetic ECLCalDigits, forward endcap. */
    uint16_t m_nECLCalDigitsOutOfTimeFWD {0};

    /** ECL: number of out of time, energetic ECLCalDigits, barrel. */
    uint16_t m_nECLCalDigitsOutOfTimeBarrel {0};

    /** ECL: number of out of time, energetic ECLCalDigits, backward endcap. */
    uint16_t m_nECLCalDigitsOutOfTimeBWD {0};

    /** KLM: number of multi-strip KLMDigits, forward endcap. */
    uint16_t m_nKLMDigitsMultiStripFWD{0};

    /** KLM: number of multi-strip KLMDigits, barrel. */
    uint16_t m_nKLMDigitsMultiStripBarrel{0};

    /** KLM: number of multi-strip KLMDigits, backward endcap. */
    uint16_t m_nKLMDigitsMultiStripBWD{0};

    /** ECL: number of photon ECLShowers that are rejected before storing to mdst (max. 255), endcap forward. */
    uint8_t m_nECLShowersRejectedFWD {0};

    /** ECL: cumber of photon ECLShowers that are rejected before storing to mdst (max. 255), barrel. */
    uint8_t m_nECLShowersRejectedBarrel {0};

    /** ECL: number of photon ECLShowers that are rejected before storing to mdst (max. 255), backward endcap. */
    uint8_t m_nECLShowersRejectedBWD {0};

    ClassDef(EventLevelClusteringInfo, 2); /**< ROOT. */
  };
}
