/*
    UsageToken - holds information about a single parameter
    
    Copyright 2014 Vale

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0
*/

// parses a string into a field structure
class UsageToken : public CString
{
public:
    CString m_name;
    CString m_type;
    CString m_data;
    static const CString sTypeList; // holds the characters which represent each type
    static int nId;
    enum {
        TYPE_BASE = 0, // :
        TYPE_LIST = 1, // =
        TYPE_FILE = 2, // *
        TYPE_BOOL = 3, // @
        TYPE_GRID = 4, // #
        TYPE_NONE = -1 // blank
    };
    int m_rows; // number of visible rows a control should have
    UsageToken(CString sText = _T("")) : CString(sText), m_rows(1) {
        nId++;
        int nIndex = sText.FindOneOf(sTypeList);
        
        if(nIndex == -1) {
            m_name = sText;
        } else {
            m_name = sText.Left(nIndex);
            m_type = sText.Mid(nIndex, 1);
            m_data = sText.Mid(nIndex + 1);
        }
        // handle the possible case of blank name
        // create a automatic name using the number of instances of this class
        if(m_name.IsEmpty()) {
            m_name.Format(_T("n%d"),nId);
        }
    }
    ~UsageToken() {
        nId--;
    }
    // for grids we need to split the m_data into new tokens for each column
    std::vector<UsageToken> GetGridTokens() {
        auto vGrid = CString_split(m_data, _T("#"));
        std::vector<UsageToken> vRet;
        for(auto i=vGrid.begin();i != vGrid.end();++i) {
            vRet.push_back(UsageToken(*i));
        }
        return(vRet);
    }
    int type() {
        if(!m_type.IsEmpty()) {
            return(sTypeList.Find(m_type));
        }
        return(TYPE_NONE);
    }
};
// initialize the "constant" list of type characters
const CString UsageToken::sTypeList = _T(":=*@#");
int UsageToken::nId = 1000;
