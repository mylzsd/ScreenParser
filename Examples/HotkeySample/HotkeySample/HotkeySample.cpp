// HotkeySample.cpp : Defines the entry point for the application.
//

#include "stdafx.h"		// this is Windows developemnt library
#include "HotkeySample.h"

#define MAX_LOADSTRING 100
#define	TEXTFIELD	6666

// Mode identifier
#define FREE_MODE 1
#define SELECT_MODE 2
#define RECORD_MODE 3
#define COMMAND_MODE 4

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

// Text window for message output
HWND tField;

// This is the entry point of Windows Desktop Application
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
  _In_opt_ HINSTANCE hPrevInstance,
  _In_ LPWSTR    lpCmdLine,
  _In_ int       nCmdShow) {
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  // TODO: Place code here.

  // Initialize global strings
  LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
  LoadStringW(hInstance, IDC_HOTKEYSAMPLE, szWindowClass, MAX_LOADSTRING);
  MyRegisterClass(hInstance);

  // Perform application initialization:
  if (!InitInstance(hInstance, nCmdShow)) {
    return FALSE;
  }

  HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_HOTKEYSAMPLE));

  MSG msg;

  // Main message loop:
  while (GetMessage(&msg, nullptr, 0, 0)) {
    if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  return (int)msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance) {
  WNDCLASSEXW wcex;

  wcex.cbSize = sizeof(WNDCLASSEX);

  wcex.style = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc = WndProc;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = 0;
  wcex.hInstance = hInstance;
  wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_HOTKEYSAMPLE));
  wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_HOTKEYSAMPLE);
  wcex.lpszClassName = szWindowClass;
  wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
  hInst = hInstance; // Store instance handle in our global variable

  HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

  if (!hWnd) {
    return FALSE;
  }

  // create a new text window
  tField = CreateWindowW(L"static", 0, WS_CHILD | WS_VISIBLE, 70, 70, 280, 20, hWnd, (HMENU)TEXTFIELD, hInstance, 0);

  // register hotkey to text window
  if (!RegisterHotKey(hWnd, FREE_MODE, MOD_CONTROL | MOD_SHIFT, 0x46)) {
    OutputDebugString(L"Failed to register Free Mode");
    return FALSE;
  }
  if (!RegisterHotKey(hWnd, SELECT_MODE, MOD_CONTROL | MOD_SHIFT, 0x53)) {
    OutputDebugString(L"Failed to register Select Mode");
    return FALSE;
  }
  if (!RegisterHotKey(hWnd, RECORD_MODE, MOD_CONTROL | MOD_SHIFT, 0x52)) {
    OutputDebugString(L"Failed to register Record Mode");
    return FALSE;
  }
  if (!RegisterHotKey(hWnd, COMMAND_MODE, MOD_CONTROL | MOD_SHIFT, 0x43)) {
    OutputDebugString(L"Failed to register Command Mode");
    return FALSE;
  }
  SetWindowText(tField, L"All hotkeys are registered");

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
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message) {
    // Handle message from hotkey
  case WM_HOTKEY:
  {
    // Construct a formatted string, convert into LPCWSTR, and show in output
    // TODO: Looking for an easier way
    char outstr[64];
    sprintf_s(outstr, "wParam: %d, lParam: %d\n", (int)wParam, (int)lParam);
    wchar_t wtext[64];
    mbstowcs_s(NULL, wtext, 64, outstr, strlen(outstr) + 1);
    OutputDebugString(wtext);

    int hkId = LOWORD(wParam);
    // Parse hotkey selections:
    switch (hkId) {
    case FREE_MODE:
      SetWindowText(tField, L"Free Mode");
      break;
    case SELECT_MODE:
      SetWindowText(tField, L"Select Mode");
      break;
    case RECORD_MODE:
      SetWindowText(tField, L"Record Mode");
      break;
    case COMMAND_MODE:
      SetWindowText(tField, L"Command Mode");
      break;
    default:
      return DefWindowProc(hWnd, message, wParam, lParam);
    }
  }
  break;
  case WM_COMMAND:
  {
    int wmId = LOWORD(wParam);
    // Parse the menu selections:
    switch (wmId) {
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
  case WM_PAINT:
  {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);
    // TODO: Add any drawing code that uses hdc here...
    EndPaint(hWnd, &ps);
  }
  break;
  case WM_DESTROY:
    PostQuitMessage(0);
    break;
  default:
    return DefWindowProc(hWnd, message, wParam, lParam);
  }
  return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
  UNREFERENCED_PARAMETER(lParam);
  switch (message) {
  case WM_INITDIALOG:
    return (INT_PTR)TRUE;

  case WM_COMMAND:
    if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
      EndDialog(hDlg, LOWORD(wParam));
      return (INT_PTR)TRUE;
    }
    break;
  }
  return (INT_PTR)FALSE;
}
