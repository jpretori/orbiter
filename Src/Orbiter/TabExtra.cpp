// Copyright (c) Martin Schweiger
// Licensed under the MIT License

//=============================================================================
// ExtraTab class
//=============================================================================

#define OAPI_IMPLEMENTATION

#include <windows.h>
#include <commctrl.h>
#include <winuser.h>
#include "Launchpad.h"
#include "TabExtra.h"
#include "ExtraRender.h"
#include "Orbiter.h"
#include "Rigidbody.h"
#include "Log.h"
#include "Help.h"
#include "resource.h"
#include "resource2.h"

extern Orbiter *g_pOrbiter;

//-----------------------------------------------------------------------------
// ExtraTab class

ExtraTab::ExtraTab (const MainDialog *lp): LaunchpadTab (lp)
{
	nExtPrm = 0;
}

//-----------------------------------------------------------------------------

ExtraTab::~ExtraTab ()
{
	if (nExtPrm) {
		for (DWORD i = 0; i < nExtPrm; i++) delete ExtPrm[i];
		delete []ExtPrm;
	}
}

//-----------------------------------------------------------------------------

void ExtraTab::Create ()
{
	hTab = CreateTab (IDD_PAGE_EXT);

	r_lst0 = GetClientPos (hTab, GetDlgItem (hTab, IDC_EXT_LIST));  // REMOVE!
	r_dsc0 = GetClientPos (hTab, GetDlgItem (hTab, IDC_EXT_TEXT));  // REMOVE!
	r_pane  = GetClientPos (hTab, GetDlgItem (hTab, IDC_EXT_SPLIT1));
	r_edit0 = GetClientPos (hTab, GetDlgItem (hTab, IDC_EXT_OPEN));
	splitListDesc.SetHwnd (GetDlgItem (hTab, IDC_EXT_SPLIT1), GetDlgItem (hTab, IDC_EXT_LIST), GetDlgItem (hTab, IDC_EXT_TEXT));
}

//-----------------------------------------------------------------------------

void ExtraTab::GetConfig (const Config *cfg)
{
	HTREEITEM ht;
	ht = RegisterExtraParam (new ExtraPropagation (this), NULL); TRACENEW
	RegisterExtraParam (new ExtraDynamics (this), ht); TRACENEW
	//RegisterExtraParam (new ExtraAngDynamics (this), ht); TRACENEW
	RegisterExtraParam (new ExtraStabilisation (this), ht); TRACENEW
	ht = RegisterExtraParam (new ExtraInstruments (this), NULL); TRACENEW
	RegisterExtraParam (new ExtraMfdConfig (this), ht); TRACENEW
	RegisterExtraParam (new ExtraVesselConfig (this), NULL); TRACENEW
	RegisterExtraParam (new ExtraPlanetConfig (this), NULL); TRACENEW
	ht = RegisterExtraParam (new ExtraDebug (this), NULL); TRACENEW
	RegisterExtraParam (new ExtraShutdown (this), ht); TRACENEW
	RegisterExtraParam (new ExtraFixedStep (this), ht); TRACENEW
	RegisterExtraParam (new ExtraRenderingOptions (this), ht); TRACENEW
	RegisterExtraParam (new ExtraTimerSettings (this), ht); TRACENEW
	RegisterExtraParam (new ExtraLaunchpadOptions (this), ht); TRACENEW
	RegisterExtraParam (new ExtraLogfileOptions (this), ht); TRACENEW
	RegisterExtraParam (new ExtraPerformanceSettings (this), ht); TRACENEW
	SetWindowText (GetDlgItem (hTab, IDC_EXT_TEXT), "Advanced and addon-specific configuration parameters.\r\n\r\nClick on an item to get a description.\r\n\r\nDouble-click to open or expand.");
	int listw = cfg->CfgWindowPos.LaunchpadExtListWidth;
	if (!listw) {
		RECT r;
		GetClientRect (GetDlgItem (hTab, IDC_EXT_LIST), &r);
		listw = r.right;
	}
	splitListDesc.SetStaticPane (SplitterCtrl::PANE1, listw);
}

//-----------------------------------------------------------------------------

void ExtraTab::SetConfig (Config *cfg)
{
	cfg->CfgWindowPos.LaunchpadExtListWidth = splitListDesc.GetPaneWidth (SplitterCtrl::PANE1);
}

//-----------------------------------------------------------------------------

bool ExtraTab::OpenHelp ()
{
	OpenDefaultHelp (pLp->GetWindow(), pLp->GetInstance(), "tab_extra");
	return true;
}

//-----------------------------------------------------------------------------

BOOL ExtraTab::Size (int w, int h)
{
	int dw = w - (int)(pos0.right-pos0.left);
	int dh = h - (int)(pos0.bottom-pos0.top);
	int w0 = r_pane.right - r_pane.left; // initial splitter pane width
	int h0 = r_pane.bottom - r_pane.top; // initial splitter pane height

	// the elements below may need updating
	int lstw0 = r_lst0.right-r_lst0.left;
	int lsth0 = r_lst0.bottom-r_lst0.top;
	int dscw0 = r_dsc0.right-r_dsc0.left;
	int wg  = r_dsc0.right - r_lst0.left - lstw0 - dscw0;  // gap width
	int wl  = lstw0 + (dw*lstw0)/(lstw0+dscw0);
	wl = max (wl, lstw0/2);
	int xr = r_lst0.left+wl+wg;
	int wr = max(10,lstw0+dscw0+dw-wl);

	///SetWindowPos (GetDlgItem (hTab, IDC_EXT_LIST), NULL,
	//	0, 0, wl, lsth0+dh,
	//	SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOOWNERZORDER|SWP_NOZORDER);
	//SetWindowPos (GetDlgItem (hTab, IDC_EXT_TEXT), NULL,
	//	xr, r_dsc0.top, wr, lsth0+dh,
	//	SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOZORDER);
	SetWindowPos (GetDlgItem (hTab, IDC_EXT_SPLIT1), NULL,
		0, 0, w0+dw, h0+dh,
		SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOOWNERZORDER|SWP_NOZORDER);
	SetWindowPos (GetDlgItem (hTab, IDC_EXT_OPEN), NULL,
		r_edit0.left, r_edit0.top+dh, 0, 0,
		SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOOWNERZORDER|SWP_NOZORDER);

	return NULL;
}

//-----------------------------------------------------------------------------

HTREEITEM ExtraTab::RegisterExtraParam (LaunchpadItem *item, HTREEITEM parent)
{
	// first check that the item doesn't already exist
	HTREEITEM hti = FindExtraParam (item->Name(), parent);
	if (hti) return hti;

	// add extra parameter instance to list
	LaunchpadItem **tmp = new LaunchpadItem*[nExtPrm+1]; TRACENEW
	if (nExtPrm) {
		memcpy (tmp, ExtPrm, nExtPrm*sizeof(LaunchpadItem*));
		delete []ExtPrm;
	}
	ExtPrm = tmp;
	ExtPrm[nExtPrm++] = item;

	// if a name is provided, add item to tree list
	char *name = item->Name();
	if (name) {
		TV_INSERTSTRUCT tvis;
		tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
		tvis.item.pszText = name;
		tvis.item.lParam = (LPARAM)item;
		tvis.hInsertAfter = TVI_LAST;
		tvis.hParent = (parent ? parent : NULL);
		hti = TreeView_InsertItem (GetDlgItem (hTab, IDC_EXT_LIST), &tvis);
	} else hti = 0;
	item->hItem = (LAUNCHPADITEM_HANDLE)hti;
	return hti;
}

//-----------------------------------------------------------------------------

