///////////////////////////////////////////////////////////////////////////
//	File:		CCrystalEditView.cpp
//	Version:	1.0.0.0
//	Created:	29-Dec-1998
//
//	Copyright:	Stcherbatchenko Andrei
//	E-mail:		windfall@gmx.de
//
//	Implementation of the CCrystalEditView class, a part of the Crystal Edit -
//	syntax coloring text editor.
//
//	You are free to use or modify this code to the following restrictions:
//	- Acknowledge me somewhere in your about box, simple "Parts of code by.."
//	will be enough. If you can't (or don't want to), contact me personally.
//	- LEAVE THIS HEADER INTACT
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//	21-Feb-99
//		Paul Selormey, James R. Twine:
//	+	FEATURE: description for Undo/Redo actions
//	+	FEATURE: multiple MSVC-like bookmarks
//	+	FEATURE: 'Disable backspace at beginning of line' option
//	+	FEATURE: 'Disable drag-n-drop editing' option
//
//	+	FEATURE: Auto indent
//	+	FIX: ResetView() was overriden to provide cleanup
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//	??-Aug-99
//		Sven Wiegand (search for "//BEGIN SW" to find my changes):
//	+ FEATURE: "Go to last change" (sets the cursor on the position where
//			the user did his last edit actions
//	+ FEATURE: Support for incremental search in CCrystalTextView
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//	24-Oct-99
//		Sven Wiegand
//	+ FEATURE: Supporting [Return] to exit incremental-search-mode
//		     (see OnChar())
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//	26-Apr-2000
//		Sven Wiegand
//	+ FEATURE: Deleting words with VK_CTRL + VK_DELETE or VK_CTRL + VK_BACK
//             (see BEGIN_SW(word deleting))
////////////////////////////////////////////////////////////////////////////

/********************************************************************
*
* $RCSfile$
* $Revision$
* $Date$
*
* $Author$
*
* $Log$
* Revision 1.6  2003/12/16 20:04:38  svenwiegand
* Implemented Feature 726766: "Option for selecting the language for the GUI"
*
* Revision 1.5  2003/12/06 19:59:58  svenwiegand
* - Implemented Feature 601708 + additions: The user can now set the styles for
*   the text cursor independant for the insert and the overwrite mode. The cursor
*   style is defined by its form (line or block) and its mode (blinking or
*   not-blinking). The defaults are 'line' for insert cursor and 'block' for
*   overwrite cursor -- both are blinking.
*
* Revision 1.4  2002/06/27 14:43:26  svenwiegand
* Instead of the character test functions (isalpha, isdigit, etc.) from the standard library, our own wrappers from the character.h file (myspell directory) are used now, to fix character recognition problems (bug 551033).
*
* Revision 1.3  2002/04/23 21:45:09  cnorris
* realtime spell check
*
* Revision 1.2  2002/04/06 05:19:05  cnorris
* ReplaceSelection inserts text at the cursor if there is no selected text
*
* Revision 1.1.1.1  2002/02/26 08:11:51  svenwiegand
* Initial revision
*
* Revision 1.0  2000-05-31 21:55:21+02  sven_wiegand
* Initial revision
*
*
********************************************************************/

#include "stdafx.h"
#include "editcmd.h"
#include "editreg.h"
#include "CCrystalEditView.h"
#include "CCrystalTextBuffer.h"
#include "CEditReplaceDlg.h"
#include "../../MySpell/Character.h"

#ifndef __AFXPRIV_H__
#pragma message("Include <afxpriv.h> in your stdafx.h to avoid this message")
#include <afxpriv.h>
#endif
#ifndef __AFXOLE_H__
#pragma message("Include <afxole.h> in your stdafx.h to avoid this message")
#include <afxole.h>
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define DRAG_BORDER_X		5
#define DRAG_BORDER_Y		5


/////////////////////////////////////////////////////////////////////////////
// CEditDropTargetImpl class declaration

class CEditDropTargetImpl : public COleDropTarget
{
private:
	CCrystalEditView *m_pOwner;
public:
	CEditDropTargetImpl(CCrystalEditView *pOwner) { m_pOwner = pOwner; };

	virtual DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual void OnDragLeave(CWnd* pWnd);
	virtual DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
	virtual DROPEFFECT OnDragScroll(CWnd* pWnd, DWORD dwKeyState, CPoint point);
};


/////////////////////////////////////////////////////////////////////////////
// CCrystalEditView

IMPLEMENT_DYNCREATE(CCrystalEditView, CCrystalTextView)

CCrystalEditView::CCrystalEditView()
{
	AFX_ZERO_INIT_OBJECT(CCrystalTextView);
	m_bAutoIndent = TRUE;
}

CCrystalEditView::~CCrystalEditView()
{
}


