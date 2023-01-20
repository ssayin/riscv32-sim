#include "mti_source.hpp"
#include <future>

namespace {
std::mutex              mtx;
std::condition_variable cond;
} // namespace

void runner(mti_source::opt opts, sparse_memory &mem,
            std::atomic_bool &is_exiting) {
  while (!is_exiting) {
    std::unique_lock<std::mutex> lck{mtx};

    auto mtime = mem.read_dword(opts.mtime);

    /* to overcome possible write_dword delay when acquiring lock */
    std::future<void> commit =
        std::async(std::launch::async, &sparse_memory::write_dword, &mem,
                   opts.mtime, mtime + 1);

    cond.wait_for(lck, std::chrono::microseconds{opts.interval},
                  [&]() { return is_exiting.load(); });
  }
}

mti_source::mti_source(mti_source::opt opts, sparse_memory &mem)
    : opts{opts}, mem{mem}, t{runner, this->opts, std::ref(mem),
                              std::ref(is_exiting)} {}

void mti_source::set_exiting(bool exiting) {
  is_exiting.store(exiting);
  cond.notify_one();
  if (t.joinable()) t.join();
}

mti_source::~mti_source() { set_exiting(); }

bool mti_source::interrupting() const {
  auto mtime    = mem.read_dword(opts.mtime);
  auto mtimecmp = mem.read_dword(opts.mtimecmp);
  return mtime > mtimecmp;
}
