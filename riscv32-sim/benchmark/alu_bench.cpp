// SPDX-FileCopyrightText: 2023 Serdar Sayın <https://serdarsayin.com>
//
// SPDX-License-Identifier: MIT

#include <array>
#include <benchmark/benchmark.h>
#include <random>
#include <stdexcept>

#include "alu_static_variadic.hpp"
#include "arith.hpp"
#include "common/common.hpp"

uint32_t do_alu(alu opt, uint32_t opd_1, uint32_t opd_2);

constexpr size_t num_test_cases = 90;

std::array<std::tuple<enum alu, uint32_t, uint32_t>, num_test_cases>
generate_random_test_cases() {
  std::array<std::tuple<enum alu, uint32_t, uint32_t>, num_test_cases> cases;

  std::random_device              rd;
  std::mt19937                    gen(rd());
  std::uniform_int_distribution<> dis_alu(0, static_cast<int>(alu::_jalr));
  std::uniform_int_distribution<uint32_t> dis_operand;

  for (auto &t : cases) {
    auto     op   = static_cast<enum alu>(dis_alu(gen));
    uint32_t opd1 = dis_operand(gen);
    uint32_t opd2 = dis_operand(gen);
    t             = {op, opd1, opd2};
  }

  return cases;
}

static void BM_AluSwitch(benchmark::State                             &state,
                         const decltype(generate_random_test_cases()) &cases) {
  benchmark::DoNotOptimize(cases);
  for (auto _ : state) {
    for (auto &[op, opd1, opd2] : cases) {
      auto result = do_alu(op, opd1, opd2);
      // benchmark::ClobberMemory();
    }
  }
}

static void BM_AluStatic(benchmark::State                             &state,
                         const decltype(generate_random_test_cases()) &cases) {
  benchmark::DoNotOptimize(cases);
  for (auto _ : state) {
    for (auto &[op, opd1, opd2] : cases) {
      auto result = do_alu_static(op, opd1, opd2);
      // benchmark::ClobberMemory();
    }
  }
}

auto random_test_cases = generate_random_test_cases();

BENCHMARK_CAPTURE(BM_AluStatic, Vector1, random_test_cases);
BENCHMARK_CAPTURE(BM_AluSwitch, Vector1, random_test_cases);

BENCHMARK_MAIN();

/*int main() {
  auto _ = generate_random_test_cases();
  for (auto &[op, a, b] : _) {
    auto x = do_alu(op, a, b);
    auto y = do_alu_static(op, a, b);
    std::cout << static_cast<uint32_t>(op) << " " << a << " " << b << " " << x
              << " " << y << " " << (x == y) << std::endl;
  }

  return 0;
}*/
