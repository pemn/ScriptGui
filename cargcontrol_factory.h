/*
    Copyright 2014 Vale
 
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0
*/

// Implements the control factory which makes persistent objects
// thanks to the shared pointer which prevent garbage colections
std::shared_ptr<CArgControl> CArgControl::Factory(UsageToken &oToken, BOOL bRegister) {
    std::shared_ptr<CArgControl> pCtrl;
    // file browse control
    if(oToken.m_type == TOKEN_FILE) {
        pCtrl = std::make_shared<CArgBrowse>(oToken);
    } else
    // checkbox control
    if(oToken.m_type == TOKEN_BOOL) {
        pCtrl = std::make_shared<CArgCheck>(oToken);
    } else
    // grid special control
    if(oToken.m_type == TOKEN_GRID) {
        oToken.m_rows = CArgControl::CS_GRID_ROWS;
        pCtrl = std::make_shared<CArgListview>(oToken);
    } else { // combo control, handles both predefined lists and derived fields
        pCtrl = std::make_shared<CArgCombo>(oToken);
    }
    // add this new instance to the static list of instances
    // but dont add unamed, temporary instances
    if(bRegister) {
        m_instances.push_back(pCtrl);
    }
    return(pCtrl);
}

