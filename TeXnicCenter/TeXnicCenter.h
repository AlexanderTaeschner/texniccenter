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

#if !defined(AFX_TEXNICCENTER_H__447410E1_1058_11D3_929E_D75EECD5B417__INCLUDED_)
#define AFX_TEXNICCENTER_H__447410E1_1058_11D3_929E_D75EECD5B417__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // Hauptsymbole
#include "LatexProject.h"
#include "LatexEdit.h"
#include "MDIFrameManager.h"
#include "ProjectSupportingWinApp.h"
#include "ProjectTemplate.h"

extern class CTeXnicCenterApp	theApp;

class CTeXnicCenterApp : public CProjectSupportingWinApp, public CBCGWorkspace, public CSpellerSource
{
public:
	CTeXnicCenterApp();

// overridings
public:
	//virtual void ForwardCommands( HCONV hconvServer );

	// Override from CBCGWorkspace
	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	//{{AFX_VIRTUAL(CTeXnicCenterApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual BOOL OnDDECommand(LPTSTR lpszCommand);
	//}}AFX_VIRTUAL

// message handlers
protected:
	//{{AFX_MSG(CTeXnicCenterApp)
	afx_msg void OnAppAbout();
	afx_msg void OnTip();
	afx_msg void OnFileNew();
	afx_msg void OnFileSaveAll();
	afx_msg void OnDisableStdCmd(CCmdUI* pCmdUI);
	afx_msg void OnProjectOpen();
	afx_msg void OnUpdateProjectClose(CCmdUI* pCmdUI);
	afx_msg void OnLatexNew();
	afx_msg void OnExtrasOptions();
	afx_msg void OnLatexProfile();
	afx_msg void OnDocumentSaved();
	afx_msg void OnUpdateFileMRU(CCmdUI* pCmdUI);
	afx_msg void OnLatexProfileSel();
	afx_msg void OnFileOpen();
	afx_msg void OnAppLicense();
	afx_msg void OnProjectSave();
	afx_msg void OnUpdateProjectSave(CCmdUI* pCmdUI);
	afx_msg void OnLatexEditProfiles();
	afx_msg void OnUpdateWindowCloseAll(CCmdUI* pCmdUI);
	afx_msg void OnWindowCloseAll();
	afx_msg void OnHelp();
	afx_msg void OnUpdateProject();
	//}}AFX_MSG
	afx_msg void OnUpdateFileMRUProjectList( CCmdUI* pCmdUI );
	afx_msg void OnUpdateFileMRUFileList( CCmdUI* pCmdUI );
	afx_msg void OnFileMRUProject( UINT unID );
	afx_msg void OnFileMRUFile( UINT unID );
	afx_msg void OnUpdateURL( CCmdUI* pCmdUI );
	afx_msg void OnURL( UINT unID );
	DECLARE_MESSAGE_MAP()
private:
	void ShowTipAtStartup(void);
private:
	void ShowTipOfTheDay(void);

// operations
public:
	/**
	Returns a pointer to the document template, all LaTeX-documents 
	are based on.
	*/
	CMultiDocTemplate *GetLatexDocTemplate();

	/**
	Returns a pointer to the active CLatexEdit-object or NULL, if there is
	none.
	*/
	CLatexEdit *GetActiveEditView();

	/** Returns a pointer to the LaTeX-document with the specified path and read-only
	attribute. If the document is not open it is NOT opened and NULL is returned.

	@param lpszFileName
		Path of the file to find.
	@param bReadOnly
		TRUE, if the file is write protected.

    @return
		A pointer to an already opened document, or NULL if the document is
		not already opend.
	*/
	CDocument *GetOpenLatexDocument(LPCTSTR lpszFileName, BOOL bReadOnly = FALSE);

	/**
	Returns a pointer to the LaTeX-document with the specified path.

	If this document is already opened with the same read-write-status,
	this method returns a pointer to this open document, otherwise the
	document is created at read.

	@param lpszFileName
		Path of the file to open.
	@param bReadOnly
		TRUE, if the file should be opened write protected.

	@return 
		A pointer to the specified document or NULL, if no such document
		exists.
	*/
	CDocument *GetLatexDocument(LPCTSTR lpszFileName, BOOL bReadOnly = FALSE);

	/**
	Opens the specified File at the specified line. If the file is allready
	open, its main view is activated an the cursor is set to the specified
	line.

	@param lpszFileName 
		Path of the file to open.
	@param bReadOnly 
		TRUE, it the file should be opened write protected.
	@param nLineNumber 
		Number of the line to set the caret on or -1.
	@param bError 
		If bError is TRUE, a mark is set in the document on the specified line.

	@return A pointer to the document.
	*/
	CDocument *OpenLatexDocument( LPCTSTR lpszFileName, BOOL bReadOnly = FALSE, int nLineNumber = -1, BOOL bError = FALSE );

