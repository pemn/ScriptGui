/*
    Create a list of options based on the file extension

    Copyright 2014 Vale

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0
*/


std::map<CString, std::vector<CString>> g_smartcolumns_cache;

/*

// generic function to call an OLE method or to set/get an OLE property
HRESULT OLEMethod(int nType, VARIANT *pvResult, IDispatch *pDisp,LPOLESTR ptName, int cArgs...)
{
    if(!pDisp) return E_FAIL;

    va_list marker;
    va_start(marker, cArgs);

    DISPPARAMS dp = { NULL, NULL, 0, 0 };
    DISPID dispidNamed = DISPID_PROPERTYPUT;
    DISPID dispID;

    // Get DISPID for name passed...
    HRESULT hr= pDisp->GetIDsOfNames(IID_NULL, &ptName, 1, LOCALE_USER_DEFAULT, &dispID);
    if(FAILED(hr)) {
        return hr;
    }
    // Allocate memory for arguments...
    VARIANT *pArgs = new VARIANT[cArgs+1];
    // Extract arguments...
    for(int i=0; i<cArgs; i++) {
        pArgs[i] = va_arg(marker, VARIANT);
        //~ odprintf("id %d i %d vt %d", dispID, i, pArgs[i].vt);
    }

    // Build DISPPARAMS
    dp.cArgs = cArgs;
    dp.rgvarg = pArgs;

    // Handle special-case for property-puts!
    if(nType & DISPATCH_PROPERTYPUT) {
        dp.cNamedArgs = 1;
        dp.rgdispidNamedArgs = &dispidNamed;
    }
    // we must startup the result variable before using it
    VariantInit(pvResult);
    // Make the call!
    hr = pDisp->Invoke(dispID, IID_NULL, LOCALE_SYSTEM_DEFAULT, nType, &dp, pvResult, NULL, NULL);
    
    // End variable-argument section...
    va_end(marker);

    delete [] pArgs;
    return hr;
}

//~ #include <comutil.h>
#include <comdef.h>
// open a excel workbook and retrieve the data of the first row of the activesheet
// input: path to workbook
// output: pointer to a array of variant types
HRESULT GetExcelHeader(LPCTSTR szFullPath, SAFEARRAY* &psa) {
    CLSID clsid;
    HRESULT hr;
    //~ = CLSIDFromProgID(L"Excel.Application", &clsid);

    if(SUCCEEDED(CLSIDFromProgID(L"Excel.Application", &clsid))) {
        IDispatch *pApp;
        if(SUCCEEDED(CoCreateInstance(clsid, NULL, CLSCTX_LOCAL_SERVER, IID_IDispatch, (void **)&pApp))) {

            IDispatch *pBooks;
            // Get Workbooks
            {
                VARIANT result;
                if(FAILED(hr=OLEMethod(DISPATCH_PROPERTYGET, &result, pApp, L"Workbooks", 0))) return hr;
                pBooks = result.pdispVal;
            }
            // Open Workbook
            IDispatch *pActiveBook;
            {
                VARIANT result;
                if(FAILED(hr=OLEMethod(DISPATCH_METHOD, &result, pBooks, L"Open", 1, _variant_t(szFullPath)))) return hr;
                pActiveBook = result.pdispVal;
            }
            
            // Get ActiveSheet
            IDispatch* pActiveSheet;
            {
                VARIANT result;
                if(FAILED(hr=OLEMethod(DISPATCH_PROPERTYGET, &result, pActiveBook, L"ActiveSheet", 0))) return hr;
                pActiveSheet = result.pdispVal;
            }
            
            // get usedrange
            IDispatch* pUsedRange;
            {
                VARIANT result;
                if(FAILED(hr=OLEMethod(DISPATCH_PROPERTYGET, &result, pActiveSheet, L"UsedRange", 0))) return hr;
                pUsedRange = result.pdispVal;
            }

            // get the Range object for the first row
            IDispatch* pRow1;
            {
                VARIANT result;
                if(FAILED(hr=OLEMethod(DISPATCH_PROPERTYGET, &result, pUsedRange , L"Rows", 1, _variant_t(1)))) return hr;
                pRow1 = result.pdispVal;
            }
            
            // read the value from first row
            {
                VARIANT result;
                if(FAILED(hr=OLEMethod(DISPATCH_PROPERTYGET, &result, pRow1, L"Value", 0))) return hr;
                psa = V_ARRAY(&result);
            }

            // Close Excel
            {
                VARIANT result;
                if(FAILED(hr=OLEMethod(DISPATCH_METHOD, &result, pApp, L"Quit", 0))) return hr;
            }
            
            pApp->Release();
        }
    }
    
    return(hr);
}

// open a datamine using COM automation
// return a vector with the values of the first row
//"C:\\Arquivos de programas\\Arquivos comuns\\Datamine\\Components\\DmFile.dll"
//~ #import "C:\Arquivos de programas\Arquivos comuns\Datamine\Components\DmFile.dll"
    //~ $self->{super} = Win32::OLE->new('DmFile.DmTableADO');
    //~ unless($self->{super}) {
        //~ print STDERR Win32::OLE->LastError(),"\n";
        //~ return();
    //~ }
    //~ # use the dm object to load the input dm as readwrite
    //~ $self->{super}->Open($path, 0);

    //~ return() unless($self->{super}->Name);
    //~ # create the header
    //~ $self->{field_list} = [map {$self->{super}->Schema->GetFieldName($_)} (1 .. $self->{super}->Schema->FieldCount())];

HRESULT GetDatamineHeader(LPCTSTR szFullPath = NULL) {
    
    CLSID clsid;
    HRESULT hr;
    if(SUCCEEDED(CLSIDFromProgID(L"DmFile.DmTable", &clsid))) {
        //~ odprintf("CLSIDFromProgID");
        IDispatch *pDisp;
        // create a instance of the DmTableADO
        if(SUCCEEDED(CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_IDispatch, (void **)&pDisp))) {
            //~ odprintf("CoCreateInstance");
            IDispatch *pTable;
            // open the datamine table object
            {
                VARIANT result;
                hr=OLEMethod(DISPATCH_METHOD, &result, pDisp, L"Open", 2, _variant_t(szFullPath), _variant_t(long(0)));
                //~ odprintf("open vt %d hr %d u %d", result.vt, hr, 0);
                if(FAILED(hr)) return hr;
                pTable = result.pdispVal;
            }
            // get the table schema
            //~ IDispatch* pSchema;
            //~ {
                //~ VARIANT result;
                //~ hr=OLEMethod(DISPATCH_PROPERTYGET, &result, pTable, L"Schema", 0);
                //~ if(FAILED(hr)) return hr;
                //~ pSchema = result.pdispVal;
            //~ }

            //~ // get the table schema field count
            //~ IDispatch* pFieldCount;
            //~ {
                //~ VARIANT result;
                //~ if(FAILED(hr=OLEMethod(DISPATCH_PROPERTYGET, &result, pSchema, L"FieldCount", 0))) return hr;
                //~ pFieldCount = result.pdispVal;
                //~ odprintf("FieldCount vt %d", result.vt);
            //~ }
        }
    }
    
    return S_OK;
}


// open a excel using COM automation
// return a vector with the values of the first row of a excel sheet
HRESULT OLETableHeader(CString szFullPath, std::vector<CString> &vRet) {
    // default to 
    HRESULT hr=TYPE_E_UNSUPFORMAT;
    
    if(szFullPath.Find(_T(".xls")) != -1 || szFullPath.Find(_T(".csv")) != -1) {
        SAFEARRAY* psa = NULL;
        if(FAILED(hr=GetExcelHeader(szFullPath, psa))) {
            return(hr);
        }
        long nRows, nCols;
        SafeArrayGetUBound(psa, 1, &nRows);
        SafeArrayGetUBound(psa, 2, &nCols);
        
        VARIANT* rawArray;
        SafeArrayAccessData(psa, (void**)&rawArray);
        for (long i = 0; i < nCols; ++i) {
            // the value array is a single dimensional array
            // we have to calculate the index of each column
            long iCol = i * nRows;
            
            if(rawArray[iCol].vt == VT_BSTR) {
                CString strValue;
                strValue = COLE2T((_bstr_t)rawArray[iCol]);
                vRet.push_back(strValue);
            }
        }
        SafeArrayUnaccessData(psa);
    }
    //~ if(szPath.Right(3).CompareNoCase(_T(".dm")) == 0) {
        //~ if(SUCCEEDED(GetDatamineHeader(szFullPath))) {
        //~ }
    //~ }
    
    return(hr);
}
*/

