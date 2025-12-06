#pragma once
#include "defs.h"
#include <filesystem>

namespace fs = std::filesystem;

class Dumper {
protected:
  bool Full = true;
  bool Wait = false;
  bool Spacing = false;
  fs::path Directory;
  const char* PackageName = nullptr;
  void* Image = nullptr;
  
  // Manual offsets (0 means use signature scan)
  uint64 GNamesOffset = 0;
  uint64 GObjectsOffset = 0;
 

private:
  Dumper(){};

public:
  static Dumper* GetInstance() {
    static Dumper dumper;
    return &dumper;
  }
  ~Dumper();
  STATUS Init(int argc, char *argv[]);
  STATUS Dump();
};
