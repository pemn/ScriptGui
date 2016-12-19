// boilerplate for a worker dll
// used by wtlscriptgui as a runnable dll target
// Copyright 2016 Vale

// #include <tchar.h>
#include <stdio.h>

template <class T, class U>
T run (U strArg) {
    // return EXIT_FAILURE;
    printf("strArg %s\n", strArg);
    printf("Sleep 5\n");
    Sleep(5000);
    return EXIT_SUCCESS;
}

// C++ compatible DLL export
extern "C"
{
    // instantiate the template for link time export
    template __declspec(dllexport) DWORD CALLBACK run(LPCSTR);
}

// worker function pattern
// typedef DWORD(__stdcall *prun)(LPCSTR);
