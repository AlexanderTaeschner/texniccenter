/********************************************************************
*
* This file is part of the TeXnicCenter-system
*
* Copyright (C) 1999-2000 Sven Wiegand
* Copyright (C) 2000-$CurrentYear$ ToolsCenter
* 
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of
* the License, or (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
* If you have further questions or if you want to support
* further TeXnicCenter development, visit the TeXnicCenter-homepage
*
*    http://www.ToolsCenter.org
*
*********************************************************************/

/********************************************************************
*
* $Id$
*
********************************************************************/

#include "stdafx.h"
#include "global.h"
#include "resource.h"
#include "BiBTeXFile.h"
#include "BiBTeXEntry.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern const TCHAR* BibTypeVerbose[];
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBiBTeXFile::CBiBTeXFile(CString file)
:CObject()
{
	m_Filename = file;
	m_ErrorCount = 0;
	m_IsATSignInBracesAllowed = TRUE;
	m_WarnWrongLevelAT = TRUE;
}

CBiBTeXFile::~CBiBTeXFile()
{
	DropAllEntries();
}


BOOL CBiBTeXFile::ProcessFile()
{
	CFile f;
	BOOL ok;

	try {
		f.Open(m_Filename, CFile::modeRead);
		DWORD l = f.GetLength();

		TCHAR *buf = new TCHAR[l];

		f.Read(buf, l);
		m_ErrorCount = 0;
		ok = ParseFile(buf);

		delete [] buf;
	} catch(CFileException &ex) {
		TRACE("Error opening style file: %s\n",ex);
		f.Close();				
		return FALSE;
	} 
	
	f.Close();
	return ok;
}

BOOL CBiBTeXFile::ParseFile(const TCHAR *buf)
{
	const TCHAR *begin, *lastChar = NULL;
	CBiBTeXEntry::BibType type;
	int depth = 0, line = 1, col = 1;
	BOOL inComment = FALSE, inQuote = FALSE, insideEntry = FALSE;

	begin = buf;	
	type = CBiBTeXEntry::Unknown;

	while (*buf) { // Parser state machine
		switch(*buf) {
		case _T('%'): // Toggle comment
			if (depth != 0) break;
			inComment = TRUE;
			break;
		case _T('\"'): // Toggle quote
			if (inComment || !insideEntry) break;

			if (depth ==1 && inQuote && (lastChar && lastChar[0] == _T('\\'))) {
				// STE_BIBTEX_ERR_QOUTEWITHINQUOTE
				HandleParseError(STE_BIBTEX_ERR_QOUTEWITHINQUOTE, line, col);
			}

			/* Quotes are taken "as is", if included in arbitrary braces */
			if (depth >= 2 || (lastChar && lastChar[0] == _T('\\'))) break;
			
			inQuote = !inQuote;
			//TRACE("Toggle quote to %d on line %d, col %d (depth = %d)\n", inQuote, line, col, depth);
			break;
		case _T('@'): // Start type
			/* The @ sign has a very strong meaning in BibTeX. It is indicating the beginning of an
			   entry, no matter which text precedes. So the entry "steward03" is considered by BibTeX!

				@Comment{
				  @Book{steward03, [...]
				  }
				}
			   You can have a @ inside a quoted values but not inside a braced value. 
			 */
			if (inComment || inQuote) break;			
			/* More relaxed handling of @? */
			if (m_IsATSignInBracesAllowed && depth>= 2) break;
			if (/*0 != depth*/ insideEntry && m_WarnWrongLevelAT) { 
				HandleParseError(STE_BIBTEX_ERR_WRONGLEVELAT, line, col);
			}
			depth = 0;
			insideEntry = TRUE;
			begin = buf; // update pointer if on top level
			break;
		case _T('('):
			if (inComment || !insideEntry) break;
			if (0 == depth) { // process entry type, e. g. @Article
				type = ProcessEntryType(begin, buf-begin, line);
				begin = buf;
				++depth; // only on level 0!
			}			
			break;
		case _T('{'):
			if (inComment || !insideEntry) break;
			if (0 == depth) { // process entry type, e. g. @Article
				type = ProcessEntryType(begin, buf-begin, line);
				begin = buf;
			} 
			++depth;			
			break;
		case _T(','):
			if (inComment || inQuote || !insideEntry) break;
			if (1 == depth) { // process entry field
				ProcessArgument(begin, buf-begin, type, line);
				begin = buf;
			}			
			break;
		case _T(')'):
			if (inComment || inQuote || !insideEntry) break;
			if (1 == depth) { // process entry field and decrease stack depth
				ProcessArgument(begin, buf-begin, type, line);

				--depth;
				if (depth == 0) {
					insideEntry = FALSE;
					FinalizeItem();
				}
				/*
				if (depth < 0) {
					HandleParseError(buf, _T("Too many ')'"),  line, col);
					return FALSE;
				}*/
			} 			
			break;
		case _T('}'):
			if (inComment || !insideEntry) break;
			if (1 == depth) { // process entry field and decrease stack depth
				ProcessArgument(begin, buf-begin, type, line);
			} 
			--depth;
			if (depth == 0) {
				insideEntry = FALSE;
				FinalizeItem();
			}
			/*
			if (depth < 0) {
				HandleParseError(buf, _T("Too many '}'"),  line, col);				
				depth = 0;
				//return FALSE;
			}*/
			break;		
		}

		if (*buf == _T('\n')) { // update line number			
			inComment = FALSE;
			col = 0;
			++line;
		}
		col++;
		lastChar = buf;
		buf++;
	}

	// Check if parser finished correctly
	if (depth > 0) {
		HandleParseError(STE_BIBTEX_ERR_INVALID_EOF, line, col);
	}

	TRACE("%s: Found %d entries\n", m_Filename, m_Entries.GetCount());
	return TRUE;
}

