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

#if !defined(AFX_LATEXOUTPUTFILTER_H__10B312A4_21EB_11D5_A222_006097239934__INCLUDED_)
#define AFX_LATEXOUTPUTFILTER_H__10B312A4_21EB_11D5_A222_006097239934__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "OutputFilter.h"
#include "Stack.h"

/**
An object of this class is used to parse the output messages 
generated by a TeX/LaTeX-compiler.

@author Sven Wiegand
*/
class CLatexOutputFilter : public COutputFilter  
{
// construction/destruction
public:
	CLatexOutputFilter(BOOL bAutoDelete = FALSE);

// implementation
public:
	/**
	Returns the number of output pages, that have been created by LaTeX.
	*/
	int GetNumberOfOutputPages() const;

// implementation helpers
protected:
	/**
	Parses the given line for the start of new files or the end of
	old files.
	*/
	void UpdateFileStack(const CString &strLine);

	/**
	Forwards the currently parsed item to the item list.
	*/
	void FlushCurrentItem();

	/**
	Extracts the line number, that begins at the specified character
	(zero based) of the specified string.
	*/
	long ExtractLineNumber(const CString &strLine, int nStartChar);

// overridings
public:
	virtual CString GetResultString();

protected:
	virtual BOOL OnPreCreate();
	virtual DWORD ParseLine(CString strLine, DWORD dwCookie);
	virtual BOOL OnTerminate();

// types
protected:
	/** 
	These constants are describing, which item types is currently 
	parsed.
	*/
	enum tagCookies
	{
		itmNone = 0,
		itmError,
		itmWarning,
		itmBadBox
	};

// attributes
private:
	/** number or errors detected */
	int m_nErrors;

	/** number of warning detected */
	int m_nWarnings;

	/** number of bad boxes detected */
	int m_nBadBoxes;

	/** number of output pages detected */
	int m_nOutputPages;

	/** 
	Stack containing the files parsed by the compiler. The top-most
	element is the currently parsed file.
	*/
	CStack<CString, CString&> m_stackFile;

	/** The item currently parsed. */
	COutputInfo m_currentItem;
};

#endif // !defined(AFX_LATEXOUTPUTFILTER_H__10B312A4_21EB_11D5_A222_006097239934__INCLUDED_)
