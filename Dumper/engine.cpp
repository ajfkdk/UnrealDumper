#include <Windows.h>
#include <fmt/core.h>
#include "engine.h"
#include "generic.h"
#include "memory.h"
#include "utils.h"
#include "wrappers.h"

Offsets offsets;

ansi_fn Decrypt_ANSI = nullptr;
// wide_fn Decrypt_WIDE = nullptr;

struct {
  uint16 Stride = 2;
  struct {
    uint16 Size = 24;
  } FUObjectItem;
  struct {
    uint16 Number = 4;
  } FName;
  struct {
    uint16 Info = 0;
    uint16 WideBit = 0;
    uint16 LenBit = 6;
    uint16 HeaderSize = 2;
  } FNameEntry;
  struct {
    uint16 Index = 0xC;
    uint16 Class = 0x10;
    uint16 Name = 0x18;
    uint16 Outer = 0x20;
  } UObject;
  struct {
    uint16 Next = 0x28;
  } UField;
  struct {
    uint16 SuperStruct = 0x40;
    uint16 Children = 0x48;
    uint16 ChildProperties = 0x50;
    uint16 PropertiesSize = 0x58;
  } UStruct;
  struct {
    uint16 Names = 0x40;
  } UEnum;
  struct {
    uint16 FunctionFlags = 0xB0;
    uint16 Func = 0xB0 + 0x28;
  } UFunction;
  struct {
    uint16 Class = 0x8;
    uint16 Next = 0x20;
    uint16 Name = 0x28;
  } FField;
  struct {
    uint16 ArrayDim = 0x38;
    uint16 ElementSize = 0x3C;
    uint16 PropertyFlags = 0x40;
    uint16 Offset = 0x4C;
    uint16 Size = 0x78;
  } FProperty;
  struct {
    uint16 ArrayDim = 0;
    uint16 ElementSize = 0;
    uint16 PropertyFlags = 0;
    uint16 Offset = 0;
    uint16 Size = 0; // sizeof(UProperty)
  } UProperty;
} Default;
static_assert(sizeof(Default) == sizeof(Offsets));

struct {
  uint16 Stride = 4;
  struct {
    uint16 Size = 24;
  } FUObjectItem;
  struct {
    uint16 Number = 8;
  } FName;
  struct {
    uint16 Info = 4;
    uint16 WideBit = 0;
    uint16 LenBit = 1;
    uint16 HeaderSize = 6;
  } FNameEntry;
  struct {
    uint16 Index = 0xC;
    uint16 Class = 0x10;
    uint16 Name = 0x18;
    uint16 Outer = 0x28;
  } UObject;
  struct {
    uint16 Next = 0x30;
  } UField;
  struct {
    uint16 SuperStruct = 0x48;
    uint16 Children = 0x50;
    uint16 ChildProperties = 0x58;
    uint16 PropertiesSize = 0x60;
  } UStruct;
  struct {
    uint16 Names = 0x48;
  } UEnum;
  struct {
    uint16 FunctionFlags = 0xB8;
    uint16 Func = 0xB8 + 0x28; // ue3-ue4, always +0x28 from flags location.
  } UFunction;
  struct {
    uint16 Class = 0x8;
    uint16 Next = 0x20;
    uint16 Name = 0x28;
  } FField;
  struct {
    uint16 ArrayDim = 0x38;
    uint16 ElementSize = 0x3C;
    uint16 PropertyFlags = 0x40;
    uint16 Offset = 0x4C;
    uint16 Size = 0x80;
  } FProperty;
  struct {
    uint16 ArrayDim = 0;
    uint16 ElementSize = 0;
    uint16 PropertyFlags = 0;
    uint16 Offset = 0;
    uint16 Size = 0; // sizeof(UProperty)
  } UProperty;
} DeadByDaylight;
static_assert(sizeof(DeadByDaylight) == sizeof(Offsets));

