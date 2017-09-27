// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__526E3320_2AFF_4751_A65B_F5FE14E3B851__INCLUDED_)
#define AFX_STDAFX_H__526E3320_2AFF_4751_A65B_F5FE14E3B851__INCLUDED_

#pragma once

// Change these values to use different versions
#define WINVER          0x0501
#define _WIN32_WINNT    0x0501
#define _WIN32_IE       0x0400
#define _RICHEDIT_VER   0x0100
#define _CRT_SECURE_NO_WARNINGS

// #define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN          // Exclude rarely-used stuff from Windows headers

// #define _WTL_USE_CSTRING
#define _WTL_NO_CSTRING

// #include <atlbase.h>
#include <atlstr.h>

#include <atlapp.h>
//~ extern CAppModule _Module;

#include <vector>

// output a debug message to windows debug channel
void __cdecl odprintf(const char *format, ...) {
    TCHAR    buf[4096], *p = buf;
    va_list args;
    int     n;

    va_start(args, format);
    // n = _vstprintf(p, format, args);
    n = _vsnprintf(p, sizeof buf - 3, format, args); // buf-3 is room for CR/LF/NUL
    va_end(args);

    p += (n < 0) ? sizeof buf - 3 : n;

    while ( p > buf  &&  isspace(p[-1]) )
            *--p = '\0';

    *p++ = '\r';
    *p++ = '\n';
    *p   = '\0';

    OutputDebugString(buf);
}

// split a string into a 1d vector
// this function will strip leading and consecutive separators
std::vector<CString> CString_split(CString strInput, CString strDelim = _T(";")) {
    std::vector<CString> aRet;
    int nTokenPos = 0;
    CString strToken = strInput.Tokenize(strDelim, nTokenPos);
    while (!strToken.IsEmpty()) {
        aRet.push_back(strToken);
        strToken = strInput.Tokenize(strDelim, nTokenPos);
    }
    return(aRet);
}

// Split a 2d table
// uses a default of ; for record separator and , for field separator
std::vector<std::vector<CString>> CString_split(std::vector<CString> aInput, CString strDelim = _T(",")) {
    std::vector<std::vector<CString>> aRet;
    for(size_t i=0;i < aInput.size();i++) {
        aRet.push_back(CString_split(aInput[i], strDelim));
    }
    return(aRet);
}

// filter a list of file names 
// returns only those which extension is in a hardcoded list
template <class T>
std::vector<T> VectorFilterSufix(std::vector<T> vInput, std::vector<T> vExt) {
    std::vector<T> vRet;
    
    // use the the extensions as the priority list
    for(auto j = vExt.begin(); j != vExt.end();++j) {
        for (auto i = vInput.begin(); i != vInput.end();++i) {
            // compare the extension of the name with the current test
            if(i->Right(j->GetLength()).CompareNoCase(*j) == 0) {
                vRet.push_back(*i);
            }
        }
    }
    return(vRet);
};

// return a list of all client script files 
template <class T>
T GetClientScriptPath() {
    std::vector<T> vExt;
    // to add a new supported file, insert a new line here with the file extension
    // the order is important. the first matched file will be used
    // Python
    vExt.push_back(T("PY"));
    // Perl
    vExt.push_back(T("LAVA"));
    vExt.push_back(T("PL"));
    // Windows Batch
    vExt.push_back(T("BAT"));
    vExt.push_back(T("CMD"));
    // Unix C Shell
    vExt.push_back(T("CSH"));
    // Custom DLL with a run function
    vExt.push_back(T("DLL"));
    // HTML application
    vExt.push_back(T("HTA"));
    // Windows Script Host
    vExt.push_back(T("VBS"));
    vExt.push_back(T("JS"));
    auto vFilter = VectorFilterSufix(FileSearch<T>(NULL, GetModuleName()), vExt);
    if(vFilter.size() > 0) {
        // pick the first matched file
        return(vFilter.front());
    }
    return(T());
}

// return a list of all help files 
template <class T>
T GetHelpFilePath() {
    std::vector<T> vExt;
    vExt.push_back(T("CHM"));
    vExt.push_back(T("HLP"));
    vExt.push_back(T("PDF"));
    auto vFilter = VectorFilterSufix(FileSearch<T>(NULL, GetModuleName()), vExt);
    if(vFilter.size() > 0) {
        return(vFilter.front());
    }
    return(T());
}

