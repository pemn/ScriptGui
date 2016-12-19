/*
    maindlg.h : interface of the CMainDlg class

    Copyright 2014 Vale

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0
*/


#if !defined(AFX_MAINDLG_H__4DAAD2C6_BE53_4E7A_BF77_C393A39420D3__INCLUDED_)
#define AFX_MAINDLG_H__4DAAD2C6_BE53_4E7A_BF77_C393A39420D3__INCLUDED_

#pragma once

#include "usagetoken.h"

// global persistence map
// used for save load of usageframe
// and for storing other settings like vulcan_exe path
#include "xmlmap.h"
XmlMap<CString, CString> g_xmlmap;

// create the clientscript global that handles the parameters defined by the detected client script
#include "clientscript.h"
ClientScript g_clientscript;

// dynamic control classes
#include "fontsymbol.h"
#include "CArgControl.h"
#include "usageframe.h"
#include "CArgListview.h"
#include "cargcontrol_factory.h"

// maindlg is our primary mainframe
class CMainDlg : public CDialogImpl<CMainDlg>, public CDialogResize<CMainDlg>
{
public:
    enum { IDD = IDD_MAINDLG };
    
    typedef CDialogImpl<CMainDlg> _baseClass;
    
    UsageFrame m_usage;
    CStatusBarCtrl m_status;
    // cache storing how much space is reserved to the side menu
    int m_logowidth;
  
    BEGIN_MSG_MAP(CMainDlg)
        //~ odprintf("CMainDlg uMsg %x wParam %x,%x lParam %x hWnd %x", uMsg, HIWORD(wParam), LOWORD(wParam), lParam, hWnd);
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
        COMMAND_ID_HANDLER(ID_APP_RUN, OnAppRun)
        COMMAND_ID_HANDLER(ID_APP_MENU, OnAppMenu)
        COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
        COMMAND_ID_HANDLER(ID_APP_HELP, OnAppHelp)
        COMMAND_ID_HANDLER(ID_APP_LOAD, OnAppSaveLoad)
        COMMAND_ID_HANDLER(ID_APP_SAVE, OnAppSaveLoad)
        COMMAND_ID_HANDLER(ID_APP_CLEAR, OnAppSaveLoad)
        COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
        COMMAND_ID_HANDLER(IDOK, OnOK)
        CHAIN_MSG_MAP(CDialogResize<CMainDlg>)
        REFLECT_NOTIFICATIONS()
    END_MSG_MAP()

    BEGIN_DLGRESIZE_MAP(CMainDlg)
        DLGRESIZE_CONTROL(ID_APP_RUN, DLSZ_MOVE_X)
        DLGRESIZE_CONTROL(ID_APP_MENU, DLSZ_MOVE_X)
        DLGRESIZE_CONTROL(ID_APP_ABOUT, DLSZ_MOVE_X)
        DLGRESIZE_CONTROL(ID_APP_HELP, DLSZ_MOVE_X)
        DLGRESIZE_CONTROL(ID_APP_SAVE, DLSZ_MOVE_X)
        DLGRESIZE_CONTROL(ID_APP_LOAD, DLSZ_MOVE_X)
        DLGRESIZE_CONTROL(ID_APP_CLEAR, DLSZ_MOVE_X)
        DLGRESIZE_CONTROL(IDC_COMBO_XML, DLSZ_MOVE_X)
        DLGRESIZE_CONTROL(IDC_LOGO, DLSZ_MOVE_X)
    END_DLGRESIZE_MAP()
    
