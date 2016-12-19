/*
    Dynamic controls used by the UsageFrame
    
    Copyright 2014 Vale
 
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

*/

#pragma once

#ifndef IDC_BROWSE
    #define IDC_BROWSE 1101
#endif
#ifndef IDC_COMBO
    #define IDC_COMBO  1102
#endif

#include "smartcolumns.h"

// base class for our custom controls
// they are used a normal controls for each argument
// and also as temporary editor for the Listview grid
class CArgControl {
public:
    typedef CWindowImpl<CArgControl> _baseClass;

    enum {
        // number of rows a grid control will require
        CS_GRID_ROWS = 4,
        CS_COMBO_ROWS = 10
    };

    static std::vector<std::shared_ptr<CArgControl>> m_instances;
    UsageToken m_token;

    // constructor
    CArgControl(UsageToken oToken = UsageToken()) : m_token(oToken) {};
    
    // this is causing a crash on most machines!!!
    // destructor
    //~ ~CArgControl() {
        // remove this instances from the active instance list
        //~ for(auto i = m_instances.begin();i != m_instances.end();++i) {
            //~ if(i->get() == this) {
                //~ m_instances.erase(i);
            //~ }
        //~ }
    //~ }
    virtual LRESULT SendMessage(UINT message, WPARAM wParam = 0, LPARAM lParam = 0) = 0;
    virtual HWND Create(HWND) = 0;

    // create prototypes which will be implemented by CWindow::MoveWindow
    virtual BOOL WINAPI MoveWindow(_In_ int, _In_ int, _In_ int, _In_ int, _In_ BOOL) = 0;
    virtual BOOL WINAPI MoveWindow(_In_ RECT rc, _In_ BOOL bRepaint = TRUE) {
        return MoveWindow(rc.left, rc.top, rc.right - rc.left, rc.bottom-rc.top, bRepaint);
    }
    
    virtual CString name() {
        return(m_token.m_name);
    }
    virtual CString get() {
        return(_T(""));
    }
    
    virtual void set(CString strText) {}
    virtual CString GetType() {
        return typeid(this).name();
    }
    int rows() {
        return(m_token.m_rows);
    }

    // prototypes implemented later
    static std::shared_ptr<CArgControl> Factory(UsageToken & = UsageToken(), BOOL bRegister = TRUE);
    static CString childGet(CString);
    
    // return the number of controls
    static int size() {
        return(int(m_instances.size()));
    }
    // return the number of standard rows all controls will need
    static int sizeRows() {
        int nRows = 0;
        for(auto i = m_instances.begin();i != m_instances.end();++i) {
            nRows += (*i)->m_token.m_rows;
        }
        return(nRows);
    }
    
    virtual BOOL DestroyWindow() = 0;
	virtual BOOL IsWindow() = 0;
};

// init the static instance list
std::vector<std::shared_ptr<CArgControl>> CArgControl::m_instances;

// --------------------------------------------------------------
// custom class which combines a label, a edit and a browse button
// the browse button is only used in the CArgBrowse class
class CArgCombo : public CArgControl, public CWindowImpl<CArgCombo>
{
public:
    typedef CWindowImpl<CArgCombo> _baseClass;
    // declare a new class so we can define the background color
    DECLARE_WND_CLASS_EX(NULL, 0, COLOR_MENU)

    CStatic m_label;
    CComboBox m_combo;
    CButton m_button;

    CArgCombo(UsageToken oToken = UsageToken()) : CArgControl(oToken) {};

	//~CArgCombo() {
		//odprintf("destructor");
		//DestroyWindow();
	//};
        
    virtual LRESULT SendMessage(UINT message, WPARAM wParam, LPARAM lParam) {
        return _baseClass::SendMessage(message, wParam, lParam);
    }
    
    virtual HWND Create(HWND hWndParent) {
        return _baseClass::Create(hWndParent, rcDefault, m_token.m_name, WS_CHILD | WS_VISIBLE);
    }

    virtual CString GetType() {
        return typeid(this).name();
    }
    
    virtual LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
        if(!m_token.m_name.IsEmpty()) {
            m_label.Create(m_hWnd, rcDefault, m_token.m_name, WS_CHILD | WS_VISIBLE);
            m_label.SetFont( AtlCreateBoldFont() );
        }
        