bool ExtraTab::UnregisterExtraParam (LaunchpadItem *item)
{
	DWORD i, j, k;
	for (i = 0; i < nExtPrm; i++) {
		if (ExtPrm[i] == item) {
			TreeView_DeleteItem (GetDlgItem (hTab, IDC_EXT_LIST), item->hItem);
			LaunchpadItem **tmp = 0;
			if (nExtPrm > 1) {
				tmp = new LaunchpadItem*[nExtPrm-1]; TRACENEW
				for (j = k = 0; j < nExtPrm; j++)
					if (j != i) tmp[k++] = ExtPrm[j];
			}
			delete []ExtPrm;
			ExtPrm = tmp;
			nExtPrm--;
			item->clbkWriteConfig (); // save state before removing
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------

HTREEITEM ExtraTab::FindExtraParam (const char *name, const HTREEITEM parent)
{
	HTREEITEM hti = FindExtraParamChild (parent);
	if (!name) return hti; // no name given - return first child

	char cbuf[256];
	HWND hCtrl = GetDlgItem (hTab, IDC_EXT_LIST);
	TV_ITEM tvi;
	tvi.pszText = cbuf;
	tvi.cchTextMax = 256;
	tvi.hItem = hti;
	tvi.mask = TVIF_HANDLE | TVIF_TEXT;
	
	// step through the list
	while (TreeView_GetItem (hCtrl, &tvi)) {
		if (!_stricmp (name, tvi.pszText)) return tvi.hItem;
		tvi.hItem = TreeView_GetNextSibling (hCtrl, tvi.hItem);
	}

	return 0;
}

//-----------------------------------------------------------------------------

HTREEITEM ExtraTab::FindExtraParamChild (const HTREEITEM parent)
{
	HWND hCtrl = GetDlgItem (hTab, IDC_EXT_LIST);
	if (parent) return TreeView_GetChild (hCtrl, parent);
	else        return TreeView_GetRoot (hCtrl);
}

//-----------------------------------------------------------------------------

void ExtraTab::WriteExtraParams ()
{
	for (DWORD i = 0; i < nExtPrm; i++)
		ExtPrm[i]->clbkWriteConfig ();
}

//-----------------------------------------------------------------------------

BOOL ExtraTab::TabProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	NM_TREEVIEW *pnmtv;

	switch (uMsg) {
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_EXT_OPEN: {
			TVITEM tvi;
			tvi.hItem = TreeView_GetSelection (GetDlgItem (hWnd, IDC_EXT_LIST));
			tvi.mask = TVIF_PARAM;
			if (TreeView_GetItem (GetDlgItem (hWnd, IDC_EXT_LIST), &tvi) && tvi.lParam) {
				BuiltinLaunchpadItem *func = (BuiltinLaunchpadItem*)tvi.lParam;
				func->clbkOpen (pLp->GetWindow());
			}
			} return TRUE;
		}
		break;
	case WM_NOTIFY:
		switch (LOWORD(wParam)) {
		case IDC_EXT_LIST:
			pnmtv = (NM_TREEVIEW FAR *)lParam;
			switch (pnmtv->hdr.code) {
			case TVN_SELCHANGED: {
				LaunchpadItem *func = (LaunchpadItem*)pnmtv->itemNew.lParam;
				char *desc = func->Description();
				if (desc) SetWindowText (GetDlgItem (hTab, IDC_EXT_TEXT), desc);
				else SetWindowText (GetDlgItem (hTab, IDC_EXT_TEXT), "");
				} return TRUE;
			case NM_DBLCLK: {
				TVITEM tvi;
				tvi.hItem = TreeView_GetSelection (GetDlgItem (hWnd, IDC_EXT_LIST));
				tvi.mask = TVIF_PARAM;
				if (TreeView_GetItem (GetDlgItem (hWnd, IDC_EXT_LIST), &tvi) && tvi.lParam) {
					BuiltinLaunchpadItem *func = (BuiltinLaunchpadItem*)tvi.lParam;
					func->clbkOpen (pLp->GetWindow());
				}
				} return TRUE;
			}
			break;
		}
		break;
	}
	return FALSE;
}


// ****************************************************************************
// ****************************************************************************

//-----------------------------------------------------------------------------
// Additional functions (under the "Extra" tab)
//-----------------------------------------------------------------------------

BuiltinLaunchpadItem::BuiltinLaunchpadItem (const ExtraTab *tab): LaunchpadItem ()
{
	pTab = tab;
}

bool BuiltinLaunchpadItem::OpenDialog (HWND hParent, int resid, DLGPROC pDlg)
{
	return LaunchpadItem::OpenDialog (pTab->Launchpad()->GetInstance(), hParent, resid, pDlg);
}

void BuiltinLaunchpadItem::Error (const char *msg)
{
	MessageBox (pTab->Launchpad()->GetWindow(), msg, "Orbiter configuration error", MB_OK|MB_ICONERROR);
}

BOOL CALLBACK BuiltinLaunchpadItem::DlgProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		SetWindowLong (hWnd, DWL_USER, lParam);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD (wParam)) {
		case IDCANCEL:
			EndDialog (hWnd, 0);
		}
		break;
	case WM_CLOSE:
		EndDialog (hWnd, 0);
		return 0;
	}
	return FALSE;
}

//-----------------------------------------------------------------------------
// Physics engine
//-----------------------------------------------------------------------------

char *ExtraPropagation::Name ()
{
	return "Time propagation";
}

char *ExtraPropagation::Description ()
{
	static char *desc = "Select and configure the time propagation methods Orbiter uses to update vessel positions and velocities from one time frame to the next.";
	return desc;
}

//-----------------------------------------------------------------------------
// Physics engine: Parameters for dynamic state propagation
//-----------------------------------------------------------------------------

int ExtraDynamics::PropId[NPROP_METHOD] = {
	PROP_RK2, PROP_RK4, PROP_RK5, PROP_RK6, PROP_RK7, PROP_RK8,
	PROP_SY2, PROP_SY4, PROP_SY6, PROP_SY8
};

char *ExtraDynamics::Name ()
{
	return "Dynamic state propagators";
}

char *ExtraDynamics::Description ()
{
	static char *desc = "Select the numerical integration methods used for dynamic state updates.\r\n\r\nState propagators affect the accuracy and stability of spacecraft orbits and trajectory calculations.";
	return desc;
}

bool ExtraDynamics::clbkOpen (HWND hParent)
{
	OpenDialog (hParent, IDD_EXTRA_DYNAMICS, DlgProc);
	return true;
}

void ExtraDynamics::InitDialog (HWND hWnd)
{
	DWORD i, j;
	for (i = 0; i < 5; i++) {
		SendDlgItemMessage (hWnd, IDC_PROP_PROP0+i, CB_RESETCONTENT, 0, 0);
		for (j = 0; j < NPROP_METHOD; j++)
			SendDlgItemMessage (hWnd, IDC_PROP_PROP0+i, CB_ADDSTRING, 0, (LPARAM)RigidBody::PropagatorStr(j));
	}
	SetDialog (hWnd, pTab->Cfg()->CfgPhysicsPrm);
}

void ExtraDynamics::ResetDialog (HWND hWnd)
{
	extern CFG_PHYSICSPRM CfgPhysicsPrm_default;
	SetDialog (hWnd, CfgPhysicsPrm_default);
}

void ExtraDynamics::SetDialog (HWND hWnd, const CFG_PHYSICSPRM &prm)
{
	char cbuf[64];
	int i, j;
	int n = prm.nLPropLevel;
	for (i = 0; i < 5; i++) {
		SendDlgItemMessage (hWnd, IDC_PROP_ACTIVE0+i, BM_SETCHECK, i < n ? BST_CHECKED : BST_UNCHECKED, 0);
		EnableWindow (GetDlgItem (hWnd, IDC_PROP_ACTIVE0+i), i < n-1 || i > n || i == 0 ? FALSE : TRUE);
		ShowWindow (GetDlgItem (hWnd, IDC_PROP_PROP0+i), i < n ? SW_SHOW : SW_HIDE);
		ShowWindow (GetDlgItem (hWnd, IDC_PROP_TTGT0+i), i < n ? SW_SHOW : SW_HIDE);
		ShowWindow (GetDlgItem (hWnd, IDC_PROP_ATGT0+i), i < n ? SW_SHOW : SW_HIDE);
		if (i < 4) {
			ShowWindow (GetDlgItem (hWnd, IDC_PROP_TLIMIT01+i), i < n-1 ? SW_SHOW : SW_HIDE);
			ShowWindow (GetDlgItem (hWnd, IDC_PROP_ALIMIT01+i), i < n-1 ? SW_SHOW : SW_HIDE);
		}
		if (i < n) {
			int id = prm.PropMode[i];
			for (j = 0; j < NPROP_METHOD; j++)
				if (id == PropId[j]) {
					SendDlgItemMessage (hWnd, IDC_PROP_PROP0+i, CB_SETCURSEL, j, 0);
					break;
				}
			sprintf (cbuf, "%0.2f", prm.PropTTgt[i]);
			SetWindowText (GetDlgItem (hWnd, IDC_PROP_TTGT0+i), cbuf);
			sprintf (cbuf, "%0.1f", prm.PropATgt[i]*DEG);
			SetWindowText (GetDlgItem (hWnd, IDC_PROP_ATGT0+i), cbuf);
			if (i < n-1) {
				sprintf (cbuf, "%0.2f", prm.PropTLim[i]);
				SetWindowText (GetDlgItem (hWnd, IDC_PROP_TLIMIT01+i), cbuf);
				sprintf (cbuf, "%0.1f", prm.PropALim[i]*DEG);
				SetWindowText (GetDlgItem (hWnd, IDC_PROP_ALIMIT01+i), cbuf);
			}
		}
	}
	sprintf (cbuf, "%d", prm.PropSubMax);
	SetWindowText (GetDlgItem (hWnd, IDC_PROP_MAXSAMPLE), cbuf);
}

