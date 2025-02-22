// Copyright (c) Martin Schweiger
// Licensed under the MIT License

// ======================================================================
// Help window
// ======================================================================

#ifndef __DLGHELP_H
#define __DLGHELP_H

#include "DialogWin.h"

// ======================================================================

class DlgHelp: public DialogWin {
public:
	DlgHelp (HINSTANCE hInstance, HWND hParent, void *context);
	~DlgHelp ();
	BOOL OnInitdialog (HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL OnRequest (HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL OnSize (HWND hWnd, WPARAM wParam, int w, int h);
	static void InitHelp (HWND hWnd, HELPCONTEXT *hcontext = 0);
	static void SetScenarioHelp (const char *_helpf);
	static void SetVesselHelp (const char *_helpf);
	static LONG FAR PASCAL ClientProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

protected:
	void RegisterClientClass (HINSTANCE hInstance);
	void UnregisterClientClass (HINSTANCE hInstance);
	static BOOL CALLBACK DlgProc (HWND, UINT, WPARAM, LPARAM);

private:
	int hfooter;
	int dlgw, dlgh;
	POINT pClose;
};

#endif // !__DLGHELP_H