    // custom onsize
    // implements a layout manager behavior controling the size of the dynamic controls frame
    LRESULT OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
        // resize the dynamic controls, reserve the space for the status bar and borders
        m_usage.ResizeClient(LOWORD(lParam) - m_logowidth, HIWORD(lParam) - ::GetSystemMetrics(SM_CYSMSIZE) * 2, true);
        // update the status bar position
		if(m_status.IsWindow()) {
			m_status.MoveWindow(&rcDefault);
		}
        bHandled = FALSE;
        return 0;
    }
    
    // startup most of the dialog parameters,
    // including configuration of the fixed controls
    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
        // Init the CDialogResize code
        DlgResize_Init(true, true, WS_THICKFRAME | WS_CLIPCHILDREN);

        // set the window icon from the resource
        SetIcon((HICON)::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, NULL, NULL, LR_DEFAULTCOLOR), TRUE);

        // create the usageframe using a blank rectangle
        // the actual layout size will be done by the OnSize handler
        if (g_clientscript.SetScript() == EXIT_SUCCESS) {
            m_usage.Create(m_hWnd, CRect(::GetSystemMetrics(SM_CXBORDER),::GetSystemMetrics(SM_CYBORDER),0,0), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER, 0, IDC_USAGEFRAME);
        } else {
            MessageBox(_T("Cant find a script with the same name as this interface"), NULL, MB_ICONERROR);
            EndDialog(IDCANCEL);
        }

        // change the font of the load and save buttons so they now display symbols
        SendDlgItemMessage(ID_APP_LOAD ,WM_SETFONT, WPARAM(fontWingdings.m_hFont));
        SendDlgItemMessage(ID_APP_SAVE ,WM_SETFONT, WPARAM(fontWingdings.m_hFont));
        SendDlgItemMessage(ID_APP_CLEAR,WM_SETFONT, WPARAM(fontWingdings.m_hFont));
        SendDlgItemMessage(ID_APP_ABOUT,WM_SETFONT, WPARAM( fontWebdings.m_hFont));
        SendDlgItemMessage(ID_APP_MENU ,WM_SETFONT, WPARAM( fontWebdings.m_hFont));
        SendDlgItemMessage(ID_APP_HELP ,WM_SETFONT, WPARAM( fontWebdings.m_hFont));

        // resize the window vertically to encompass the actual height of the usage frame
        // but dont exceed 80% of screen height or the screen width minus window borders
        ResizeClient(min(::GetSystemMetrics(SM_CXVIRTUALSCREEN) - ::GetSystemMetrics(SM_CXFRAME) * 2, ::GetSystemMetrics(SM_CXSMSIZE) * 50), min(int(::GetSystemMetrics(SM_CYVIRTUALSCREEN) * 0.8), m_usage.m_sizeAll.cy + ::GetSystemMetrics(SM_CYSMSIZE) * 2), TRUE);
        
        // center the dialog on the screen
        CenterWindow();
        
        // set the window title to match the executable name
        SetWindowText(GetModuleName());
        
        // create the status bar
        m_status.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE);
        
        // calculate the width of the right side menu
        CRect rc;
        ::GetWindowRect(GetDlgItem(IDC_LOGO), rc);
        m_logowidth = rc.Width() + ::GetSystemMetrics(SM_CXSMSIZE);
        
        HRSRC hLogo = FindResource(NULL, MAKEINTRESOURCE(IDR_LOGO), "emf");

        //~ LPVOID pResData = LockResource(LoadResource(GetModuleHandle(NULL), hRes));

        SendDlgItemMessage(IDC_LOGO, STM_SETIMAGE, IMAGE_ENHMETAFILE, (LPARAM)::SetEnhMetaFileBits(SizeofResource(NULL, hLogo), (LPBYTE)LockResource(LoadResource(NULL, hLogo))));

        
        return TRUE;
    }
    
    // set the status bar text, and optional also display a OK message box
    void SetStatus(LPCSTR lpcstrText = _T(""), BOOL bMessage = FALSE) {
        m_status.SetText(ID_DEFAULT_PANE, lpcstrText);
        if(bMessage) {
            MessageBox(lpcstrText, GetModuleName(), MB_ICONINFORMATION);
        }
    }

    // handle the Load and Save buttons
    LRESULT OnAppSaveLoad(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        TCHAR sBuffer[MAX_PATH];
        //~ LPTSTR sBuffer = (LPTSTR) LocalAlloc(LMEM_ZEROINIT, 4096 * sizeof(TCHAR));
        GetDlgItemText(IDC_COMBO_XML, sBuffer, MAX_PATH);
        if(wID == ID_APP_LOAD) {
            m_usage.load(sBuffer);
            SetStatus(_T("Settings loaded"));
        }
        if(wID == ID_APP_SAVE) {
            m_usage.save(sBuffer);
            UpdateComboXml();
            SetStatus(_T("Settings saved"));
        }
        
        if(wID == ID_APP_CLEAR) {
            m_usage.clear(sBuffer);
            UpdateComboXml();
            SetStatus(_T("Settings cleared"));
        }
        
        return 0;
    }
    
    // populates the combo with xml files available to load
    void UpdateComboXml() {
		auto aList = g_xmlmap.GetSelectors();
        SendDlgItemMessage(IDC_COMBO_XML,CB_RESETCONTENT, 0, 0);
        for(auto i=aList.begin();i!=aList.end();++i) {
            SendDlgItemMessage(IDC_COMBO_XML,CB_ADDSTRING, 0, (LPARAM)i->GetBuffer());
        }
    }
    
    // copy the command line to the clipboard
    LRESULT OnAppMenu(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
		//WORD bResult = SetClipboardCString(m_hWnd, g_clientscript.BuildCommandLine(m_usage.get()));
		//if(bResult) {
		if(SetClipboardCString(m_hWnd, g_clientscript.BuildCommandLine(m_usage.get()))) {
		    MessageBox(_T("Failed to copy data into the clipboard"), NULL, MB_ICONERROR); 
        } else {
            SetStatus(_T("Command Line copied to clipboard"), TRUE);
        }

        return 0;
    }      
    // show the help
    LRESULT OnAppHelp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        // check for help files with same name as this executable
        auto strPath = GetHelpFilePath<CString>();
        if(!strPath.IsEmpty()) {
            // start the help file with the default program
            ShellExecute(NULL, _T("open"), strPath, NULL, NULL, SW_SHOWDEFAULT);
        } else {
            MessageBox(_T("Help file not found"), NULL, MB_ICONEXCLAMATION); 
            //~ AtlMessageBox(m_hWnd, _T("Help file not found"), NULL, MB_ICONEXCLAMATION); 
        }
        return 0;
    }    
    // show the about dialog, which is stored on the resources
    LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        CSimpleDialog<IDD_ABOUTBOX, FALSE> dlg;
        dlg.DoModal();
        return 0;
    }
    
    LRESULT OnAppRun(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		// save the current settings to a xml file
        m_usage.save();

        SetStatus(_T("Running script"));
        SetCursor(LoadCursor(NULL,IDC_WAIT)); // show buzy cursor
        g_clientscript.Run(m_usage.get());
        SetCursor(LoadCursor(NULL,IDC_ARROW)); // show normal cursor
        SetStatus(_T("Finished"));
        return 0;
    }

    // when user presses ENTER key in a dialog, a IDOK message will be create
    // even if we dont have any buttons with that id and the default behavior is close the entire dialog
    // we have to implement some more useful behavior
    LRESULT OnOK(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        // send a message to the window with focus telling it o give focus to its parent
        // if its a dynamic editor combo box inside a Listview grid
        // this will trigger the cell update procedure associated with it losing focus
        ::SendMessage(::GetFocus(), WM_KILLFOCUS, (WPARAM)::GetParent(::GetFocus()), NULL);
        return 0;
    }

    // whe user presses ESC key a IDCANCEL will be create even if we dont have a cancel button
    // this also happens when user clicks the close (X) button 
    LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
		// save the current settings to a xml file
        m_usage.save();
        // close the dialog
        EndDialog(wID);
        return 0;
    }
    
    // capture the show window event to do some one-time initialization
    // after the dialog was created
    LRESULT OnShowWindow(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
        m_usage.load();
        UpdateComboXml();
        return 0;
    }
};


#endif // !defined(AFX_MAINDLG_H__4DAAD2C6_BE53_4E7A_BF77_C393A39420D3__INCLUDED_)