// return the base name of the current running process
CString GetModuleName() {
    TCHAR szPath[MAX_PATH];
    GetModuleFileName(NULL, szPath, MAX_PATH );
    //~ LPTSTR szName = (LPTSTR) LocalAlloc(LMEM_ZEROINIT, MAX_PATH * sizeof(TCHAR));
    TCHAR szName[MAX_PATH];
    // split the path from the file name
    _tsplitpath(szPath, NULL, NULL, szName, NULL);
    return CString(szName);
}

/* return the directory where this executable resides
CString GetModuleDirectory() {
    // create the temporary buffers on the stack
    TCHAR szResult[MAX_PATH];
    TCHAR szPath[MAX_PATH];
    TCHAR szDir[MAX_PATH];

    // get the full path to this module
    GetModuleFileName(NULL, szPath, MAX_PATH );

    // use the CRT to split the path to the executable into individual components
    _tsplitpath(szPath, szResult, szDir, NULL, NULL);
    // concatenate the drive with the dir to obtain the complete path to dir
    lstrcat(szResult, szDir);

    return CString(szResult);
}
*/

// when a file resides in the current directory or above
// strip the current directory portion from the file path
CString MakePathRelative(LPCTSTR szPath) {
    CString strText(szPath);
    TCHAR szCurrentDir[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, szCurrentDir);
    if(PathIsPrefix(szCurrentDir, szPath)) {
        strText.Delete(0, lstrlen(szCurrentDir) + 1);
        
    }
    return(strText);
}

// convert a extension into a File Dialog buffer
// the return buffer is not a standard string since it
// will contain null bytes separating each extension
// the calling routine should to free the allocated memory
// or it will create a small leak, which may be aceptable 
LPTSTR CreateFileFilter(CString sExt) {
    //~ TCHAR strFilter[BUFSIZ];
    LPTSTR strFilter = (LPTSTR)LocalAlloc(LMEM_ZEROINIT, BUFSIZ);
    // we cant use str functions to generate the filters because of the null bytes in the data
    // so we have to resort to pointers and buffers
    TCHAR* lpstrFilter = strFilter;
    
    // create the specified wildcard filter using the user supplied extensions
    if(sExt.GetLength() > 0) {
        auto vExt = CString_split(sExt, _T(","));
        // the wildcard filter can be two equal zero terminated strings
        // one is the description, other is the actual filter
        for(int j=0;j<2;j++) {
            for(auto i = vExt.begin();i != vExt.end();++i) {
                // multiple extensions can be separated by semicollon
                if(i != vExt.begin()) {
                    CopyMemory(lpstrFilter, _T(";"), 1);
                    lpstrFilter++;
                }
                CopyMemory(lpstrFilter, _T("*."), 2);
                lpstrFilter += 2;
                CopyMemory(lpstrFilter, *i, i->GetLength() + 1);
                lpstrFilter += i->GetLength();
            }
            lpstrFilter++;
        }
    }
    // always have the all files option on the wildcards
    CopyMemory(lpstrFilter, _T("All Files (*.*)\0*.*\0\0"), sizeof(TCHAR) * 21);
    return(strFilter);
}

// Format a readable error message, display a message box, 
// and exit from the application.
void ErrorExit(LPCTSTR lpszFunction = _T("Function"), WORD nIcon = MB_ICONERROR) { 
    LPVOID lpMsgBuf;
    //~ LPVOID lpDisplayBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    LPTSTR lpDisplayBuf = (LPTSTR)LocalAlloc(LMEM_ZEROINIT, (lstrlen((LPCTSTR)lpMsgBuf)+lstrlen((LPCTSTR)lpszFunction)+40)*sizeof(TCHAR)); 
    //~ _sntprintf((LPTSTR)lpDisplayBuf, LocalSize(lpDisplayBuf) / sizeof(TCHAR), TEXT("%s failed with error %d: %s"), lpszFunction, dw, lpMsgBuf); 
    wsprintf((LPTSTR)lpDisplayBuf, TEXT("%s failed with error %d: %s"), lpszFunction, dw, lpMsgBuf); 
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), nIcon); 

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    if(nIcon == MB_ICONERROR) ExitProcess(1);
}

