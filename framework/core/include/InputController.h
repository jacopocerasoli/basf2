/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <string>
#include <utility>

class TChain;

namespace Belle2 {
  /** A static class to control supported input modules.
   *
   *  You can use setNextEntry() to request loading of any event in 0..numEntries()-1,
   *  which will be done the next time the input module's event() function is called.
   *
   *  Use canControlInput() to check wether control is actually possible.
   */
  class InputController {
  public:
    /** Is there an input module to be controlled. */
    static bool canControlInput() { return s_canControlInput; }

    /** Call this function from supported input modules. */
    static void setCanControlInput(bool on) { s_canControlInput = on; }

    /** Should the events from two input paths be mixed. */
    static bool getEventMixing() { return s_doEventMixing; }

    /** Should the events from two input paths be mixed. */
    static void enableEventMixing() { s_doEventMixing = true; };

    /** Set the file entry to be loaded the next time event() is called.
     *
     * This is mainly useful for interactive applications (e.g. event display).
     *
     * The input module should call eventLoaded() after the entry was loaded.
     */
    static void setNextEntry(long entry, bool independentPath = false) { (!independentPath) ? s_nextEntry.first = entry : s_nextEntry.second = entry; }

    /** Return entry number set via setNextEntry(). */
    static long getNextEntry(bool independentPath = false) { return (!independentPath) ? s_nextEntry.first : s_nextEntry.second;};

    /** Set the file entry to be loaded the next time event() is called, by evt/run/exp number.
     *
     * The input module should call eventLoaded() after the entry was loaded.
     */
    static void setNextEntry(long exp, long run, long event) { s_nextExperiment = exp; s_nextRun = run; s_nextEvent = event; }

    /** Return experiment number set via setNextEntry(). */
    static long getNextExperiment() { return s_nextExperiment; }

    /** Return run number set via setNextEntry(). */
    static long getNextRun() { return s_nextRun; }

    /** Return event number set via setNextEntry(). */
    static long getNextEvent() { return s_nextEvent; }

    /** returns the entry number currently loaded. */
    static long getCurrentEntry() { return s_currentEntry; }

    /** Returns total number of entries in the event tree.
     *
     * If no file is opened, zero is returned.
     */
    static long numEntries();

    /** Returns number of entries in the event tree if two input modules are used.
     */
    static std::pair<long, long> numEntriesPair() { return s_eventNumbers; }

    /** Return name of current file in loaded chain (or empty string if none loaded). */
    static std::string getCurrentFileName();

    /** Indicate that an event (in the given entry) was loaded and reset all members related to the next entry. */
    static void eventLoaded(long entry, bool independentPath = false)
    {
      if (!independentPath) s_nextEntry.first = -1;
      else s_nextEntry.second = -1;
      s_nextExperiment = -1;
      s_nextRun = -1;
      s_nextEvent = -1;
      s_currentEntry = entry;
    }

    /** Set the loaded TChain (event durability). */
    static void setChain(const TChain* chain);

    /** Reset InputController (e.g. after forking a thread) */
    static void resetForChildProcess();

  private:
    InputController() { }
    ~InputController() { }

    /** Is there an input module to be controlled? */
    static bool s_canControlInput;

    /** Do we want to mix events from two paths? */
    static bool s_doEventMixing;

    /** entry to be loaded the next time event() is called in an input module.
     *
     *  Storing two values (second one if independent path is executed)
     *  -1 indicates that execution should continue normally.
     */
    static std::pair<long, long> s_nextEntry;

    /** Experiment number to load next.
     *
     * -1 by default.
     */
    static long s_nextExperiment;

    /** Run number to load next. */
    static long s_nextRun;

    /** Event (not entry!) to load next. */
    static long s_nextEvent;

    /** number of events in paths if two input modules are used (independent paths) */
    static std::pair<long, long> s_eventNumbers;

    /** current entry in file. */
    static long s_currentEntry;

    /** Opened TChain (event durability). */
    static const TChain* s_chain;
  };
}