BEGIN_MESSAGE_MAP(CCrystalEditView, CCrystalTextView)
	//{{AFX_MSG_MAP(CCrystalEditView)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCut)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_COMMAND(ID_EDIT_DELETE, OnEditDelete)
	ON_WM_CHAR()
	ON_COMMAND(ID_EDIT_DELETE_BACK, OnEditDeleteBack)
	ON_COMMAND(ID_EDIT_UNTAB, OnEditUntab)
	ON_COMMAND(ID_EDIT_TAB, OnEditTab)
	ON_COMMAND(ID_EDIT_SWITCH_OVRMODE, OnEditSwitchOvrmode)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SWITCH_OVRMODE, OnUpdateEditSwitchOvrmode)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_COMMAND(ID_EDIT_REPLACE, OnEditReplace)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateEditRedo)
	ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI(ID_EDIT_INDICATOR_READ, OnUpdateIndicatorRead)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_OVR, OnUpdateIndicatorOvr)
	ON_UPDATE_COMMAND_UI(ID_EDIT_INDICATOR_COL, OnUpdateIndicatorCol)
	//BEGIN_SW
	ON_UPDATE_COMMAND_UI(ID_EDIT_GOTO_LAST_CHANGE, OnUpdateEditGotoLastChange)
	ON_COMMAND(ID_EDIT_GOTO_LAST_CHANGE, OnEditGotoLastChange)
	//END_SW
	//BEGIN_SW(word deleting)
	ON_COMMAND(ID_EDIT_DELETE_WORD, OnEditDeleteWord)
	ON_COMMAND(ID_EDIT_DELETE_WORD_BACK, OnEditDeleteWordBack)
	//END_SW
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CCrystalEditView message handlers

void CCrystalEditView::ResetView()
{
	m_bAutoIndent = TRUE;
	m_bOvrMode = FALSE;
	CCrystalTextView::ResetView();
}

BOOL CCrystalEditView::QueryEditable()
{
	if (m_pTextBuffer == NULL)
		return FALSE;
	return ! m_pTextBuffer->GetReadOnly();
}

void CCrystalEditView::OnEditPaste() 
{
	Paste();
}

void CCrystalEditView::OnUpdateEditPaste(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(TextInClipboard());
}

void CCrystalEditView::OnUpdateEditCut(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(IsSelection());
}

void CCrystalEditView::OnEditCut() 
{
	Cut();
}

BOOL CCrystalEditView::DeleteCurrentSelection()
{
	if (IsSelection())
	{
		CPoint ptSelStart, ptSelEnd;
		GetSelection(ptSelStart, ptSelEnd);

		CPoint ptCursorPos = ptSelStart;
		ASSERT_VALIDTEXTPOS(ptCursorPos);
		SetAnchor(ptCursorPos);
		SetSelection(ptCursorPos, ptCursorPos);
		EnsureVisible(ptCursorPos);

		// [JRT]:
		m_pTextBuffer->DeleteText(this, ptSelStart.y, ptSelStart.x, ptSelEnd.y, ptSelEnd.x, CE_ACTION_DELSEL);

		SetCursorPos(ptCursorPos); //PV just moved
		return TRUE;
	}
	return FALSE;
}

void CCrystalEditView::Paste()
{
	if (! QueryEditable())
		return;
	if (m_pTextBuffer == NULL)
		return;

	m_pTextBuffer->BeginUndoGroup();

	DeleteCurrentSelection();

	CString text;
	if (GetFromClipboard(text))
	{
		CPoint ptCursorPos = GetCursorPos();
		ASSERT_VALIDTEXTPOS(ptCursorPos);
		int x, y;
		m_pTextBuffer->InsertText(this, ptCursorPos.y, ptCursorPos.x, text, y, x, CE_ACTION_PASTE); //	[JRT]
		ptCursorPos.x = x;
		ptCursorPos.y = y;
		ASSERT_VALIDTEXTPOS(ptCursorPos);
		SetAnchor(ptCursorPos);
		SetSelection(ptCursorPos, ptCursorPos);
		SetCursorPos(ptCursorPos);
		EnsureVisible(ptCursorPos);
	}

	m_pTextBuffer->FlushUndoGroup(this);
}

void CCrystalEditView::Cut()
{
	if (! QueryEditable())
		return;
	if (m_pTextBuffer == NULL)
		return;
	if (! IsSelection())
		return;

	CPoint ptSelStart, ptSelEnd;
	GetSelection(ptSelStart, ptSelEnd);
	CString text;
	GetText(ptSelStart, ptSelEnd, text);
	PutToClipboard(text);

	CPoint ptCursorPos = ptSelStart;
	ASSERT_VALIDTEXTPOS(ptCursorPos);
	SetAnchor(ptCursorPos);
	SetSelection(ptCursorPos, ptCursorPos);
	EnsureVisible(ptCursorPos);

	m_pTextBuffer->DeleteText(this, ptSelStart.y, ptSelStart.x, ptSelEnd.y, ptSelEnd.x, CE_ACTION_CUT); // [JRT]

	SetCursorPos(ptCursorPos); //PV just moved
}

void CCrystalEditView::OnEditDelete() 
{
	if (! QueryEditable() || m_pTextBuffer == NULL)
		return;

	CPoint ptSelStart, ptSelEnd;
	GetSelection(ptSelStart, ptSelEnd);
	if (ptSelStart == ptSelEnd)
	{
		if (ptSelEnd.x == GetLineLength(ptSelEnd.y))
		{
			if (ptSelEnd.y == GetLineCount() - 1)
				return;
			ptSelEnd.y ++;
			ptSelEnd.x = 0;
		}
		else
			ptSelEnd.x ++;
	}

	CPoint ptCursorPos = ptSelStart;
	ASSERT_VALIDTEXTPOS(ptCursorPos);
	SetAnchor(ptCursorPos);
	SetSelection(ptCursorPos, ptCursorPos);
	EnsureVisible(ptCursorPos);

	m_pTextBuffer->DeleteText(this, ptSelStart.y, ptSelStart.x, ptSelEnd.y, ptSelEnd.x, CE_ACTION_DELETE); // [JRT]

	SetCursorPos(ptCursorPos); //PV just moved
}

