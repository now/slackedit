//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This source code is a part of BCGControlBar library.
// You may use, compile or redistribute it as part of your application 
// for free. You cannot redistribute it as a part of a software development 
// library without the agreement of the author. If the sources are 
// distributed along with the application, you should leave the original 
// copyright notes in the source code without any changes.
// This code can be used WITHOUT ANY WARRANTIES on your own risk.
// 
// For the latest updates to this library, check my site:
// http://welcome.to/bcgsoft
// 
// Stas Levin <bcgsoft@yahoo.com>
//*******************************************************************************

// BCGEditListBox.cpp : implementation file
//

#include "stdafx.h"
#include "bcgcontrolbar.h"
#include "BCGEditListBox.h"
#include "KeyHelper.h"
#include "bcglocalres.h"
#include "bcgbarres.h"
#include "globals.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const int iListId = 1;
static const int nTextMargin = 5;

/////////////////////////////////////////////////////////////////////////////
// CBCGEditListBox

CBCGEditListBox::CBCGEditListBox()
{
	m_sizeButton = CSize (0, 0);
	m_uiStandardBtns = 0;
	m_bNewItem = FALSE;
}
//**************************************************************************
CBCGEditListBox::~CBCGEditListBox()
{
	while (!m_lstButtons.IsEmpty ())
	{
		delete m_lstButtons.RemoveHead ();
	}
}

BEGIN_MESSAGE_MAP(CBCGEditListBox, CStatic)
	//{{AFX_MSG_MAP(CBCGEditListBox)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_SETFOCUS()
	//}}AFX_MSG_MAP
	ON_NOTIFY(LVN_KEYDOWN, iListId, OnKeyDown)
	ON_NOTIFY(NM_DBLCLK, iListId, OnDblclkList)
	ON_NOTIFY(LVN_GETDISPINFO, iListId, OnGetdispinfo)
	ON_NOTIFY(LVN_ENDLABELEDIT, iListId, OnEndLabelEdit)
	ON_NOTIFY(LVN_ITEMCHANGED, iListId, OnItemChanged)
    ON_MESSAGE(WM_SETFONT, OnSetFont)
    ON_MESSAGE(WM_GETFONT, OnGetFont)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGEditListBox message handlers
