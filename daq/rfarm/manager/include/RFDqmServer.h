/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef RF_DQMSERVER_H
#define RF_DQMSERVER_H

#include <string>

#include "daq/rfarm/manager/RFConf.h"
#include "daq/rfarm/manager/RFProcessManager.h"
#include "daq/rfarm/manager/RFLogManager.h"

#include "daq/rfarm/manager/RFServerBase.h"

#define MAXNODES 256

namespace Belle2 {

  class RFDqmServer : public RFServerBase {
  public:
    RFDqmServer(std::string conffile);
    ~RFDqmServer();

    // Functions to be hooked to NSM
    int Configure(NSMmsg*, NSMcontext*) override;
    int UnConfigure(NSMmsg*, NSMcontext*) override;
    int Start(NSMmsg*, NSMcontext*) override;
    int Stop(NSMmsg*, NSMcontext*) override;
    int Restart(NSMmsg*, NSMcontext*) override;

    // Server function
    void server();

    // Cleanup
    void cleanup();

  private:
    RFConf*            m_conf;
    RFProcessManager*  m_proc;
    RFLogManager*      m_log;

    int m_pid_dqm;
    int m_pid_relay;

    int m_expno;
    int m_runno;

  };
}
#endif