void CCrystalEditView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	//BEGIN SW
	// check if incremental search is active before call to CCrystalTextView::OnChar()
	BOOL bIncrementalSearch = m_bIncrementalSearchForward || m_bIncrementalSearchBackward;
	//END Sw
	CCrystalTextView::OnChar(nChar, nRepCnt, nFlags);
	//BEGIN SW
	// if we are in incremental search mode ignore the character
	if( m_bIncrementalSearchForward || m_bIncrementalSearchBackward )
		return;

	// if we *were* in incremental search mode and CCrystalTextView::OnChar()
	// exited it the ignore the character (VK_RETURN)
	if( bIncrementalSearch && !m_bIncrementalSearchForward && !m_bIncrementalSearchBackward )
		return;
	//END SW

	if ((::GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0 ||
			(::GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0)
		return;

	BOOL bTranslated = FALSE;
	if (nChar == VK_RETURN)
	{
		if (m_bOvrMode)
		{
			CPoint ptCursorPos = GetCursorPos();
			ASSERT_VALIDTEXTPOS(ptCursorPos);
			if (ptCursorPos.y < GetLineCount() - 1)
			{
				ptCursorPos.x = 0;
				ptCursorPos.y++;

				ASSERT_VALIDTEXTPOS(ptCursorPos);
				SetSelection(ptCursorPos, ptCursorPos);
				SetAnchor(ptCursorPos);
				SetCursorPos(ptCursorPos);
				EnsureVisible(ptCursorPos);
				return;
			}
		}

		m_pTextBuffer->BeginUndoGroup();

		if (QueryEditable() && m_pTextBuffer != NULL)
		{
			DeleteCurrentSelection();

			CPoint ptCursorPos = GetCursorPos();
			ASSERT_VALIDTEXTPOS(ptCursorPos);
			const static TCHAR pszText[3] = _T("\r\n");

			int x, y;
			m_pTextBuffer->InsertText(this, ptCursorPos.y, ptCursorPos.x, pszText, y, x, CE_ACTION_TYPING); //	[JRT]
			ptCursorPos.x = x;
			ptCursorPos.y = y;
			ASSERT_VALIDTEXTPOS(ptCursorPos);
			SetSelection(ptCursorPos, ptCursorPos);
			SetAnchor(ptCursorPos);
			SetCursorPos(ptCursorPos);
			EnsureVisible(ptCursorPos);
		}

		m_pTextBuffer->FlushUndoGroup(this);
		return;
	}

	if (nChar > 31)
	{
		if (QueryEditable() && m_pTextBuffer != NULL)
		{
			m_pTextBuffer->BeginUndoGroup(nChar != _T(' '));

			CPoint ptSelStart, ptSelEnd;
			GetSelection(ptSelStart, ptSelEnd);
			CPoint ptCursorPos;
			if (ptSelStart != ptSelEnd)
			{
				ptCursorPos = ptSelStart;
				DeleteCurrentSelection();
			}
			else
			{
				ptCursorPos = GetCursorPos();
				if (m_bOvrMode && ptCursorPos.x < GetLineLength(ptCursorPos.y))
					m_pTextBuffer->DeleteText(this, ptCursorPos.y, ptCursorPos.x, ptCursorPos.y, ptCursorPos.x + 1, CE_ACTION_TYPING); // [JRT]
			}

			ASSERT_VALIDTEXTPOS(ptCursorPos);

			char pszText[2];
			pszText[0] = (char) nChar;
			pszText[1] = 0;

			int x, y;
			USES_CONVERSION;
			m_pTextBuffer->InsertText(this, ptCursorPos.y, ptCursorPos.x, A2T(pszText), y, x, CE_ACTION_TYPING); // [JRT]
			ptCursorPos.x = x;
			ptCursorPos.y = y;
			ASSERT_VALIDTEXTPOS(ptCursorPos);
			SetSelection(ptCursorPos, ptCursorPos);
			SetAnchor(ptCursorPos);
			SetCursorPos(ptCursorPos);
			EnsureVisible(ptCursorPos);

			m_pTextBuffer->FlushUndoGroup(this);
		}
	}
}


//
//	[JRT]: Added Support For "Disable Backspace At Start Of Line"
//
void CCrystalEditView::OnEditDeleteBack() 
{
	//BEGIN SW
	// if we are in incremental search mode ignore backspace
	CCrystalTextView::OnEditDeleteBack();
	if( m_bIncrementalSearchForward || m_bIncrementalSearchBackward )
		return;
	//END SW

	if (IsSelection())
	{
		OnEditDelete();
		return;
	}

	if (! QueryEditable() || m_pTextBuffer == NULL)
		return;

	CPoint ptCursorPos = GetCursorPos();
	CPoint ptCurrentCursorPos = ptCursorPos;
	bool	bDeleted = false;

	if( !( ptCursorPos.x ) )							// If At Start Of Line
	{
		if( !m_bDisableBSAtSOL )						// If DBSASOL Is Disabled
		{
			if( ptCursorPos.y > 0 )						// If Previous Lines Available
			{
				ptCursorPos.y--;						// Decrement To Previous Line
				ptCursorPos.x = GetLineLength(
						ptCursorPos.y );				// Set Cursor To End Of Previous Line
				bDeleted = true;						// Set Deleted Flag
			}
		}
	}
	else												// If Caret Not At SOL
	{
		ptCursorPos.x--;								// Decrement Position
		bDeleted = true;								// Set Deleted Flag
	}
/*
	if (ptCursorPos.x == 0)
	{
		if (ptCursorPos.y == 0)
			return;
		ptCursorPos.y --;
		ptCursorPos.x = GetLineLength(ptCursorPos.y);
	}
	else
		ptCursorPos.x --;
*/
	ASSERT_VALIDTEXTPOS(ptCursorPos);
	SetAnchor(ptCursorPos);
	SetSelection(ptCursorPos, ptCursorPos);
	EnsureVisible(ptCursorPos);

	if (bDeleted)
	{
		m_pTextBuffer->DeleteText(this, ptCursorPos.y, ptCursorPos.x, ptCurrentCursorPos.y, ptCurrentCursorPos.x, CE_ACTION_BACKSPACE); // [JRT]
	}

	SetCursorPos(ptCursorPos); //PV just moved
	return;
}

void CCrystalEditView::OnEditTab() 
{
	if (! QueryEditable() || m_pTextBuffer == NULL)
		return;

	BOOL bTabify = FALSE;
	CPoint ptSelStart, ptSelEnd;
	if (IsSelection())
	{
		GetSelection(ptSelStart, ptSelEnd);
		bTabify = ptSelStart.y != ptSelEnd.y;
	}

	if (bTabify)
	{
		m_pTextBuffer->BeginUndoGroup();

		int nStartLine = ptSelStart.y;
		int nEndLine = ptSelEnd.y;
		ptSelStart.x = 0;
		if (ptSelEnd.x > 0)
		{
			if (ptSelEnd.y == GetLineCount() - 1)
			{
				ptSelEnd.x = GetLineLength(ptSelEnd.y);
			}
			else
			{
				ptSelEnd.x = 0;
				ptSelEnd.y ++;
			}
		}
		else
			nEndLine --;
		SetSelection(ptSelStart, ptSelEnd);
		SetCursorPos(ptSelEnd);
		EnsureVisible(ptSelEnd);

		//	Shift selection to right
		m_bHorzScrollBarLocked = TRUE;
		static const TCHAR pszText[] = _T("\t");
		for (int L = nStartLine; L <= nEndLine; L ++)
		{
			int x, y;
			m_pTextBuffer->InsertText(this, L, 0, pszText, y, x, CE_ACTION_INDENT); //	[JRT]
		}
		m_bHorzScrollBarLocked = FALSE;
		RecalcHorzScrollBar();

		m_pTextBuffer->FlushUndoGroup(this);
		return;
	}

	if (m_bOvrMode)
	{
		CPoint ptCursorPos = GetCursorPos();
		ASSERT_VALIDTEXTPOS(ptCursorPos);

		int nLineLength = GetLineLength(ptCursorPos.y);
		LPCTSTR pszLineChars = GetLineChars(ptCursorPos.y);
		if (ptCursorPos.x < nLineLength)
		{
			int nTabSize = GetTabSize();
			int nChars = nTabSize - CalculateActualOffset(ptCursorPos.y, ptCursorPos.x) % nTabSize;
			ASSERT(nChars > 0 && nChars <= nTabSize);

			while (nChars > 0)
			{
				if (ptCursorPos.x == nLineLength)
					break;
				if (pszLineChars[ptCursorPos.x] == _T('\t'))
				{
					ptCursorPos.x ++;
					break;
				}
				ptCursorPos.x ++;
				nChars --;
			}
			ASSERT(ptCursorPos.x <= nLineLength);
			ASSERT_VALIDTEXTPOS(ptCursorPos);

			SetSelection(ptCursorPos, ptCursorPos);
			SetAnchor(ptCursorPos);
			SetCursorPos(ptCursorPos);
			EnsureVisible(ptCursorPos);
			return;
		}
	}

	m_pTextBuffer->BeginUndoGroup();

	DeleteCurrentSelection();

	CPoint ptCursorPos = GetCursorPos();
	ASSERT_VALIDTEXTPOS(ptCursorPos);

	static const TCHAR pszText[] = _T("\t");
	int x, y;
	m_pTextBuffer->InsertText(this, ptCursorPos.y, ptCursorPos.x, pszText, y, x, CE_ACTION_TYPING); //	[JRT]
	ptCursorPos.x = x;
	ptCursorPos.y = y;
	ASSERT_VALIDTEXTPOS(ptCursorPos);
	SetSelection(ptCursorPos, ptCursorPos);
	SetAnchor(ptCursorPos);
	SetCursorPos(ptCursorPos);
	EnsureVisible(ptCursorPos);

	m_pTextBuffer->FlushUndoGroup(this);
}

void CCrystalEditView::OnEditUntab() 
{
	if (! QueryEditable() || m_pTextBuffer == NULL)
		return;

	BOOL bTabify = FALSE;
	CPoint ptSelStart, ptSelEnd;
	if (IsSelection())
	{
		GetSelection(ptSelStart, ptSelEnd);
		bTabify = ptSelStart.y != ptSelEnd.y;
	}

	if (bTabify)
	{
		m_pTextBuffer->BeginUndoGroup();

		CPoint ptSelStart, ptSelEnd;
		GetSelection(ptSelStart, ptSelEnd);
		int nStartLine = ptSelStart.y;
		int nEndLine = ptSelEnd.y;
		ptSelStart.x = 0;
		if (ptSelEnd.x > 0)
		{
			if (ptSelEnd.y == GetLineCount() - 1)
			{
				ptSelEnd.x = GetLineLength(ptSelEnd.y);
			}
			else
			{
				ptSelEnd.x = 0;
				ptSelEnd.y ++;
			}
		}
		else
			nEndLine --;
		SetSelection(ptSelStart, ptSelEnd);
		SetCursorPos(ptSelEnd);
		EnsureVisible(ptSelEnd);

		//	Shift selection to left
		m_bHorzScrollBarLocked = TRUE;
		for (int L = nStartLine; L <= nEndLine; L ++)
		{
			int nLength = GetLineLength(L);
			if (nLength > 0)
			{
				LPCTSTR pszChars = GetLineChars(L);
				int nPos = 0, nOffset = 0;
				while (nPos < nLength)
				{
					if (pszChars[nPos] == _T(' '))
					{
						nPos ++;
						if (++ nOffset >= GetTabSize())
							break;
					}
					else
					{
						if (pszChars[nPos] == _T('\t'))
							nPos ++;
						break;
					}
				}

				if (nPos > 0)
					m_pTextBuffer->DeleteText(this, L, 0, L, nPos, CE_ACTION_INDENT); // [JRT]

				SetCursorPos(ptSelEnd);  //PV
			}
		}
		m_bHorzScrollBarLocked = FALSE;
		RecalcHorzScrollBar();

		m_pTextBuffer->FlushUndoGroup(this);
	}
	else
	{
		CPoint ptCursorPos = GetCursorPos();
		ASSERT_VALIDTEXTPOS(ptCursorPos);
		if (ptCursorPos.x > 0)
		{
			int nTabSize = GetTabSize();
			int nOffset = CalculateActualOffset(ptCursorPos.y, ptCursorPos.x);
			int nNewOffset = nOffset / nTabSize * nTabSize;
			if (nOffset == nNewOffset && nNewOffset > 0)
				nNewOffset -= nTabSize;
			ASSERT(nNewOffset >= 0);

			LPCTSTR pszChars = GetLineChars(ptCursorPos.y);
			int nCurrentOffset = 0;
			int I = 0;
			while (nCurrentOffset < nNewOffset)
			{
				if (pszChars[I] == _T('\t'))
					nCurrentOffset = nCurrentOffset / nTabSize * nTabSize + nTabSize;
				else
					nCurrentOffset ++;
				I ++;
			}

			ASSERT(nCurrentOffset == nNewOffset);

			ptCursorPos.x = I;
			ASSERT_VALIDTEXTPOS(ptCursorPos);
			SetSelection(ptCursorPos, ptCursorPos);
			SetAnchor(ptCursorPos);
			SetCursorPos(ptCursorPos);
			EnsureVisible(ptCursorPos);
		}
	}
}

void CCrystalEditView::OnUpdateIndicatorCol(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(FALSE);
}

void CCrystalEditView::OnUpdateIndicatorOvr(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_bOvrMode);
}

