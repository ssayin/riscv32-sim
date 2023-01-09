#include "iss_model.hpp"
#include "loader.hpp"
#include "memory/sparse_memory.hpp"

#include <CLI/App.hpp>
#include <CLI/Config.hpp>
#include <CLI/Formatter.hpp>

int main(int argc, char **argv) {
  std::string    target;
  iss_model_opts opts;

  CLI::App app{"An easy-to-use, still-in-development RISC-V 32-bit simulator"};
  app.add_flag("--trace", opts.trace, "Enable logging trace to a file");

  app.add_option(
      "--tohost", opts.tohost_sym,
      "Use a custom termination symbol defined in your linker script");

  app.option_defaults()->required();
  app.add_option("target", target, "Executable target");

  try {
    app.parse(argc, argv);
    sparse_memory mem;
    iss_model     model{opts, loader(target, mem), mem};

    if (opts.trace) {
      fmt::ostream out{fmt::output_file("trace.log")};
      while (!model.done()) {
        model.trace(out);
        model.step();
      }
    }

    else {
      while (!model.done()) {
        model.step();
      }
    }

    fmt::print("{} Exited with 0x{:X} ({})\n", target, model.tohost(),
               static_cast<int32_t>(model.tohost()));

  } catch (const CLI::ParseError &e) {
    return app.exit(e);
  } catch (const std::exception &ex) {
    fmt::print("{}\n{}\n", ex.what(), target);
  }

  return 0;
}
