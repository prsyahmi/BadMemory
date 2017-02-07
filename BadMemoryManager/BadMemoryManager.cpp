/*
	Manage bad memory regions for the BadMemory driver
	Copyright (C) 2016  Syahmi Azhar

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "stdafx.h"
#include "BadMemoryManager.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

HWND ListBoxHwnd;
HWND LowerBoundHwnd;
HWND UpperBoundHwnd;
HWND AddBtnHwnd;
HWND DelBtnHwnd;
std::vector<BAD_REGION> BadRegions;
std::vector<BAD_REGION_STATUS> BadRegionStatus;


// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void OnCreate(HWND hWnd);
void RefreshListbox();
void SaveEntries();
void AddEntry();
void DelEntry();


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Place code here.
	INITCOMMONCONTROLSEX c;
	c.dwSize = sizeof(c);
	c.dwICC = 0;
	InitCommonControlsEx(&c);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_BADMEMORYMANAGER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_BADMEMORYMANAGER));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_BADMEMORYMANAGER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_BTNFACE + 1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_BADMEMORYMANAGER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle,
	   WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
	   CW_USEDEFAULT, 0, 640, 610, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
			int notId = HIWORD(wParam);

			if (notId == BN_CLICKED)
			{
				if ((HWND)lParam == AddBtnHwnd) {
					AddEntry();
				} else if ((HWND)lParam == DelBtnHwnd) {
					DelEntry();
				}
			}
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
	case WM_CREATE:
		OnCreate(hWnd);
		break;
    case WM_DESTROY:
		//SaveEntries();
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void OnCreate(HWND hWnd)
{
	HWND hTempWnd;
	HDC hDC = GetDC(hWnd);
	int nFontHeight = -MulDiv(10, GetDeviceCaps(hDC, LOGPIXELSY), 72);
	ReleaseDC(hWnd, hDC);

	HFONT hFont = CreateFont(nFontHeight, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
		CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Trebuchet MS"));

	hTempWnd = CreateWindowEx(0, WC_STATIC, L"Address:", WS_VISIBLE | WS_CHILD | SS_RIGHT, 10, 10, 90, 26, hWnd, NULL, NULL, 0);
	SendMessage(hTempWnd, WM_SETFONT, (LPARAM)hFont, 0);

	hTempWnd = CreateWindowEx(0, WC_STATIC, L"-", WS_VISIBLE | WS_CHILD | SS_CENTER, 310, 10, 20, 26, hWnd, NULL, NULL, 0);
	SendMessage(hTempWnd, WM_SETFONT, (LPARAM)hFont, 0);

	hTempWnd = CreateWindowEx(0, WC_STATIC, L"Bad Regions:", WS_VISIBLE | WS_CHILD | SS_RIGHT, 10, 40, 90, 26, hWnd, NULL, NULL, 0);
	SendMessage(hTempWnd, WM_SETFONT, (LPARAM)hFont, 0);

	LowerBoundHwnd = CreateWindowEx(0, WC_EDIT, L"", WS_VISIBLE | WS_CHILD | WS_BORDER, 110, 10, 200, 26, hWnd, NULL, NULL, 0);
	SendMessage(LowerBoundHwnd, WM_SETFONT, (LPARAM)hFont, 0);

	UpperBoundHwnd = CreateWindowEx(0, WC_EDIT, L"", WS_VISIBLE | WS_CHILD | WS_BORDER, 330, 10, 200, 26, hWnd, NULL, NULL, 0);
	SendMessage(UpperBoundHwnd, WM_SETFONT, (LPARAM)hFont, 0);

	ListBoxHwnd = CreateWindowEx(0, WC_LISTBOX, L"", WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | LBS_NOTIFY | LBS_HASSTRINGS, 110, 40, 420, 500, hWnd, NULL, NULL, 0);
	SendMessage(ListBoxHwnd, WM_SETFONT, (LPARAM)hFont, 0);

	AddBtnHwnd = CreateWindowEx(0, WC_BUTTON, L"Add", WS_VISIBLE | WS_CHILD, 540, 10, 70, 26, hWnd, NULL, NULL, 0);
	SendMessage(AddBtnHwnd, WM_SETFONT, (LPARAM)hFont, 0);

	DelBtnHwnd = CreateWindowEx(0, WC_BUTTON, L"Del", WS_VISIBLE | WS_CHILD, 540, 40, 70, 26, hWnd, NULL, NULL, 0);
	SendMessage(DelBtnHwnd, WM_SETFONT, (LPARAM)hFont, 0);

	RefreshListbox();
}

void LoadEntries()
{
	HKEY hKey;
	DWORD dwSize;
	unsigned char* pBuffer;
	
	if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\BadMemory\\Parameters", 0, KEY_READ, &hKey)) {
		return;
	}

	if (ERROR_SUCCESS != RegQueryValueEx(hKey, L"BadRegions", NULL, NULL, NULL, &dwSize)) {
		RegCloseKey(hKey);
		return;
	}

	pBuffer = new unsigned char[dwSize];
	BadRegions.clear();
	while (SendMessage(ListBoxHwnd, LB_DELETESTRING, 0, 0) > 0);

	if (ERROR_SUCCESS == RegQueryValueEx(hKey, L"BadRegions", NULL, NULL, pBuffer, &dwSize)) {
		int nTotal = dwSize / sizeof(BAD_REGION);
		PBAD_REGION pBadRegions = (PBAD_REGION)pBuffer;

		for (int i = 0; i < nTotal; i++)
		{
			BadRegions.push_back(pBadRegions[i]);
		}
	}

	if (ERROR_SUCCESS != RegQueryValueEx(hKey, L"BadRegionStatus", NULL, NULL, NULL, &dwSize)) {
		RegCloseKey(hKey);
		return;
	}

	pBuffer = new unsigned char[dwSize];
	BadRegionStatus.clear();

	if (ERROR_SUCCESS == RegQueryValueEx(hKey, L"BadRegionStatus", NULL, NULL, pBuffer, &dwSize)) {
		int nTotal = dwSize / sizeof(BAD_REGION);
		PBAD_REGION_STATUS pBadRegionStatus = (PBAD_REGION_STATUS)pBuffer;

		for (int i = 0; i < nTotal; i++)
		{
			BadRegionStatus.push_back(pBadRegionStatus[i]);
		}
	}

	RegCloseKey(hKey);
	delete[] pBuffer;
}

void SaveEntries()
{
	HKEY hKey;
	unsigned int nBuffer = sizeof(BAD_REGION) * (unsigned int)BadRegions.size();
	unsigned char* pBuffer;

	if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\BadMemory\\Parameters", 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hKey, NULL)) {
		MessageBoxA(0, "Unable to create registry entry. Make sure to run this app with admin privilege.", "Failed to create registry", MB_OK);
		return;
	}

	pBuffer = new unsigned char[nBuffer];
	PBAD_REGION pBadRegion = (PBAD_REGION)pBuffer;
	for (auto it = BadRegions.begin(); it != BadRegions.end(); it++)
	{
		*pBadRegion++ = (*it);
	}

	if (ERROR_SUCCESS != RegSetValueEx(hKey, L"BadRegions", 0, REG_BINARY, pBuffer, nBuffer)) {
		MessageBoxA(0, "Unable to save region data. Make sure to run this app with admin privilege.", "Failed to save data", MB_OK);
	}

	delete[] pBuffer;
	RegCloseKey(hKey);
}

void RefreshListbox()
{
	LoadEntries();

	for (auto it = BadRegions.begin(); it != BadRegions.end(); it++)
	{
		char* status = "Restart";
		char szAddr[64];

		for (auto itStatus = BadRegionStatus.begin(); itStatus != BadRegionStatus.end(); itStatus++)
		{
			if (it->LowerBound == itStatus->LowerBound && it->UpperBound == itStatus->UpperBound)
			{
				if (itStatus->Status) {
					status = "OK";
				} else {
					status = "FAIL";
				}

				break;
			}
		}

		sprintf_s(szAddr, "%llx - %llx [%s]", it->LowerBound, it->UpperBound, status);
		SendMessageA(ListBoxHwnd, LB_ADDSTRING, 0, (LPARAM)&szAddr);
	}
}

void AddEntry()
{
	char szLower[32];
	char szUpper[32];
	char* p;
	BAD_REGION region;

	GetWindowTextA(LowerBoundHwnd, szLower, 32);
	GetWindowTextA(UpperBoundHwnd, szUpper, 32);

	region.LowerBound = _strtoui64(szLower, &p, 16);
	region.UpperBound = _strtoui64(szUpper, &p, 16);

	if (region.LowerBound == 0 && region.UpperBound == 0) {
		MessageBoxA(0, "Please specify correct address for both lower address and upper address", "Invalid input", MB_OK);
		return;
	} else if (region.LowerBound >= region.UpperBound) {
		MessageBoxA(0, "Incorrect range", "Invalid input", MB_OK);
		return;
	}

	SetWindowTextA(LowerBoundHwnd, "");
	SetWindowTextA(UpperBoundHwnd, "");

	BadRegions.push_back(region);

	SaveEntries();
	RefreshListbox();
}

void DelEntry()
{
	LRESULT nSel = SendMessage(ListBoxHwnd, LB_GETCURSEL, 0, 0);
	if (nSel == LB_ERR) return;

	BadRegions.erase(BadRegions.begin() + nSel);

	SaveEntries();
	RefreshListbox();
}