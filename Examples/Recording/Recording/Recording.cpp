// Recording.cpp : Record user mouse/keyboard action as soon as the program starts,
//                 replay those actions 2 seconds after the program exists.

#include "stdafx.h"
#include "Recording.h"
#include <vector>
// Honkwan: For time/sleep manipulation.
#include <chrono>
#include <thread>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


using namespace std::chrono;

// Honkwan: datastructures for recording events.
enum ActionType {
  MOUSE,
  KEYBOARD,
  SLEEP,
};

struct Action {
  enum ActionType type;
  WPARAM wparam;
  POINT mouse_point;
  DWORD l_param;
  milliseconds milli_s;
};

// All stored actions.
std::vector<Action> actions;

HHOOK hMouseHook;
HHOOK hKeyboardHook;
milliseconds ms_last;

// Push a sleep action to actions according to last time this function is called.
void PushbackSleepAction() {
  // Ingore delay for the first action.
  if (!actions.empty()) {
    milliseconds ms_now = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    actions.push_back(Action{ SLEEP,  0, POINT(), 0, ms_now - ms_last });
    ms_last = ms_now;
  }
  else {
    ms_last = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
  }
}

void SetMousePosition(POINT& mp) {
  long fScreenWidth = GetSystemMetrics(SM_CXSCREEN) - 1;
  long fScreenHeight = GetSystemMetrics(SM_CYSCREEN) - 1;

  // http://msdn.microsoft.com/en-us/library/ms646260(VS.85).aspx
  // If MOUSEEVENTF_ABSOLUTE value is specified, dx and dy contain normalized absolute coordinates between 0 and 65,535.
  // The event procedure maps these coordinates onto the display surface.
  // Coordinate (0,0) maps onto the upper-left corner of the display surface, (65535,65535) maps onto the lower-right corner.
  float fx = mp.x * (65535.0f / fScreenWidth);
  float fy = mp.y * (65535.0f / fScreenHeight);

  INPUT Input = { 0 };
  Input.type = INPUT_MOUSE;

  Input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;

  Input.mi.dx = (long)fx;
  Input.mi.dy = (long)fy;

  SendInput(1, &Input, sizeof(INPUT));
}

LRESULT CALLBACK mouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
  MOUSEHOOKSTRUCT * pMouseStruct = (MOUSEHOOKSTRUCT *)lParam;
  if (pMouseStruct != NULL) {
    // Ignoring mouse movements.
    if (wParam == WM_LBUTTONDOWN || wParam == WM_LBUTTONUP || wParam == WM_RBUTTONDOWN || wParam == WM_RBUTTONUP) {
      PushbackSleepAction();
      actions.push_back(Action{ MOUSE,  wParam, pMouseStruct->pt, 0 });
    }
  }
  return CallNextHookEx(hMouseHook, nCode, wParam, lParam);
}

LRESULT CALLBACK keyboardProc(int code, WPARAM wParam, LPARAM lParam) {
  KBDLLHOOKSTRUCT * hooked_key = ((KBDLLHOOKSTRUCT*)lParam);
  if (hooked_key != NULL) {
    PushbackSleepAction();
    actions.push_back(Action{ KEYBOARD,  wParam, POINT(), hooked_key->vkCode });
  }
  return CallNextHookEx(hKeyboardHook, code, wParam, lParam);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
  _In_opt_ HINSTANCE hPrevInstance,
  _In_ LPWSTR    lpCmdLine,
  _In_ int       nCmdShow) {
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  // Initialize global strings
  LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
  LoadStringW(hInstance, IDC_RECORDING, szWindowClass, MAX_LOADSTRING);
  MyRegisterClass(hInstance);

  // Perform application initialization:
  if (!InitInstance(hInstance, nCmdShow)) {
    return FALSE;
  }

  HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_RECORDING));

  MSG msg;

  // Honkwan: Register mouse and keyboard hooks.
  hMouseHook = SetWindowsHookEx(WH_MOUSE_LL, mouseProc, hInstance, NULL);
  hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, keyboardProc, hInstance, NULL);

  // Main message loop:
  while (GetMessage(&msg, nullptr, 0, 0)) {
    if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  // Honkwan: Unhook. Otherwise replayed actions will be further recorded.
  UnhookWindowsHookEx(hMouseHook);
  UnhookWindowsHookEx(hKeyboardHook);
  // Honkwan: Hard sleep 2 seconds allow user to do a quick manual reset.
  std::this_thread::sleep_for(std::chrono::seconds(2));

  // Honkwan: Reply all actions.
  for (auto item : actions) {
    if (item.type == MOUSE) {
      // Stardard way to debug log in windows
      OutputDebugStringA("LOG: MOUSE\n");
      SetMousePosition(item.mouse_point);
      INPUT    Input = { 0 };													// Create our input.
      Input.type = INPUT_MOUSE;									// Let input know we are using the mouse.
      if (item.wparam == WM_LBUTTONDOWN) {
        Input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
      }
      else if (item.wparam == WM_RBUTTONDOWN) {
        Input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
      }
      else if (item.wparam == WM_LBUTTONUP) {
        Input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
      }
      else if (item.wparam == WM_RBUTTONUP) {
        Input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
      }
      SendInput(1, &Input, sizeof(INPUT));
    }
    else if (item.type == KEYBOARD) {
      OutputDebugStringA("LOG: KEYBOARD\n");
      // Set up a generic keyboard event.
      INPUT input;
      input.type = INPUT_KEYBOARD;
      input.ki.wVk = item.l_param;
      input.ki.dwFlags = ((item.wparam == WM_SYSKEYDOWN) || (item.wparam == WM_KEYDOWN)) ? 0 : KEYEVENTF_KEYUP; // 0 for key press
      SendInput(1, &input, sizeof(INPUT));
    }
    else if (item.type == SLEEP) {
      OutputDebugStringA("LOG: SLEEP\n");
      std::this_thread::sleep_for(item.milli_s);
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
  wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_RECORDING));
  wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_RECORDING);
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
  case WM_DESTROY:
    PostQuitMessage(0);
    break;
  default:
    return DefWindowProc(hWnd, message, wParam, lParam);
  }
  return 0;
}