void CCrystalEditView::OnUpdateIndicatorRead(CCmdUI* pCmdUI)
{
	if (m_pTextBuffer == NULL)
		pCmdUI->Enable(FALSE);
	else
		pCmdUI->Enable(m_pTextBuffer->GetReadOnly());
}

void CCrystalEditView::OnEditSwitchOvrmode() 
{
	SetOverwriteMode(!m_bOvrMode);
}

void CCrystalEditView::OnUpdateEditSwitchOvrmode(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_bOvrMode ? 1 : 0);
}

DROPEFFECT CEditDropTargetImpl::OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	if (! pDataObject->IsDataAvailable(CF_TEXT))
	{
		m_pOwner->HideDropIndicator();
		return DROPEFFECT_NONE;
	}
	m_pOwner->ShowDropIndicator(point);
	if (dwKeyState & MK_CONTROL)
		return DROPEFFECT_COPY;
	return DROPEFFECT_MOVE;
}

void CEditDropTargetImpl::OnDragLeave(CWnd* pWnd)
{
	m_pOwner->HideDropIndicator();
}

DROPEFFECT CEditDropTargetImpl::OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
/*
	if (! pDataObject->IsDataAvailable(CF_TEXT))
	{
		m_pOwner->HideDropIndicator();
		return DROPEFFECT_NONE;
	}
*/
	//
	// [JRT]
	//
	bool bDataSupported = false;

	if ((!m_pOwner) ||									// If No Owner 
			(!( m_pOwner->QueryEditable())) ||			// Or Not Editable
			(m_pOwner->GetDisableDragAndDrop()))		// Or Drag And Drop Disabled
	{
		m_pOwner -> HideDropIndicator();					// Hide Drop Caret
		return DROPEFFECT_NONE;							    // Return DE_NONE
	}