void ExtraDynamics::Activate (HWND hWnd, int which)
{
	int i = which-IDC_PROP_ACTIVE0;
	int check = SendDlgItemMessage (hWnd, which, BM_GETCHECK, 0, 0);
	if (check == BST_CHECKED) {
		if (i < 4) EnableWindow (GetDlgItem (hWnd, which+1), TRUE);
		if (i > 0) EnableWindow (GetDlgItem (hWnd, which-1), FALSE);
		ShowWindow (GetDlgItem (hWnd, IDC_PROP_PROP0+i), SW_SHOW);
		ShowWindow (GetDlgItem (hWnd, IDC_PROP_TTGT0+i), SW_SHOW);
		ShowWindow (GetDlgItem (hWnd, IDC_PROP_ATGT0+i), SW_SHOW);
		if (i > 0) {
			ShowWindow (GetDlgItem (hWnd, IDC_PROP_TLIMIT01+i-1), SW_SHOW);
			ShowWindow (GetDlgItem (hWnd, IDC_PROP_ALIMIT01+i-1), SW_SHOW);
		}
	} else {
		if (i > 1) EnableWindow (GetDlgItem (hWnd, which-1), TRUE);
		if (i < 4) EnableWindow (GetDlgItem (hWnd, which+1), FALSE);
		ShowWindow (GetDlgItem (hWnd, IDC_PROP_PROP0+i), SW_HIDE);
		ShowWindow (GetDlgItem (hWnd, IDC_PROP_TTGT0+i), SW_HIDE);
		ShowWindow (GetDlgItem (hWnd, IDC_PROP_ATGT0+i), SW_HIDE);
		if (i > 0) {
			ShowWindow (GetDlgItem (hWnd, IDC_PROP_TLIMIT01+i-1), SW_HIDE);
			ShowWindow (GetDlgItem (hWnd, IDC_PROP_ALIMIT01+i-1), SW_HIDE);
		}
	}
}

bool ExtraDynamics::StoreParams (HWND hWnd)
{
	char cbuf[256];
	int i, n = 0;
	double ttgt[MAX_PROP_LEVEL], atgt[MAX_PROP_LEVEL], tlim[MAX_PROP_LEVEL], alim[MAX_PROP_LEVEL];
	int mode[MAX_PROP_LEVEL];
	for (i = 0; i < MAX_PROP_LEVEL; i++) {
		if (SendDlgItemMessage (hWnd, IDC_PROP_ACTIVE0+i, BM_GETCHECK, 0, 0) == BST_CHECKED)
			n++;
	}
	for (i = 0; i < n; i++) {
		mode[i] = SendDlgItemMessage (hWnd, IDC_PROP_PROP0+i, CB_GETCURSEL, 0, 0);
		if (mode[i] == CB_ERR) {
			sprintf (cbuf, "Invalid propagator for integration stage %d.", i+1);
			Error (cbuf);
			return false;
		}
		GetWindowText (GetDlgItem (hWnd, IDC_PROP_TTGT0+i), cbuf, 256);
		if ((sscanf (cbuf, "%lf", ttgt+i) != 1) || (ttgt[i] <= 0)) {
			sprintf (cbuf, "Invalid time step target for integration stage %d.", i+1);
			Error (cbuf);
			return false;
		}
		GetWindowText (GetDlgItem (hWnd, IDC_PROP_ATGT0+i), cbuf, 256);
		if ((sscanf (cbuf, "%lf", atgt+i) != 1) || (atgt[i] <= 0)) {
			sprintf (cbuf, "Invalid angle step target for integration stage %d.", i+1);
			Error (cbuf);
			return false;
		}
		if (i < n-1) {
			GetWindowText (GetDlgItem (hWnd, IDC_PROP_TLIMIT01+i), cbuf, 256);
			if ((sscanf (cbuf, "%lf", tlim+i) != 1) || (tlim[i] <= 0)) {
				sprintf (cbuf, "Invalid time step limit for integration stage %d -> %d.", i+1, i+2);
				Error (cbuf);
				return false;
			}
			GetWindowText (GetDlgItem (hWnd, IDC_PROP_ALIMIT01+i), cbuf, 256);
			if ((sscanf (cbuf, "%lf", alim+i) != 1) || (alim[i] <= 0)) {
				sprintf (cbuf, "Invalid angle step limit for integration stage %d -> %d.", i+1, i+2);
				Error (cbuf);
				return false;
			}
			if (i > 0 && (tlim[i] <= tlim[i-1] || alim[i] <= alim[i-1])) {
				Error ("Step limits must be in ascending order");
				return false;
			}
		}
	}

	Config *cfg = pTab->Cfg();
	cfg->CfgPhysicsPrm.nLPropLevel = n;
	for (i = 0; i < n; i++) {
		cfg->CfgPhysicsPrm.PropMode[i] = PropId[mode[i]];
		cfg->CfgPhysicsPrm.PropTTgt[i] = ttgt[i];
		cfg->CfgPhysicsPrm.PropATgt[i] = atgt[i]*RAD;
		cfg->CfgPhysicsPrm.PropTLim[i] = (i < n-1 ? tlim[i] : 1e10);
		cfg->CfgPhysicsPrm.PropALim[i] = (i < n-1 ? alim[i]*RAD : 1e10);
	}

	GetWindowText (GetDlgItem (hWnd, IDC_PROP_MAXSAMPLE), cbuf, 256);
	if ((sscanf (cbuf, "%d", &i) != 1) || i < 1) {
		Error ("Invalid value for max. subsamples (integer value >= 1 required).");
		return false;
	} else {
		cfg->CfgPhysicsPrm.PropSubMax = i;
	}

	return true;
}

bool ExtraDynamics::OpenHelp (HWND hWnd)
{
	OpenDefaultHelp (hWnd, pTab->Launchpad()->GetInstance(), "extra_linprop");
	return true;
}

BOOL CALLBACK ExtraDynamics::DlgProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		((ExtraDynamics*)lParam)->InitDialog (hWnd);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_PROP_ACTIVE0:
		case IDC_PROP_ACTIVE1:
		case IDC_PROP_ACTIVE2:
		case IDC_PROP_ACTIVE3:
		case IDC_PROP_ACTIVE4:
			((ExtraDynamics*)GetWindowLong (hWnd, DWL_USER))->Activate (hWnd, LOWORD(wParam));
			break;
		case IDC_RESET:
			((ExtraDynamics*)GetWindowLong (hWnd, DWL_USER))->ResetDialog (hWnd);
			return 0;
		case IDCHELP:
			((ExtraDynamics*)GetWindowLong (hWnd, DWL_USER))->OpenHelp (hWnd);
			return 0;
		case IDOK:
			if (((ExtraDynamics*)GetWindowLong (hWnd, DWL_USER))->StoreParams (hWnd))
				EndDialog (hWnd, 0);
			break;
		}
		break;
	}
	return BuiltinLaunchpadItem::DlgProc (hWnd, uMsg, wParam, lParam);
}

//-----------------------------------------------------------------------------
// Physics engine: Parameters for angular state propagation
//-----------------------------------------------------------------------------
#ifdef UNDEF

int ExtraAngDynamics::PropId[NAPROP_METHOD] = {
	PROP_RK2, PROP_RK4, PROP_RK5, PROP_RK6, PROP_RK7, PROP_RK8
};

char *ExtraAngDynamics::Name ()
{
	return "Angular state propagators";
}

char *ExtraAngDynamics::Description ()
{
	static char *desc = "Select the numerical integration method for dynamic angular state updates.\r\n\r\nAngular propagators affect the simulation accuracy and stability of rotating vessels.";
	return desc;
}

bool ExtraAngDynamics::clbkOpen (HWND hParent)
{
	OpenDialog (hParent, IDD_EXTRA_ADYNAMICS, DlgProc);
	return true;
}

void ExtraAngDynamics::InitDialog (HWND hWnd)
{
	static char *label[NAPROP_METHOD] = {
		"Runge-Kutta, 2nd order (RK2)", "Runge-Kutta, 4th order (RK4)", "Runge-Kutta, 5th order (RK5)",
		"Runge-Kutta, 6th order (RK6)", "Runge-Kutta, 7th order (RK7)", "Runge-Kutta, 8th order (RK8)"
	};

	int i, j;
	for (i = 0; i < 5; i++) {
		SendDlgItemMessage (hWnd, IDC_COMBO1+i, CB_RESETCONTENT, 0, 0);
		for (j = 0; j < NAPROP_METHOD; j++)
			SendDlgItemMessage (hWnd, IDC_COMBO1+i, CB_ADDSTRING, 0, (LPARAM)label[j]);
	}
	SetDialog (hWnd, pTab->Cfg()->CfgPhysicsPrm);
}

