#include "iss_model.hpp"
#include "loader.hpp"
#include "memory/sparse_memory.hpp"

#include "config.hpp"
#include "mti_source.hpp"
#ifdef ENABLE_TCP
#include "tcpip.hpp"
#endif
#include <CLI/App.hpp>
#include <CLI/Config.hpp>
#include <CLI/Formatter.hpp>
#include <memory>

int main(int argc, char **argv) {
  std::string target;

  options opt;

  CLI::App app{"An easy-to-use, still-in-development RISC-V 32-bit simulator"};
  app.add_flag("--trace", opt.trace, "Enable logging trace to a file");

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
  bool  tcp_enabled;
  int   port;
  auto *flag_tcpserver = app.add_flag("--tcpserver", tcp_enabled,
                                      "Enable machine timer interrupts");

  auto *ogroup_server =
      app.add_option_group("tcpserver", "Server")->needs(flag_tcpserver);

  ogroup_server->add_option("-p,--port", port, "Port")->required();
#endif
  app.option_defaults()->required();
  app.add_option("target", target, "Executable target")
      ->check(CLI::ExistingFile);

  try {
    app.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
    return app.exit(e);
  }
#ifdef ENABLE_TCP
  if (tcp_enabled) {
    /*
     * TODO: this feature is partially implemented
     * either poll with zero timeout & big backlog size
     * / sandwich iss_model->step() between polls
     * or run in a separate thread
     */
    {
      irq_server serv(port, 5);
      while (serv.poll())
        ;
    }
  }
#endif

  sparse_memory          mem;
  sparse_memory_accessor acc{mem};
  address_router         rout{mem, opt.mtime, opt.mtimecmp};
  iss_model              model{opt, loader(target, acc), rout};

  std::unique_ptr<mti_source> mt{nullptr};
  if (opt.mti_enabled) {
    mt = std::make_unique<mti_source>(opt.interval, rout.mtime);
  }
  if (opt.trace) {
    fmt::ostream out{fmt::output_file("trace.log")};
    while (!model.done()) {
      model.trace(out);
      model.step();
    }
    out.print("{}", model.j.dump());
  }

  else {
    while (!model.done()) {
      model.step();
    }
  }

  fmt::print("{} Exited with 0x{:X} ({})\n", target, model.tohost(),
             static_cast<int32_t>(model.tohost()));

  return 0;
}
