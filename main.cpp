#include "crud.hpp"

// enable virtual terminal processing on windows to be able to use ANSI escape sequences
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
#    define WINDOWS_USED
#    include <windows.h>
#endif

int main()
{
#ifdef WINDOWS_USED
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE || hOut != NULL) {
        DWORD dwMode = 0;
        GetConsoleMode(hOut, &dwMode);
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);
    } else {
        std::cerr << "Error: Unable to get console handle.\n";
    }
#endif

    Crud crud;
    crud.run();

    return 0;
}
