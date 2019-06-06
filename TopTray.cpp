// ****************************************************************************
//
// TopTray
// Copyright (C) 1998-2010  Nikolay Redko, J.D. Purcell
// Copyright (C) 2015 Benbuck Nason
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// ****************************************************************************

#include <windows.h>
#include "TopTray.h"
#include "resource.h"

#define MAXTRAYITEMS 64

//#include<fstream>
//#include<iomanip>
//using namespace std;


static UINT WM_TASKBAR_CREATED;

static HINSTANCE _hInstance;
static HMODULE _hLib;
static HWND _hwndHook;
static HWND _hwndItems[MAXTRAYITEMS];
static HWND _hwndForMenu;


//int LogInFile(char* log) {
//	ofstream ofile;               //��������ļ�
//	ofile.open("e:\\tray_log.txt", ios::app);     //��Ϊ����ļ���
//	ofile << "**************** log begin ****************" << endl;   //����д���ļ�
//	ofile << log << endl;
//	ofile.close();
//	return 0;
//}

int FindInTray(HWND hwnd) {
    for (int i = 0; i < MAXTRAYITEMS; i++) {
        if (_hwndItems[i] == hwnd) return i;
    }
    return -1;
}

HICON GetWindowIcon(HWND hwnd) {
    HICON icon;
    if (icon = (HICON)SendMessage(hwnd, WM_GETICON, ICON_SMALL, 0)) return icon;
    if (icon = (HICON)SendMessage(hwnd, WM_GETICON, ICON_BIG, 0)) return icon;
    if (icon = (HICON)GetClassLongPtr(hwnd, GCLP_HICONSM)) return icon;
    if (icon = (HICON)GetClassLongPtr(hwnd, GCLP_HICON)) return icon;
    return LoadIcon(NULL, IDI_WINLOGO);
}

static bool AddToTray(int i) {
    NOTIFYICONDATA nid;
    ZeroMemory(&nid, sizeof(nid));
    nid.cbSize           = NOTIFYICONDATA_V2_SIZE;
    nid.hWnd             = _hwndHook;
    nid.uID              = (UINT)i;
    nid.uFlags           = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYCMD;
    nid.hIcon            = GetWindowIcon(_hwndItems[i]);
    GetWindowText(_hwndItems[i], nid.szTip, sizeof(nid.szTip) / sizeof(nid.szTip[0]));
    nid.uVersion         = NOTIFYICON_VERSION;
    if (!SUCCEEDED(Shell_NotifyIcon(NIM_ADD, &nid))) {
        return false;
    }
    if (!SUCCEEDED(Shell_NotifyIcon(NIM_SETVERSION, &nid))) {
        Shell_NotifyIcon(NIM_DELETE, &nid);
        return false;
    }
    return true;
}

static bool AddWindowToTray(HWND hwnd) {
    int i = FindInTray(NULL);
    if (i == -1) return false;
    _hwndItems[i] = hwnd;
    return AddToTray(i);
}

static void MinimizeWindowToTray(HWND hwnd) {
    // Don't minimize MDI child windows
    if ((UINT)GetWindowLongPtr(hwnd, GWL_EXSTYLE) & WS_EX_MDICHILD) return;

    // If hwnd is a child window, find parent window (e.g. minimize button in
    // Office 2007 (ribbon interface) is in a child window)
    if ((UINT)GetWindowLongPtr(hwnd, GWL_STYLE) & WS_CHILD) {
        hwnd = GetAncestor(hwnd, GA_ROOT);
    }

    // Add icon to tray if it's not already there
    if (FindInTray(hwnd) == -1) {
        if (!AddWindowToTray(hwnd)) {
            return;
        }
    }

    // Hide window
    ShowWindow(hwnd, SW_HIDE);
}

static void TransparentAndTopWindow(HWND hwnd) {
	// Don't minimize MDI child windows
	if ((UINT)GetWindowLongPtr(hwnd, GWL_EXSTYLE) & WS_EX_MDICHILD) return;

	// If hwnd is a child window, find parent window (e.g. minimize button in
	// Office 2007 (ribbon interface) is in a child window)
	if ((UINT)GetWindowLongPtr(hwnd, GWL_STYLE) & WS_CHILD) {
		hwnd = GetAncestor(hwnd, GA_ROOT);
	}

	// Add transparent attribute
	LONG nRet = GetWindowLong(hwnd, GWL_EXSTYLE);
	nRet = nRet | WS_EX_LAYERED; 
	SetWindowLong(hwnd, GWL_EXSTYLE, nRet);

	// If window already topmost, no topmost it, and set it's transparency to 255(no transparency)
	// else topmost it, and to show some differentiate between topmost window and normal window, 
	// set it's transparency to 225(little transparency)
	if (GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST){
		SetLayeredWindowAttributes(hwnd, 0, 255, 0x2);
		SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	} else{
		SetLayeredWindowAttributes(hwnd, 0, 225, 0x2);
		SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}

}