//	if ((pDataObject->IsDataAvailable( CF_TEXT ) ) ||	    // If Text Available
//			( pDataObject -> IsDataAvailable( xxx ) ) ||	// Or xxx Available
//			( pDataObject -> IsDataAvailable( yyy ) ) )		// Or yyy Available
	if (pDataObject->IsDataAvailable(CF_TEXT))		  	    // If Text Available
	{
		bDataSupported = true;								// Set Flag
	}
	if (!bDataSupported)									// If No Supported Formats Available
	{
		m_pOwner->HideDropIndicator();					    // Hide Drop Caret
		return DROPEFFECT_NONE;						   	    // Return DE_NONE
	}
	m_pOwner->ShowDropIndicator(point);
	if (dwKeyState & MK_CONTROL)
		return DROPEFFECT_COPY;
	return DROPEFFECT_MOVE;
}

BOOL CEditDropTargetImpl::OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point)
{
	//
	// [JRT] 			( m_pOwner -> GetDisableDragAndDrop() ) )		// Or Drag And Drop Disabled
	//
	bool bDataSupported = false;

	m_pOwner->HideDropIndicator();						// Hide Drop Caret
	if ((!m_pOwner) ||									// If No Owner 
			(!( m_pOwner->QueryEditable())) ||			// Or Not Editable
			(m_pOwner->GetDisableDragAndDrop()))		// Or Drag And Drop Disabled
	{
		return DROPEFFECT_NONE;							// Return DE_NONE
	}
//	if( ( pDataObject -> IsDataAvailable( CF_TEXT ) ) ||	// If Text Available
//			( pDataObject -> IsDataAvailable( xxx ) ) ||	// Or xxx Available
//			( pDataObject -> IsDataAvailable( yyy ) ) )		// Or yyy Available
	if (pDataObject->IsDataAvailable(CF_TEXT))			    // If Text Available
	{
		bDataSupported = true;								// Set Flag
	}
	if (!bDataSupported)									// If No Supported Formats Available
	{
		return DROPEFFECT_NONE;							    // Return DE_NONE
	}
	return (m_pOwner->DoDropText(pDataObject, point));	    // Return Result Of Drop
}

