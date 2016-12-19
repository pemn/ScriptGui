/*
    entry point for WtlScriptGui
    initializes WTL and creates the app window

    Copyright 2014 Vale

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0
*/

#include "stdafx.h"

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlmisc.h>

#include "resource.h"

#include "maindlg.h"

//~ CAppModule _Module;

//~ int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR /*lpstrCmdLine*/, int /*nCmdShow*/)
int _tmain(/*int argc, wchar_t* argv[]*/)
{

    // If you are running on NT 4.0 or higher you can use the following call instead to 
    // make the EXE free threaded. This means that calls come in on a random RPC thread.
    //~ HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	::CoInitializeEx(NULL, COINIT_MULTITHREADED);
    //#// ::CoInitialize(NULL);
    
    //~ hRes = _Module.Init(NULL, hInstance);
    //~ AtlAxWinInit();
    
    INT_PTR nRet = 0;
    // NOTE: CMainDlg must be in this block.  Not doing so causes the
    // CMainDlg destructor to be called after _Module.Term(), which
    // leads to a crash in release builds built with _ATL_MIN_CRT defined.
    {
        CMainDlg dlgMain;
		//MessageBox(NULL, _T("Test"), NULL, MB_ICONINFORMATION);
        nRet = dlgMain.DoModal();
    }
    
    //~ _Module.Term();
    ::CoUninitialize();
    return(static_cast<int>(nRet));
}