void CBCGEditListBox::PreSubclassWindow() 
{
	CStatic::PreSubclassWindow();

	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState ();
	if (pThreadState->m_pWndInit == NULL)
	{
		Init ();
	}
}
//**********************************************************************************
int CBCGEditListBox::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CStatic::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	Init ();
	return 0;
}
//**********************************************************************************
void CBCGEditListBox::Init ()
{
	if (GetSafeHwnd () == NULL ||
		m_wndList.GetSafeHwnd () != NULL)
	{
		return;
	}

	ASSERT (GetStyle () & WS_CHILD);

	CRect rectEmpty;
	rectEmpty.SetRectEmpty ();

	m_wndList.CreateEx (WS_EX_CLIENTEDGE, _T("SysListView32"), _T(""),
						WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_SINGLESEL | 
						LVS_NOCOLUMNHEADER | LVS_EDITLABELS,
						rectEmpty, this, iListId);

	m_wndList.SendMessage (LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_wndList.InsertColumn (0, _T(""));

	AdjustLayout ();
}
//**********************************************************************************
BOOL CBCGEditListBox::SetStandardButtons (UINT uiBtns)
{
	if (GetSafeHwnd () == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	if (!m_lstButtons.IsEmpty ())
	{
		//----------------------
		// Already have buttons
		//----------------------
		ASSERT (FALSE);
		return FALSE;
	}

	CBCGLocalResource locaRes;

	CString strButton;
	if (uiBtns & BGCEDITLISTBOX_BTN_NEW)
	{
		strButton.LoadString (IDS_BCGBARRES_NEW);
		VERIFY (AddButton (IDB_BCGBARRES_NEW, strButton, VK_INSERT));
	}

	if (uiBtns & BGCEDITLISTBOX_BTN_DELETE)
	{
		strButton.LoadString (IDS_BCGBARRES_DELETE);
		VERIFY (AddButton (IDB_BCGBARRES_DELETE, strButton, VK_DELETE));
	}

	if (uiBtns & BGCEDITLISTBOX_BTN_UP)
	{
		strButton.LoadString (IDS_BCGBARRES_MOVEUP);
		VERIFY (AddButton (IDB_BCGBARRES_UP, strButton, VK_UP, FALT));
	}

	if (uiBtns & BGCEDITLISTBOX_BTN_DOWN)
	{
		strButton.LoadString (IDS_BCGBARRES_MOVEDN);
		VERIFY (AddButton (IDB_BCGBARRES_DOWN, strButton, VK_DOWN, FALT));
	}

	m_uiStandardBtns = uiBtns;
	return TRUE;
}
//**********************************************************************************
BOOL CBCGEditListBox::AddButton (UINT uiImageResId, 
								 LPCTSTR lpszTooltip/* = NULL*/,
								 WORD wKeyAccelerator/* = 0*/,
								 BYTE fVirt/* = 0*/)
{
	if (GetSafeHwnd () == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	if (m_uiStandardBtns != 0)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CRect rectEmpty;
	rectEmpty.SetRectEmpty ();

	CBCGButton* pButton = new CBCGButton ();
	if (!pButton->Create (_T(""), WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_OWNERDRAW, 
		rectEmpty, this, m_lstButtons.GetCount () + 2))
	{
		return FALSE;
	}

	pButton->m_nFlatStyle = CBCGButton::BUTTONSTYLE_FLAT;

	pButton->SetImage (uiImageResId);
	if (lpszTooltip != NULL)
	{
		CString strTooltip = lpszTooltip;
		if (wKeyAccelerator != 0)
		{
			ACCEL acccel;
			acccel.cmd = 0;
			acccel.fVirt = (BYTE) (fVirt | FVIRTKEY);
			acccel.key = wKeyAccelerator;

			CKeyHelper helper (&acccel);
			CString strAccellKey;
			helper.Format (strAccellKey);

			strTooltip += _T(" (");
			strTooltip += strAccellKey;
			strTooltip += _T(")");
		}

		pButton->SetTooltip (strTooltip);
	}

	pButton->SizeToContent ();
	CRect rectBtn;
	pButton->GetWindowRect (rectBtn);
	CSize sizeButton = rectBtn.Size ();

	if (m_lstButtons.IsEmpty ())
	{
		m_sizeButton = sizeButton;
	}
	else
	{
		ASSERT (m_sizeButton == sizeButton);
	}

	m_lstButtons.AddTail (pButton);

	if (wKeyAccelerator == 0)
	{
		fVirt = 0;
	}

	DWORD dwKey = (fVirt << 16) | wKeyAccelerator;
	m_lstKeyAccell.AddTail (dwKey);

	AdjustLayout ();
	return TRUE;
}
//**********************************************************************************
void CBCGEditListBox::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	dc.FillRect (m_rectCaption, &globalData.brBtnFace);
	dc.Draw3dRect (m_rectCaption, globalData.clrBtnShadow, globalData.clrBtnHilite);

	CRect rectText = m_rectCaption;
	rectText.DeflateRect (nTextMargin, 0);

	dc.SetBkMode (TRANSPARENT);
	dc.SetTextColor (globalData.clrBtnText);

	CFont* pOldFont = NULL;
	
	if (m_font.GetSafeHandle () != NULL)
	{
		pOldFont = dc.SelectObject (&m_font);
	}
	else
	{
		CFont* pParentFont = GetParent ()->GetFont ();

		if (pParentFont != NULL)
		{
			pOldFont = dc.SelectObject (pParentFont);
			ASSERT (pOldFont != NULL);
		}
	}

	CString strCaption;
	GetWindowText (strCaption);
	dc.DrawText (strCaption, rectText, DT_LEFT | DT_SINGLELINE | DT_VCENTER);

	if (pOldFont != NULL)
	{
		dc.SelectObject (pOldFont);
	}
}
//**********************************************************************************
void CBCGEditListBox::OnSize(UINT nType, int cx, int cy) 
{
	CStatic::OnSize(nType, cx, cy);
	AdjustLayout ();
}
//***********************************************************************************
void CBCGEditListBox::AdjustLayout ()
{
	if (GetSafeHwnd () == NULL)
	{
		return;
	}

	CRect rectClient;
	GetClientRect (rectClient);

	m_rectCaption = rectClient;

	CClientDC dc (this);

	CFont* pOldFont = NULL;
	if (m_font.GetSafeHandle () != NULL)
	{
		pOldFont = dc.SelectObject (&m_font);
	}
	else
	{
		CFont* pParentFont = GetParent ()->GetFont ();

		if (pParentFont != NULL)
		{
			pOldFont = dc.SelectObject (pParentFont);
			ASSERT (pOldFont != NULL);
		}
	}

	TEXTMETRIC tm;
	dc.GetTextMetrics (&tm);

	if (pOldFont != NULL)
	{
		dc.SelectObject (pOldFont);
	}

	m_rectCaption.bottom = m_rectCaption.top +
		max (tm.tmHeight * 4 / 3, m_sizeButton.cy);

	int x = rectClient.right - 1 - m_sizeButton.cx;
	for (POSITION pos = m_lstButtons.GetTailPosition (); pos != NULL;)
	{
		CBCGButton* pButton = m_lstButtons.GetPrev (pos);
		ASSERT (pButton != NULL);

		pButton->MoveWindow (x, rectClient.top + 1, m_sizeButton.cx, 
			m_rectCaption.Height () - 2);
		x -= m_sizeButton.cx;
	}

	m_wndList.MoveWindow (	rectClient.left,
							rectClient.top + m_rectCaption.Height (),
							rectClient.Width (),
							rectClient.Height () - m_rectCaption.Height ());

	m_wndList.SetColumnWidth (0, rectClient.Width () -
		2 * ::GetSystemMetrics (SM_CXEDGE) -
		::GetSystemMetrics (SM_CXVSCROLL));
}
//************************************************************************************
BOOL CBCGEditListBox::OnEraseBkgnd(CDC* /*pDC*/) 
{
	return TRUE;
}
//************************************************************************************
BOOL CBCGEditListBox::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	HWND hwnd = (HWND) lParam;

	int iButton = 0;
	for (POSITION pos = m_lstButtons.GetHeadPosition (); pos != NULL; iButton ++)
	{
		CBCGButton* pButton = m_lstButtons.GetNext (pos);
		ASSERT (pButton);
	
		if (pButton->GetSafeHwnd () == hwnd)
		{
			m_wndList.SetFocus ();
			OnClickButton (iButton);
			return TRUE;
		}
	}
	
	return CStatic::OnCommand(wParam, lParam);
}
//*************************************************************************************
int CBCGEditListBox::AddItem (const CString& strText, DWORD dwData, int iIndex)
{
	if (GetSafeHwnd () == NULL)
	{
		return -1;
	}

	if (iIndex < 0)
	{
		iIndex = m_wndList.GetItemCount ();
	}

	int iItem = m_wndList.InsertItem (iIndex, strText, I_IMAGECALLBACK);
	m_wndList.SetItemData (iItem, dwData);

	if (iItem == 0)
	{
		SelectItem (0);
	}

	return iItem;
}
//*************************************************************************************
int CBCGEditListBox::GetCount () const
{
	if (GetSafeHwnd () == NULL)
	{
		return -1;
	}

	return m_wndList.GetItemCount ();
}
//*************************************************************************************
CString CBCGEditListBox::GetItemText (int iIndex) const
{
	if (GetSafeHwnd () == NULL)
	{
		return _T("");
	}

	return m_wndList.GetItemText (iIndex, 0);
}
//*************************************************************************************
DWORD CBCGEditListBox::GetItemData (int iIndex) const
{
	if (GetSafeHwnd () == NULL)
	{
		return 0;
	}

	return m_wndList.GetItemData (iIndex);
}
//*************************************************************************************
void CBCGEditListBox::SetItemData (int iIndex, DWORD dwData)
{
	if (GetSafeHwnd () == NULL)
	{
		return;
	}

	m_wndList.SetItemData (iIndex, dwData);
}
//*************************************************************************************
void CBCGEditListBox::OnKeyDown (LPNMLVKEYDOWN pKeyDown, LRESULT* pResult)
{
	*pResult = 0;

	if (pKeyDown != NULL)
	{
		BYTE fCurrVirt = 0;

		if (::GetAsyncKeyState (VK_CONTROL) & 0x8000)
		{
			fCurrVirt |= FCONTROL;
		}

		if (::GetAsyncKeyState (VK_MENU) & 0x8000)
		{
			fCurrVirt |= FALT;
		}

		if (::GetAsyncKeyState (VK_SHIFT) & 0x8000)
		{
			fCurrVirt |= FSHIFT;
		}

		int iButton = 0;
		for (POSITION pos = m_lstKeyAccell.GetHeadPosition (); pos != NULL; iButton ++)
		{
			DWORD dwKey = m_lstKeyAccell.GetNext (pos);
		
			if (dwKey != 0 && pKeyDown->wVKey == (dwKey & 0xFFFF))
			{
				//-------------------
				// Check state flags:
				//-------------------
				BYTE fVirt = (BYTE) (dwKey >> 16);
				if (fCurrVirt == fVirt)
				{
					OnClickButton (iButton);
					return;
				}
			}
		}

		OnKey (pKeyDown->wVKey, fCurrVirt);
	}
}
//*************************************************************************************
int CBCGEditListBox::GetSelItem () const
{
	if (GetSafeHwnd () == NULL)
	{
		return -1;
	}

	return m_wndList.GetNextItem (-1, LVNI_SELECTED);
}
//*************************************************************************************
BOOL CBCGEditListBox::SelectItem (int iItem)
{
	if (GetSafeHwnd () == NULL)
	{
		return FALSE;
	}

	if (!m_wndList.SetItemState (iItem,	LVIS_SELECTED | LVIS_FOCUSED, 
								LVIS_SELECTED | LVIS_FOCUSED))
	{
		return FALSE;
	}

	return m_wndList.EnsureVisible (iItem, FALSE);
}
//*************************************************************************************
void CBCGEditListBox::OnDblclkList(NMHDR* /*pNMHDR*/, LRESULT* pResult) 
{
	*pResult = 0;

	int iSelItem = GetSelItem ();

	if ((m_uiStandardBtns & BGCEDITLISTBOX_BTN_NEW) && iSelItem == -1)
	{
		CreateNewItem ();
		return;
	}

	if (iSelItem >= 0)
	{
		EditItem (iSelItem);
	}
}
//*************************************************************************************
BOOL CBCGEditListBox::RemoveItem (int iIndex)
{
	if (GetSafeHwnd () == NULL)
	{
		return FALSE;
	}

	BOOL bIsSelected = (GetSelItem () == iIndex);

	if (!m_wndList.DeleteItem (iIndex))
	{
		return FALSE;
	}

	if (!bIsSelected || GetCount () == 0)
	{
		return FALSE;
	}

	//-------------------
	// Restore selection:
	//-------------------
	if (iIndex >= GetCount ())
	{
		iIndex --;
	}

	SelectItem (iIndex);
	return TRUE;
}
//*************************************************************************************
void CBCGEditListBox::OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult) 
{
	ASSERT (pNMHDR != NULL);

	LV_ITEM* pItem = &((LV_DISPINFO*)pNMHDR)->item;
	ASSERT (pItem != NULL);

	if (pItem->mask & LVIF_IMAGE)
	{
		pItem->iImage = OnGetImage (pItem);
	}
	
	*pResult = 0;
}
//**************************************************************************
void CBCGEditListBox::OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	ASSERT (pNMHDR != NULL);

	LV_ITEM* pItem = &((LV_DISPINFO*)pNMHDR)->item;
	ASSERT (pItem != NULL);

	OnEndEditLabel (pItem->pszText);

	*pResult = 0;
}
//**************************************************************************
void CBCGEditListBox::OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	ASSERT (pNMHDR != NULL);

	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	ASSERT (pNMListView != NULL);

	if (pNMListView->uChanged == LVIF_STATE &&
		(pNMListView->uOldState & LVIS_SELECTED) != (pNMListView->uNewState & LVIS_SELECTED))
	{
		OnSelectionChanged ();
	}

	*pResult = 0;
}
//**************************************************************************
BOOL CBCGEditListBox::EditItem (int iIndex)
{
	if (GetSafeHwnd () == NULL)
	{
		return FALSE;
	}

	m_wndList.SetFocus ();
	CEdit* pEdit = m_wndList.EditLabel (iIndex);
	if (pEdit == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID (pEdit);

	CRect rectClient;
	GetClientRect (rectClient);

	CRect rectEdit;
	pEdit->GetClientRect (rectEdit);

	pEdit->SetWindowPos (NULL, -1, -1,
		rectClient.Width () - 10, rectEdit.Height (),
		SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

	return TRUE;
}
//***************************************************************************
void CBCGEditListBox::OnSetFocus(CWnd* /*pOldWnd*/)
{
	m_wndList.SetFocus ();
}
//***************************************************************************
void CBCGEditListBox::OnClickButton (int iButton)
{
	if (m_uiStandardBtns == 0)
	{
		return;
	}

	int iSelItem = GetSelItem ();

	if (iButton == GetStdButtonNum (BGCEDITLISTBOX_BTN_NEW))
	{
		CreateNewItem ();
	}
	else if (iButton == GetStdButtonNum (BGCEDITLISTBOX_BTN_DELETE))
	{
		if (iSelItem >= 0)
		{
			if (OnBeforeRemoveItem (iSelItem))
			{
				RemoveItem (iSelItem);
			}
		}
	}
	else if (iButton == GetStdButtonNum (BGCEDITLISTBOX_BTN_UP) ||
			iButton == GetStdButtonNum (BGCEDITLISTBOX_BTN_DOWN))
	{
		if (iSelItem >= 0)
		{
			BOOL bIsUp = (iButton == GetStdButtonNum (BGCEDITLISTBOX_BTN_UP));

			if (bIsUp)
			{
				if (iSelItem == 0)
				{
					return;
				}
			}
			else
			{
				if (iSelItem == GetCount () - 1)
				{
					return;
				}
			}

			// Adjust list control:
			SetRedraw (FALSE);

			CString strLabel = GetItemText (iSelItem);
			DWORD dwData = GetItemData (iSelItem);

			RemoveItem (iSelItem);
			
			if (bIsUp)
			{
				iSelItem --;
			}
			else
			{
				iSelItem ++;
			}

			AddItem (strLabel, dwData, iSelItem);
			SelectItem (iSelItem);

			SetRedraw ();
			m_wndList.Invalidate ();

			if (bIsUp)
			{
				OnAfterMoveItemUp (iSelItem);
			}
			else
			{
				OnAfterMoveItemDown (iSelItem);
			}
		}
	}
}
//****************************************************************************
int CBCGEditListBox::GetStdButtonNum (UINT uiStdBtn) const
{
	if ((m_uiStandardBtns & uiStdBtn) == 0)
	{
		return -1;
	}

	int iBtn = 0;

	switch (uiStdBtn)
	{
	case BGCEDITLISTBOX_BTN_NEW:
		break;

	case BGCEDITLISTBOX_BTN_DELETE:
		if (m_uiStandardBtns & BGCEDITLISTBOX_BTN_NEW)
		{
			iBtn ++;
		}
		break;

	case BGCEDITLISTBOX_BTN_UP:
		if (m_uiStandardBtns & BGCEDITLISTBOX_BTN_NEW)
		{
			iBtn ++;
		}
		
		if (m_uiStandardBtns & BGCEDITLISTBOX_BTN_DELETE)
		{
			iBtn ++;
		}
		break;

	case BGCEDITLISTBOX_BTN_DOWN:
		if (m_uiStandardBtns & BGCEDITLISTBOX_BTN_NEW)
		{
			iBtn ++;
		}
		
		if (m_uiStandardBtns & BGCEDITLISTBOX_BTN_DELETE)
		{
			iBtn ++;
		}

		if (m_uiStandardBtns & BGCEDITLISTBOX_BTN_UP)
		{
			iBtn ++;
		}
		break;

	default:
		ASSERT (FALSE);
		iBtn = -1;
	}

	return iBtn;
}
//***********************************************************************************
void CBCGEditListBox::CreateNewItem ()
{
	int iLastItem = AddItem (_T(""));
	ASSERT (iLastItem >= 0);

	m_bNewItem = TRUE;
	EditItem (iLastItem);
}
//**************************************************************************
void CBCGEditListBox::OnEndEditLabel (LPCTSTR lpszLabel)
{
	int iSelItem = GetSelItem ();
	if (iSelItem < 0)
	{
		ASSERT (FALSE);
		return;
	}

	CString strLabel = (lpszLabel != NULL) ? lpszLabel : _T("");

	if (!strLabel.IsEmpty ())
	{
		m_wndList.SetItemText (iSelItem, 0, strLabel);

		if (m_bNewItem)
		{
			OnAfterAddItem (iSelItem);
		}
		else
		{
			OnAfterRenameItem (iSelItem);
		}
	}
	else
	{
		if (m_bNewItem)
		{
			RemoveItem (iSelItem);
		}
	}

	m_bNewItem = FALSE;
}
//**************************************************************************
void CBCGEditListBox::OnKey (WORD wKey, BYTE fFlags)
{
	int iSelItem = GetSelItem ();
	TCHAR cKey = (TCHAR) LOWORD (::MapVirtualKey (wKey, 2));

	if (fFlags == 0 &&	// No Ctrl, Shift or Alt
		iSelItem >= 0 &&
		(cKey == _T(' ') || wKey == VK_F2))
	{
		int iSelItem = GetSelItem ();

		if (iSelItem >= 0)
		{
			EditItem (iSelItem);
		}
	}
}
//**************************************************************************
LRESULT CBCGEditListBox::OnGetFont(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    return (LRESULT) m_font.GetSafeHandle ();
}
//**************************************************************************
LRESULT CBCGEditListBox::OnSetFont(WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult = Default();

	CFont* pFont = CFont::FromHandle ((HFONT) wParam);
	if (pFont != NULL)
	{
		LOGFONT lf;
		pFont->GetLogFont (&lf);

		m_font.DeleteObject ();
		m_font.CreateFontIndirect (&lf);
	}

	if (::IsWindow(GetSafeHwnd ()) != NULL)
	{
		AdjustLayout ();

		if (lParam != 0)
		{
			Invalidate ();
			UpdateWindow ();
		}
	}

	return lResult;
}