DROPEFFECT CEditDropTargetImpl::OnDragScroll(CWnd* pWnd, DWORD dwKeyState, CPoint point)
{
	ASSERT(m_pOwner == pWnd);
	m_pOwner->DoDragScroll(point);

	if (dwKeyState & MK_CONTROL)
		return DROPEFFECT_COPY;
	return DROPEFFECT_MOVE;
}

void CCrystalEditView::DoDragScroll(const CPoint &point)
{
	CRect rcClientRect;
	GetClientRect(rcClientRect);
	if (point.y < rcClientRect.top + DRAG_BORDER_Y)
	{
		HideDropIndicator();
		ScrollUp();
		UpdateWindow();
		ShowDropIndicator(point);
		return;
	}
	if (point.y >= rcClientRect.bottom - DRAG_BORDER_Y)
	{
		HideDropIndicator();
		ScrollDown();
		UpdateWindow();
		ShowDropIndicator(point);
		return;
	}
	if (point.x < rcClientRect.left + GetMarginWidth() + DRAG_BORDER_X)
	{
		HideDropIndicator();
		ScrollLeft();
		UpdateWindow();
		ShowDropIndicator(point);
		return;
	}
	if (point.x >= rcClientRect.right - DRAG_BORDER_X)
	{
		HideDropIndicator();
		ScrollRight();
		UpdateWindow();
		ShowDropIndicator(point);
		return;
	}
}

BOOL CCrystalEditView::DoDropText(COleDataObject *pDataObject, const CPoint &ptClient)
{
	HGLOBAL hData = pDataObject->GetGlobalData(CF_TEXT);
	if (hData == NULL)
		return FALSE;

	CPoint ptDropPos = ClientToText(ptClient);
	if (IsDraggingText() && IsInsideSelection(ptDropPos))
	{
		SetAnchor(ptDropPos);
		SetSelection(ptDropPos, ptDropPos);
		SetCursorPos(ptDropPos);
		EnsureVisible(ptDropPos);
		return FALSE;
	}

	LPSTR pszText = (LPSTR) ::GlobalLock(hData);
	if (pszText == NULL)
		return FALSE;

	int x, y;
	USES_CONVERSION;
	m_pTextBuffer->InsertText(this, ptDropPos.y, ptDropPos.x, A2T(pszText), y, x, CE_ACTION_DRAGDROP); //	[JRT]
	CPoint ptCurPos(x, y);
	ASSERT_VALIDTEXTPOS(ptCurPos);
	SetAnchor(ptDropPos);
	SetSelection(ptDropPos, ptCurPos);
	SetCursorPos(ptCurPos);
	EnsureVisible(ptCurPos);

	::GlobalUnlock(hData);
	return TRUE;
}

int CCrystalEditView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CCrystalTextView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	ASSERT(m_pDropTarget == NULL);
	m_pDropTarget = new CEditDropTargetImpl(this);
	if (! m_pDropTarget->Register(this))
	{
		TRACE0("Warning: Unable to register drop target for CCrystalEditView.\n");
		delete m_pDropTarget;
		m_pDropTarget = NULL;
	}

	return 0;
}

void CCrystalEditView::OnDestroy() 
{
	if (m_pDropTarget != NULL)
	{
		m_pDropTarget->Revoke();
		delete m_pDropTarget;
		m_pDropTarget = NULL;
	}

	CCrystalTextView::OnDestroy();
}

void CCrystalEditView::ShowDropIndicator(const CPoint &point)
{
	if (! m_bDropPosVisible)
	{
		HideCursor();
		m_ptSavedCaretPos = GetCursorPos();
		m_bDropPosVisible = TRUE;
		::CreateCaret(m_hWnd, (HBITMAP) 1, 2, GetLineHeight());
	}
	m_ptDropPos = ClientToText(point);
	if (m_ptDropPos.x >= m_nOffsetChar)
	{
		SetCaretPos(TextToClient(m_ptDropPos));
		ShowCaret();
	}
	else
	{
		HideCaret();
	}
}

void CCrystalEditView::HideDropIndicator()
{
	if (m_bDropPosVisible)
	{
		SetCursorPos(m_ptSavedCaretPos);
		ShowCursor();
		m_bDropPosVisible = FALSE;
	}
}

DROPEFFECT CCrystalEditView::GetDropEffect()
{
	return DROPEFFECT_COPY | DROPEFFECT_MOVE;
}

