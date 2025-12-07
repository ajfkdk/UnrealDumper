#include <Windows.h>
#include <fmt/core.h>
#include "dumper.h"
#include "engine.h"
#include "memory.h"
#include "utils.h"
#include "wrappers.h"

Dumper::~Dumper() {
  if (Image) VirtualFree(Image, 0, MEM_RELEASE);
}

STATUS Dumper::Init(int argc, char *argv[]) {
  for (auto i = 1; i < argc; i++) {
    auto arg = argv[i];
    uint16 arg16 = *(uint16*)arg;
    if (arg16 == 'h-') {
      printf("'-p' - dump only names and objects\n"
             "'-w' - wait for input (it gives me time to inject mods)\n"
             "'-f packageNameHere' - specifies package where we should look for pointers in paddings (can take a lot of time)\n"
             "'-gn 0x1234' - GNames offset (hex)\n"
             "'-go 0x1234' - GObjects offset (hex)\n"
             "'--spacing' - add spacing in output");
      return STATUS::FAILED;
    } else if (arg16 == 'p-') {
      Full = false;
    } else if (arg16 == 'w-') {
      Wait = true;
    } else if (arg16 == 'f-') {
      i++;
      if (i < argc) {  PackageName = argv[i]; }
      else { return STATUS::FAILED; }
    }
    else if (!strcmp(arg, "-gn")) {
      i++;
      if (i < argc) { GNamesOffset = strtoull(argv[i], nullptr, 16); }
      else { return STATUS::FAILED; }
    }
    else if (!strcmp(arg, "-go")) {
      i++;
      if (i < argc) { GObjectsOffset = strtoull(argv[i], nullptr, 16); }
      else { return STATUS::FAILED; }
    }
    else if (!strcmp(arg, "--spacing")) {
      Spacing = true;
    }
  }

  if (Wait) {
    system("pause");
  }

  uint32_t pid = 0;

  {
    HWND hWnd = FindWindowA("UnrealWindow", nullptr);
    if (!hWnd) {
      return STATUS::WINDOW_NOT_FOUND;
    };
    GetWindowThreadProcessId(hWnd, (DWORD*)(&pid));
    if (!pid) {
      return STATUS::PROCESS_NOT_FOUND;
    };
  }

  if (!ReaderInit(pid)) {
    return STATUS::READER_ERROR;
  };

  fs::path processName;

  {
    wchar_t processPath[MAX_PATH]{};
    if (!GetProccessPath(pid, processPath, MAX_PATH)) { return STATUS::CANNOT_GET_PROCNAME; };
    processName = fs::path(processPath).filename();
    printf("Found UE4 game: %ls\n", processName.c_str());
  }

  {
    auto root = fs::path(argv[0]);
    root.remove_filename();
    auto game = processName.stem();
    Directory = root / "Games" / game;
    fs::create_directories(Directory);

    uint64 size = GetImageSize();
    if (!size) { return STATUS::MODULE_NOT_FOUND; }
    
    // If manual offsets are provided, use them directly
    if (GNamesOffset && GObjectsOffset) {
      return EngineInitWithOffsets(GNamesOffset, GObjectsOffset);
    }
    
    // Otherwise, use signature scan (requires reading image)
    Image = VirtualAlloc(0, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (!Read((void*)Base, Image, size)) {
      return STATUS::CANNOT_READ;
    }

    return EngineInit(game.string(), Image);
  }
}

STATUS Dumper::Dump() {
  // Debug: Check if critical StaticClass objects are found
  fmt::print("\nDebug: Checking StaticClass objects...\n");
  auto classClass = UE_UClass::StaticClass();
  auto structClass = UE_UStruct::StaticClass();
  auto scriptStructClass = UE_UScriptStruct::StaticClass();
  auto enumClass = UE_UEnum::StaticClass();
  auto functionClass = UE_UFunction::StaticClass();
  auto objectClass = UE_UObject::StaticClass();
  
  fmt::print("  UE_UObject::StaticClass() = {}\n", objectClass ? "OK" : "NULL");
  fmt::print("  UE_UClass::StaticClass() = {}\n", classClass ? "OK" : "NULL");
  fmt::print("  UE_UStruct::StaticClass() = {}\n", structClass ? "OK" : "NULL");
  fmt::print("  UE_UScriptStruct::StaticClass() = {}\n", scriptStructClass ? "OK" : "NULL");
  fmt::print("  UE_UEnum::StaticClass() = {}\n", enumClass ? "OK" : "NULL");
  fmt::print("  UE_UFunction::StaticClass() = {}\n", functionClass ? "OK" : "NULL");
  
  if (!classClass) {
    fmt::print("\nWARNING: UE_UClass::StaticClass() is NULL!\n");
    fmt::print("  This means 'Class CoreUObject.Class' was not found in GObjects.\n");
    fmt::print("  Searching for similar objects...\n");
    
    // Try to find any object with "Class" in the name
    int found = 0;
    ObjObjects.Dump([&found](uint8* obj) {
      UE_UObject object(obj);
      auto name = object.GetFullName();
      if (name.find("CoreUObject.Class") != std::string::npos || 
          name.find("Class CoreUObject") != std::string::npos) {
        fmt::print("  Found: {}\n", name);
        found++;
        if (found >= 10) return; // Limit output
      }
    });
    if (found == 0) {
      fmt::print("  No matching objects found. GNames may have issues.\n");
    }
  } else {
    // Debug: Test IsA on a known Class object
    fmt::print("\nDebug: Testing IsA functionality...\n");
    fmt::print("  classClass address: 0x{:X}\n", (uint64)classClass.GetAddress());
    fmt::print("  classClass name: {}\n", classClass.GetFullName());
    
    // Check if classClass itself is a UClass
    auto classOfClass = classClass.GetClass();
    fmt::print("  classClass.GetClass() address: 0x{:X}\n", (uint64)classOfClass.GetAddress());
    fmt::print("  classClass.GetClass() name: {}\n", classOfClass.GetFullName());
    
    // Test IsA
    bool isClass = classClass.IsA<UE_UClass>();
    fmt::print("  classClass.IsA<UE_UClass>() = {}\n", isClass ? "true" : "false");
    
    // Find a real game class and test it
    fmt::print("\n  Looking for a game class to test...\n");
    int testCount = 0;
    ObjObjects.Dump([&testCount, &classClass](uint8* obj) {
      if (testCount >= 5) return;
      UE_UObject object(obj);
      auto name = object.GetFullName();
      // Look for something that should be a Class
      if (name.find("Class Engine.Actor") != std::string::npos ||
          name.find("Class Engine.Pawn") != std::string::npos ||
          name.find("Class Engine.Character") != std::string::npos) {
        fmt::print("    Testing: {}\n", name);
        fmt::print("      GetClass() = {}\n", object.GetClass().GetFullName());
        fmt::print("      IsA<UE_UClass>() = {}\n", object.IsA<UE_UClass>() ? "true" : "false");
        fmt::print("      IsA<UE_UStruct>() = {}\n", object.IsA<UE_UStruct>() ? "true" : "false");
        testCount++;
      }
    });
  }
  fmt::print("\n");

  /*
   * Names dumping.
   * We go through each block, except last, that is not fully filled.
   * In each block we calculate next entry depending on previous entry size.
   */
  {
    File file(Directory / "NamesDump.txt", "w");
    if (!file) { return STATUS::FILE_NOT_OPEN; }
    size_t size = 0;
    NamePoolData.Dump([&file, &size](std::string_view name, uint32_t id) {
      fmt::print(file, "[{:0>6}] {}\n", id, name);
      size++;
    });
    fmt::print("Names: {}\n", size);
  }
  {
    // Why we need to iterate all objects twice? We dumping objects and filling
    // packages simultaneously.
    std::unordered_map<uint8*, std::vector<UE_UObject>> packages;
    {
      File file(Directory / "ObjectsDump.txt", "w");
      if (!file) { return STATUS::FILE_NOT_OPEN; }
      size_t size = 0;


      std::function<void(UE_UObject)> callback;
      if (Full) {
        callback = [&file, &size, &packages](UE_UObject object) {

          auto isFunction = object.IsA<UE_UFunction>();
          if (isFunction) {
            fmt::print(file, "[{:0>6}] <{}> <{}> {} {:x}\n", object.GetIndex(), object.GetAddress(), Read<void*>(object.GetAddress()), object.GetFullName(), object.Cast<UE_UFunction>().GetFunc() - Base);
          }
          else {
            fmt::print(file, "[{:0>6}] <{}> <{}> {}\n", object.GetIndex(), object.GetAddress(), Read<void*>(object.GetAddress()), object.GetFullName());
          }
          size++;
          if (isFunction || object.IsA<UE_UStruct>() || object.IsA<UE_UEnum>()) {
            auto packageObj = object.GetPackageObject();
            packages[packageObj].push_back(object);
          }
        };
      }
      else {
        callback = [&file, &size](UE_UObject object){
          fmt::print(file, "[{:0>6}] <{}> <{}> {}\n", object.GetIndex(), object.GetAddress(), Read<void*>(object.GetAddress()), object.GetFullName());
          size++;
        };
      }

      ObjObjects.Dump(callback);

      fmt::print("Objects: {}\n", size);
    }

    if (!Full) {
      return STATUS::SUCCESS;
    }

    //{
    //    // Clearing all packages with small amount of objects (comment this if
    //    you need all packages to be dumped) size_t size = packages.size();
    //    size_t erased = std::erase_if(packages, [](std::pair<byte* const,
    //    std::vector<UE_UObject>>& package) { return package.second.size() < 2;
    //    });

    //    fmt::print("Wiped {} out of {}\n", erased, size);
    //}

    // Checking if we have any package after clearing.
    if (!packages.size()) {
      return STATUS::ZERO_PACKAGES;
    }

    fmt::print("Packages: {}\n", packages.size());

    {
      auto path = Directory / "DUMP";
      fs::create_directories(path);

      int i = 1;
      int saved = 0;
      std::string unsaved{};

      bool lock = true;
      if (PackageName) lock = false;

      for (UE_UPackage package : packages) {
        fmt::print("\rProcessing: {}/{}", i++, packages.size());

        if (!lock && package.GetObject().GetName() == PackageName) {
          package.FindPointers = true;
          lock = true;
        }

        package.Process();
        if (package.Save(path, Spacing)) {
          saved++;
        } else {
          unsaved += (package.GetObject().GetName() + ", ");
        };
      }

      fmt::print("\nSaved packages: {}\n", saved);

      if (unsaved.size()) {
        unsaved.erase(unsaved.size() - 2);
        fmt::print("Unsaved empty packages: [ {} ]\n", unsaved);
      }
    }
  }
  return STATUS::SUCCESS;
}