CBiBTeXEntry::BibType CBiBTeXFile::ProcessEntryType(const TCHAR *buf, int len, int line)
{
	TCHAR tmp[MAX_BIBTEX_ARG_LENGTH];
	
	ASSERT(len > 0 && len < MAX_BIBTEX_ARG_LENGTH);
	strncpy(tmp, buf + 1, len - 1);
	tmp[len - 1] = 0;
	//TRACE("Entry: %s\n", tmp);

	for (int i = 0; i < CBiBTeXEntry::Unknown; i++) {
		if (0 == stricmp(tmp, BibTypeVerbose[i])) {
			return (CBiBTeXEntry::BibType)i;
		}
	}
	HandleParseError(STE_BIBTEX_ERR_INVALID_TYPE, line, 1, tmp);
	return CBiBTeXEntry::Unknown;
}

void CBiBTeXFile::ProcessArgument(const TCHAR *buf, int len, CBiBTeXEntry::BibType type, int line)
{
	TCHAR tmp[MAX_BIBTEX_ARG_LENGTH];
	CBiBTeXEntry *be, *dummy;

	ASSERT(len > 0 && len < MAX_BIBTEX_ARG_LENGTH);
	strncpy(tmp, buf + 1, len - 1);
	tmp[len - 1] = 0;

	/* Skip comments and preamble */
	if (type == CBiBTeXEntry::Preamble || type == CBiBTeXEntry::Comment) {
		return;
	}

	if (type == CBiBTeXEntry::Unknown) {
		if (strlen(tmp) > 40) {
			tmp[40] = 0; // cut it here
		}
		TRACE("** Ignore unknown entry at line %d: %s\n", line, tmp);
		//STE_BIBTEX_ERR_INVALID_TYPE		
		return;
	}

	// strings have no explicit key, instead we are using the first field name as key
	if (type == CBiBTeXEntry::String && NULL != strstr(tmp, _T("="))) { 
		CString name, val;
		ParseField(tmp, name, val);

		name.TrimLeft();
		name.TrimRight();
		val.TrimLeft();
		val.TrimRight();

		m_Strings.SetAt(name, val);
		return;
	}

	if (NULL == strstr(tmp, _T("="))) { // argument is key?		
		CString key = tmp;
		key.TrimLeft();
		key.TrimRight();

		if (key.IsEmpty()) { // invalid key?
			return;
		}

		if (!m_Entries.Lookup(key, (CObject*&)dummy)) { // key already exists?
			be = new CBiBTeXEntry(tmp, this, type);
			/* add entries used by structure parser */
			be->m_nLine = line;			
			
			m_Entries.SetAt(key, be);
			m_LastKey = key;
			m_Keys.Add(key);
		} else {			
			HandleParseError(STE_BIBTEX_ERR_DUP_KEY, line, 1, key);
			TRACE("WARNING: Invalid or duplicate key <%s> (%s)\n", key, BibTypeVerbose[type]);
		}
	} else { // extract name-value pair and add it to the entry
		if (m_Entries.Lookup(m_LastKey, (CObject*&)be)) {
			CString name, val;
			ParseField(tmp, name, val);

			//TRACE("Set Value: <%s> = <%s>\n", name, val);
			name.MakeLower();
			name.TrimLeft();
			name.TrimRight();
			ReplaceSpecialChars(val);
			val.TrimLeft();
			val.TrimRight();
			be->SetField(name, val);
		} else { // error: key not found -> likely an error in the bibtex file			
			HandleParseError(STE_BIBTEX_ERR_MISSING_KEY, line, 1);
		}
	}
}