void CCrystalEditView::OnDropSource(DROPEFFECT de)
{
	if (! IsDraggingText())
		return;

	ASSERT_VALIDTEXTPOS(m_ptDraggedTextBegin);
	ASSERT_VALIDTEXTPOS(m_ptDraggedTextEnd);

	if (de == DROPEFFECT_MOVE)
	{
		m_pTextBuffer->DeleteText(this, m_ptDraggedTextBegin.y, m_ptDraggedTextBegin.x, m_ptDraggedTextEnd.y, 
			m_ptDraggedTextEnd.x, CE_ACTION_DRAGDROP); // [JRT]
	}
}

void CCrystalEditView::UpdateView(CCrystalTextView *pSource, CUpdateContext *pContext, DWORD dwFlags, int nLineIndex /*= -1*/)
{
	CCrystalTextView::UpdateView(pSource, pContext, dwFlags, nLineIndex);

	if (m_bSelectionPushed && pContext != NULL)
	{
		pContext->RecalcPoint(m_ptSavedSelStart);
		pContext->RecalcPoint(m_ptSavedSelEnd);
		ASSERT_VALIDTEXTPOS(m_ptSavedSelStart);
		ASSERT_VALIDTEXTPOS(m_ptSavedSelEnd);
	}
}

void CCrystalEditView::OnEditReplace() 
{
	if (! QueryEditable())
		return;

	CWinApp *pApp = AfxGetApp();
	ASSERT(pApp != NULL);

	CCrystalResources	cr;
	CEditReplaceDlg dlg(this);

	//	Take search parameters from registry
	dlg.m_bMatchCase = pApp->GetProfileInt(REG_REPLACE_SUBKEY, REG_MATCH_CASE, FALSE);
	dlg.m_bWholeWord = pApp->GetProfileInt(REG_REPLACE_SUBKEY, REG_WHOLE_WORD, FALSE);
	dlg.m_sText = pApp->GetProfileString(REG_REPLACE_SUBKEY, REG_FIND_WHAT, _T(""));
	dlg.m_sNewText = pApp->GetProfileString(REG_REPLACE_SUBKEY, REG_REPLACE_WITH, _T(""));

	if (IsSelection())
	{
		GetSelection(m_ptSavedSelStart, m_ptSavedSelEnd);
		m_bSelectionPushed = TRUE;

		dlg.m_nScope = 0;	//	Replace in current selection
		dlg.m_ptCurrentPos = m_ptSavedSelStart;
		dlg.m_bEnableScopeSelection = TRUE;
		dlg.m_ptBlockBegin = m_ptSavedSelStart;
		dlg.m_ptBlockEnd = m_ptSavedSelEnd;
	}
	else
	{
		dlg.m_nScope = 1;	//	Replace in whole text
		dlg.m_ptCurrentPos = GetCursorPos();
		dlg.m_bEnableScopeSelection = FALSE;
	}

	//	Execute Replace dialog
	m_bShowInactiveSelection = TRUE;
	dlg.DoModal();
	m_bShowInactiveSelection = FALSE;

	//	Restore selection
	if (m_bSelectionPushed)
	{
		SetSelection(m_ptSavedSelStart, m_ptSavedSelEnd);
		m_bSelectionPushed = FALSE;
	}

	//	Save search parameters to registry
	pApp->WriteProfileInt(REG_REPLACE_SUBKEY, REG_MATCH_CASE, dlg.m_bMatchCase);
	pApp->WriteProfileInt(REG_REPLACE_SUBKEY, REG_WHOLE_WORD, dlg.m_bWholeWord);
	pApp->WriteProfileString(REG_REPLACE_SUBKEY, REG_FIND_WHAT, dlg.m_sText);
	pApp->WriteProfileString(REG_REPLACE_SUBKEY, REG_REPLACE_WITH, dlg.m_sNewText);
}

BOOL CCrystalEditView::ReplaceSelection(LPCTSTR pszNewText)
{
	ASSERT(pszNewText != NULL);
	/*
	// If there is no selection perform an insert
	if (! IsSelection())
		return FALSE;
	*/

	DeleteCurrentSelection();

	CPoint ptCursorPos = GetCursorPos();
	ASSERT_VALIDTEXTPOS(ptCursorPos);
	int x, y;
	m_pTextBuffer->InsertText(this, ptCursorPos.y, ptCursorPos.x, pszNewText, y, x, CE_ACTION_REPLACE); //	[JRT]
	CPoint ptEndOfBlock = CPoint(x, y);
	ASSERT_VALIDTEXTPOS(ptCursorPos);
	ASSERT_VALIDTEXTPOS(ptEndOfBlock);
	SetAnchor(ptEndOfBlock);
	SetSelection(ptCursorPos, ptEndOfBlock);
	SetCursorPos(ptEndOfBlock);
	EnsureVisible(ptEndOfBlock);
	return TRUE;
}

void CCrystalEditView::OnUpdateEditUndo(CCmdUI* pCmdUI) 
{
	BOOL bCanUndo = m_pTextBuffer != NULL && m_pTextBuffer->CanUndo();
	pCmdUI->Enable(bCanUndo);

	//	Since we need text only for menus...
	if (pCmdUI->m_pMenu != NULL)
	{
		//	Tune up 'resource handle'
		CCrystalResources	cr;

		CString menu;
		if (bCanUndo)
		{
			//	Format menu item text using the provided item description
			CString desc;
			m_pTextBuffer->GetUndoDescription(desc);
			menu.Format(IDS_MENU_UNDO_FORMAT, desc);
		}
		else
		{
			//	Just load default menu item text
			menu.LoadString(IDS_MENU_UNDO_DEFAULT);
		}

		//	Set menu item text
		pCmdUI->SetText(menu);
	}
}

