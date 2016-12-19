/*
    Copyright 2014 Vale
 
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0
*/

#ifndef __PROPERTYITEMEDITORS__H
#define __PROPERTYITEMEDITORS__H

#pragma once

#define PROP_TEXT_INDENT 2

// create a font using the symbol charset
CFontHandle GetFontSymbol(LPTSTR szFaceName = _T(""), int nHeight = NULL) {
    // Create a log font structure from default
    CLogFont lf;
    lf.lfCharSet = SYMBOL_CHARSET;
    if(nHeight != NULL) {
        lf.lfHeight = nHeight;
    }
    if(lstrlen(szFaceName) > 0) {
        lstrcpy(lf.lfFaceName, szFaceName);
    }

    // change the font of button so it now display symbols
    return(lf.CreateFontIndirect());
}

static const CFontHandle fontSymbol = GetFontSymbol();

class CArgEditor : 
   //~ public CWindowImpl< CArgEditor, CEdit, CControlWinTraits >
   public CWindowImpl< CArgEditor, CEdit,  CWinTraitsOR<WS_CHILD | WS_VISIBLE | WS_BORDER> >
{
public:
   DECLARE_WND_SUPERCLASS(_T("WTL_InplacePropertyEdit"), CEdit::GetWndClassName())

   bool m_fCancel;

   CArgEditor() : m_fCancel(false)
   {
   }
   
   virtual void OnFinalMessage(HWND /*hWnd*/)
   {
      delete this;
   }

   BEGIN_MSG_MAP(CPropertyEditWindow)
      MESSAGE_HANDLER(WM_CREATE, OnCreate)
      MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
      MESSAGE_HANDLER(WM_CHAR, OnChar)
      MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
      MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
      MESSAGE_HANDLER(WM_GETDLGCODE, OnGetDlgCode)
   END_MSG_MAP()

   LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
   {
      LRESULT lRes = DefWindowProc();
      SetFont( CWindow(GetParent()).GetFont() );
      SetMargins(PROP_TEXT_INDENT, 0);   // Force EDIT margins so text doesn't jump
      return lRes;
   }

   LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
   {
      switch( wParam ) {
      case VK_ESCAPE:
         m_fCancel = true;
         // FALL THROUGH...
      case VK_RETURN:
         // Force focus to parent to update value (see OnKillFocus()...)
         ::SetFocus(GetParent());
         // FIX: Allowing a multiline EDIT to VK_ESCAPE will send a WM_CLOSE
         //      to the list control if it's embedded in a dialog!?
         return 0;
      case VK_TAB:
          ;
      case VK_UP:
          ;
      case VK_DOWN:
          ;
      case VK_LEFT:
          ;
      case VK_RIGHT:
          ;
      }
      bHandled = FALSE;
      return 0;
   }

   LRESULT OnChar(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
   {
      switch( LOWORD(wParam) ) {
      case VK_RETURN:
      case VK_ESCAPE:
         // Do not BEEP!!!!
         return 0;
      }
      bHandled = FALSE;
      return 0;
   }

   LRESULT OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
   {
      m_fCancel = false;
      bHandled = FALSE;
      return 0;
   }

   LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
   {
      LRESULT lRes = DefWindowProc(uMsg, wParam, lParam);
      m_fCancel |= (GetModify() == FALSE);
      //~ ::SendMessage(GetParent(), m_fCancel ? WM_USER_PROP_CANCELPROPERTY : WM_USER_PROP_UPDATEPROPERTY, 0, (LPARAM) m_hWnd);
      return lRes;
   }

   LRESULT OnGetDlgCode(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
   {
      return DefWindowProc(uMsg, wParam, lParam) | DLGC_WANTALLKEYS | DLGC_WANTARROWS;
   }
};


#endif // __PROPERTYITEMEDITORS__H
