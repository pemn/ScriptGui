/*
    provide constant handles to symbol fonts
    
    Copyright 2014 Vale

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0
*/

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

static const CFontHandle fontWingdings = GetFontSymbol();
static const CFontHandle fontWebdings  = GetFontSymbol(_T("Webdings"));
