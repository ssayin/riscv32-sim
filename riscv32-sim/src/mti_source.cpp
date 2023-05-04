// SPDX-FileCopyrightText: 2022 - 2023 Serdar Sayın <https://serdarsayin.com>
//
// SPDX-License-Identifier: MIT

#include <csignal>
#include <future>

#include "mti_source.hpp"

namespace {
std::mutex              mtx;
std::condition_variable cond;
} // namespace

extern volatile std::sig_atomic_t pending_interrupt;

void runner(uint32_t interval, std::array<std::atomic<uint8_t>, 8> &mtime,
            std::atomic_bool &is_exiting) {

  while (!(is_exiting || pending_interrupt)) {
    mtime.at(0).fetch_add(1);
    std::unique_lock<std::mutex> lck{mtx};
    // auto mtime = mem.read_dword(opts.mtime);

    /* to overcome possible write_dword delay when acquiring lock */
    // std::future<void> commit =
    //    std::async(std::launch::async, &sparse_memory::write_dword, &mem,
    //              opts.mtime, mtime + 1);

    cond.wait_for(lck, std::chrono::microseconds{interval},
                  [&]() { return is_exiting.load(); });
  }
}

void mti_source::set_exiting(bool exiting) {
  is_exiting.store(exiting);
  cond.notify_one();
  if (t.joinable()) t.join();
}

mti_source::~mti_source() { set_exiting(); }
mti_source::mti_source(uint32_t                             interval,
                       std::array<std::atomic<uint8_t>, 8> &mtime)
    : t{runner, interval, std::ref(mtime), std::ref(is_exiting)} {}
