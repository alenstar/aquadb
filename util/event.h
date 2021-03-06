// Copyright 2014 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

// Multithreaded event class. This allows waiting in a thread for an event to
// be triggered in another thread. While waiting, the CPU will be available for
// other tasks.
// * Set(): triggers the event and wakes up the waiting thread.
// * Wait(): waits for the event to be triggered.
// * Reset(): tries to reset the event before the waiting thread sees it was
//            triggered. Usually a bad idea.

#pragma once

#ifdef _WIN32
#include <concrt.h>
#endif

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>

namespace util
{

class Flag final
{
public:
  // Declared as explicit since we do not want "= true" to work on a flag
  // object - it should be made explicit that a flag is *not* a normal
  // variable.
  explicit Flag(bool initial_value = false) : m_val(initial_value) {}
  void Set(bool val = true) { m_val.store(val); }
  void Clear() { Set(false); }
  bool IsSet() const { return m_val.load(); }
  bool TestAndSet(bool val = true)
  {
    bool expected = !val;
    return m_val.compare_exchange_strong(expected, val);
  }

  bool TestAndClear() { return TestAndSet(false); }

private:
  std::atomic_bool m_val;
};

class Event final
{
public:
  void Set()
  {
    if (m_flag.TestAndSet())
    {
      std::lock_guard<std::mutex> lk(m_mutex);
      m_condvar.notify_one();
    }
  }

  void Wait()
  {
    if (m_flag.TestAndClear())
      return;

    std::unique_lock<std::mutex> lk(m_mutex);
    m_condvar.wait(lk, [&] { return m_flag.TestAndClear(); });
  }

  template <class Rep, class Period>
  bool WaitFor(const std::chrono::duration<Rep, Period>& rel_time)
  {
    if (m_flag.TestAndClear())
      return true;

    std::unique_lock<std::mutex> lk(m_mutex);
    bool signaled = m_condvar.wait_for(lk, rel_time, [&] { return m_flag.TestAndClear(); });

    return signaled;
  }

  void Reset()
  {
    // no other action required, since wait loops on
    // the predicate and any lingering signal will get
    // cleared on the first iteration
    m_flag.Clear();
  }

private:
  Flag m_flag;
  std::condition_variable m_condvar;
  std::mutex m_mutex;
};

}  // namespace util
