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

#ifndef _SPLASH_SCRN_
#define _SPLASH_SCRN_

/** Splash Screen of TeXnicCenter displayed during startup.

@ingroup dialogs

@author Sven Wiegand
 */
class CSplashWnd : public CWnd
{
	CProgressCtrl progress_bar_;
	CStatic message_;
	CFont font_;
// Konstruktion
protected:
	CSplashWnd();

// Attribute:
public:
	CBitmap m_bitmap;

// Operationen
public:
	static void EnableSplashScreen(BOOL bEnable = TRUE);
	static void ShowSplashScreen(CWnd* pParentWnd = NULL);
	static BOOL PreTranslateAppMessage(MSG* pMsg);

	// ‹berschreibungen
	// Vom Klassen-Assistenten generierte ‹berschreibungen virtueller Funktionen
	//{{AFX_VIRTUAL(CSplashWnd)
	//}}AFX_VIRTUAL

// Implementierung
public:
	virtual ~CSplashWnd();
	virtual void PostNcDestroy();

public:
	/** Hides the Splash Window. Do not use the public pointer after calling this function! Will be NULL.*/
	void HideSplashScreen();
	static CSplashWnd* c_pSplashWndPublic;

protected:
	BOOL Create(CWnd* pParentWnd = NULL);
	static BOOL c_bShowSplashWnd;
	static CSplashWnd* c_pSplashWnd;

// Generierte Nachrichtenzuordnungsfunktionen
protected:
	//{{AFX_MSG(CSplashWnd)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};


#endif
