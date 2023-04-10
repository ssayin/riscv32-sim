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

#ifdef ENABLE_TCP
#include "ipc.hpp"
#endif

#include "common/serialize.hpp"

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

  nlohmann::json state;
  while ((opt.fstep && std::cin.get() == 'q') || !model.done()) {
    model.step();
    if (!opt.disas_output.empty()) {
      static fmt::ostream out{fmt::output_file(opt.disas_output)};
      model.trace_disasm(out);
    }
    if (!opt.json_output.empty()) model.trace<nlohmann::json>(state);
    model.commit();
  }
  if (!opt.json_output.empty()) {
    fmt::ostream state_file{fmt::output_file(opt.json_output)};
    state_file.print("{}", state.dump());
  }

  fmt::print("{} Exited with 0x{:X} ({})\n", opt.target, model.tohost(),
             static_cast<int32_t>(model.tohost()));
}
