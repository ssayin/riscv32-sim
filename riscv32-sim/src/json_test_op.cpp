#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

int main(int argc, char *argv[]) {
  std::ifstream  ifs("trace.json");
  nlohmann::json in = nlohmann::json::parse(ifs);
  ifs.close();

  std::string str{in.dump()};

  std::ofstream ofs("trace2.json");
  ofs << str;
  ofs.close();

  ifs.open("trace2.json");
  nlohmann::json in2 = nlohmann::json::parse(ifs);
  ifs.close();

  std::string str2{in2.dump()};

  std::cout << nlohmann::json::diff(in, in2);

  return str.compare(str2);
}