struct {
  uint16 Stride = 2;
  struct {
    uint16 Size = 24;
  } FUObjectItem;
  struct {
    uint16 Number = 4;
  } FName;
  struct {
    uint16 Info = 0;
    uint16 WideBit = 0;
    uint16 LenBit = 6;
    uint16 HeaderSize = 2;
  } FNameEntry;
  struct {
    uint16 Index = 0xC;
    uint16 Class = 0x10;
    uint16 Name = 0x18;
    uint16 Outer = 0x20;
  } UObject;
  struct {
    uint16 Next = 0x28;
  } UField;
  struct {
    uint16 SuperStruct = 0x40;
    uint16 Children = 0x48;
    uint16 ChildProperties = 0x50;
    uint16 PropertiesSize = 0x58;
  } UStruct;
  struct {
    uint16 Names = 0x40;
  } UEnum;
  struct {
    uint16 FunctionFlags = 0xB0;
    uint16 Func = 0xB0 + 0x30;
  } UFunction;
  struct {
    uint16 Class = 0x8;
    uint16 Next = 0x20;
    uint16 Name = 0x28;
  } FField;
  struct {
    uint16 ArrayDim = 0x38;
    uint16 ElementSize = 0x3C;
    uint16 PropertyFlags = 0x40;
    uint16 Offset = 0x4C;
    uint16 Size = 0x78;
  } FProperty;
  struct {
    uint16 ArrayDim = 0;
    uint16 ElementSize = 0;
    uint16 PropertyFlags = 0;
    uint16 Offset = 0;
    uint16 Size = 0; // sizeof(UProperty)
  } UProperty;
} Scavengers;
static_assert(sizeof(Scavengers) == sizeof(Offsets));

struct {
  uint16 Stride = 2;
  struct {
    uint16 Size = 32;
  } FUObjectItem;
  struct {
    uint16 Number = 4;
  } FName;
  struct {
    uint16 Info = 0;
    uint16 WideBit = 0;
    uint16 LenBit = 6;
    uint16 HeaderSize = 2;
  } FNameEntry;
  struct {
    uint16 Index = 0xC;
    uint16 Class = 0x10;
    uint16 Name = 0x18;
    uint16 Outer = 0x20;
  } UObject;
  struct {
    uint16 Next = 0x28;
  } UField;
  struct {
    uint16 SuperStruct = 0x40;
    uint16 Children = 0x48;
    uint16 ChildProperties = 0x50;
    uint16 PropertiesSize = 0x58;
  } UStruct;
  struct {
    uint16 Names = 0x40;
  } UEnum;
  struct {
    uint16 FunctionFlags = 0xB0;
    uint16 Func = 0xB0 + 0x28;
  } UFunction;
  struct {
    uint16 Class = 0x8;
    uint16 Next = 0x20;
    uint16 Name = 0x28;
  } FField;
  struct {
    uint16 ArrayDim = 0x38;
    uint16 ElementSize = 0x3C;
    uint16 PropertyFlags = 0x40;
    uint16 Offset = 0x4C;
    uint16 Size = 0x78;
  } FProperty;
  struct {
    uint16 ArrayDim = 0;
    uint16 ElementSize = 0;
    uint16 PropertyFlags = 0;
    uint16 Offset = 0;
    uint16 Size = 0; // sizeof(UProperty)
  } UProperty;
} Brickadia;
static_assert(sizeof(Brickadia) == sizeof(Offsets));

