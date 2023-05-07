// SPDX-FileCopyrightText: 2023 Serdar Sayın <https://serdarsayin.com>
//
// SPDX-License-Identifier: MIT

#include <fstream>
#include <limits>
#include <random>

#include "nlohmann/json.hpp"
#include "gtest/gtest.h"

#include "../include/util/json_export_helpers.hpp"
#include "common/common.hpp"

class JsonTraceTest : public ::testing::Test {
protected:
  void SetUp() override {
    std::random_device                      rd;
    std::mt19937                            g(rd());
    nlohmann::json                          j;
    std::uniform_int_distribution<uint32_t> distrib(
        0, std::numeric_limits<uint32_t>::max());

    // generate random data
    for (int i = 0; i < 10; ++i) {
      hart_state hs{distrib(g)};
      hs.instr             = distrib(g);
      hs.dec.rd            = distrib(g) % 32;
      hs.dec.rs1           = distrib(g) % 32;
      hs.dec.rs2           = distrib(g) % 32;
      hs.dec.imm           = distrib(g);
      hs.dec.tgt           = static_cast<target>(distrib(g) % 5);
      hs.dec.is_compressed = static_cast<bool>(distrib(g) % 2);
      hs.dec.use_pc        = static_cast<bool>(distrib(g) % 2);
      hs.dec.has_imm       = static_cast<bool>(distrib(g) % 2);
      hs.gpr_staged.emplace_back(gpr_change{hs.dec.rd, distrib(g), distrib(g)});
      j.emplace_back(hs);
    }

    // Create the input JSON file
    std::ofstream file("input.json");
    file << j;
    file.close();
  }

  void TearDown() override {
    // Remove the input JSON file
    std::remove("input.json");
  }
};

// Define a test that reads input.json and writes to output.json
TEST_F(JsonTraceTest, RandomTrace) {
  // Read input.json
  std::ifstream ifs("input.json");
  ASSERT_TRUE(ifs.good()) << "Could not open input file";
  nlohmann::json in = nlohmann::json::parse(ifs);
  ifs.close();

  // Write to output.json
  std::string   str{in.dump()};
  std::ofstream ofs("output.json");
  ofs << str;
  ofs.close();

  // Read output.json and compare to input.json
  std::ifstream ifs2("output.json");
  ASSERT_TRUE(ifs2.good()) << "Could not open output file";
  nlohmann::json out = nlohmann::json::parse(ifs2);
  ifs2.close();
  ASSERT_EQ(in, out) << "Input and output JSON do not match";
  std::remove("output.json");
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