        m_combo.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWN | CBS_AUTOHSCROLL | WS_VSCROLL, 0, IDC_COMBO);
        m_combo.SetFont( AtlGetDefaultGuiFont() );
        
        // in the case of simple combo boxes, its safe to set the focus to it without
        // triggering the parent update which happens in the case of browse controls
        if(m_button) {
            m_button.SetFocus();
        } else {
            m_combo.SetFocus();
        }
        
        return 0;
    }
    
    // implement the generic get for this control
    virtual CString get() {
        CString strText;
		if(::IsWindow(m_combo.m_hWnd)) {
		//if(m_combo) {
			int nLen = m_combo.GetWindowTextLength();
			m_combo.GetWindowText(strText.GetBuffer(nLen+1), nLen+1);
			strText.ReleaseBuffer(nLen);
		}
		//odprintf("get %s", strText);
        return(strText);
    }
    
    // implement the generic set for this control
    virtual void set(CString strText) {
        m_combo.SetWindowText(strText);
    }
    
    // implement the populate, which will define the drop down list of option to the given vector
    void Populate(std::vector<CString> vList) {
        for(int i=0;i < static_cast<int>(vList.size());i++) {
            // preserve the current list (avoids losing the current value), but add more items if needed
            if(i < m_combo.GetCount()) {
                m_combo.SetItemData(i, DWORD_PTR(vList[i].GetBuffer()));
            } else {
                m_combo.AddString(vList[i]);
            }
            // special case:
            // in the case one item of the list is exactly equal the control name
            // fill the control with the same value as its own name
            if(vList[i].CompareNoCase(m_token.m_name) == 0) {
                set(m_token.m_name);
            }
        }
    }

    // handle the drop down menu
    // populate it using a procedure adequate for the data source
    virtual LRESULT OnCombo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
        if(m_token.m_type == TOKEN_DATA) {
            // populate the combo list directly with the comma separated data
            Populate(CString_split(m_token.m_data,_T(",")));
        }
        if(m_token.m_type == TOKEN_BASE) {
            // populate the combo list with data read from a file pointed by another control
            SetCursor(LoadCursor(NULL,IDC_WAIT)); // show buzy cursor
            Populate(SmartColumns(CArgControl::childGet(m_token.m_data)));
            SetCursor(LoadCursor(NULL,IDC_ARROW)); // show normal cursor
        }
        
        return 0;
    }

    // when the combo loses focus, change focus to our parent
    // which my be a Listview grid triggering the procedure to update cell value
    virtual LRESULT OnKillFocus(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
        ::SendMessage(GetParent(), WM_SETFOCUS, NULL, NULL);
        return 0;
    }
    
    virtual BOOL WINAPI MoveWindow(_In_ int X, _In_ int Y, _In_ int nWidth, _In_ int nHeight, _In_ BOOL bRepaint) {
        int nLeft = 0;
        int nRight = nWidth;
        if(m_label.IsWindow()) {
            // ensure label is relative to total width, but never too small
            nLeft = max(::GetSystemMetrics(SM_CXSMSIZE) * 2, int(nWidth * 0.25));
            m_label.MoveWindow(0, 0, nLeft, nHeight, bRepaint);
        }
        if(m_button.IsWindow()) {
            // ensure button has a fixed size
            nRight = nWidth - ::GetSystemMetrics(SM_CXSMSIZE) * 2;
            m_button.MoveWindow(nRight, 0, nWidth - nRight, nHeight, bRepaint);
        }
        m_combo.MoveWindow(nLeft, 0, nRight - nLeft, nHeight * CS_COMBO_ROWS, bRepaint);
        return _baseClass::MoveWindow(X,  Y,  nWidth,  nHeight,  bRepaint);
    }

    virtual BOOL DestroyWindow() {
		//odprintf("DestroyWindow %d, %d", m_hWnd, ::IsWindow(m_hWnd));
		if(IsWindow()) {
			return _baseClass::DestroyWindow();
		}
		return 0;
    }

	virtual BOOL IsWindow() {
		return ::IsWindow(m_hWnd);
	}
    
    // stub for polymorphic compatibility with CArgBrowse
    virtual LRESULT OnBrowse(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
        return 0;
    }
    // this event is a notification generated by the browse button when clicked
    // just a stub in the case of a combo
    virtual LRESULT OnClick(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/) {
        return 0;
    }

    // implement OnMouseActivate on this control just so our parent does not steal focus
    LRESULT OnMouseActivate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
        // all we need to do is nothing
        return 0;
    }
    
    BEGIN_MSG_MAP(CArgCombo)
        NOTIFY_CODE_HANDLER(NM_CLICK, OnClick)
        COMMAND_CODE_HANDLER(CBN_KILLFOCUS, OnKillFocus)
        COMMAND_CODE_HANDLER(CBN_DROPDOWN, OnCombo)
        COMMAND_ID_HANDLER(IDC_BROWSE, OnBrowse)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate)
    END_MSG_MAP()
};