void ExtraAngDynamics::ResetDialog (HWND hWnd)
{
	extern CFG_PHYSICSPRM CfgPhysicsPrm_default;
	SetDialog (hWnd, CfgPhysicsPrm_default);
}

void ExtraAngDynamics::SetDialog (HWND hWnd, const CFG_PHYSICSPRM &prm)
{
	char cbuf[64];
	int i, j;
	int n = prm.nAPropLevel;
	for (i = 0; i < 5; i++) {
		SendDlgItemMessage (hWnd, IDC_CHECK1+i, BM_SETCHECK, i < n ? BST_CHECKED : BST_UNCHECKED, 0);
		EnableWindow (GetDlgItem (hWnd, IDC_CHECK1+i), i < n-1 || i > n || i == 0 ? FALSE : TRUE);
		ShowWindow (GetDlgItem (hWnd, IDC_COMBO1+i), i < n ? SW_SHOW : SW_HIDE);
		if (i < 4) {
			ShowWindow (GetDlgItem (hWnd, IDC_EDIT1+i), i < n-1 ? SW_SHOW : SW_HIDE);
			ShowWindow (GetDlgItem (hWnd, IDC_EDIT5+i), i < n-1 ? SW_SHOW : SW_HIDE);
		}
		if (i < n) {
			int id = prm.APropMode[i];
			for (j = 0; j < NAPROP_METHOD; j++)
				if (id == PropId[j]) {
					SendDlgItemMessage (hWnd, IDC_COMBO1+i, CB_SETCURSEL, j, 0);
					break;
				}
			if (i < n-1) {
				sprintf (cbuf, "%0.2f", prm.APropTLimit[i]);
				SetWindowText (GetDlgItem (hWnd, IDC_EDIT1+i), cbuf);
				sprintf (cbuf, "%0.1f", prm.PropALimit[i]*DEG);
				SetWindowText (GetDlgItem (hWnd, IDC_EDIT5+i), cbuf);
			}
		}
	}
	sprintf (cbuf, "%0.1f", prm.APropSubLimit*DEG);
	SetWindowText (GetDlgItem (hWnd, IDC_EDIT9), cbuf);
	sprintf (cbuf, "%d", prm.APropSubMax);
	SetWindowText (GetDlgItem (hWnd, IDC_EDIT10), cbuf);
	sprintf (cbuf, "%0.1f", prm.APropCouplingLimit*DEG);
	SetWindowText (GetDlgItem (hWnd, IDC_EDIT11), cbuf);
	sprintf (cbuf, "%0.1f", prm.APropTorqueLimit*DEG);
	SetWindowText (GetDlgItem (hWnd, IDC_EDIT12), cbuf);
}

void ExtraAngDynamics::Activate (HWND hWnd, int which)
{
	int i = which-IDC_CHECK1;
	int check = SendDlgItemMessage (hWnd, which, BM_GETCHECK, 0, 0);
	if (check == BST_CHECKED) {
		if (i < 4) EnableWindow (GetDlgItem (hWnd, which+1), TRUE);
		if (i > 0) EnableWindow (GetDlgItem (hWnd, which-1), FALSE);
		ShowWindow (GetDlgItem (hWnd, IDC_COMBO1+i), SW_SHOW);
		if (i > 0) {
			ShowWindow (GetDlgItem (hWnd, IDC_EDIT1+i-1), SW_SHOW);
			ShowWindow (GetDlgItem (hWnd, IDC_EDIT5+i-1), SW_SHOW);
		}
	} else {
		if (i > 1) EnableWindow (GetDlgItem (hWnd, which-1), TRUE);
		if (i < 4) EnableWindow (GetDlgItem (hWnd, which+1), FALSE);
		ShowWindow (GetDlgItem (hWnd, IDC_COMBO1+i), SW_HIDE);
		if (i > 0) {
			ShowWindow (GetDlgItem (hWnd, IDC_EDIT1+i-1), SW_HIDE);
			ShowWindow (GetDlgItem (hWnd, IDC_EDIT5+i-1), SW_HIDE);
		}
	}
}

bool ExtraAngDynamics::StoreParams (HWND hWnd)
{
	char cbuf[256];
	int i, n = 0;
	double val, tlimit[5], alimit[5], couplim, torqlim;
	int mode[5];
	for (i = 0; i < 5; i++) {
		if (SendDlgItemMessage (hWnd, IDC_CHECK1+i, BM_GETCHECK, 0, 0) == BST_CHECKED)
			n++;
	}
	for (i = 0; i < n-1; i++) {
		GetWindowText (GetDlgItem (hWnd, IDC_EDIT1+i), cbuf, 256);
		if ((sscanf (cbuf, "%lf", tlimit+i) != 1) || (tlimit[i] <= 0)) {
			Error ("Invalid step limit entry.");
			return false;
		}
		GetWindowText (GetDlgItem (hWnd, IDC_EDIT5+i), cbuf, 256);
		if ((sscanf (cbuf, "%lf", alimit+i) != 1) || (alimit[i] <= 0)) {
			Error ("Invalid angle step limit entry.");
			return false;
		}
		if (i > 0 && (tlimit[i] <= tlimit[i-1] || alimit[i] <= alimit[i-1])) {
			Error ("Step limits must be in ascending order");
			return false;
		}
	}
	for (i = 0; i < n; i++) {
		mode[i] = SendDlgItemMessage (hWnd, IDC_COMBO1+i, CB_GETCURSEL, 0, 0);
		if (mode[i] == CB_ERR) {
			Error ("Invalid propagator.");
			return false;
		}
	}
	GetWindowText (GetDlgItem (hWnd, IDC_EDIT11), cbuf, 256);
	if ((sscanf (cbuf, "%lf", &couplim) != 1) || (couplim < 0.0)) {
		Error ("Invalid coupling step limit");
		return false;
	}
	GetWindowText (GetDlgItem (hWnd, IDC_EDIT12), cbuf, 256);
	if ((sscanf (cbuf, "%lf", &torqlim) != 1) || (torqlim < couplim)) {
		Error ("Torque step limit must be greater than coupling limit");
		return false;
	}

	Config *cfg = pTab->Cfg();
	cfg->CfgPhysicsPrm.nAPropLevel = n;
	for (i = 0; i < n; i++) {
		cfg->CfgPhysicsPrm.APropMode[i] = PropId[mode[i]];
		cfg->CfgPhysicsPrm.APropTLimit[i] = (i < n-1 ? tlimit[i]     : 1e10);
		cfg->CfgPhysicsPrm.PropALimit[i] = (i < n-1 ? alimit[i]*RAD : 1e10);
	}
	cfg->CfgPhysicsPrm.APropCouplingLimit = couplim*RAD;
	cfg->CfgPhysicsPrm.APropTorqueLimit = torqlim*RAD;

	GetWindowText (GetDlgItem (hWnd, IDC_EDIT9), cbuf, 256);
	if ((sscanf (cbuf, "%lf", &val) != 1 || val < 0)) {
		Error ("Invalid subsampling target step.");
		return false;
	} else cfg->CfgPhysicsPrm.APropSubLimit = val*RAD;
	GetWindowText (GetDlgItem (hWnd, IDC_EDIT10), cbuf, 256);
	if ((sscanf (cbuf, "%d", &i) != 1) || (i < 1)) {
		Error ("Invalid subsampling steps.");
		return false;
	} else cfg->CfgPhysicsPrm.APropSubMax = i;

	return true;
}

bool ExtraAngDynamics::OpenHelp (HWND hWnd)
{
	OpenDefaultHelp (hWnd, pTab->Launchpad()->GetInstance(), "extra_angprop");
	return true;
}

BOOL CALLBACK ExtraAngDynamics::DlgProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		((ExtraAngDynamics*)lParam)->InitDialog (hWnd);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_CHECK1:
		case IDC_CHECK2:
		case IDC_CHECK3:
		case IDC_CHECK4:
		case IDC_CHECK5:
			((ExtraAngDynamics*)GetWindowLong (hWnd, DWL_USER))->Activate (hWnd, LOWORD(wParam));
			break;
		case IDC_BUTTON1:
			((ExtraAngDynamics*)GetWindowLong (hWnd, DWL_USER))->ResetDialog (hWnd);
			return 0;
		case IDC_BUTTON2:
			((ExtraAngDynamics*)GetWindowLong (hWnd, DWL_USER))->OpenHelp (hWnd);
			return 0;
		case IDOK:
			if (((ExtraAngDynamics*)GetWindowLong (hWnd, DWL_USER))->StoreParams (hWnd))
				EndDialog (hWnd, 0);
			break;
		}
		break;
	}
	return BuiltinLaunchpadItem::DlgProc (hWnd, uMsg, wParam, lParam);
}