// if the expression evaluates to FALSE, terminate the program with a error message
void CheckExit(BOOL bCheck) {
    if(!bCheck) {
        ErrorExit(GetModuleName());
    }
}
// if the expression evaluates to FALSE, show a message to the user but dont exit
void CheckWarn(BOOL bCheck) {
    if(!bCheck) {
        ErrorExit(GetModuleName(), MB_ICONEXCLAMATION);
    }
}

#include <atlfile.h>
// read a entire file into a buffer
LPSTR ReadFile(LPCTSTR szFilename) {
    CAtlFile oFile;
    CheckWarn(SUCCEEDED(oFile.Create(szFilename, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING)));
    
    ULONGLONG nFileSize;
    CheckWarn(SUCCEEDED(oFile.GetSize(nFileSize)));
    // allocate a buffer that will hold the entire file plus a zero byte to terminate the string
    LPSTR pBuffer = (LPSTR)LocalAlloc(LMEM_ZEROINIT, static_cast<ULONG>(nFileSize) + 1);
    ULONG nBytesRead = 0;
    ULONG nTotalRead = 0;
    // read chunks from the file up to the file size
    while(SUCCEEDED(oFile.Read(pBuffer + nTotalRead, static_cast<DWORD>(nFileSize - nTotalRead), nBytesRead)) && nTotalRead < nFileSize) {
        nTotalRead += nBytesRead;
    }
    return(pBuffer);
}

// check if two rectangles overlap
//~ if (RectA.X1 < RectB.X2 && RectA.X2 > RectB.X1 &&
    //~ RectA.Y1 < RectB.Y2 && RectA.Y2 > RectB.Y1) 
template <class T>
bool rectOverlap(T RectA, T RectB) {
    if (RectA.left > RectB.right || RectA.right < RectB.left ||
        RectA.top > RectB.bottom || RectA.bottom < RectB.top) 
        return(FALSE);
    return(TRUE);
}

// for a directory, return a list of files that match the given file prefix
// input:
// diretory to search (Ex.: C:\\data
// string with prefix of files to search (Ex.: backup_data_)
// boolean value indicating if the search should recurse to subfolders
template <class T>
std::vector<T> FileSearch( LPCTSTR lpszWorkdir = NULL, LPCTSTR lpszPattern = NULL, BOOL bRecursive = FALSE ) {
    std::vector<T> vRet;
    CString strWorkdir = lpszWorkdir;
    if( !strWorkdir.IsEmpty() && (strWorkdir.Right(1) != '\\')  )
        strWorkdir += "\\";

    WIN32_FIND_DATA file_data = {0};
    HANDLE hFile = FindFirstFile( strWorkdir + "*", &file_data );
    
    if( hFile != INVALID_HANDLE_VALUE ) {
        TCHAR sBuffer[MAX_PATH];

        do {
            // concatenate the base folder with the current file
            lstrcpy(sBuffer, strWorkdir);
            lstrcat(sBuffer, file_data.cFileName);
            if( bRecursive && file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
            {
                // recurse to subfolders
                if( lstrcmp(file_data.cFileName, _T(".")) != 0 && lstrcmp(file_data.cFileName, _T("..")) != 0 ) {
                    auto vRec = FileSearch<T>( sBuffer, lpszPattern, bRecursive );
                    vRet.insert(vRet.end(), vRec.begin(), vRec.end());
                }
            } else {
                if( (file_data.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)) == 0 ) {
                    // check if the current file matches the pattern, if any was given
                    if(lpszPattern == NULL || _strnicmp(file_data.cFileName, lpszPattern, lstrlen(lpszPattern)) == 0) {
                        //lstrcmpi
                        vRet.push_back(T(sBuffer));
                    }
                }
            }
        } while( FindNextFile( hFile, &file_data ) );
        FindClose( hFile );
    }

    return(vRet);
}