static bool RemoveFromTray(int i) {
    NOTIFYICONDATA nid;
    ZeroMemory(&nid, sizeof(nid));
    nid.cbSize = NOTIFYICONDATA_V2_SIZE;
    nid.hWnd   = _hwndHook;
    nid.uID    = (UINT)i;
    if (!SUCCEEDED(Shell_NotifyIcon(NIM_DELETE, &nid))) {
        return false;
    }
    return true;
}

static bool RemoveWindowFromTray(HWND hwnd) {
    int i = FindInTray(hwnd);
    if (i == -1) return false;
    if (!RemoveFromTray(i)) {
        return false;
    }
    _hwndItems[i] = NULL;
    return true;
}

static void RestoreWindowFromTray(HWND hwnd) {
    ShowWindow(hwnd, SW_SHOW);
    SetForegroundWindow(hwnd);
    RemoveWindowFromTray(hwnd);
}

static void CloseWindowFromTray(HWND hwnd) {
    // Use PostMessage to avoid blocking if the program brings up a dialog on exit.
    // Also, Explorer windows ignore WM_CLOSE messages from SendMessage.
    PostMessage(hwnd, WM_CLOSE, 0, 0);

    Sleep(50);
    if (IsWindow(hwnd)) Sleep(50);

    if (!IsWindow(hwnd)) {
        // Closed successfully
        RemoveWindowFromTray(hwnd);
    }
}

void RefreshWindowInTray(HWND hwnd) {
    int i = FindInTray(hwnd);
    if (i == -1) return;
    if (!IsWindow(hwnd) || IsWindowVisible(hwnd)) {
        RemoveWindowFromTray(hwnd);
    }
    else {
        NOTIFYICONDATA nid;
        ZeroMemory(&nid, sizeof(nid));
        nid.cbSize = NOTIFYICONDATA_V2_SIZE;
        nid.hWnd   = _hwndHook;
        nid.uID    = (UINT)i;
        nid.uFlags = NIF_TIP;
        GetWindowText(hwnd, nid.szTip, sizeof(nid.szTip) / sizeof(nid.szTip[0]));
        Shell_NotifyIcon(NIM_MODIFY, &nid);
    }
}

void ExecuteMenu() {
    HMENU hMenu;
    POINT point;

    hMenu = CreatePopupMenu();
    if (!hMenu) {
        MessageBox(NULL, L"Error creating menu.", L"TopTray", MB_OK | MB_ICONERROR);
        return;
    }
    AppendMenu(hMenu, MF_STRING, IDM_ABOUT,   L"About TopTray");
    AppendMenu(hMenu, MF_STRING, IDM_EXIT,    L"Exit TopTray");
    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL); //--------------
    AppendMenu(hMenu, MF_STRING, IDM_CLOSE,   L"Close Window");
    AppendMenu(hMenu, MF_STRING, IDM_RESTORE, L"Restore Window");

    GetCursorPos(&point);
    SetForegroundWindow(_hwndHook);

    TrackPopupMenu(hMenu, TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_RIGHTALIGN | TPM_BOTTOMALIGN, point.x, point.y, 0, _hwndHook, NULL);

    PostMessage(_hwndHook, WM_USER, 0, 0);
    DestroyMenu(hMenu);
}

BOOL CALLBACK AboutDlgProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    switch (Msg) {
        case WM_CLOSE:
            PostMessage(hWnd, WM_COMMAND, IDCANCEL, 0);
            break;
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDOK:
                    EndDialog(hWnd, TRUE);
                    break;
                case IDCANCEL:
                    EndDialog(hWnd, FALSE);
                    break;
            }
            break;
        default:
            return FALSE;
    }
    return TRUE;
}