#endif

//-----------------------------------------------------------------------------
// Physics engine: Parameters for orbit stabilisation
//-----------------------------------------------------------------------------

char *ExtraStabilisation::Name ()
{
	return "Orbit stabilisation";
}

char *ExtraStabilisation::Description ()
{
	static char *desc = "Select the parameters that determine the conditions when Orbiter switches between dynamic and stabilised state updates.";
	return desc;
}

bool ExtraStabilisation::clbkOpen (HWND hParent)
{
	OpenDialog (hParent, IDD_EXTRA_STABILISATION, DlgProc);
	return true;
}

void ExtraStabilisation::InitDialog (HWND hWnd)
{
	SetDialog (hWnd, pTab->Cfg()->CfgPhysicsPrm);
}

void ExtraStabilisation::ResetDialog (HWND hWnd)
{
	extern CFG_PHYSICSPRM CfgPhysicsPrm_default;
	SetDialog (hWnd, CfgPhysicsPrm_default);
}

void ExtraStabilisation::SetDialog (HWND hWnd, const CFG_PHYSICSPRM &prm)
{
	char cbuf[256];
	SendDlgItemMessage (hWnd, IDC_STAB_ENABLE, BM_SETCHECK, prm.bOrbitStabilise ? BST_CHECKED : BST_UNCHECKED, 0);
	sprintf (cbuf, "%0.4g", prm.Stabilise_PLimit*100.0);
	SetWindowText (GetDlgItem (hWnd, IDC_EDIT1), cbuf);
	sprintf (cbuf, "%0.4g", prm.Stabilise_SLimit*100.0);
	SetWindowText (GetDlgItem (hWnd, IDC_EDIT2), cbuf);
	sprintf (cbuf, "%0.4g", prm.PPropSubLimit*100.0);
	SetWindowText (GetDlgItem (hWnd, IDC_EDIT3), cbuf);
	sprintf (cbuf, "%d", prm.PPropSubMax);
	SetWindowText (GetDlgItem (hWnd, IDC_EDIT4), cbuf);
	sprintf (cbuf, "%0.4g", prm.PPropStepLimit*100.0);
	SetWindowText (GetDlgItem (hWnd, IDC_EDIT5), cbuf);
	ToggleEnable (hWnd);
}

bool ExtraStabilisation::StoreParams (HWND hWnd)
{
	char cbuf[256];
	int i;
	double plimit, slimit, val;
	Config *cfg = pTab->Cfg();
	GetWindowText (GetDlgItem (hWnd, IDC_EDIT1), cbuf, 256);
	if (sscanf (cbuf, "%lf", &plimit) != 1 || plimit < 0.0 || plimit > 100.0) {
		Error ("Invalid perturbation limit.");
		return false;
	}
	GetWindowText (GetDlgItem (hWnd, IDC_EDIT2), cbuf, 256);
	if (sscanf (cbuf, "%lf", &slimit) != 1 || slimit < 0.0) {
		Error ("Invalid step limit.");
		return false;
	}
	GetWindowText (GetDlgItem (hWnd, IDC_EDIT3), cbuf, 256);
	if ((sscanf (cbuf, "%lf", &val) != 1 || val < 0)) {
		Error ("Invalid subsampling target step.");
		return false;
	} else cfg->CfgPhysicsPrm.PPropSubLimit = val*0.01;
	GetWindowText (GetDlgItem (hWnd, IDC_EDIT4), cbuf, 256);
	if ((sscanf (cbuf, "%d", &i) != 1) || (i < 1)) {
		Error ("Invalid subsampling steps.");
		return false;
	} else cfg->CfgPhysicsPrm.PPropSubMax = i;
	GetWindowText (GetDlgItem (hWnd, IDC_EDIT5), cbuf, 256);
	if ((sscanf (cbuf, "%lf", &val) != 1 || val < 0)) {
		Error ("Invalid perturbation limit value.");
		return false;
	} else cfg->CfgPhysicsPrm.PPropStepLimit = val*0.01;

	cfg->CfgPhysicsPrm.bOrbitStabilise = (SendDlgItemMessage (hWnd, IDC_STAB_ENABLE, BM_GETCHECK, 0, 0) == BST_CHECKED);
	cfg->CfgPhysicsPrm.Stabilise_PLimit = plimit * 0.01;
	cfg->CfgPhysicsPrm.Stabilise_SLimit = slimit * 0.01;
	return true;
}

void ExtraStabilisation::ToggleEnable (HWND hWnd)
{
	int i;
	bool bstab = (SendDlgItemMessage (hWnd, IDC_STAB_ENABLE, BM_GETCHECK, 0, 0) == BST_CHECKED);
	for (i = IDC_EDIT1; i <= IDC_EDIT5; i++)
		EnableWindow (GetDlgItem (hWnd, i), bstab);
	for (i = IDC_STATIC1; i <= IDC_STATIC13; i++)
		EnableWindow (GetDlgItem (hWnd, i), bstab);
}

bool ExtraStabilisation::OpenHelp (HWND hWnd)
{
	OpenDefaultHelp (hWnd, pTab->Launchpad()->GetInstance(), "extra_orbitstab");
	return true;
}

BOOL CALLBACK ExtraStabilisation::DlgProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		((ExtraStabilisation*)lParam)->InitDialog (hWnd);
		break;
	case WM_COMMAND:
		switch (LOWORD (wParam)) {
		case IDC_STAB_ENABLE:
			if (HIWORD (wParam) == BN_CLICKED) {
				((ExtraStabilisation*)GetWindowLong (hWnd, DWL_USER))->ToggleEnable (hWnd);
				return TRUE;
			}
			break;
		case IDC_BUTTON1:
			((ExtraStabilisation*)GetWindowLong (hWnd, DWL_USER))->ResetDialog (hWnd);
			return 0;
		case IDC_BUTTON2:
			((ExtraStabilisation*)GetWindowLong (hWnd, DWL_USER))->OpenHelp (hWnd);
			return 0;
		case IDOK:
			if (((ExtraStabilisation*)GetWindowLong (hWnd, DWL_USER))->StoreParams(hWnd))
				EndDialog (hWnd, 0);
			break;
		}
		break;
	}
	return BuiltinLaunchpadItem::DlgProc (hWnd, uMsg, wParam, lParam);
}


//=============================================================================
// Instruments and panels
//=============================================================================

char *ExtraInstruments::Name ()
{
	return "Instruments and panels";
}

char *ExtraInstruments::Description ()
{
	static char *desc = "Select general configuration parameters for spacecraft instruments, MFD displays and instrument panels.";
	return desc;
}

//-----------------------------------------------------------------------------
// Instruments and panels: MFDs
//-----------------------------------------------------------------------------

char *ExtraMfdConfig::Name()
{
	return "MFD parameter configuration";
}

char *ExtraMfdConfig::Description ()
{
	static char *desc = "Select display parameters for multifunctional displays (MFD).";
	return desc;
}

bool ExtraMfdConfig::clbkOpen (HWND hParent)
{
	OpenDialog (hParent, IDD_EXTRA_MFDCONFIG, DlgProc);
	return true;
}

void ExtraMfdConfig::InitDialog (HWND hWnd)
{
	SetDialog (hWnd, pTab->Cfg()->CfgInstrumentPrm);
}

void ExtraMfdConfig::ResetDialog (HWND hWnd)
{
	extern CFG_INSTRUMENTPRM CfgInstrumentPrm_default;
	SetDialog (hWnd, CfgInstrumentPrm_default);
}

void ExtraMfdConfig::SetDialog (HWND hWnd, const CFG_INSTRUMENTPRM &prm)
{
	char cbuf[256];
	int i, idx;
	for (i = 0; i < 3; i++)
		SendDlgItemMessage (hWnd, IDC_RADIO1+i, BM_SETCHECK, i == prm.bMfdPow2 ? BST_CHECKED : BST_UNCHECKED, 0);
	sprintf (cbuf, "%d", prm.MfdHiresThreshold);
	SetWindowText (GetDlgItem (hWnd, IDC_EDIT1), cbuf);

	idx = (prm.VCMFDSize == 256 ? 0 : prm.VCMFDSize == 512 ? 1 : 2);
	for (i = 0; i < 3; i++)
		SendDlgItemMessage (hWnd, IDC_RADIO4+i, BM_SETCHECK, i == idx ? BST_CHECKED : BST_UNCHECKED, 0);
}