struct {
  uint16 Stride = 4;
  struct {
    uint16 Size = 24;
  } FUObjectItem;
  struct {
    uint16 Number = 8;
  } FName;
  struct {
    uint16 Info = 4;
    uint16 WideBit = 0;
    uint16 LenBit = 1;
    uint16 HeaderSize = 6;
  } FNameEntry;
  struct {
    uint16 Index = 0xC;
    uint16 Class = 0x10;
    uint16 Name = 0x18;
    uint16 Outer = 0x28;
  } UObject;
  struct {
      uint16 Next = 0x30;
  } UField;
  struct {
    uint16 SuperStruct = 0x48;
    uint16 Children = 0x50;
    uint16 ChildProperties = 0x58;
    uint16 PropertiesSize = 0x60;
  } UStruct;
  struct {
    uint16 Names = 0x48;
  } UEnum;
  struct {
    uint16 FunctionFlags = 0xB8;
    uint16 Func = 0xB8 + 0x28;
  } UFunction;
  struct {
    uint16 Class = 0x8;
    uint16 Next = 0x20;
    uint16 Name = 0x28;
  } FField;
  struct {
    uint16 ArrayDim = 0x38;
    uint16 ElementSize = 0x3C;
    uint16 PropertyFlags = 0x40;
    uint16 Offset = 0x4C;
    uint16 Size = 0x80;
  } FProperty;
  struct {
    uint16 ArrayDim = 0;
    uint16 ElementSize = 0;
    uint16 PropertyFlags = 0;
    uint16 Offset = 0;
    uint16 Size = 0; // sizeof(UProperty)
  } UProperty;
} Core;
static_assert(sizeof(Core) == sizeof(Offsets));

struct {
  void* offsets; // address to filled offsets structure
  std::pair<const char*, uint32> names; // NamePoolData signature
  std::pair<const char*, uint32> objects; // ObjObjects signature
  std::function<bool(void*, void*)> callback;
  // Hardcoded offsets (0 = use signature scan)
  uint64 gnamesOffset;
  uint64 gobjectsOffset;
} engines[] = {
 
    {//DeathlyStillnessGame
        &Default,
        { "\x48\x8D\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\xC6\x05\x00\x00\x00\x00\x00\x0F\x10\x03\x4C\x8D\x44\x24\x00\x48\x8B\xC8\x48\x8D\x54\x24\x00", 36 }, //GName
        { "\x48\x8B\x05\x00\x00\x00\x00\x48\x8B\x0C\xC8\x48\x8D\x04\xD1\xEB", 17 }, //Gobject 
        nullptr,
        0, 0 // Use signature scan
    },
    {//Strinova
        &Default,
        { nullptr, 0 }, // Not used (hardcoded offset)
        { nullptr, 0 }, // Not used (hardcoded offset)
        nullptr,
        0x71BF300, // GNames offset (NamePoolData at 0x1471BF300)
        0x71D8850  // GObjects offset (ObjObjects at 0x1471D8850)
    }
};

std::unordered_map<std::string, decltype(&engines[0])> games = {
  {"DeathlyStillnessGame-Win64-Shipping", &engines[0]},
  {"SCUM", &engines[0]},
  {"Strinova-Win64-Shipping", &engines[1]}
};

STATUS EngineInit(std::string game, void* image) {

  auto it = games.find(game);
  if (it == games.end()) { return STATUS::ENGINE_NOT_FOUND; }

  auto engine = it->second;
  offsets = *(Offsets*)(engine->offsets);

  // Check if hardcoded offsets are provided
  if (engine->gnamesOffset && engine->gobjectsOffset) {
    fmt::print("Using hardcoded offsets for this game:\n");
    fmt::print("  GNames:   0x{:X}\n", engine->gnamesOffset);
    fmt::print("  GObjects: 0x{:X}\n", engine->gobjectsOffset);
    return EngineInitWithOffsets(engine->gnamesOffset, engine->gobjectsOffset);
  }

  void* names = nullptr; 
  void* objects = nullptr;

  uint8 found = 0;
  if (!engine->callback) {
    found |= 4;
  }

  IterateExSections(
    image,
    [&](void* start, void* end)->bool {
    if (!(found & 1)) if (names = FindPointer(start, end, engine->names.first, engine->names.second))  found |= 1;
    if (!(found & 2)) if (objects = FindPointer(start, end, engine->objects.first, engine->objects.second)) found |= 2;
    if (!(found & 4)) if (engine->callback(start, end)) found |= 4;
    if (found == 7) return 1;
    return 0;
    }
  );

  if (found != 7) return STATUS::ENGINE_FAILED;

  NamePoolData = *(decltype(NamePoolData)*)names;
  ObjObjects = *(decltype(ObjObjects)*)objects;

  auto entry = UE_FNameEntry(NamePoolData.GetEntry(0));

  // exception handler exclusively for Decrypt_ANSI
  try {
    if (*(uint32*)entry.String().data() != 'enoN') return STATUS::ENGINE_FAILED;
  }
  catch (...) {
    return STATUS::ENGINE_FAILED;
  }

  return STATUS::SUCCESS;
}

