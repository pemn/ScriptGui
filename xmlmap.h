/*
    class to bind a std map to a xml file
    
    Copyright 2014 Vale

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0
*/


#include <comdef.h>
#include <map>

template <class K, class V>
class XmlMap : public std::map<K, V> {
public:
    
    // return a list of all help script files 
    std::vector<K> GetSelectors() {
        std::vector<K> vExt;
        vExt.push_back(K("XML"));
        auto aList = VectorFilterSufix(FileSearch<K>(NULL, GetModuleName()), vExt);
        for(auto i=aList.begin();i!=aList.end();++i) {
            int nTrace = i->ReverseFind('-') + 1;
            int nDot = i->ReverseFind('.');
            *i = i->Mid(nTrace, nDot - nTrace);
        }
        return(aList);
    }

    // Convert a simple selector work to a actual file
    // by adding the script base name and a .xml extension
    K SelectFilename(K strSelector) {
        K strFilename(GetModuleName());
        if(strSelector.GetLength() == 0) {
            strFilename += _T("-default.xml");
        } else {
            strFilename.Format(_T("%s-%s.xml"), strFilename, strSelector);
        }
        //~ odprintf("SelectFilename %s selector %s size %d", strFilename, strSelector, strSelector.GetLength());
        return(strFilename);
    }
    
    // write this map to a file corresponding to a given selector
    HRESULT save(K strSelector = _T("")) {
        HRESULT hr = S_OK;
        IXMLDOMDocumentPtr ptrNamespacesXML = NULL;
        IXMLDOMElementPtr ptrNamespacesRoot = NULL;
        IXMLDOMElement* oElement;
        IXMLDOMNode* oNode;
        IXMLDOMCDATASection* oCData;
        hr = ptrNamespacesXML.CreateInstance(CLSID_DOMDocument);
        //~ hr = ptrNamespacesXML->createNode(NODE_ELEMENT, NULL, NULL, &oNode);
        hr = ptrNamespacesXML->createElement(SysAllocString(L"root"), &oElement);
        hr = ptrNamespacesXML->putref_documentElement(oElement);
        hr = ptrNamespacesXML->get_documentElement(&ptrNamespacesRoot);
        
        
        for(auto i = begin(); i != end();++i) {
            hr = ptrNamespacesXML->createElement((*i).first.AllocSysString(), &oElement);
            hr = ptrNamespacesRoot->appendChild(oElement, &oNode);
            //~ oNode = oChild;
            hr = ptrNamespacesXML->createCDATASection((*i).second.AllocSysString(), &oCData);
            hr = oNode->appendChild(oCData, NULL);
        }
        
        hr = ptrNamespacesXML->save(_variant_t(SelectFilename(strSelector)));
		// odprintf("hr %s, %d", SelectFilename(strSelector), hr);
        return hr;
    }
    
    // load the a file corresponding to a given selector
    HRESULT load(K strSelector = _T("")) {

        HRESULT hr = S_OK;
        IXMLDOMDocumentPtr ptrNamespacesXML = NULL;
        IXMLDOMElementPtr ptrNamespacesRoot = NULL;
        IXMLDOMNodeList* oNodeList;
        IXMLDOMNode* oNode;
        IXMLDOMCDATASection* oCData;
        VARIANT_BOOL vbLoaded = VARIANT_FALSE;
        hr = ptrNamespacesXML.CreateInstance(CLSID_DOMDocument);
        // Load the XML file.
        hr = ptrNamespacesXML->load(_variant_t(SelectFilename(strSelector)), &vbLoaded);
        if (vbLoaded != VARIANT_TRUE) {
            return hr;
        }
        // Get the documents root element
        hr = ptrNamespacesXML->get_documentElement(&ptrNamespacesRoot);
        
        hr = ptrNamespacesRoot->get_childNodes(&oNodeList);
        long nNodeListLength;
        hr = oNodeList->get_length(&nNodeListLength);
        BSTR sBuffer;
        VARIANT sVar;
        // clear base class
        clear();

        for(long i = 0;i < nNodeListLength;i++) {
            hr = oNodeList->get_item(i, &oNode);
            hr = oNode->get_nodeName(&sBuffer);
            // we know the first child is a CData
            hr = oNode->get_firstChild((IXMLDOMNode**)&oCData);
            hr = oCData->get_nodeValue(&sVar);
            insert(std::pair<K, V>(CString(sBuffer),CString(sVar)));
        }
        
        return hr;
    }
};
