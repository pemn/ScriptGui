/*
    clientscript

    hosts classes which handle the client script usage and execution

    Copyright 2014 Vale
 
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0
*/


// we cant use the preprocessor to store strings, so we use actual variables
const CString TOKEN_BASE(_T(":"));
const CString TOKEN_DATA(_T("="));
const CString TOKEN_FILE(_T("*"));
const CString TOKEN_BOOL(_T("@"));
const CString TOKEN_GRID(_T("#"));

// class to wrap everything about our client script
class ClientScript {
public:
    STARTUPINFO m_si;
    PROCESS_INFORMATION m_pi;

    HANDLE g_hChildStd_OUT_Rd;
    HANDLE g_hChildStd_OUT_Wr;
    // creation flags
    DWORD m_flag;

    CString m_output;

    //~ std::vector<CString> m_files;
    CString m_script;
    CString m_exe;
    // flag to skip vulcan specific startup
    BOOL m_vulcan;

    ClientScript(CString sScript = _T(""), CString sExe = _T(""), BOOL bCaptureOutput = FALSE) : m_flag(0), m_vulcan(1) {
        ZeroMemory( &m_si, sizeof(m_si) );
        m_si.cb = sizeof(m_si);
        ZeroMemory( &m_pi, sizeof(m_pi) );
        
        if(bCaptureOutput) {
            m_flag = CREATE_NO_WINDOW;
            
            SECURITY_ATTRIBUTES saAttr; 

            // Set the bInheritHandle flag so pipe handles are inherited. 
            saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
            saAttr.bInheritHandle = TRUE; 
            saAttr.lpSecurityDescriptor = NULL; 

            // Create a pipe for the child process's STDOUT. 
            if ( ! CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0) ) 
                ErrorExit(_T("StdoutRd CreatePipe"));

            // Ensure the read handle to the pipe for STDOUT is not inherited.
            if ( ! SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0) )
                ErrorExit(_T("Stdout SetHandleInformation"));
            // set the I/O handles
            m_si.hStdError = g_hChildStd_OUT_Wr;
            m_si.hStdOutput = g_hChildStd_OUT_Wr;
            m_si.dwFlags |= STARTF_USESTDHANDLES;
        }

        m_script = sScript;
        
        m_exe = sExe;
    }

	int SetScript(CString sScript = _T(""), CString sExe = _T("")) {
        m_script = sScript;
        // detect script path from this executable path
        if(m_script.IsEmpty()) {
            m_script = GetClientScriptPath<CString>();
            if(m_script.IsEmpty()) {
                return(EXIT_FAILURE);
            }
        }
		m_exe = sExe;
        // detect script executable from the script extension
        if(m_exe.IsEmpty()) {
            if(m_script.Right(3).CompareNoCase(_T("csh")) == 0) {
                m_exe = _T("csh");
            } else if(sScript.Right(2).CompareNoCase(_T("pl")) == 0 
            || m_script.Right(4).CompareNoCase(_T("lava")) == 0) {
                m_exe = _T("perl");
            } else if(sScript.Right(2).CompareNoCase(_T("js")) == 0 
            || m_script.Right(3).CompareNoCase(_T("vbs")) == 0) {
                m_exe = _T("cscript /nologo");
            // } else if(sScript.Right(3).CompareNoCase(_T("dll")) == 0 ) {
            } else { // default to windows command
                m_exe = _T("cmd /c");
                m_vulcan = 0;
            }
        }
		return(EXIT_SUCCESS);
	}
    
    CString BuildCommandLine(CString sArguments = _T("")) {
        CString sCmd;
        
        sCmd.Format(_T("%s %s %s"), m_exe, m_script, sArguments);
        return(sCmd);
    }
    
    int Run(CString sArguments) {
        if(sArguments.IsEmpty()) {
            return(TRUE);
        }
        if(m_script.Right(3).CompareNoCase(_T("dll")) == 0) {
            return(RunDLL<int>(m_script, sArguments));
        }
        return(RunCmd(BuildCommandLine(sArguments)));
    }
        
    int RunCmd(CString sCmd) {
        // retrieve vulcan path and set variables
        if(m_vulcan) {
            PrepareVulcanEnviroment();
        }

        // Start the child process. 
        if( !CreateProcess( NULL,   // No module name (use command line)
            sCmd.GetBuffer(),         // Command line
            NULL,           // Process handle not inheritable
            NULL,           // Thread handle not inheritable
            TRUE,           // handles are inherited 
            m_flag,         // Creation flags
            NULL,           // Use parent's environment block
            NULL,           // Use parent's starting directory 
            &m_si,          // Pointer to STARTUPINFO structure
            &m_pi )         // Pointer to PROCESS_INFORMATION structure
        ) 
        {
            ErrorExit( "CreateProcess" );
            return(TRUE);
        }
        sCmd.ReleaseBuffer();
        // Wait until child process exits.
        WaitForSingleObject( m_pi.hProcess, INFINITE );

        CloseHandle(g_hChildStd_OUT_Wr);
        
        ReadFromPipe(); 
        
        return(FALSE);
    }
    
    ~ClientScript() {
        // Close process and thread handles. 
        CloseHandle( m_pi.hProcess );
        CloseHandle( m_pi.hThread );
    }
    
    BOOL ReadFromPipe() {
        // Read output from the child process's pipe for STDOUT
        // and write to the parent process's pipe for STDOUT. 
        // Stop when there is no more data. 
        DWORD dwRead; 
        CHAR chBuf[BUFSIZ]; 
        BOOL bSuccess = FALSE;

        for (;;) { 
            ZeroMemory( &chBuf, BUFSIZ );
            bSuccess = ReadFile( g_hChildStd_OUT_Rd, chBuf, BUFSIZ - 1, &dwRead, NULL);
            if( ! bSuccess || dwRead == 0 ) break; 
            
            m_output += chBuf;
        } 
        return FALSE;
    }
    
    // ask the user to select the directory where vulcan is installed
    // return 0 on cancel or failure, 1 on sucess
    int PickVlauncher(LPTSTR szPath) {
		// multi-use buffer for directory paths
		TCHAR szDir[MAX_PATH];
		// the first use is to store the current directory
		// since the CFileDialog most probably will change it!
		GetCurrentDirectory(MAX_PATH, szDir);

        CFileDialog dlg(TRUE, NULL, _T("vlauncher.exe"), OFN_FILEMUSTEXIST, _T("vlauncher.exe\0vlauncher.exe\0"));
        if(dlg.DoModal() == IDCANCEL) {
            return(FALSE);
        }
        // restore the working directory
		SetCurrentDirectory(szDir);

        // use the CRT to split the path to the executable into individual components
        _tsplitpath(dlg.m_ofn.lpstrFile, szPath, szDir, NULL, NULL);

        // concatenate the drive with the dir to obtain the complete path to dir
        lstrcat(szPath, szDir);
        return(TRUE);
    }
    
    // retrieve the vulcan_exe path either from the enviroment varible
    // or from a user panel
    // returns TRUE on failure
    // returns FALSE on success
    BOOL PrepareVulcanEnviroment() {
        // create a big generic buffer which will be reused on many operations
        TCHAR szBuffer[MAX_PATH * 10];
        CString strVulcan;
        TCHAR* szSearch[] = {_T("ProgramFiles"),_T("ProgramFiles(x86)"),_T("ProgramW6432"),_T("APPDATA")};

        for(int nTry = 0;nTry <= 7;nTry++) {
            // check if the enviroment variable VULCAN_EXE is defined
            // this usually means we are a child process within a Vulcan Enviroment
            if(nTry == 0
            && GetEnvironmentVariable(_T("VULCAN_EXE"), szBuffer, MAX_PATH) > 0) {
                strVulcan = szBuffer;
            }
            
            // check if the vulcan_exe was stored in the persistence XML
            if(nTry == 1 
            && g_xmlmap.find(_T("vulcan_exe")) != g_xmlmap.end()) {
                // retrieve the stored vulcan_exe
                strVulcan = g_xmlmap["vulcan_exe"];
            }
            
            // check if we have a vulcan directory on any directory contained on the PATH
            if(nTry == 2
            && SearchPath(NULL, _T("vlauncher.exe"), NULL, MAX_PATH, szBuffer, NULL)) {
                // remove the vlauncher.exe leaving the path to parent directory
                szBuffer[lstrlen(szBuffer) - 14] = 0;
                strVulcan = szBuffer;
            }
            
            // search default ProgramFiles for a vulcan installation 
            // then search 32 bit program files
            // then search 64 bit program files
            // then search APPDATA
            // 3,4,5,6
            if(nTry >= 3 && nTry <= 6) {
                // copy the program files directory path into the buffer
                TCHAR szPath[MAX_PATH];
                GetEnvironmentVariable(szSearch[nTry - 3], szPath, MAX_PATH);
                // convert path to short form to avoid some glitches with vulcan csh startup scripts
                GetShortPathName(szPath, szBuffer, MAX_PATH);
                
                // search recursively for the vlauncher.exe, which is the vulcan entry point
                auto vFound = FileSearch<CString>(szBuffer, "vlauncher.exe", TRUE);
                if(vFound.size() > 0) {
                    // use the last found file, which we presume is the latest version
                    // get the path to the parent directory of vlauncher.exe and set the variable
                    strVulcan =  vFound.back().Left(vFound.back().GetLength() - 14);
                }
            }

            // ask user to pick the vulcan directory
            if(nTry == 7 
            && PickVlauncher(szBuffer)) {
                // convert path to short form to avoid some glitches with vulcan csh startup scripts
                GetShortPathName(szBuffer, strVulcan.GetBuffer(MAX_PATH), MAX_PATH);
                strVulcan.ReleaseBuffer();
            }
            // check if the last try was able to find a valid vulcan_exe path
            if((!strVulcan.IsEmpty()) && PathFileExists(strVulcan)) {
                // set the variable to the found path
                SetEnvironmentVariable(_T("VULCAN_EXE"), strVulcan);
                break;
            }
        }

        if(strVulcan.IsEmpty()) {
            return(0);
        }
        // cache the vulcan_exe path into the persistence map so the next time we run its much faster
        g_xmlmap["vulcan_exe"] = strVulcan;
        
        // convert VULCAN_EXE into VULCAN_BIN
        strVulcan = strVulcan.Left(strVulcan.GetLength() - 4);
        // create the vulcan_bin pointing to vulcan base directory
        if(GetEnvironmentVariable(_T("VULCAN_BIN"), szBuffer, MAX_PATH) == 0) {
            SetEnvironmentVariable(_T("VULCAN_BIN"), strVulcan);
        }
        
        // convert VULCAN_BIN into VULCAN
        strVulcan = strVulcan.Left(strVulcan.GetLength() - 4);
        // create the vulcan_bin pointing to vulcan base directory
        if(GetEnvironmentVariable(_T("VULCAN"), szBuffer, MAX_PATH) == 0) {
            SetEnvironmentVariable(_T("VULCAN"), strVulcan);
        }
        // create the PERLLIB variable pointing to perk lib directory inside vulcan
        if(GetEnvironmentVariable(_T("PERLLIB"), szBuffer, MAX_PATH) == 0) {
            lstrcpy(szBuffer, strVulcan);
            lstrcat(szBuffer, _T("\\lib\\perl;"));
            lstrcat(szBuffer, strVulcan);
            lstrcat(szBuffer, _T("\\lib\\perl\\site\\lib;"));
            SetEnvironmentVariable(_T("PERLLIB"), szBuffer);
        }
        // ensure we have very large triangulation dynamic settings
        //$ENV{ENVIS_MAXTRI} = 12000000;
        //$ENV{ENVIS_MAXTRP} = 12000000;

        // if we dont have any vulcan directory on the path, add it
        if(SearchPath(NULL, _T("vlauncher.exe"), NULL, MAX_PATH, szBuffer, NULL) == 0) {
            // create a custom PATH, with bare minimum search locations
            ExpandEnvironmentStrings(_T("%VULCAN_EXE%;%vulcan_bin%\\oda;%VULCAN_BIN%\\cygnus\\bin;%PATH%"),szBuffer,MAX_PATH * 10);
            SetEnvironmentVariable(_T("PATH"), szBuffer);
        }
        
        return(0);
    }
    
    // return the output from the external script as a vector of lines
    std::vector<CString> get() {
        return(CString_split(m_output, _T("\r\n")));
    }
    CString GetUsage() {
        if(!PathFileExists(m_script)) {
            return(_T(""));
        } else if(m_script.Right(3).CompareNoCase(_T("dll")) == 0) {
            return(GetFileVersion<LPTSTR>(m_script.GetBuffer()));
        } else {
            return(ParseUsage());
        }
    }

    // retrieve the list of arguments from the client script
    CString ParseUsage() {
        auto lpBuffer = ReadFile(m_script.GetBuffer());
        CString csFile(lpBuffer);
        LocalFree(lpBuffer);
        int nPos = csFile.MakeLower().Find(_T("usage: "));
        if(nPos != -1) {
            // after the "usage:" we have the script name
            // what we want is everything after that
            csFile = csFile.Mid(nPos + 7);
            // find the first space after the script name and grab everything after that
            csFile = csFile.Mid(csFile.Find(_T(" "))+1);
            // search for the end of the quoted string or a end of line
            int nPos = csFile.FindOneOf(_T("\'\"\r\n"));
            
            if(nPos != -1) {
                return(csFile.Left(nPos));
            }
        }
        return(_T(""));
    }

};