void CCrystalEditView::OnEditUndo() 
{
	if (m_pTextBuffer != NULL && m_pTextBuffer->CanUndo())
	{
		CPoint ptCursorPos;
		if (m_pTextBuffer->Undo(ptCursorPos))
		{
			ASSERT_VALIDTEXTPOS(ptCursorPos);
			SetAnchor(ptCursorPos);
			SetSelection(ptCursorPos, ptCursorPos);
			SetCursorPos(ptCursorPos);
			EnsureVisible(ptCursorPos);
		}
	}
}

//	[JRT]
void CCrystalEditView::SetDisableBSAtSOL(BOOL bDisableBSAtSOL)
{
	m_bDisableBSAtSOL = bDisableBSAtSOL;
}

void CCrystalEditView::OnEditRedo() 
{
	if (m_pTextBuffer != NULL && m_pTextBuffer->CanRedo())
	{
		CPoint ptCursorPos;
		if (m_pTextBuffer->Redo(ptCursorPos))
		{
			ASSERT_VALIDTEXTPOS(ptCursorPos);
			SetAnchor(ptCursorPos);
			SetSelection(ptCursorPos, ptCursorPos);
			SetCursorPos(ptCursorPos);
			EnsureVisible(ptCursorPos);
		}
	}
}

void CCrystalEditView::OnUpdateEditRedo(CCmdUI* pCmdUI) 
{
	BOOL bCanRedo = m_pTextBuffer != NULL && m_pTextBuffer->CanRedo();
	pCmdUI->Enable(bCanRedo);

	//	Since we need text only for menus...
	if (pCmdUI->m_pMenu != NULL)
	{
		CCrystalResources	cr;

		CString menu;
		if (bCanRedo)
		{
			//	Format menu item text using the provided item description
			CString desc;
			m_pTextBuffer->GetRedoDescription(desc);
			menu.Format(IDS_MENU_REDO_FORMAT, desc);
		}
		else
		{
			//	Just load default menu item text
			menu.LoadString(IDS_MENU_REDO_DEFAULT);
		}

		//	Set menu item text
		pCmdUI->SetText(menu);
	}
}

void CCrystalEditView::OnEditOperation(int nAction, LPCTSTR pszText)
{
	if ( m_pBackgroundThread )
		m_pBackgroundThread->PostThreadMessage(ID_BG_UPDATE_LINE, (long)(GetCursorPos().y), (long)this);

	if (m_bAutoIndent)
	{
		//	Analyse last action...
		if (nAction == CE_ACTION_TYPING && _tcscmp(pszText, _T("\r\n")) == 0 && ! m_bOvrMode)
		{
			//	Enter stroke!
			CPoint ptCursorPos = GetCursorPos();
			ASSERT(ptCursorPos.y > 0);

			//	Take indentation from the previos line
			int nLength = m_pTextBuffer->GetLineLength(ptCursorPos.y - 1);
			LPCTSTR pszLineChars = m_pTextBuffer->GetLineChars(ptCursorPos.y - 1);
			int nPos = 0;
			while (nPos < nLength && IsSpace(pszLineChars[nPos]))
				nPos ++;

			if (nPos > 0)
			{
				//	Insert part of the previos line
				TCHAR *pszInsertStr = (TCHAR *) _alloca(sizeof(TCHAR) * (nLength + 1));
				_tcsncpy(pszInsertStr, pszLineChars, nPos);
				pszInsertStr[nPos] = 0;

				int x, y;
				m_pTextBuffer->InsertText(NULL, ptCursorPos.y, ptCursorPos.x,
															pszInsertStr, y, x, CE_ACTION_AUTOINDENT);
				CPoint pt(x, y);
				SetCursorPos(pt);
				SetSelection(pt, pt);
				SetAnchor(pt);
				EnsureVisible(pt);
			}
		}
	}
}


//BEGIN SW
void CCrystalEditView::OnUpdateEditGotoLastChange( CCmdUI *pCmdUI )
{
	CPoint	ptLastChange = m_pTextBuffer->GetLastChangePos();
	pCmdUI->Enable( ptLastChange.x > -1 && ptLastChange.y > -1 );
}

void CCrystalEditView::OnEditGotoLastChange()
{
	CPoint	ptLastChange = m_pTextBuffer->GetLastChangePos();
	if( ptLastChange.x < 0 || ptLastChange.y < 0 )
		return;

	// goto last change
	SetCursorPos( ptLastChange );
	SetSelection( ptLastChange, ptLastChange );
	EnsureVisible( ptLastChange );
}
//END SW

//BEGIN SW(word deleting)
void CCrystalEditView::OnEditDeleteWord()
{
	// if something is selected, then only delete selection...
	CPoint	ptSelStart, ptSelEnd;
	GetSelection( ptSelStart, ptSelEnd );

	if( ptSelStart != ptSelEnd )
	{
		OnEditDelete();
		return;
	}

	// ... else select word right from the cursor an delete it
	MoveWordRight( TRUE );
	OnEditDelete();
}


void CCrystalEditView::OnEditDeleteWordBack()
{
	// if something is selected, then only delete selection...
	CPoint	ptSelStart, ptSelEnd;
	GetSelection( ptSelStart, ptSelEnd );

	if( ptSelStart != ptSelEnd )
	{
		OnEditDeleteBack();
		return;
	}

	// ... else select word left from the cursor an delete it
	MoveWordLeft( TRUE );
	OnEditDeleteBack();
}
//END SW