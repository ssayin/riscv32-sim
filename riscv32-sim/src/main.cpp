// SPDX-FileCopyrightText: 2022 - 2023 Serdar Sayın <https://serdarsayin.com>
//
// SPDX-License-Identifier: MIT

#include "config.hpp"
#include "iss/model.hpp"
#include "mti_source.hpp"

#include <CLI/App.hpp>
#include <CLI/Config.hpp>
#include <CLI/Formatter.hpp>
#include <CLI/Validators.hpp>

#include <csignal>
#include <cstdlib>

#include <fstream>

#ifdef ENABLE_TCP
#include "ipc.hpp"
#endif

#include "common/serialize.hpp"

#include "spdlog/spdlog.h"
#include "util/format_helpers.hpp"

volatile std::sig_atomic_t pending_interrupt = 0;

namespace {
void sighandler(int signal) { pending_interrupt = signal; }
} // namespace

void run(options &opt);

void register_signals() {
  signal(SIGINT, sighandler);
  signal(SIGTERM, sighandler);
#if defined(SIGQUIT)
  signal(SIGQUIT, sighandler);
#endif
}

std::ostream &operator<<(std::ostream &os, const hart_state &state) {
  std::array<char, 128> buf{};
  auto                  pc = static_cast<uint32_t>(state.pc);
  disasm_inst(buf.data(), buf.size(), rv32, pc, state.instr);
  fmt::format_to(std::ostream_iterator<char>(os), "{:>#10x}\t{}", pc,
                 buf.data());
  return os;
}

int main(int argc, char **argv) {
  options opt;

  CLI::App app{"An easy-to-use, still-in-development RISC-V 32-bit simulator"};

  app.add_option("-j,--json-output", opt.json_output, "JSON output filename");

  app.add_option("-d,--disas-output", opt.disas_output,
                 "Disas trace output filename");

  app.add_flag("--step", opt.fstep, "Enable manual step");

  app.add_flag("--dump", opt.dump_exit, "Dump elf file then exit.");

  app.add_option(
      "--tohost", opt.tohost_sym,
      "Use a custom termination symbol defined in your linker script");

  auto *flag_timer = app.add_flag("--timer", opt.mti_enabled,
                                  "Enable machine timer interrupts");

  auto *ogroup_mti = app.add_option_group("timer", "Machine timer interrupts")
                         ->needs(flag_timer);

  ogroup_mti->add_option("--mtime", opt.mtime,
                         "Memory-mapped 64-bit register incremented by timer");

  ogroup_mti->add_option(
      "--mtimecmp", opt.mtimecmp,
      "Memory-mapped 64-bit register written by the simulator");

  ogroup_mti->add_option("--interval", opt.interval, "In microseconds");

  ogroup_mti->require_option(3);

#ifdef ENABLE_TCP
  auto *flag_tcpserver = app.add_flag("--tcpserver", opt.tcp_enabled,
                                      "Enable machine timer interrupts");
  auto *ogroup_server =
      app.add_option_group("tcpserver", "Server")->needs(flag_tcpserver);

  ogroup_server->add_option("-p,--port", opt.port, "Port")->required();
#endif
  app.option_defaults()->required();
  app.add_option("target", opt.target, "Executable target")
      ->check(CLI::ExistingFile)
      ->required()
      ->take_first();

  try {
    app.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
    return app.exit(e);
  }

  run(opt);

  return 0;
}

void run(options &opt) {
  if (opt.dump_exit) {
    loader prog{opt.target};
    prog.dump(std::cout);
    std::exit(EXIT_SUCCESS);
  }

#ifdef ENABLE_TCP
  if (opt.tcp_enabled) {
    {
      try {
        boost::asio::io_context io_context;
        ipc::server             s(io_context, opt.port, 5);
        s.run();
      } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
      }
    }
  }
#endif
  mem::sparse_memory  mem;
  mem::address_router rout{mem, opt.mtime, opt.mtimecmp};
  iss::model          model{opt, loader(opt.target, rout), rout};

  std::unique_ptr<mti_source> mt =
      opt.mti_enabled ? std::make_unique<mti_source>(opt.interval, rout.mtime)
                      : nullptr;

  nlohmann::json state_export;
  while ((opt.fstep && std::cin.get() == 'q') || !model.done()) {
    model.step();

    const auto &state = model.state();
    spdlog::info(state);
    fmt::print("{}", state.dec);
    auto log_changes = [](const auto &changes) {
      std::for_each(changes.begin(), changes.end(),
                    [](const auto &change) { spdlog::info("{}", change); });
    };
    log_changes(model.state().gpr_staged);
    log_changes(model.state().csr_staged);
    if (!opt.disas_output.empty()) {
      static std::ofstream out(opt.disas_output);
      out << state;
    }
    if (!opt.json_output.empty()) state_export.emplace_back(state);
    model.commit();
  }
  if (!opt.json_output.empty()) {
    std::ofstream state_file(opt.json_output);
    state_file << state_export.dump();
  }
  spdlog::info("{} Exited with 0x{:X} ({})\n", opt.target, model.tohost(),
               static_cast<int32_t>(model.tohost()));
}
