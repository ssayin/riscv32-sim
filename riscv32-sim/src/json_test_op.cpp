#include "decoder/decoder.hpp"
#include "nlohmann/json_fwd.hpp"
#include <fstream>
#include <iostream>

int main(int argc, char *argv[]) {
  std::ifstream  ifs("trace.log");
  nlohmann::json in = nlohmann::json::parse(ifs);
  ifs.close();

  std::string str{in.dump()};

  std::ofstream ofs("trace2.log");
  ofs << str;
  ofs.close();

  ifs.open("trace2.log");
  nlohmann::json in2 = nlohmann::json::parse(ifs);
  ifs.close();

  std::string str2{in2.dump()};

  std::cout << nlohmann::json::diff(in, in2);

  return str.compare(str2);
}