bool ExtraMfdConfig::StoreParams (HWND hWnd)
{
	Config *cfg = pTab->Cfg();
	char cbuf[256];
	int i, size, check;
	GetWindowText (GetDlgItem (hWnd, IDC_EDIT1), cbuf, 256);
	if ((sscanf (cbuf, "%d", &size) != 1) || size < 8) {
		return false;
	} else
		cfg->CfgInstrumentPrm.MfdHiresThreshold = size;

	for (i = 0; i < 3; i++) {
		check = SendDlgItemMessage (hWnd, IDC_RADIO1+i, BM_GETCHECK, 0, 0);
		if (check == BST_CHECKED) {
			cfg->CfgInstrumentPrm.bMfdPow2 = i;
			break;
		}
	}

	size = 256;
	for (i = 0; i < 3; i++) {
		check = SendDlgItemMessage (hWnd, IDC_RADIO4+i, BM_GETCHECK, 0, 0);
		if (check == BST_CHECKED) {
			cfg->CfgInstrumentPrm.VCMFDSize = size;
			break;
		}
		size *= 2;
	}

	return true;
}

void ExtraMfdConfig::ToggleEnable (HWND hWnd)
{
	// todo
}

bool ExtraMfdConfig::OpenHelp (HWND hWnd)
{
	OpenDefaultHelp (hWnd, pTab->Launchpad()->GetInstance(), "extra_mfdconfig");
	return true;
}

BOOL CALLBACK ExtraMfdConfig::DlgProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		((ExtraMfdConfig*)lParam)->InitDialog (hWnd);
		break;
	case WM_COMMAND:
		switch (LOWORD (wParam)) {
		case IDC_BUTTON1:
			((ExtraMfdConfig*)GetWindowLong (hWnd, DWL_USER))->ResetDialog (hWnd);
			return 0;
		case IDC_BUTTON2:
			((ExtraMfdConfig*)GetWindowLong (hWnd, DWL_USER))->OpenHelp (hWnd);
			return 0;
		case IDOK:
			if (((ExtraMfdConfig*)GetWindowLong (hWnd, DWL_USER))->StoreParams (hWnd))
				EndDialog (hWnd, 0);
			break;
		}
		break;
	}
	return BuiltinLaunchpadItem::DlgProc (hWnd, uMsg, wParam, lParam);
}


//=============================================================================
// Root item for vessel configurations (sub-items to be added by modules)
//=============================================================================

char *ExtraVesselConfig::Name ()
{
	return "Vessel configuration";
}

char *ExtraVesselConfig::Description ()
{
	static char *desc = "Configure spacecraft parameters";
	return desc;
}

//=============================================================================
// Root item for planet configurations (sub-items to be added by modules)
//=============================================================================

char *ExtraPlanetConfig::Name ()
{
	return "Celestial body configuration";
}

char *ExtraPlanetConfig::Description ()
{
	static char *desc = "Configure options for celestial objects";
	return desc;
}

//=============================================================================
// Debugging parameters
//=============================================================================

char *ExtraDebug::Name ()
{
	return "Debugging options";
}

char *ExtraDebug::Description ()
{
	static char *desc = "Various options that are useful for debugging and special tasks. Not generally used for standard simulation sessions.";
	return desc;
}

//-----------------------------------------------------------------------------
// Debugging parameters: shutdown options
//-----------------------------------------------------------------------------

char *ExtraShutdown::Name ()
{
	return "Orbiter shutdown options";
}

char *ExtraShutdown::Description ()
{
	static char *desc = "Set the behaviour of Orbiter after closing the simulation window: return to Launchpad, respawn or terminate.";
	return desc;
}

bool ExtraShutdown::clbkOpen (HWND hParent)
{
	OpenDialog (hParent, IDD_EXTRA_SHUTDOWN, DlgProc);
	return true;
}

void ExtraShutdown::InitDialog (HWND hWnd)
{
	SetDialog (hWnd, pTab->Cfg()->CfgDebugPrm);
}

void ExtraShutdown::ResetDialog (HWND hWnd)
{
	extern CFG_DEBUGPRM CfgDebugPrm_default;
	SetDialog (hWnd, CfgDebugPrm_default);
}

void ExtraShutdown::SetDialog (HWND hWnd, const CFG_DEBUGPRM &prm)
{
	for (int i = 0; i < 3; i++)
		SendDlgItemMessage (hWnd, IDC_RADIO1+i, BM_SETCHECK, (i==prm.ShutdownMode ? BST_CHECKED:BST_UNCHECKED), 0);
}

bool ExtraShutdown::StoreParams (HWND hWnd)
{
	Config *cfg = pTab->Cfg();
	int mode;
	for (mode = 0; mode < 2; mode++)
		if (SendDlgItemMessage (hWnd, IDC_RADIO1+mode, BM_GETCHECK, 0, 0) == BST_CHECKED) break;
	cfg->CfgDebugPrm.ShutdownMode = mode;
	return true;
}

bool ExtraShutdown::OpenHelp (HWND hWnd)
{
	OpenDefaultHelp (hWnd, pTab->Launchpad()->GetInstance(), "extra_shutdown");
	return true;
}

BOOL CALLBACK ExtraShutdown::DlgProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		((ExtraShutdown*)lParam)->InitDialog (hWnd);
		break;
	case WM_COMMAND:
		switch (LOWORD (wParam)) {
		case IDC_BUTTON1:
			((ExtraShutdown*)GetWindowLong (hWnd, DWL_USER))->ResetDialog (hWnd);
			return 0;
		case IDC_BUTTON2:
			((ExtraShutdown*)GetWindowLong (hWnd, DWL_USER))->OpenHelp (hWnd);
			return 0;
		case IDOK:
			if (((ExtraShutdown*)GetWindowLong (hWnd, DWL_USER))->StoreParams (hWnd))
				EndDialog (hWnd, 0);
			break;
		}
		break;
	}
	return BuiltinLaunchpadItem::DlgProc (hWnd, uMsg, wParam, lParam);
}

//-----------------------------------------------------------------------------
// Debugging parameters: fixed time steps
//-----------------------------------------------------------------------------

char *ExtraFixedStep::Name ()
{
	return "Fixed time steps";
}

char *ExtraFixedStep::Description ()
{
	static char *desc = "This option assigns a fixed simulation time interval to each frame. Useful for debugging, and when numerical accuracy and stability of the dynamic propagators are important (for example, to generate trajectory data or when recording high-fidelity playbacks).\r\n\r\nWarning: Selecting this option leads to nonlinear time flow and a simulation that is no longer real-time.";
	return desc;
}

bool ExtraFixedStep::clbkOpen (HWND hParent)
{
	OpenDialog (hParent, IDD_EXTRA_FIXEDSTEP, DlgProc);
	return true;
}

void ExtraFixedStep::InitDialog (HWND hWnd)
{
	SetDialog (hWnd, pTab->Cfg()->CfgDebugPrm);
}

void ExtraFixedStep::ResetDialog (HWND hWnd)
{
	extern CFG_DEBUGPRM CfgDebugPrm_default;
	SetDialog (hWnd, CfgDebugPrm_default);
}

void ExtraFixedStep::SetDialog (HWND hWnd, const CFG_DEBUGPRM &prm)
{
	char cbuf[256];
	double step = prm.FixedStep;
	SendDlgItemMessage (hWnd, IDC_CHECK1, BM_SETCHECK, step ? BST_CHECKED : BST_UNCHECKED, 0);
	sprintf (cbuf, "%0.4g", step ? step : 0.01);
	SetWindowText (GetDlgItem (hWnd, IDC_EDIT1), cbuf);
	ToggleEnable (hWnd);
}

bool ExtraFixedStep::StoreParams (HWND hWnd)
{
	Config *cfg = pTab->Cfg();
	bool fixed = (SendDlgItemMessage (hWnd, IDC_CHECK1, BM_GETCHECK, 0, 0) == BST_CHECKED);
	if (!fixed) {
		cfg->CfgDebugPrm.FixedStep = 0;
	} else {
		char cbuf[256];
		double dt;
		GetWindowText (GetDlgItem (hWnd, IDC_EDIT1), cbuf, 256);
		if (sscanf (cbuf, "%lf", &dt) != 1 || dt <= 0) {
			Error ("Invalid frame interval length");
			return false;
		}
		cfg->CfgDebugPrm.FixedStep = dt;
	}
	return true;
}