LRESULT CALLBACK HookWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDM_RESTORE:
                    RestoreWindowFromTray(_hwndForMenu);
                    break;
                case IDM_CLOSE:
                    CloseWindowFromTray(_hwndForMenu);
                    break;
                case IDM_ABOUT:
                    DialogBox(_hInstance, MAKEINTRESOURCE(IDD_ABOUT), _hwndHook, (DLGPROC)AboutDlgProc);
                    break;
                case IDM_EXIT:
                    SendMessage(_hwndHook, WM_DESTROY, 0, 0);
                    break;
            }
            break;
        case WM_ADDTRAY:
            MinimizeWindowToTray((HWND)lParam);
            break;
        case WM_REMTRAY:
            RestoreWindowFromTray((HWND)lParam);
            break;
        case WM_REFRTRAY:
            RefreshWindowInTray((HWND)lParam);
            break;
        case WM_TRAYCMD:
            switch ((UINT)lParam) {
                case NIN_SELECT:
                    RestoreWindowFromTray(_hwndItems[wParam]);
                    break;
                case WM_CONTEXTMENU:
                    _hwndForMenu = _hwndItems[wParam];
                    ExecuteMenu();
                    break;
                case WM_MOUSEMOVE:
                    RefreshWindowInTray(_hwndItems[wParam]);
                    break;
            }
            break;
		case WM_TOP:
			TransparentAndTopWindow((HWND)lParam);
			break;
        case WM_HOTKEY:
        {
            HWND fgWnd = GetForegroundWindow();
            if (!fgWnd)
                break;

            LONG style = GetWindowLong(fgWnd, GWL_STYLE);
            if (!(style & WS_MINIMIZEBOX)) {
                // skip, no minimize box
                break;
            }

            MinimizeWindowToTray(fgWnd);

            break;
        }
        case WM_DESTROY:
            for (int i = 0; i < MAXTRAYITEMS; i++) {
                if (_hwndItems[i]) {
                    RestoreWindowFromTray(_hwndItems[i]);
                }
            }
            UnRegisterHook();
            FreeLibrary(_hLib);
            PostQuitMessage(0);
            break;
        default:
            if (msg == WM_TASKBAR_CREATED) {
                for (int i = 0; i < MAXTRAYITEMS; i++) {
                    if (_hwndItems[i]) {
                        AddToTray(i);
                    }
                }
            }
            break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR szCmdLine, int iCmdShow) {
    WNDCLASS wc;
    MSG msg;
    _hInstance = hInstance;
    _hwndHook = FindWindow(NAME, NAME);
    if (_hwndHook) {
        if (strstr(szCmdLine, "--exit")) {
            SendMessage(_hwndHook, WM_CLOSE, 0, 0);
        }
        else {
            MessageBox(NULL, L"TopTray is already running.", L"TopTray", MB_OK | MB_ICONINFORMATION);
        }
        return 0;
    }
    if (!(_hLib = LoadLibrary(L"TopHook.dll"))) {
        MessageBox(NULL, L"Error loading TopHook.dll.", L"TopTray", MB_OK | MB_ICONERROR);
        return 0;
    }
    if (!RegisterHook(_hLib)) {
        MessageBox(NULL, L"Error setting hook procedure.", L"TopTray", MB_OK | MB_ICONERROR);
        return 0;
    }
    wc.style         = 0;
    wc.lpfnWndProc   = HookWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = NULL;
    wc.hCursor       = NULL;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = NAME;
    if (!RegisterClass(&wc)) {
        MessageBox(NULL, L"Error creating window class", L"TopTray", MB_OK | MB_ICONERROR);
        return 0;
    }
    if (!(_hwndHook = CreateWindow(NAME, NAME, WS_OVERLAPPED, 0, 0, 0, 0, (HWND)NULL, (HMENU)NULL, (HINSTANCE)hInstance, (LPVOID)NULL))) {
        MessageBox(NULL, L"Error creating window", L"TopTray", MB_OK | MB_ICONERROR);
        return 0;
    }
    for (int i = 0; i < MAXTRAYITEMS; i++) {
        _hwndItems[i] = NULL;
    }
    WM_TASKBAR_CREATED = RegisterWindowMessage(L"TaskbarCreated");

    //BOOL registeredHotKey = RegisterHotKey(_hwndHook, 0, MOD_WIN | MOD_ALT, VK_DOWN);
    //if (!registeredHotKey)
        //MessageBox(NULL, L"Couldn't register hotkey", L"TopTray", MB_OK | MB_ICONERROR);

    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    //if (registeredHotKey)
    //UnregisterHotKey(_hwndHook, 0);

    return (int)msg.wParam;
}