	/**
	Returns a string associated with the project document template.

	See CDocTemplate::GetDocString() for further explanation.
	*/
	CString GetProjectString( enum CProjectTemplate::ProjectStringIndex index );

	/**
	Returns a string associated with the latex document template.

	See CDocTemplate::GetDocString() for further explanation.
	*/
	CString GetLatexString( enum CDocTemplate::DocStringIndex index );

	/**
	Returns a complete filter string for the lpszFilter parameter to the
	CFileDialog-constructor.
	*/
	CString GetProjectFileFilter();

	/**
	Returns a complete filter string for the lpszFilter parameter to the
	CFileDialog-constructor.
	*/
	CString GetLatexFileFilter();

	/**
	Saves all modified files, without prompting the user.
	*/
	void SaveAllModifiedWithoutPrompt();

	/**
	Opens the procject with the specified path.

	If there is allready a project open, it is closed using
	OnCloseProject().

	@see #OnCloseProject

	@param lpszPath
		Path of the project to open.

	@return
		TRUE if successfull, FALSE otherwise.
	*/
	BOOL OpenProject( LPCTSTR lpszPath );

	/**
	Returns a pointer to the one and only <var>CLatexProject</var>-object
	of the application or <var>NULL</var> if no such object exists.
	*/
	CLatexProject *GetProject();

	/**
	Returns the path of the directory, that was the current one, while
	TeXnicCenter was started (working direcotry).
	*/
	CString GetWorkingDir();

	/**
	Prepares for ending the session.

	Returns to the caller, before application has quit. Does some simple
	cleaning up, i.e. closing current project, etc.
	*/
	void EndSession();

	/**
	Updates the ComboBox for output profile selection.
	*/
	void UpdateLatexProfileSel();

	/**
	Opens the specified URL.

	@return
		FALSE if an error occurd, TRUE otherwise.
	*/
	BOOL OpenURL( LPCTSTR lpszURL );

	/**
	Retrieves the background thread.

	@return 
		Pointer to background thread.
		else NULL.
	*/
	CWinThread* GetBackgroundThread();

// overrides
public:
	// CSpellerSource
	virtual MySpell* GetSpeller();

// implementation helpers
protected:

// attributes
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	/** Accelerator manager */
	//CBCGKeyboardManager m_keyboardManager;

	/** Context menu manager */
	//CBCGContextMenuManager m_contextMenuManager;

	/** TRUE, if the session is ending, FALSE otherwise. */
	BOOL m_bEndSession;

	/** The path of the application in the registry unter "HKCU\\Software". */
	CString m_strRegistryRoot;

	/** Frame manager that handle the MDI-child-frames. */
	CMDIFrameManager *m_pMDIFrameManager;

	/** List of recently used projects. */
	CRecentFileList m_recentProjectList;

	/** 
	Specifies, whether to draw flat normal borders around tab-controls
	or flat ones.
	*/
	BOOL m_bTabFlatBorders;

	/** Specifies, whether to show document tabs or not */
	BOOL m_bMDITabs;

	/** Specifies, whether to show icons on document tabs or not */
	BOOL m_bMDITabIcons;

	/** Location, where to display the document tabs. */
	int m_nMDITabLocation;

protected:
	/** Document template for normal latex-documents */
	CMultiDocTemplate	*m_pLatexDocTemplate;

	/** Document template for projects */
	CSingleProjectTemplate *m_pProjectDocTemplate;

	/** TRUE, if we are currently saving all files, FALSE otherwise. */
	BOOL m_bSavingAll;

	/** 
	Working directory .
	(Directory that was the current one while start up of TeXnicCenter). 
	*/
	CString m_strWorkingDir;

	/** 
	Personal directory .
	(File system directory that serves as a common repository for documents.)
	*/
	CString m_strPersonalDir;

	/**
	Handle to localized version of BCGCB-resource-DLL.
	*/
	HINSTANCE m_hInstBCGCBRes;

	/** Spell checker */
	MySpell *m_pSpell;

	/** Background thread that processes spelling and other tasks */
	CWinThread *m_pBackgroundThread;

	/** Critical section to protect lazy resource initialization */
	CRITICAL_SECTION m_csLazy;
};

inline
CLatexProject *CTeXnicCenterApp::GetProject()
{
	if( !m_pProjectDocTemplate )
		return NULL;

	POSITION	pos = m_pProjectDocTemplate->GetFirstProjectPosition();
	if( !pos )
		return NULL;

	return (CLatexProject*)m_pProjectDocTemplate->GetNextProject(pos);
}


inline
CString CTeXnicCenterApp::GetWorkingDir()
{
	return m_strWorkingDir;
}


inline
CMultiDocTemplate	*CTeXnicCenterApp::GetLatexDocTemplate()
{
	return m_pLatexDocTemplate;
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ f�gt unmittelbar vor der vorhergehenden Zeile zus�tzliche Deklarationen ein.

#endif // !defined(AFX_TEXNICCENTER_H__447410E1_1058_11D3_929E_D75EECD5B417__INCLUDED_)
