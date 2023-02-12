#include "config.hpp"
#include "iss_model.hpp"
#include "mti_source.hpp"

#ifdef ENABLE_TCP
#include "ipc/irq_server.hpp"
#endif

#include <CLI/App.hpp>
#include <CLI/Config.hpp>
#include <CLI/Formatter.hpp>
#include <CLI/Validators.hpp>

#include <csignal>
#include <cstdlib>

#include "common/serialize.hpp"

volatile std::sig_atomic_t pending_interrupt = 0;

namespace {
void sighandler(int signal) { pending_interrupt = signal; }
} // namespace

void run(options &opt);

int main(int argc, char **argv) {
  signal(SIGINT, sighandler);

  options opt;

  CLI::App app{"An easy-to-use, still-in-development RISC-V 32-bit simulator"};
  app.add_flag("--trace", opt.trace, "Enable logging trace to a file");
  app.add_flag("--step", opt.fstep, "Enable manual step");

  app.add_flag("-d,--dump", opt.dump_exit, "Dump elf file then exit.");

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
      ->check(CLI::ExistingFile);

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
    /*
     * TODO: this feature is partially implemented
     * either poll with zero timeout & big backlog size
     * / sandwich iss_model->step() between polls
     * or run in a separate thread
     */
    {
      irq_server serv(opt.port, 5);
      while (serv.poll())
        ;
    }
  }
#endif

  sparse_memory          mem;
  sparse_memory_accessor acc{mem};
  address_router         rout{mem, opt.mtime, opt.mtimecmp};
  iss_model              model{opt, loader(opt.target, acc), rout};

  std::unique_ptr<mti_source> mt =
      opt.mti_enabled ? std::make_unique<mti_source>(opt.interval, rout.mtime)
                      : nullptr;

  if (opt.trace) {
    fmt::ostream   out{fmt::output_file("trace.log")};
    nlohmann::json state;
    while (!model.done()) {
      model.step();
      model.trace_disasm(out);
      model.trace<nlohmann::json>(state);
      if (opt.fstep)
        if (std::cin.get() == 'q') break;
    }
    fmt::ostream state_file{fmt::output_file("trace.json")};
    state_file.print("{}", state.dump());
  } else {
    if (opt.fstep) {
      while (!model.done()) {
        model.step();
        if (std::cin.get() == 'q') break;
      }

    } else {
      while (!model.done()) {
        model.step();
      }
    }
  }

  fmt::print("{} Exited with 0x{:X} ({})\n", opt.target, model.tohost(),
             static_cast<int32_t>(model.tohost()));
}
