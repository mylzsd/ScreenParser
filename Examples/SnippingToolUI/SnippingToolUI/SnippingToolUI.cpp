// SnippingToolUI.cpp : Freeze the screen and allow user to select rectangles.

#include "stdafx.h"
#include "SnippingToolUI.h"
#include <windows.h>
#include <objidl.h>
#include <vector>

// Honkwan: GDI+ includes
#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

                                                // Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
  _In_opt_ HINSTANCE hPrevInstance,
  _In_ LPWSTR    lpCmdLine,
  _In_ int       nCmdShow) {
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  // Initialize global strings
  LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
  LoadStringW(hInstance, IDC_SNIPPINGTOOLUI, szWindowClass, MAX_LOADSTRING);
  MyRegisterClass(hInstance);

  // Perform application initialization:
  if (!InitInstance(hInstance, nCmdShow)) {
    return FALSE;
  }

  HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SNIPPINGTOOLUI));

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
  wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SNIPPINGTOOLUI));
  wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_SNIPPINGTOOLUI);
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

Bitmap* screenshot_bitmap = nullptr;

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {


  hInst = hInstance; // Store instance handle in our global variable

  HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

  // Honkwan: GDI+ inits
  GdiplusStartupInput gdiplusStartupInput;
  ULONG_PTR           gdiplusToken;
  GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

  // Honkwan: Copy current screenshot to hBitmap
  HDC     hScreen = GetDC(NULL);
  HDC     hDC = CreateCompatibleDC(hScreen);
  HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, 1920, 1080);
  HGDIOBJ old_obj = SelectObject(hDC, hBitmap);
  BOOL    bRet = BitBlt(hDC, 0, 0, 1920, 1080, hScreen, 0, 0, SRCCOPY);
  // Convert to Bitmap
  auto pBmpOdElem = new Bitmap(0, 0, PixelFormatDontCare);
  screenshot_bitmap = pBmpOdElem->FromHBITMAP(hBitmap, 0);
  SelectObject(hDC, old_obj);

  // Honkwan: Fullscreen boarderless window.
  int w = GetSystemMetrics(SM_CXSCREEN);
  int h = GetSystemMetrics(SM_CYSCREEN);
  SetWindowLongPtr(hWnd, GWL_STYLE, WS_VISIBLE | WS_POPUP);

  SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
  SetLayeredWindowAttributes(hWnd, 0, 0xFF / 3, 0x02);

  SetWindowPos(hWnd, HWND_TOP, 0, 0, w, h, SWP_FRAMECHANGED);

  if (!hWnd) {
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
struct Field {
  POINT point_a;
  POINT point_b;
};

std::vector<Field> fields;

POINT point;
POINT curpoint;
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
  // Honkwan: Mouse manipulation.
  switch (message) {
  case WM_LBUTTONDOWN:
    GetCursorPos(&point);
    break;
  case WM_LBUTTONUP:
    fields.push_back(Field{ point, curpoint });
    break;
  case WM_MOUSEMOVE:
    GetCursorPos(&curpoint);
    if (wParam == MK_LBUTTON) {
      RedrawWindow(hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE);
    }
    break;
    // Honkwan: RedrawWindow will first clear window to background color then redraw,
    // this will ignore the clear step as by default it cause a flicky behaviour.
  case WM_ERASEBKGND:
    return (LRESULT)1;
  case WM_PAINT:
  {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);
    // TODO: Add any drawing code that uses hdc here...

    // Honkwan: Double buffering: Instead of modifying the current frame:
    // 1. Create a hidden frame.
    // 2. Add all modifications to the hidden frame.
    // 3. Publish the hidden frame.
    RECT rc;
    GetClientRect(hWnd, &rc);
    HDC memdc = CreateCompatibleDC(hdc);
    HBITMAP hbitmap = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
    HGDIOBJ oldbmp = SelectObject(memdc, hbitmap);
    // FillRect(memdc, &rc, WHITE_BRUSH);
    // Draw on hidden frame memdc
    Gdiplus::Graphics gr(memdc);
    // gr.DrawImage(screenshot_bitmap, 0, 0);
    SolidBrush mist_brush(Color(120, 255, 255, 255));
    Pen blackPen(Color(255, 255, 0, 0), 1);
    auto top_y = min(curpoint.y, point.y);
    auto bot_y = max(curpoint.y, point.y);
    auto left_x = min(curpoint.x, point.x);
    auto right_x = max(curpoint.x, point.x);
    // Draw everything
    gr.FillRectangle(&mist_brush, left_x, top_y, right_x - left_x, bot_y - top_y);
    gr.DrawRectangle(&blackPen, left_x, top_y, right_x - left_x, bot_y - top_y);
    for (auto field : fields) {
      top_y = min(field.point_a.y, field.point_b.y);
      bot_y = max(field.point_a.y, field.point_b.y);
      left_x = min(field.point_a.x, field.point_b.x);
      right_x = max(field.point_a.x, field.point_b.x);
      gr.FillRectangle(&mist_brush, left_x, top_y, right_x - left_x, bot_y - top_y);
      gr.DrawRectangle(&blackPen, left_x, top_y, right_x - left_x, bot_y - top_y);
    }
    //gr.FillRectangle(&mist_brush, 0, 0, 1920, top_y);
    //gr.FillRectangle(&mist_brush, 0, bot_y, 1920, 1080);
    //gr.FillRectangle(&mist_brush, 0, top_y, left_x, bot_y - top_y);
    //gr.FillRectangle(&mist_brush, right_x, top_y, 1920, bot_y - top_y);
    // Publish hidden frame memdc.
    BitBlt(hdc, 0, 0, rc.right, rc.bottom, memdc, 0, 0, SRCCOPY);
    // Clean up
    SelectObject(memdc, oldbmp);
    DeleteObject(hbitmap);
    DeleteDC(memdc);
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