// add a file browse button to the CArgCombo class
class CArgBrowse : public CArgCombo
{
public:
    typedef CArgCombo _baseClass;

    CArgBrowse(UsageToken oToken = UsageToken()) : CArgCombo(oToken) {};

    CString GetType() {
        return typeid(this).name();
    }
    
    virtual LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
        m_button.Create(m_hWnd, rcDefault, _T("1"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, IDC_BROWSE);

        // change the font of button so it now display symbols
        m_button.SetFont( fontWingdings );
        
        return _baseClass::OnCreate(uMsg, wParam, lParam, bHandled);
    }
    
    // handle the drop down menu
    // populate it using a procedure adequate for the data source
    virtual LRESULT OnCombo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
        Populate(VectorFilterSufix(FileSearch<CString>(), CString_split(m_token.m_data, _T(","))));
        
        return 0;
    }

    
    // callback to handle the browse button click
    virtual LRESULT OnBrowse(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
        CMultiFileDialog oDialog(NULL, NULL, OFN_ALLOWMULTISELECT | OFN_NOCHANGEDIR, CreateFileFilter(m_token.m_data));
        
        if(oDialog.DoModal() == IDOK) {
            TCHAR sBuffer[MAX_PATH];
            int nRet = oDialog.GetFirstPathName(sBuffer, MAX_PATH);
            CString strText = MakePathRelative(sBuffer);
            // concatenate multiple files using comma ,
            while(nRet != 0) {
                nRet = oDialog.GetNextPathName(sBuffer, MAX_PATH);
                if(nRet == 0) {
                    break;
                }
                strText += _T(",");
                strText += MakePathRelative(sBuffer);
            }
            
            set(strText);
        }
        
        SendMessage(WM_COMMAND, MAKEWPARAM(NULL, CBN_KILLFOCUS), NULL);
        return 0;
    }
    
    // custom override of the onclick so we may check if user indirectly clicked
    // the combo control or the browse button when this control was not created yet
    virtual LRESULT OnClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
        // get the button rectangle relative to the CArgControl frame
        CRect rc;
        m_button.GetClientRect(&rc);
        // make the rectangle with coordinates relative to the parent Listview
        m_button.MapWindowPoints(GetParent(), rc);
        // get the point stored on the notification which our parent received and forwarded to us
        NMITEMACTIVATE nmia = *(LPNMITEMACTIVATE)pnmh;
        // check if the point clicked hits our browse button
        if(PtInRect(rc, nmia.ptAction)) {
            // now we convert the click which then was into empty space 
            // into a click on the dynamicaly created button
            m_button.Click();
        }
        
        return 0;
    }
};

// --------------------------------------------------------------
// a specialized checkbutton for boolean values (1 or 0)
class CArgCheck : public CButton, public CArgControl
{
public:
    typedef CButton _baseClass;

    CArgCheck(UsageToken oToken = UsageToken()) : CArgControl(oToken) {};

    // Implement Create so we can define styles
	HWND Create(HWND hWndParent) {
		HWND hWnd = _baseClass::Create(hWndParent, rcDefault, m_token.m_name,  WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_NOTIFY);
        // change the hideous default font to the standard gui font
        _baseClass::SetFont( AtlCreateBoldFont(), TRUE );
        // set this control to the default suplied
        set(m_token.m_data);
        return hWnd;
	}
    virtual LRESULT SendMessage(UINT message, WPARAM wParam, LPARAM lParam) {
        return _baseClass::SendMessage(message, wParam, lParam);
    }
    
    virtual BOOL WINAPI MoveWindow(_In_ int X, _In_ int Y, _In_ int nWidth, _In_ int nHeight, _In_ BOOL bRepaint) {
        return _baseClass::MoveWindow(X, Y, nWidth, nHeight, bRepaint);
    }
    virtual BOOL DestroyWindow() {
        return _baseClass::DestroyWindow();
    }

	virtual BOOL IsWindow() {
		return ::IsWindow(m_hWnd);
	}
    virtual void set(CString strText) {
        SetCheck(atoi(strText));
    }
    virtual CString get() {
        CString strText;
        strText.Format(_T("%d"), GetCheck());
        return(strText);
    }
    CString GetType() {
        return typeid(this).name();
    }
};