void ExtraFixedStep::ToggleEnable (HWND hWnd)
{
	bool fixed = (SendDlgItemMessage (hWnd, IDC_CHECK1, BM_GETCHECK, 0, 0) == BST_CHECKED);
	EnableWindow (GetDlgItem (hWnd, IDC_STATIC1), fixed);
	EnableWindow (GetDlgItem (hWnd, IDC_EDIT1), fixed);
}

bool ExtraFixedStep::OpenHelp (HWND hWnd)
{
	OpenDefaultHelp (hWnd, pTab->Launchpad()->GetInstance(), "extra_fixedstep");
	return true;
}

BOOL CALLBACK ExtraFixedStep::DlgProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		((ExtraFixedStep*)lParam)->InitDialog (hWnd);
		break;
	case WM_COMMAND:
		switch (LOWORD (wParam)) {
		case IDC_CHECK1:
			if (HIWORD (wParam) == BN_CLICKED) {
				((ExtraFixedStep*)GetWindowLong (hWnd, DWL_USER))->ToggleEnable (hWnd);
				return TRUE;
			}
			break;
		case IDC_BUTTON1:
			((ExtraFixedStep*)GetWindowLong (hWnd, DWL_USER))->ResetDialog (hWnd);
			return 0;
		case IDC_BUTTON2:
			((ExtraFixedStep*)GetWindowLong (hWnd, DWL_USER))->OpenHelp (hWnd);
			return 0;
		case IDOK:
			if (((ExtraFixedStep*)GetWindowLong (hWnd, DWL_USER))->StoreParams (hWnd))
				EndDialog (hWnd, 0);
			break;
		}
		break;
	}
	return BuiltinLaunchpadItem::DlgProc (hWnd, uMsg, wParam, lParam);
}

//-----------------------------------------------------------------------------
// Debugging parameters: rendering options
//-----------------------------------------------------------------------------

char *ExtraRenderingOptions::Name ()
{
	return "Rendering options";
}

char *ExtraRenderingOptions::Description ()
{
	static char *desc = "Some rendering options that can be used for debugging problems.";
	return desc;
}

bool ExtraRenderingOptions::clbkOpen (HWND hParent)
{
	OpenDialog (hParent, IDD_EXTRA_DBGRENDER, DlgProc);
	return true;
}

void ExtraRenderingOptions::InitDialog (HWND hWnd)
{
	SetDialog (hWnd, pTab->Cfg()->CfgDebugPrm);
}

void ExtraRenderingOptions::ResetDialog (HWND hWnd)
{
	extern CFG_DEBUGPRM CfgDebugPrm_default;
	SetDialog (hWnd, CfgDebugPrm_default);
}

void ExtraRenderingOptions::SetDialog (HWND hWnd, const CFG_DEBUGPRM &prm)
{
	SendDlgItemMessage (hWnd, IDC_CHECK1, BM_SETCHECK, prm.bWireframeMode ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage (hWnd, IDC_CHECK2, BM_SETCHECK, prm.bNormaliseNormals ? BST_CHECKED : BST_UNCHECKED, 0);
}

bool ExtraRenderingOptions::StoreParams (HWND hWnd)
{
	Config *cfg = pTab->Cfg();
	cfg->CfgDebugPrm.bWireframeMode = (SendDlgItemMessage (hWnd, IDC_CHECK1, BM_GETCHECK, 0, 0) == BST_CHECKED);
	cfg->CfgDebugPrm.bNormaliseNormals = (SendDlgItemMessage (hWnd, IDC_CHECK2, BM_GETCHECK, 0, 0) == BST_CHECKED);
	return true;
}

BOOL CALLBACK ExtraRenderingOptions::DlgProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		((ExtraRenderingOptions*)lParam)->InitDialog (hWnd);
		break;
	case WM_COMMAND:
		switch (LOWORD (wParam)) {
		case IDC_BUTTON1:
			((ExtraRenderingOptions*)GetWindowLong (hWnd, DWL_USER))->ResetDialog (hWnd);
			return 0;
	//	case IDC_BUTTON2:
	//		((ExtraTimerSettings*)GetWindowLong (hWnd, DWL_USER))->OpenHelp (hWnd);
	//		return 0;
		case IDOK:
			if (((ExtraRenderingOptions*)GetWindowLong (hWnd, DWL_USER))->StoreParams (hWnd))
				EndDialog (hWnd, 0);
			break;
		}
		break;
	}
	return BuiltinLaunchpadItem::DlgProc (hWnd, uMsg, wParam, lParam);
}

//-----------------------------------------------------------------------------
// Debugging parameters: timer settings
//-----------------------------------------------------------------------------

char *ExtraTimerSettings::Name ()
{
	return "Timer settings";
}

char *ExtraTimerSettings::Description ()
{
	static char *desc = "This option allows the selection of the timer used by Orbiter to calculate time step intervals. Useful for testing and working around buggy hardware timers.";
	return desc;
}

bool ExtraTimerSettings::clbkOpen (HWND hParent)
{
	OpenDialog (hParent, IDD_EXTRA_TIMER, DlgProc);
	return true;
}

void ExtraTimerSettings::InitDialog (HWND hWnd)
{
	SetDialog (hWnd, pTab->Cfg()->CfgDebugPrm);
}

void ExtraTimerSettings::ResetDialog (HWND hWnd)
{
	extern CFG_DEBUGPRM CfgDebugPrm_default;
	SetDialog (hWnd, CfgDebugPrm_default);
}

void ExtraTimerSettings::SetDialog (HWND hWnd, const CFG_DEBUGPRM &prm)
{
	SendDlgItemMessage (hWnd, IDC_COMBO1, CB_RESETCONTENT, 0, 0);
	SendDlgItemMessage (hWnd, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)"Automatic selection");
	SendDlgItemMessage (hWnd, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)"High-performance hardware timer");
	SendDlgItemMessage (hWnd, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)"Software timer");
	SendDlgItemMessage (hWnd, IDC_COMBO1, CB_SETCURSEL, prm.TimerMode, 0);
}

bool ExtraTimerSettings::StoreParams (HWND hWnd)
{
	Config *cfg = pTab->Cfg();
	int idx = SendDlgItemMessage (hWnd, IDC_COMBO1, CB_GETCURSEL, 0, 0);
	if (idx == CB_ERR) idx = 0;
	cfg->CfgDebugPrm.TimerMode = idx;
	return true;
}

bool ExtraTimerSettings::OpenHelp (HWND hWnd)
{
	OpenDefaultHelp (hWnd, pTab->Launchpad()->GetInstance(), "extra_timer");
	return true;
}

BOOL CALLBACK ExtraTimerSettings::DlgProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		((ExtraTimerSettings*)lParam)->InitDialog (hWnd);
		break;
	case WM_COMMAND:
		switch (LOWORD (wParam)) {
		case IDC_BUTTON1:
			((ExtraTimerSettings*)GetWindowLong (hWnd, DWL_USER))->ResetDialog (hWnd);
			return 0;
		case IDC_BUTTON2:
			((ExtraTimerSettings*)GetWindowLong (hWnd, DWL_USER))->OpenHelp (hWnd);
			return 0;
		case IDOK:
			if (((ExtraTimerSettings*)GetWindowLong (hWnd, DWL_USER))->StoreParams (hWnd))
				EndDialog (hWnd, 0);
			break;
		}
		break;
	}
	return BuiltinLaunchpadItem::DlgProc (hWnd, uMsg, wParam, lParam);
}

//-----------------------------------------------------------------------------
// Debugging parameters: performance options
//-----------------------------------------------------------------------------

char *ExtraPerformanceSettings::Name ()
{
	return "Performance options";
}

char *ExtraPerformanceSettings::Description ()
{
	static char *desc = "This option can be used to modify Windows environment parameters that can improve the simulator performance.";
	return desc;
}

bool ExtraPerformanceSettings::clbkOpen (HWND hParent)
{
	OpenDialog (hParent, IDD_EXTRA_PERFORMANCE, DlgProc);
	return true;
}

void ExtraPerformanceSettings::InitDialog (HWND hWnd)
{
	SetDialog (hWnd, pTab->Cfg()->CfgDebugPrm);
}

void ExtraPerformanceSettings::ResetDialog (HWND hWnd)
{
	extern CFG_DEBUGPRM CfgDebugPrm_default;
	SetDialog (hWnd, CfgDebugPrm_default);
}