// create a list of options specific to each file type
// excel or csv: column names from first row
// datamine dm: schema field name
// vulcan dgd: list of layers
// vulcan isis: list of fields
// vulcan block model: list of variables
std::vector<CString> SmartColumns(CString szPath) {
    // ensure a lower case to simplify the extension comparison
    szPath.MakeLower();
    if(g_smartcolumns_cache.find(szPath) != g_smartcolumns_cache.end()) {
        // do nothing, this path is already cached
    } else {
        // default to a empty string
        g_smartcolumns_cache[szPath] = std::vector<CString>();
        // check if path points to a existing file
        if(PathFileExists(szPath)) {
            // check if its a sheet
            //~ if(FAILED(OLETableHeader(szPath, g_smartcolumns_cache[szPath]))) {
            // if is not a sheet it may be some kind of Vulcan data
            ClientScript oPerlPipe(_T("-e"), _T("perl.exe"), 1);
            CString sCode(_T("\"$, = chr(10);"));
            if(szPath.Find(_T(".bmf")) != -1) { // vulcan block model
                sCode += _T("require vulcan;");
                sCode += _T("my $db = new vulcan::block_model($ARGV[0], 'r');");
                sCode += _T("print $db->field_list();");
                sCode += _T("$db->close();");
            } else
            if(szPath.Find(_T(".dgd.isis")) != -1) { // vulcan design database
                sCode += _T("require vulcan;");
                sCode += _T("my $db = new vulcan::isisdb($ARGV[0], '', ''); my @r;");
                sCode += _T("for($db->rewind(); !$db->eof(); $db->next_key()) {");
                sCode += _T("    push @r, $db->get_key();}");
                // layers that begin with $ are deleted
                sCode += _T("print grep(/^[^\\$]/, @r);");
                sCode += _T("$db->close();");
            } else
            if(szPath.Find(_T(".isis")) != -1) { // vulcan hole database
                sCode += _T("require vulcan;");
                sCode += _T("my $db = new vulcan::isisdb($ARGV[0], '', ''); my @r;");
                sCode += _T("foreach my $t ($db->table_list()) {");
                sCode += _T("    push @r, map {$t . ':' . $_} $db->field_list($t);}");
                sCode += _T("print @r;");
                sCode += _T("$db->close();");
            } else
            if(szPath.Find(_T(".dm")) != -1) { // datamine table
                sCode += _T("use Win32::OLE;");
                sCode += _T("my $db = Win32::OLE->new('DmFile.DmTableADO');");
                sCode += _T("$db->Open($ARGV[0], 0);");
                sCode += _T("exit unless($db->Name);");
                sCode += _T("print map {$db->Schema->GetFieldName($_)} (1 .. $db->Schema->FieldCount());");
                sCode += _T("$db->Close()");
            } else
            if(szPath.Find(_T(".xls")) != -1 || szPath.Find(_T(".csv")) != -1) { // excel table
                sCode += _T("use Win32::OLE;");
                sCode += _T("my $db = Win32::OLE->GetObject($ARGV[0]);");
                sCode += _T("exit unless($db->Name);");
                sCode += _T("print map {$db->ActiveSheet->Cells(1, $_)->Value} (1 .. $db->ActiveSheet->UsedRange->Columns->Count);");
                sCode += _T("$db->Close(0);");
            } else {
                // if we did not detect a valid extension, dont run any code
                sCode.Empty();
            }
            // if we have valid code, finish the string and run
            if(!sCode.IsEmpty()) {
                // ensure the path to the file is always a full absolute path
                TCHAR szFullPath[MAX_PATH];
                GetFullPathName(szPath, MAX_PATH, szFullPath, NULL);
                
                sCode += _T("\" \"");
                sCode += szFullPath;
                sCode += _T("\"");
                oPerlPipe.Run(sCode);
                g_smartcolumns_cache[szPath] = oPerlPipe.get();
            }
        }
    }
    return(g_smartcolumns_cache[szPath]);
}

