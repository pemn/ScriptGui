/*
    CArgListview.h
    Creates a table grid
    subclasses the windows api CListViewCtrl
    Uses CCustomDraw to enhace the funcionality

    Copyright 2014 Vale
 
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0
*/

//~ #pragma once

template< class T, class TBase = CListViewCtrl, class TWinTraits = CWinTraitsOR<WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_OWNERDRAWFIXED | LVS_SINGLESEL> >
//~ class ATL_NO_VTABLE CArgListviewImpl : public CWindowImpl<T, TBase, TWinTraits>,
class CArgListviewImpl : public CWindowImpl<T, TBase, TWinTraits>,
    public COwnerDraw<T>,
    public CArgControl
{
public:
    DECLARE_WND_SUPERCLASS(NULL, TBase::GetWndClassName())
    //~ DECLARE_WND_SUPERCLASS(_T("WTL_CArgListview"), GetWndClassName())
    //~ DECLARE_WND_CLASS_EX(NULL,0,COLOR_MENU)


    typedef CWindowImpl<T, TBase, TWinTraits> _baseClass;
    //~ typedef std::vector<std::vector<CString>> _baseVector;
    //~ typedef CListViewCtrl TBase;
    std::vector<std::vector<CString>> m_data;
    std::vector<UsageToken> m_token_cols;
    std::shared_ptr<CArgControl> m_editor;
    //~ std::shared_ptr<CArgComboGrid> m_editor;
    int m_editorItem;
    int m_editorSubItem;

    CArgListviewImpl(UsageToken oToken = UsageToken()) : CArgControl(oToken), m_token_cols(m_token.GetGridTokens()) {};

    HWND Create(HWND hWndParent) {
        HWND hWnd = _baseClass::Create(hWndParent, rcDefault);
        //~ SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

        // Update colours and text
        //~ SendMessage(WM_SETTINGCHANGE);

        return hWnd;
    }

    virtual BOOL WINAPI MoveWindow(_In_ int X, _In_ int Y, _In_ int nWidth, _In_ int nHeight, _In_ BOOL bRepaint) {
        // resize all columns but the first to the available space
        int nColWidth = (nWidth - ::GetSystemMetrics(SM_CXVSCROLL) - GetColumnWidth(0)) / (GetColumnCount() - 1);
        for(int i=1;i< GetColumnCount();i++) {
            SetColumnWidth(i, nColWidth);
        }
        return _baseClass::MoveWindow(X, Y, nWidth, nHeight, bRepaint);
    }
    
    virtual BOOL DestroyWindow() {
        return _baseClass::DestroyWindow();
    }
	virtual BOOL IsWindow() {
		return ::IsWindow(m_hWnd);
	}
    virtual LRESULT SendMessage(UINT message, WPARAM wParam = 0, LPARAM lParam = 0) {
        return _baseClass::SendMessage(message, wParam, lParam);
    }
    
    CString GetType() {
        return typeid(this).name();
    }
    
    // fix the BUGGED WINDOWS BUG which prevents correctly retrieving the rect for the 0 item
    BOOL GetSubItemRect(int nItem, int nSubItem, int nFlag, LPRECT lpRect) const
    {
        BOOL bRet = _baseClass::GetSubItemRect(nItem, nSubItem, nFlag, lpRect);
        if( nSubItem == 0 ) {
            lpRect->right = lpRect->left + GetColumnWidth(nSubItem);
        }
        return bRet;
    }

    // return all data in this grid as a comma separated text string
    CString get() {
        CString strText;
        TCHAR pstr[MAX_PATH * 20];
        // dont retrieve the last line, since its a "add row" pseudo control
        for(int i=0;i<GetItemCount()-1;i++) {
            if(i > 0) {
                strText += _T(";");
            }
            // ignore the first column since its row label
            for(int j=1;j<GetColumnCount();j++) {
                if(j > 1) {
                    strText += _T(",");
                }

                GetItemText(i, j, pstr, MAX_PATH * 20);
                strText += pstr;
            }
        }
        return(strText);
    }
    
    // implement the generic set for this control
    // input is a comma and semicolomn separated string of tabular data
    virtual void set(CString strText) {
        auto vText = CString_split(CString_split(strText));
        for(int i = 0; i < static_cast<int>(vText.size());i++) {
            // add more rows as needed
            if(i >= GetItemCount() - 1) {
                AddRow();
            }
            for(int j = 1; j < GetColumnCount();j++) {
                if(j <= static_cast<int>(vText[i].size())) {
                    // the data is offset by one column due to the row label column
                    SetItemText(i, j, vText[i][j-1]);
                } else {
                    // clear extra columns
                    SetItemText(i, j, _T(""));
                }
            }
        }
        // delete extra rows
        // we MUST cast the size() as int because i will become -1 and since size_t is UINT
        // meaning we will check if (-BIGNUMBER >= 0) creating a near infinite loop
        for(int i = GetItemCount() - 2;i >= int(vText.size());--i) {
            _baseClass::DeleteItem(i);
        }
    }
    
    // return the number of column on this grid
    int GetColumnCount() {
        return GetHeader().GetItemCount();
    }
    
    // get the item data as a CString
    CString GetItemTextString(int nItem, int nSubItem) {
        CString sBuffer;
        GetItemText(nItem, nSubItem, sBuffer.GetBuffer(MAX_PATH * 20), MAX_PATH * 20);
        sBuffer.ReleaseBuffer();
        return(sBuffer);
    }
    
    // convert a column name to a column index
    // returns -1 if no column have the given name
    int GetColumnIndex(CString strName) {
        LVCOLUMN column = { LVCF_TEXT };
        //~ column.pszText = (LPTSTR)LocalAlloc(LMEM_ZEROINIT, BUFFER_SIZE_GET * sizeof(TCHAR));
        TCHAR szBuffer[MAX_PATH];
        column.pszText = szBuffer;
        column.cchTextMax = MAX_PATH;
        // find which column index has the given column name
        // skip the row label column (0)
        for(int i=1;i< GetColumnCount();i++) {
            if(GetColumn(i, &column)) { // getcolumn will return false on failure
                if(strName == CString(column.pszText)) {
                    return(i);
                }
            }
        }
        return(-1);
    }
    
    // retrieve the selected row value of a given named column
    // return a blank string if either there is no row selected
    // or if a column with the given name was not found
    CString GetSelectedRowValue(CString strName) {
        CString strText;
        if(GetSelectedIndex() != -1) { // check if we have a selected row
            int nColumn = 1; // default to the first column (0 is row label)
            if(!strName.IsEmpty()) { // if a column name was specified
                GetColumnIndex(strName);
            }
            if(nColumn > 0) { // check a column with that name was found (label column doesnt count)
                // get the specified column value at the selected row
                GetItemText(GetSelectedIndex(), nColumn, strText);
            }
        }
        return(strText);
    }
    
    // create a new row where only the special label column has any data
    int AddRow() {
        return(InsertItem(GetItemCount() - 1, _T("Ö")));
    }

    LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
        LRESULT lRes = DefWindowProc();
        
        for(auto i = m_token_cols.rbegin();i != m_token_cols.rend();++i) {
            InsertColumn(0, i->m_name, LVCFMT_LEFT, 100);
            // after we insert the column, clear this token so the editor wont waste
            // screen space with redundant information
            i->m_name.Empty();
        }
        
        InsertColumn(0, _T(""), LVCFMT_LEFT, ::GetSystemMetrics(SM_CXSMSIZE) * 2);
        
        // create the last row, which is a button to add more rows
        InsertItem(0, _T("ò"));

        AddRow();
        
        //~ SetItemText(int nItem, int nSubItem, LPCTSTR lpszText)

        return(lRes);
    }

    //~ NMITEMACTIVATE(NMHDR  hdr,int    iItem,int    iSubItem,UINT   uNewState,UINT   uOldState,
    //~ UINT   uChanged, POINT  ptAction,  LPARAM lParam,  UINT   uKeyFlags)
    LRESULT OnClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled) {
        NMITEMACTIVATE nmia = *(LPNMITEMACTIVATE)pnmh;
        //~ odprintf("OnClick %d,%d", LPNMITEMACTIVATE(pnmh)->iItem, LPNMITEMACTIVATE(pnmh)->iSubItem);
        
        
        if(nmia.iSubItem < 0 || nmia.iItem < 0) {
            // click outside grid, do nothing
        } else if(nmia.iItem == GetItemCount() - 1) { // the row clicked is the extra row
            if(nmia.iSubItem == 0) { // column clicked is the row label column
                AddRow();
            }
        } else if(nmia.iSubItem == 0) {
            // delete the entire row
            _baseClass::DeleteItem(nmia.iItem);
        } else { // valid column
            CString strText = GetItemTextString(nmia.iItem, nmia.iSubItem);
            // special case, the checkbox is fully implemented using custom draw
            if(m_token_cols[nmia.iSubItem - 1].type() == UsageToken::TYPE_BOOL) {
                if(strText.Find(_T("1")) == -1) {
                    SetItemText(nmia.iItem, nmia.iSubItem, _T("1"));
                } else {
                    SetItemText(nmia.iItem, nmia.iSubItem, _T("0"));
                }
                return 0;
            }
            //~ SetItemState(nmia.iItem, LVIS_SELECTED, LVIS_SELECTED);
            
            // set the selected column
            m_editorItem = nmia.iItem;
            m_editorSubItem = nmia.iSubItem;
            // retrieve the rectangle which will contain the dynamic control
            CRect rc;
            GetSubItemRect(nmia.iItem, nmia.iSubItem, LVIR_BOUNDS, &rc);
            // create a temporary editor using the versatile CArgControl class
            m_editor = CArgControl::Factory(m_token_cols[nmia.iSubItem - 1], FALSE);
            m_editor->Create(m_hWnd);
            m_editor->MoveWindow(rc);
            m_editor->set(strText);
            // forward this NM_CLICK to the editor, which may use it depending on type
            m_editor->SendMessage(WM_NOTIFY, NULL, (LPARAM)&nmia);
            
            // prevent the focus from changing from the editor back to this frame
            //~ bHandled = TRUE;
        }
        return 0;
    }
    
    // when user uses the scroll bar, reset the editor after the scrolling
    LRESULT OnScroll(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
        if(LOWORD(wParam) == SB_ENDSCROLL) {
            ResetEditor();
        }
        bHandled = FALSE;
        return 0;
    }
    
    // when this window gets focus, close any open editors
    LRESULT OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
        ResetEditor();
        return 0;
    }
    
    // destroy any current editor, saving the data to its related cell
    void ResetEditor() {
		if(m_editor && m_editor->IsWindow()) {
			SetItemText(m_editorItem, m_editorSubItem, m_editor->get());
			m_editor->DestroyWindow();
			//this is causing an assertion:
			//m_editor.reset();
			//m_editor->SendMessageA(WM_DESTROY);
			//m_editor->SendMessage(WM_CLOSE);
        }
    }
    
    // Overridables for Ownerdraw block
    void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) {
        CDCHandle dc = lpDrawItemStruct->hDC;
        //~ CRect rc = lpDrawItemStruct->rcItem;
        int nItem = lpDrawItemStruct->itemID;
        int nSubItem = 0;
        // Figure out rectangle
        CRect rc;
        // multiple use buffer
        //~ TCHAR lpItemText[MAX_PATH];
        
        int nTokenType = -1;
        
        // clear everything by setting the entire control to the background color
        dc.FillSolidRect(&lpDrawItemStruct->rcItem, dc.GetBkColor());
        
        for(int nSubItem=0;nSubItem<GetColumnCount();nSubItem++) {
            GetSubItemRect(nItem, nSubItem, LVIR_BOUNDS, &rc);
            //~ GetItemText(nItem, nSubItem, lpItemText, MAX_PATH);
            

            if(nSubItem == 0) { // button to indicate each row
                dc.SelectFont(fontWingdings);
                // draw a pseudo button on the background
                dc.DrawFrameControl(&rc, DFC_BUTTON, DFCS_BUTTONPUSH | DFCS_ADJUSTRECT);
            } else {
                nTokenType = m_token_cols[nSubItem - 1].type();
                //~ odprintf("DrawItem %d,%d", lpDrawItemStruct->itemID, nTokenType);
                if(nItem < GetItemCount() - 1) {
                    dc.DrawEdge(&rc, BDR_RAISEDOUTER, BF_BOTTOMRIGHT | BF_FLAT);
                    if(nTokenType == UsageToken::TYPE_FILE) {
                        // draw the control button
                        DrawRightButton(dc, rc);
                    }
                    if(nTokenType == UsageToken::TYPE_BOOL) {
                        // draw a pseudo checkbutton
                        DrawCheckBox(dc, rc, GetItemTextString(nItem, nSubItem));
                        // dont draw any text
                        continue;
                    }
                }
                dc.SelectFont(AtlGetDefaultGuiFont());
            }
            
            dc.DrawText(GetItemTextString(nItem, nSubItem), -1, &rc, DT_CENTER | DT_SINGLELINE | DT_EDITCONTROL | DT_NOPREFIX | DT_END_ELLIPSIS | DT_VCENTER);
        }
    }
    
    // draw a checkbox inside the cell, showing the current binary value
    void DrawCheckBox(CDCHandle& dc, CRect& rc, CString strText) {
        WORD wSTATE = 0;
        if(strText.Find(_T("1")) != -1) {
            wSTATE = DFCS_CHECKED;
        }
        // draw a pseudo button on the background
        dc.DrawFrameControl(rc, DFC_BUTTON, DFCS_BUTTONCHECK | wSTATE);
    }
    
    void DrawRightButton(CDCHandle& dc, CRect& rc) {
        CRect rButton(rc);
        rButton.left = rButton.right - ::GetSystemMetrics(SM_CXSMSIZE) * 2;
        // draw a pseudo button on the background
        dc.DrawFrameControl(&rButton, DFC_BUTTON, DFCS_BUTTONPUSH);
        // modify the input rectangle to exclude the newly created button
        rc.right -= rButton.Width();
        // draw a symbol as the button label
        dc.SelectFont(fontWingdings);
        dc.DrawText(_T("1"), -1, &rButton, DT_CENTER | DT_SINGLELINE | DT_EDITCONTROL | DT_NOPREFIX | DT_END_ELLIPSIS | DT_VCENTER);
    }
    
    void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct) {
        lpMeasureItemStruct->itemHeight = UINT(::GetSystemMetrics(SM_CYSMSIZE) * 1.5);
    }
    
    // We are required to have this empty DeleteItem to avoid ambiguity issues
    // with COwnerDraw::DeleteItem and CListViewCtrl::DeleteItem.
    // ---------------------------------------------------------------------
    void DeleteItem(LPDELETEITEMSTRUCT lpDeleteItemStruct) {
        COwnerDraw<T>::DeleteItem(lpDeleteItemStruct);
    };
    
    //~ int GetSelectSubIndex() {
        //~ if(m_selectedSubItem <= 0) {
            //~ LVHITTESTINFO hti = { 0 };
            //~ DWORD dwPos = ::GetMessagePos();
            //~ POINTSTOPOINT(hti.pt, dwPos);
            //~ ScreenToClient(&hti.pt);
            //~ SubItemHitTest(&hti);
            //~ odprintf("GetSelectSubIndex %d", hti.iSubItem);
            //~ m_selectedSubItem = hti.iSubItem;
        //~ }
        //~ return(m_selectedSubItem);
    //~ }
    
    //~ LRESULT OnMyMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
    //~ {
        //~ odprintf("CArgListviewImpl OnMyMessage");
        //~ return 0;
    //~ }
    
    //~ virtual LRESULT OnMyCommand(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
        //~ // create a notification structure
        //~ odprintf("CArgListviewImpl OnMyCommand");
        //~ return 0;
    //~ }
    
    //~ LRESULT OnMyNotification(int idCtrl, LPNMHDR pnmh, BOOL& /*bHandled*/)
    //~ {
        //~ odprintf("CArgListviewImpl OnMyNotification");
        //~ return 0;
    //~ }

    
    // Message map and handlers
    BEGIN_MSG_MAP(CArgListviewImpl)
        //~ odprintf("uMsg %x wParam %x,%x lParam %x hWnd %x", uMsg, HIWORD(wParam), LOWORD(wParam), lParam, hWnd);
        //~ REFLECTED_NOTIFY_CODE_HANDLER(LVN_BEGINSCROLL, OnBeginScroll)
        //~ REFLECTED_NOTIFY_CODE_HANDLER(LVN_COLUMNCLICK, OnColumnClick)

        MESSAGE_HANDLER(WM_VSCROLL, OnScroll)
        REFLECTED_NOTIFY_CODE_HANDLER(NM_CLICK, OnClick)
        MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        CHAIN_MSG_MAP_ALT(COwnerDraw<T>, 1)
        //~ FORWARD_NOTIFICATIONS()
        //~ DEFAULT_REFLECTION_HANDLER()
    END_MSG_MAP()
};

class CArgListview : public CArgListviewImpl<CArgListview> {
public:
    DECLARE_WND_SUPERCLASS(_T("WTL_Listview"), GetWndClassName())

    CArgListview(UsageToken oToken = UsageToken()) : CArgListviewImpl<CArgListview>(oToken) {};
};

// get the value of a single child by its token name
CString CArgControl::childGet(CString sName) {
    CString strText;
    for(auto i = m_instances.begin();i != m_instances.end();++i) {
        // in the case of GRID object, look in each its column names
        if((*i)->m_token.type() == UsageToken::TYPE_GRID) {
            auto oGrid = std::static_pointer_cast<CArgListview>(*i);
            if(oGrid->GetColumnIndex(sName) > 0) {
                strText = oGrid->GetSelectedRowValue(sName);
                break;
            }
        } else if((*i)->name() == sName) {
            strText = (*i)->get();
            break;
        }
    }
    return(strText);
}

