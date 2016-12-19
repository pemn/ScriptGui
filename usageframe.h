/*
    UsageFrame.h : child frame to host the client script options

    Copyright 2014 Vale

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0
*/

#pragma once

#include <atlscrl.h>

#define USAGE_ROW_MINHEIGHT    long(::GetSystemMetrics(SM_CYSMSIZE) * 2)

// --------------------------------------------------------------
// Container class for the data-drive usage controls
// reads the executable name for the script name which contains
// our control construction string
class UsageFrame : public CScrollWindowImpl<UsageFrame>
{
public:
    typedef CScrollWindowImpl<UsageFrame> _baseClass;

    DECLARE_WND_CLASS_EX(NULL,0,COLOR_MENU)

    CEdit m_about;
    int m_about_height;

    virtual LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {

        CString strUsage = g_clientscript.GetUsage();
        if(!strUsage.IsEmpty()) {
            // read the usage: magic line from the first helper script
            std::vector<CString> aUsage = CString_split(strUsage, " ");
            for(auto i = aUsage.begin(); i != aUsage.end();++i) {
                UsageToken oToken(*i);
                // create a control according to the type described on the token
                // some controls will require a larger number of rows
                CArgControl::Factory(oToken)->Create(m_hWnd);
            }
        }
        
        // create a edit windows showing the script header
        // split the raw text into rows
        auto vFile = CString_split(ReadFile(g_clientscript.m_script), _T("\r\n"));
        m_about.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY | WS_DISABLED);
        m_about_height = 0;
        for(size_t i = 1; i < min(11, vFile.size()); ++i) {
            
            if(vFile[i].Left(1) != '#' // stop when we leave the header comment lines
            || vFile[i].ReverseFind('#') > vFile[i].Find('#')) // stop when we find a line with multiple '#'s
                break;
            
            m_about_height++;
            // add this row to the about edit, removing any leading spaces or #s
            m_about.AppendText(vFile[i].Mid(vFile[i].SpanIncluding(_T("# ")).GetLength()));
            // restore the end of lines removed by CString_split
            m_about.AppendText(_T("\r\n"));
        }
        m_about_height *= ::GetSystemMetrics(SM_CYSMCAPTION);
        
        return 0;
    }
    
    // get the value from all child as a single space separated string
    CString get() {
        CString sText;
        for(auto i = CArgControl::m_instances.begin();i != CArgControl::m_instances.end();++i) {
            if(i != CArgControl::m_instances.begin()) {
                sText += _T(" ");
            }
            sText += QuoteString((*i)->get());
        }
        return(sText);
    }
    // from single space separated string, set each child control
    void set(CString sText) {
        auto vText = CString_split(sText, _T(" "));
        for(size_t i = 0;i < CArgControl::m_instances.size();i++) {

            if(i < vText.size()) {
                CArgControl::m_instances[i]->set(vText[i]);
            }
        }
    }
    
    // retrieve previous values from the xml map
    void load(CString strSelector = _T("")) {
        g_xmlmap.load(strSelector);
        for(auto i = CArgControl::m_instances.begin();i != CArgControl::m_instances.end();++i) {
            (*i)->set(g_xmlmap[(*i)->name()]);
        }
        
    }
    // store the current values to a xml map
    void save(CString strSelector = _T("")) {
        for(auto i = CArgControl::m_instances.begin();i != CArgControl::m_instances.end();++i) {
            g_xmlmap[(*i)->name()] = (*i)->get();
        }
		TCHAR szBuffer[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, szBuffer);

		// MessageBox(szBuffer, GetModuleName(), MB_ICONINFORMATION);
        g_xmlmap.save(strSelector);
    }
    // clear the panel and reset the stored settings
    void clear(CString strSelector = _T("")) {
        DeleteFile(g_xmlmap.SelectFilename(strSelector));
        g_xmlmap.clear();
        load(strSelector);
    }
    
    // implement the WM_SIZE message so we can update the child controls using a custom function
    LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
        int nWidth = LOWORD(lParam) - ::GetSystemMetrics(SM_CXVSCROLL);

        // add to this frame contents
        int nLeft = int(::GetSystemMetrics(SM_CXSMSIZE) * 0.5);
        int nTop = 0;
        // position the about text above the input controls
        if(m_about.IsWindow()) {
            m_about.MoveWindow(nLeft, nTop, nWidth, m_about_height, TRUE);
            nTop += m_about_height;
        }
        nTop += ::GetSystemMetrics(SM_CYSMSIZE);
        
        for(auto i = CArgControl::m_instances.begin();i != CArgControl::m_instances.end();++i) {
            // each object will occupy a size proportional to its required number of rows
            int nHeight = (*i)->rows() * USAGE_ROW_MINHEIGHT;
            
            // add a small border to the bottom of the control
            (*i)->MoveWindow(nLeft, nTop, nWidth, nHeight - ::GetSystemMetrics(SM_CYBORDER) * 2, TRUE);
            // increment the top pixel reference
            nTop += nHeight;
        }
        
        // the total size is the number of rows + borders + scroll bar
        SetScrollSize(1, nTop);

        return _baseClass::OnSize(uMsg, wParam, lParam, bHandled);
    }

    // windows does not set input focus by default to a window when clicked
    // we will set focus to our usage panel so the scroll bar works
    LRESULT OnMouseActivate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
        SetFocus();
        return 0;
    }
	// Overrideables
	void DoPaint(CDCHandle /*dc*/)
	{
		// implemented to avoid a ASSERT in base class CScrollImpl
	}

    
    // Message map and handlers
    // The REFLECT_NOTIFICATION allows this frame to correctly handle child events
    BEGIN_MSG_MAP(UsageFrame)
        //~ odprintf("UsageFrame uMsg %x wParam %x,%x lParam %x hWnd %x", uMsg, HIWORD(wParam), LOWORD(wParam), lParam, hWnd);
        MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        CHAIN_MSG_MAP(_baseClass)
        REFLECT_NOTIFICATIONS()
    END_MSG_MAP()    
};