// if a string contains a space or is empty, quote it with double quotes
template <class T>
T QuoteString(T strText) {
    if(strText.IsEmpty() || strText.FindOneOf(_T(" ;&|*<>\\")) > 0) {
        strText.Insert(0, '"');
        strText += '"';
    }
    return strText;
}

// set the windows clipboard to a given string
// return non-zero on error
WORD SetClipboardCString(HWND hWnd, CString strData) {
    // lock the clipboard to our thread
    if(! ::OpenClipboard(hWnd)) 
        return(EXIT_FAILURE); 

    // we only really get ownership of the clipboard when we empty it
    if(! EmptyClipboard())
        return(EXIT_FAILURE);
    
    // copy the CString data into a global buffer
    HGLOBAL hGlobal = GlobalAlloc(GMEM_ZEROINIT|GMEM_MOVEABLE|GMEM_DDESHARE, (strData.GetLength()+1)*sizeof(TCHAR));
    //HGLOBAL hGlobal = GlobalAlloc(GMEM_NODISCARD | GMEM_ZEROINIT, (strData.GetLength()+1)*sizeof(TCHAR));
    
    LPTSTR lpszBuf = static_cast<LPTSTR>(::GlobalLock(hGlobal));
    // lstrcpy((LPTSTR)hClipboardData,strData);
    lstrcpy(lpszBuf, strData);
    // set the clipboard to the value on the burffer
    SetClipboardData(CF_TEXT, hGlobal);
    // clean up
    GlobalUnlock(hGlobal);
    GlobalFree(hGlobal);
    CloseClipboard();

    return(EXIT_SUCCESS);
}

// return the FileUsage property of a DLL
template <class T>
T GetFileVersion( T pFilePath ) {
    DWORD               dwSize              = 0;
    BYTE                *pVersionInfo       = NULL;
    VS_FIXEDFILEINFO    *pFileInfo          = NULL;
    UINT                pLenFileInfo        = 0;
    
    /*getting the file version info size */
    dwSize = GetFileVersionInfoSize( pFilePath, NULL );
    if ( dwSize != 0 ) {
        // odprintf( "Error in GetFileVersionInfoSize: %d\n", GetLastError() );
        pVersionInfo = new BYTE[ dwSize ]; /*allocation of space for the verison size */

        /*entering all info     data to pbVersionInfo*/
        if ( GetFileVersionInfo( pFilePath, 0, dwSize, pVersionInfo ) 
        && VerQueryValue( pVersionInfo, _T("\\StringFileInfo\\040904b0\\FileDescription"), (LPVOID*) &pFileInfo, &pLenFileInfo ) ) {
            // odprintf("pFileInfo %s", pFileInfo);
            // success! pFileInfo now contains the result
        } else {
            // EXIT_FAILURE! clean up buffer
            delete[] pVersionInfo;
            pFileInfo = NULL;
        }
    }
 
    return (T)pFileInfo;
}


// worker function pattern
typedef DWORD(CALLBACK *prun)(LPCSTR);

// calls a compatible worker dll
// the dll must have as its first exported function
// a prototype that accepts a single LPCSTR and returns a DWORD
template <class T>
T RunDLL(LPCSTR lpszPath, LPCSTR lpszArg) {
  HINSTANCE hGetProcIDDLL = LoadLibrary(lpszPath);
  // odprintf("RunDLL lpszPath %s lpszArg %s", lpszPath, lpszArg);

  if (!hGetProcIDDLL) {
    // odprintf("could not load the dynamic library");
    return EXIT_FAILURE;
  }

  // resolve function address here
  // GetProcAddress usualy receives a LPCSTR with function name
  // but it can also receive a DWORD with a index
  auto fptr = GetProcAddress(hGetProcIDDLL, reinterpret_cast<LPCSTR>(1));
  if (!fptr) {
    // odprintf("could not locate the function");
    return EXIT_FAILURE;
  }
  // cast the generic functino pointer to the standard worker function call defined in worker.h
  auto pRun = reinterpret_cast<prun>(fptr);

  return(pRun(_T(lpszArg)));
}


#endif // !defined(AFX_STDAFX_H__526E3320_2AFF_4751_A65B_F5FE14E3B851__INCLUDED_)
