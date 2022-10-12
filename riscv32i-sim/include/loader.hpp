#pragma once

#include <string>

class iss_model;

struct loader {
  static void load(std::string file_name, iss_model &model);
};
