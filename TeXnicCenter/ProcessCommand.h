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

#if !defined(AFX_PROCESSCOMMAND_H__65CF1820_0593_11D5_A222_006097239934__INCLUDED_)
#define AFX_PROCESSCOMMAND_H__65CF1820_0593_11D5_A222_006097239934__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "process.h"

/**
Stores information about a process to start and provides 
functionality to start the specified process.

@author Sven Wiegand
*/
class CProcessCommand  
{
// construction/destruction
public:
	CProcessCommand();
	virtual ~CProcessCommand();

// attribute operations
public:
	/**
	Sets the properties of the command.

	Take a look at the <var>AfxExpandPlaceholders</var> to get more
	detailed information about the allowed placeholders in the argument
	list.

	@see AfxExpandPlaceholders

	@param lpszExecutable
		Path to the file to execute.
	@param lpszArguments
		Arguments to pass to the process on its command line. May contain
		placeholders allowed by AfxExpandPlaceholders.
	*/
	void Set(LPCTSTR lpszExecutable, LPCTSTR lpszArguments);

	CString GetExecutable() const;
	CString GetArguments() const;

	/**
	The error of the last operation.

	@return 0 if no error else result of ::GetLastError()
	*/
	DWORD GetLastError() const;

// operations
public:
	/**
	Starts the specified process, whereby the placeholders in the 
	argument list will be replaced by the appropriate parts of the
	given information.

	Take a look at the <var>AfxExpandPlaceholders</var> to get more
	detailed information.

	Remark: The caller is responsible for deleting the returned
	CProcess-object.

	@see AfxExpandPlaceholders

	@param lpszWorkingDir
		Path to the directory to execute the command in.
	@param lpszMainPath
		Path of the project's main file.
	@param lpszCurrentPath
		Path of the current file in the editor or NULL to prevent the
		method from replacing the current file specific placeholders.
	@param lCurrentLine
		Lineindex of the current file in the editor or -1 to prevent the
		method from replacing the current line specific placeholders.
	@param lpszCurrentSelection
		Selected text in the current document.
	@param bExpandPlaceholderSets
		Set to True, if you want Sets-Placeholders like "$qrAPF" to be expanded.

	@return
		Returns a pointer to a CProcess-object, if the process has been 
		started successfully or NULL otherwise. The caller is responsible
		for deleting the returned process object.
	*/
	CProcess* Execute(LPCTSTR lpszWorkingDir, LPCTSTR lpszMainPath,
										LPCTSTR lpszCurrentPath = NULL, long lCurrentLine = -1,
										LPCTSTR lpszCurrentSelection = NULL,
										bool bExpandPlaceholderSets = false) const;

	/**
	Starts the specified console process without a visible window, 
	whereby the placeholders in the argument list will be replaced by 
	the appropriate parts of the given information and the output 
	generated by the process will be written to the specified handle.

	Take a look at the <var>AfxExpandPlaceholders</var> to get more
	detailed information.

	Remark: The caller is responsible for deleting the returned
	CProcess-object.

	@see AfxExpandPlaceholders

	@param hOutput
		Handle to write the output, generated by the process, to.
	@param lpszWorkingDir
		Path to the directory to execute the command in.
	@param lpszMainPath
		Path of the project's main file.
	@param lpszCurrentPath
		Path of the current file in the editor or NULL to prevent the
		method from replacing the current file specific placeholders.
	@param lCurrentLine
		Lineindex of the current file in the editor or -1 to prevent the
		method from replacing the current line specific placeholders.
	@param lpszCurrentSelection
		Selected text in the current document.
	@param bExpandPlaceholderSets
		Set to True, if you want Sets-Placeholders like "$qrAPF" to be expanded.

	@return
		Returns a pointer to a CProcess-object, if the process has been 
		started successfully or NULL otherwise. The caller is responsible
		for deleting the returned process object.
	*/
	CProcess *Execute(HANDLE hOutput, LPCTSTR lpszWorkingDir, LPCTSTR lpszMainPath,
										LPCTSTR lpszCurrentPath = NULL, long lCurrentLine = -1,
										LPCTSTR lpszCurrentSelection = NULL,
										bool bExpandPlaceholderSets = false) const;

// string serialization
public:
	/**
	Packs the information of this command into one string and returns 
	this string.
	*/
	CString SerializeToString() const;

	/**
	Sets the attributes with the information received from the given
	string.

	@return
		TRUE on success, FALSE if the given string does not have the 
		right format.
	*/
	BOOL SerializeFromString(LPCTSTR lpszPackedInformation);

// attributes
protected:
	/** Executable file to start */
	CString m_strExecutable;

	/** 
	Argument list to pass to the process on the command line. May
	contain place holders.
	*/
	CString m_strArguments;

	DWORD m_nLastError;
};


// inlines
inline
CString CProcessCommand::GetExecutable() const
{
	return m_strExecutable;
}


inline
CString CProcessCommand::GetArguments() const
{
	return m_strArguments;
}


inline
DWORD CProcessCommand::GetLastError() const
{
	return m_nLastError;
}

#endif // !defined(AFX_PROCESSCOMMAND_H__65CF1820_0593_11D5_A222_006097239934__INCLUDED_)
