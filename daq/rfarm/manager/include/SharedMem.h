/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <sys/types.h>

#include <string>

namespace Belle2 {
  class SharedMem {
  public:
    SharedMem(const char* name, int size);
    SharedMem(int shm_id, int sem_id, int size);
    ~SharedMem(void);

    void* ptr(void);
    int shmid(void);

    bool IsCreated(void);

    void lock(void);
    void unlock(void);
    bool isLocked(void);

  private:
    bool m_new{false}; /**< True if we created the ring buffer ourselves (and need to clean it). */
    std::string m_pathname;
    int  m_pathfd; /** Associated file descriptor. */
    key_t m_shmkey; /**< SHM key, see shmget(2). */
    key_t m_semkey; /**< Semaphore key */

    int m_shmid{-1}; /**< shared memory id */
    int m_semid{-1}; /**< semaphore id */
    void* m_shmadr{nullptr};
    char* m_strbuf;
  };
}


