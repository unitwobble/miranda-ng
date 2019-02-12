/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-19 Miranda NG team (https://miranda-ng.org)
Copyright (c) 2000-08 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef M_XML_H__
#define M_XML_H__

#include <tchar.h>
#include <m_core.h>

/////////////////////////////////////////////////////////////////////////////////////////
// new API to replace the old one

#ifdef MIR_CORE_EXPORTS
#define TINYXML2_EXPORT 1
#else
#define TINYXML2_IMPORT 1
#endif

#include "../src/mir_core/src/tinyxml2.h"

/////////////////////////////////////////////////////////////////////////////////////////
// old API to be removed once

DECLARE_HANDLE(HXML);

EXTERN_C MIR_APP_DLL(HXML)    xmlCreateNode(LPCTSTR name, LPCTSTR text, char isDeclaration);
EXTERN_C MIR_APP_DLL(void)    xmlDestroyNode(HXML node);

EXTERN_C MIR_APP_DLL(HXML)    xmlParseString(LPCTSTR string, int *datalen, LPCTSTR tag);
EXTERN_C MIR_APP_DLL(LPTSTR)  xmlToString(HXML node, int *datalen);

EXTERN_C MIR_APP_DLL(HXML)    xmlAddChild(HXML parent, LPCTSTR name, LPCTSTR text);
EXTERN_C MIR_APP_DLL(void)    xmlAddChild2(HXML child, HXML parent);
EXTERN_C MIR_APP_DLL(HXML)    xmlCopyNode(HXML parent);
EXTERN_C MIR_APP_DLL(HXML)    xmlGetChild(HXML parent, int number);
EXTERN_C MIR_APP_DLL(int)     xmlGetChildCount(HXML);
EXTERN_C MIR_APP_DLL(HXML)    xmlGetChildByAttrValue(HXML parent, LPCTSTR name, LPCTSTR attrName, LPCTSTR attrValue);
EXTERN_C MIR_APP_DLL(HXML)    xmlGetFirstChild(HXML parent);
EXTERN_C MIR_APP_DLL(HXML)    xmlGetNthChild(HXML parent, LPCTSTR name, int i);
EXTERN_C MIR_APP_DLL(HXML)    xmlGetChildByPath(HXML parent, LPCTSTR path, char createNodeIfMissing);
EXTERN_C MIR_APP_DLL(HXML)    xmlGetNextNode(HXML node);
EXTERN_C MIR_APP_DLL(LPCTSTR) xmlGetName(HXML);
EXTERN_C MIR_APP_DLL(HXML)    xmlGetParent(HXML);
EXTERN_C MIR_APP_DLL(LPCTSTR) xmlGetText(HXML);
EXTERN_C MIR_APP_DLL(void)    xmlSetText(HXML, LPCTSTR);

EXTERN_C MIR_APP_DLL(LPCTSTR) xmlGetAttr(HXML, int i);
EXTERN_C MIR_APP_DLL(LPCTSTR) xmlGetAttrName(HXML, int i);
EXTERN_C MIR_APP_DLL(LPCTSTR) xmlGetAttrValue(HXML, LPCTSTR attrName);
EXTERN_C MIR_APP_DLL(int)     xmlGetAttrCount(HXML);
EXTERN_C MIR_APP_DLL(void)    xmlAddAttr(HXML, LPCTSTR attrName, LPCTSTR attrValue);
EXTERN_C MIR_APP_DLL(void)    xmlAddAttrInt(HXML, LPCTSTR attrName, int attrValue);

EXTERN_C MIR_APP_DLL(void)    xmlFree(void*);

// methods added in xml API v2
EXTERN_C MIR_APP_DLL(LPCTSTR) xmlGetClear(HXML, int i, LPCTSTR *openTag, LPCTSTR *closeTag);

EXTERN_C MIR_APP_DLL(HXML)     xmlParseFile(LPCTSTR filename, int *datalen, LPCTSTR tag);
EXTERN_C MIR_APP_DLL(int)      xmlToFile(HXML node, LPCTSTR filename, int withformattiing);

#endif // M_XML_H__