void CBiBTeXFile::HandleParseError(UINT msgID, int line, int col, const TCHAR *addDesc)
{

	CString s, key;	

	m_ErrorCount++;
	key.Format("Parse_Error%d", m_ErrorCount);
	

	CString errMsgFmt = AfxLoadString(msgID);

	switch (msgID) {
	case STE_BIBTEX_ERR_MISSING_KEY:	
	case STE_BIBTEX_ERR_INVALID_EOF:
	case STE_BIBTEX_ERR_QOUTEWITHINQUOTE:
	case STE_BIBTEX_ERR_WRONGLEVELAT:
		s.Format(errMsgFmt, m_Filename, line, col);
		break;
	case STE_BIBTEX_ERR_INVALID_TYPE:
	case STE_BIBTEX_ERR_DUP_KEY:
		s.Format(errMsgFmt, m_Filename, line, col, addDesc);
		break;
	default:
		TRACE("BibTeX: Warning: No handler for msgID %d\n", msgID);
	}

	TRACE(s);
	
	
	CBiBTeXEntry *be = new CBiBTeXEntry(s, this, CBiBTeXEntry::Error);			
	be->m_nLine = line;						
	be->m_nType = CStructureParser::bibItem;
	be->m_strTitle = s;
	be->m_strComment = _T("");
	be->m_strCaption = s;
	be->m_strLabel = key;
	be->m_strPath = m_Filename;
	m_ErrorMsgs.Add(be);
	//m_Entries.SetAt(key, be);	
	
}

BOOL CBiBTeXFile::ParseField(const TCHAR *field, CString &name, CString &val)
{
	TCHAR* eqChar;
	TCHAR tmp[MAX_BIBTEX_ARG_LENGTH];
	eqChar = strstr(field, _T("="));
	int len1, len2, n = strlen(field);

	if (NULL == eqChar) {
		return FALSE;
	}
	
	// Extract name and value
	len1 = eqChar - field;
	len2 = n - (eqChar - field) - 1;
	ASSERT(len1 < MAX_BIBTEX_ARG_LENGTH);
	ASSERT(len2 < MAX_BIBTEX_ARG_LENGTH);

	strncpy(tmp, field, len1);
	tmp[len1] = 0;

	name = tmp;

	strncpy(tmp, eqChar + 1, len2);
	tmp[len2] = 0;

	val = tmp;
	/// remove whitespace
	name.TrimRight();
	name.TrimLeft();
	val.TrimLeft();
	val.TrimRight();

	return TRUE;
}

void CBiBTeXFile::DropAllEntries()
{
	POSITION pos = m_Entries.GetStartPosition();
	while(pos != NULL) {
		CBiBTeXEntry *be;
		CString key;
		m_Entries.GetNextAssoc(pos, key, (CObject*&)be);
		if (be != NULL) {
			//TRACE("Deleting entry %s\n", be->ToString());
			delete be;
		} else {
			TRACE("NP detected in CBiBTeXFile %s", m_Filename);
		}
		m_Entries.RemoveKey(key);
	}

	m_Keys.RemoveAll();
	m_Strings.RemoveAll();
	for (int i=0;i < m_ErrorMsgs.GetSize(); i++) {
		CBiBTeXEntry *be = dynamic_cast<CBiBTeXEntry*>(m_ErrorMsgs.GetAt(i));
		if (be != NULL) {
			delete be;
		}
	}
}

void CBiBTeXFile::FinalizeItem()
{
	CBiBTeXEntry *be;
	if (m_Entries.Lookup(m_LastKey, (CObject*&)be)) {
		be->m_nType = CStructureParser::bibItem;
		be->m_strTitle = be->ToCaption();
		be->m_strCaption = be->ToCaption();
		be->m_strLabel = be->GetKey();
		be->m_strComment = BibTypeVerbose[be->GetType()];
		be->m_strPath = m_Filename;
		//TRACE("Finalized %s\n", be->ToString());
	} else {
		//ASSERT(FALSE);
		// TRACE("Warning: Did not found key %s\n", m_LastKey);
	}
}

const CBiBTeXEntry* CBiBTeXFile::GetEntryByKey(CString key)
{
	CBiBTeXEntry *be;
	if (m_Entries.Lookup(m_LastKey, (CObject*&)be)) {
		return (const CBiBTeXEntry*)be;
	}
	return NULL;
}

CString CBiBTeXFile::GetString(CString abbrev)
{
	CString expanded;
	if (m_Strings.Lookup(abbrev, expanded)) {
		return CString(expanded);
	}
	return CString();
}

void CBiBTeXFile::ReplaceSpecialChars(CString &value)
{
	// Strip off surrounding quotes
	if (!value.IsEmpty() && value[0] == '\"' && value[value.GetLength()-1] == '\"') {
		value.Delete(0);
		value.Delete(value.GetLength()-1);
	}
	// Strip off surrounding braces
	if (!value.IsEmpty() && value[0] == '{' && value[value.GetLength()-1] == '}') {
		value.Delete(0);
		value.Delete(value.GetLength()-1);
	}

	// Replace umlauts (incomplete!)
	value.Replace(_T("\"o"), _T("�"));
	value.Replace(_T("\"a"), _T("�"));
	value.Replace(_T("\"u"), _T("�"));
	value.Replace(_T("\"O"), _T("�"));
	value.Replace(_T("\"A"), _T("�"));
	value.Replace(_T("\"U"), _T("�"));
	value.Replace(_T("\"ss"), _T("�"));
}
