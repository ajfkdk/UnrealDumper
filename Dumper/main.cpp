#include <fmt/core.h>
#include <cstdlib>
#include "dumper.h"
#include "utils.h"

// Pause before exit to let user see error messages
int exit_with_pause(int code) {
    if (code != 0) {
        puts("\nPress any key to exit...");
        system("pause > nul");
    }
    return code;
}

int main(int argc, char* argv[])
{
    uint64 start;
    uint64 end;
    uint64 time;

    auto dumper = Dumper::GetInstance();

    start = GetTime();
    switch (dumper->Init(argc, argv))
    {
    case STATUS::WINDOW_NOT_FOUND: { puts("Can't find UE4 window"); return exit_with_pause(1); }
    case STATUS::PROCESS_NOT_FOUND: { puts("Can't find process"); return exit_with_pause(1); }
    case STATUS::READER_ERROR: { puts("Can't init reader"); return exit_with_pause(1); }
    case STATUS::CANNOT_GET_PROCNAME: { puts("Can't get process name"); return exit_with_pause(1); }
    case STATUS::ENGINE_NOT_FOUND: { puts("Can't find offsets for this game"); return exit_with_pause(1); }
    case STATUS::ENGINE_FAILED: { puts("Can't init engine for this game"); return exit_with_pause(1); }
    case STATUS::MODULE_NOT_FOUND: { puts("Can't enumerate modules (protected process?)"); return exit_with_pause(1); }
    case STATUS::CANNOT_READ: { puts("Can't read process memory"); return exit_with_pause(1); }
    case STATUS::INVALID_IMAGE: { puts("Can't get executable sections"); return exit_with_pause(1); }
    case STATUS::SUCCESS: { break; };
    default: { puts("Unknown init error"); return exit_with_pause(1); }
    }
    end = GetTime();
    time = (end - start) / 10000;
    fmt::print("Init time: {} ms\n", time);

    start = GetTime();
    switch (dumper->Dump())
    {
    case STATUS::FILE_NOT_OPEN: { puts("Can't open file"); return exit_with_pause(1); }
    case STATUS::ZERO_PACKAGES: { puts("Size of packages is zero"); return exit_with_pause(1); }
    case STATUS::SUCCESS: { break; }
    default: { puts("Unknown dump error"); return exit_with_pause(1); }
    }
    end = GetTime();
    time = (end - start) / 10000;
    fmt::print("Dump time: {} ms\n", time);

    puts("\nDone! Press any key to exit...");
    system("pause > nul");
    return 0;
}
