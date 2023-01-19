#include "iss_model.hpp"
#include "loader.hpp"
#include "memory/sparse_memory.hpp"

#include "mti_source.hpp"
#include <CLI/App.hpp>
#include <CLI/Config.hpp>
#include <CLI/Formatter.hpp>
#include <memory>

int main(int argc, char **argv) {
  constexpr static uint32_t default_interval = 1000U;
  std::string               target;
  iss_model::opts           opt;
  bool                      mti_enabled = false;
  mti_source::opt           opts{};

  CLI::App app{"An easy-to-use, still-in-development RISC-V 32-bit simulator"};
  app.add_flag("--trace", opt.trace, "Enable logging trace to a file");

  app.add_option(
      "--tohost", opt.tohost_sym,
      "Use a custom termination symbol defined in your linker script");

  auto *flag_timer =
      app.add_flag("--timer", mti_enabled, "Enable machine timer interrupts");

  auto *ogroup_mti = app.add_option_group("timer", "Machine timer interrupts")
                         ->needs(flag_timer);

  ogroup_mti->add_option("--mtime", opts.mtime,
                         "Memory-mapped 64-bit register incremented by timer");

  ogroup_mti->add_option(
      "--mtimecmp", opts.mtimecmp,
      "Memory-mapped 64-bit register written by the simulator");

  ogroup_mti->add_option("--interval", opts.interval, "In miliseconds")
      ->default_val(default_interval);
  ogroup_mti->require_option(3);

  app.option_defaults()->required();
  app.add_option("target", target, "Executable target");

  try {
    app.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
    return app.exit(e);
  }

  try {
    sparse_memory mem;
    iss_model     model{opt, loader(target, mem), mem};

    std::unique_ptr<mti_source> mt{nullptr};
    if (mti_enabled) {
      mem.write_dword(opts.mtime, 0);
      mem.write_dword(opts.mtimecmp, 0);
      mt = std::make_unique<mti_source>(opts, mem);
    }

    if (opt.trace) {
      fmt::ostream out{fmt::output_file("trace.log")};
      while (!model.done()) {
        model.trace(out);
        model.step();
      }
    }

    else {
      while (!model.done()) {
        model.step();
        if (mti_enabled && mt->interrupting()) {
          model.handle(trap_cause::int_timer_m);
        }
      }
    }

    fmt::print("{} Exited with 0x{:X} ({})\n", target, model.tohost(),
               static_cast<int32_t>(model.tohost()));

  } catch (const std::exception &ex) {
    fmt::print("{}\n{}\n", ex.what(), target);
  }

  return 0;
}