STATUS EngineInitWithOffsets(uint64 gnamesOffset, uint64 gobjectsOffset) {
  // Use default offsets
  offsets = *(Offsets*)(&Default);

  // Calculate actual addresses from base + offset
  void* names = (void*)(Base + gnamesOffset);
  void* objects = (void*)(Base + gobjectsOffset);

  fmt::print("Using manual offsets:\n");
  fmt::print("  GNames:   Base + 0x{:X} = 0x{:X}\n", gnamesOffset, (uint64)names);
  fmt::print("  GObjects: Base + 0x{:X} = 0x{:X}\n", gobjectsOffset, (uint64)objects);

  // Validate GNames
  fmt::print("Validating GNames...\n");
  NamePoolData = Read<decltype(NamePoolData)>(names);
  
  // Check if NamePoolData looks valid
  if (NamePoolData.CurrentBlock > 8192 || NamePoolData.CurrentByteCursor == 0) {
    fmt::print("Error: GNames offset is invalid!\n");
    fmt::print("  CurrentBlock: {} (should be < 8192)\n", NamePoolData.CurrentBlock);
    fmt::print("  CurrentByteCursor: {} (should be > 0)\n", NamePoolData.CurrentByteCursor);
    return STATUS::ENGINE_FAILED;
  }

  auto entry = UE_FNameEntry(NamePoolData.GetEntry(0));

  try {
    auto str = entry.String();
    if (str.empty() || *(uint32*)str.data() != 'enoN') {
      fmt::print("Error: GNames offset is invalid! First name entry is not 'None', got: '{}'\n", str);
      return STATUS::ENGINE_FAILED;
    }
  }
  catch (...) {
    fmt::print("Error: GNames offset is invalid! Failed to read first name entry.\n");
    return STATUS::ENGINE_FAILED;
  }
  fmt::print("  GNames validated OK (first entry = 'None')\n");

  // Validate GObjects
  fmt::print("Validating GObjects...\n");
  ObjObjects = Read<decltype(ObjObjects)>(objects);
  
  // Check if ObjObjects looks valid
  if (ObjObjects.NumElements == 0 || ObjObjects.NumElements > 10000000) {
    fmt::print("Error: GObjects offset is invalid!\n");
    fmt::print("  NumElements: {} (should be > 0 and < 10000000)\n", ObjObjects.NumElements);
    return STATUS::ENGINE_FAILED;
  }
  
  if (ObjObjects.Objects == nullptr) {
    fmt::print("Error: GObjects offset is invalid! Objects pointer is null.\n");
    return STATUS::ENGINE_FAILED;
  }

  // Try to read first object
  auto firstObj = ObjObjects.GetObjectPtr(0);
  if (firstObj == nullptr) {
    fmt::print("Error: GObjects offset is invalid! Cannot read first object.\n");
    return STATUS::ENGINE_FAILED;
  }
  
  fmt::print("  GObjects validated OK (NumElements = {})\n", ObjObjects.NumElements);

  fmt::print("All offsets validated successfully!\n");
  return STATUS::SUCCESS;
}