void ExtraPerformanceSettings::SetDialog (HWND hWnd, const CFG_DEBUGPRM &prm)
{
	SendDlgItemMessage (hWnd, IDC_CHECK1, BM_SETCHECK, prm.bDisableSmoothFont ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage (hWnd, IDC_CHECK2, BM_SETCHECK, prm.bForceReenableSmoothFont ? BST_CHECKED : BST_UNCHECKED, 0);
}

bool ExtraPerformanceSettings::StoreParams (HWND hWnd)
{
	Config *cfg = pTab->Cfg();
	cfg->CfgDebugPrm.bDisableSmoothFont = (SendDlgItemMessage (hWnd, IDC_CHECK1, BM_GETCHECK, 0, 0) == BST_CHECKED ? true : false);
	cfg->CfgDebugPrm.bForceReenableSmoothFont = (SendDlgItemMessage (hWnd, IDC_CHECK2, BM_GETCHECK, 0, 0) == BST_CHECKED ? true : false);
	if (cfg->CfgDebugPrm.bDisableSmoothFont)
		g_pOrbiter->ActivateRoughType();
	else
		g_pOrbiter->DeactivateRoughType();
	return true;
}

bool ExtraPerformanceSettings::OpenHelp (HWND hWnd)
{
	OpenDefaultHelp (hWnd, pTab->Launchpad()->GetInstance(), "extra_performance");
	return true;
}

BOOL CALLBACK ExtraPerformanceSettings::DlgProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		((ExtraPerformanceSettings*)lParam)->InitDialog (hWnd);
		break;
	case WM_COMMAND:
		switch (LOWORD (wParam)) {
		case IDC_BUTTON1:
			((ExtraPerformanceSettings*)GetWindowLong (hWnd, DWL_USER))->ResetDialog (hWnd);
			return 0;
		case IDC_BUTTON2:
			((ExtraPerformanceSettings*)GetWindowLong (hWnd, DWL_USER))->OpenHelp (hWnd);
			return 0;
		case IDOK:
			if (((ExtraPerformanceSettings*)GetWindowLong (hWnd, DWL_USER))->StoreParams (hWnd))
				EndDialog (hWnd, 0);
			break;
		}
		break;
	}
	return BuiltinLaunchpadItem::DlgProc (hWnd, uMsg, wParam, lParam);
}


//-----------------------------------------------------------------------------
// Debugging parameters: launchpad options
//-----------------------------------------------------------------------------

char *ExtraLaunchpadOptions::Name ()
{
	return "Launchpad options";
}

char *ExtraLaunchpadOptions::Description ()
{
	static char *desc = "Configure the behaviour of the Orbiter Launchpad dialog.";
	return desc;
}

bool ExtraLaunchpadOptions::clbkOpen (HWND hParent)
{
	OpenDialog (hParent, IDD_EXTRA_LAUNCHPAD, DlgProc);
	return true;
}

void ExtraLaunchpadOptions::InitDialog (HWND hWnd)
{
	SetDialog (hWnd, pTab->Cfg()->CfgDebugPrm);
}

void ExtraLaunchpadOptions::ResetDialog (HWND hWnd)
{
	extern CFG_DEBUGPRM CfgDebugPrm_default;
	SetDialog (hWnd, CfgDebugPrm_default);
}

void ExtraLaunchpadOptions::SetDialog (HWND hWnd, const CFG_DEBUGPRM &prm)
{
	int i;
	for (i = 0; i < 3; i++)
		SendDlgItemMessage (hWnd, IDC_RADIO1+i, BM_SETCHECK, prm.bHtmlScnDesc == i ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage (hWnd, IDC_CHECK1, BM_SETCHECK, prm.bSaveExitScreen ? BST_CHECKED : BST_UNCHECKED, 0);
}

bool ExtraLaunchpadOptions::StoreParams (HWND hWnd)
{
	int i;
	Config *cfg = pTab->Cfg();
	cfg->CfgDebugPrm.bSaveExitScreen = (SendDlgItemMessage (hWnd, IDC_CHECK1, BM_GETCHECK, 0, 0) == BST_CHECKED ? true : false);
	for (i = 0; i < 3; i++) {
		if (SendDlgItemMessage (hWnd, IDC_RADIO1+i, BM_GETCHECK, 0, 0) == BST_CHECKED) {
			break;
		}
	}
	if (i != cfg->CfgDebugPrm.bHtmlScnDesc) {
		cfg->CfgDebugPrm.bHtmlScnDesc = i;
		MessageBox (NULL, "You need to restart Orbiter for these changes to take effect.", "Orbiter settings", MB_OK | MB_ICONEXCLAMATION);
	}
	return true;
}

BOOL CALLBACK ExtraLaunchpadOptions::DlgProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		((ExtraLaunchpadOptions*)lParam)->InitDialog (hWnd);
		break;
	case WM_COMMAND:
		switch (LOWORD (wParam)) {
		case IDC_BUTTON1:
			((ExtraLaunchpadOptions*)GetWindowLong (hWnd, DWL_USER))->ResetDialog (hWnd);
			return 0;
		//case IDC_BUTTON2:
		//	((ExtraLaunchpadOptions*)GetWindowLong (hWnd, DWL_USER))->OpenHelp (hWnd);
		//	return 0;
		case IDOK:
			if (((ExtraLaunchpadOptions*)GetWindowLong (hWnd, DWL_USER))->StoreParams (hWnd))
				EndDialog (hWnd, 0);
			break;
		}
		break;
	}
	return BuiltinLaunchpadItem::DlgProc (hWnd, uMsg, wParam, lParam);
}


//-----------------------------------------------------------------------------
// Debugging parameters: logfile options
//-----------------------------------------------------------------------------

char *ExtraLogfileOptions::Name ()
{
	return "Logfile options";
}

char *ExtraLogfileOptions::Description ()
{
	static char *desc = "Configure options for log file output.";
	return desc;
}

bool ExtraLogfileOptions::clbkOpen (HWND hParent)
{
	OpenDialog (hParent, IDD_EXTRA_LOGFILE, DlgProc);
	return true;
}

void ExtraLogfileOptions::InitDialog (HWND hWnd)
{
	SetDialog (hWnd, pTab->Cfg()->CfgDebugPrm);
}

void ExtraLogfileOptions::ResetDialog (HWND hWnd)
{
	extern CFG_DEBUGPRM CfgDebugPrm_default;
	SetDialog (hWnd, CfgDebugPrm_default);
}

void ExtraLogfileOptions::SetDialog (HWND hWnd, const CFG_DEBUGPRM &prm)
{
	SendDlgItemMessage (hWnd, IDC_CHECK1, BM_SETCHECK, prm.bVerboseLog ? BST_CHECKED : BST_UNCHECKED, 0);
}

bool ExtraLogfileOptions::StoreParams (HWND hWnd)
{
	Config *cfg = pTab->Cfg();
	cfg->CfgDebugPrm.bVerboseLog = (SendDlgItemMessage (hWnd, IDC_CHECK1, BM_GETCHECK, 0, 0) == BST_CHECKED ? true : false);
	return true;
}

BOOL CALLBACK ExtraLogfileOptions::DlgProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		((ExtraLogfileOptions*)lParam)->InitDialog (hWnd);
		break;
	case WM_COMMAND:
		switch (LOWORD (wParam)) {
		case IDC_BUTTON1:
			((ExtraLogfileOptions*)GetWindowLong (hWnd, DWL_USER))->ResetDialog (hWnd);
			return 0;
		//case IDC_BUTTON2:
		//	((ExtraLogfileOptions*)GetWindowLong (hWnd, DWL_USER))->OpenHelp (hWnd);
		//	return 0;
		case IDOK:
			if (((ExtraLogfileOptions*)GetWindowLong (hWnd, DWL_USER))->StoreParams (hWnd))
				EndDialog (hWnd, 0);
			break;
		}
		break;
	}
	return BuiltinLaunchpadItem::DlgProc (hWnd, uMsg, wParam, lParam);
}

//-----------------------------------------------------------------------------
// class LaunchpadItem: addon-defined items for the "Extra" tab
// Interface in OrbiterAPI.h
//-----------------------------------------------------------------------------

LaunchpadItem::LaunchpadItem ()
{
	hItem = 0;
}

LaunchpadItem::~LaunchpadItem ()
{}

char *LaunchpadItem::Name ()
{
	return 0;
}

char *LaunchpadItem::Description ()
{
	return 0;
}

bool LaunchpadItem::OpenDialog (HINSTANCE hInst, HWND hLaunchpad, int resId, DLGPROC pDlg)
{
	DialogBoxParam (hInst, MAKEINTRESOURCE (resId), hLaunchpad, pDlg, (LPARAM)this);
	return true;
}

bool LaunchpadItem::clbkOpen (HWND hLaunchpad)
{
	return false;
}

int LaunchpadItem::clbkWriteConfig ()
{
	return 0;
}
