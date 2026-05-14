
/*

Basic 2D Simulation v3_2


*/












#pragma once
#include <Windows.h>
#include <windowsx.h>

#include "world_sim_app.h"

//=====================WINDOWS STUFF===================================

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void OnPaint(HDC hdc, HWND hwnd);
BOOL InitializeTimer(HWND hwnd);
POINT cursor_location;
int cursor_down = 0;
int dragging = 0;
//-----------------------------------------------------------------------



//============================App Stuff====================================
world_sim_app* pApp;
void buildSimApp(HWND hwnd, HDC hdc);
bool fullscreen = true;

void CreateConsole();

// Global Variables:
HINSTANCE g_hInst;  // current instance

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR, INT iCmdShow)
{
	g_hInst = hInstance;
	CreateConsole();




	HWND hwnd;
	MSG msg;
	WNDCLASS wndClass;

	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = WndProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = hInstance;
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = TEXT("BasicWorld");


	RegisterClass(&wndClass);
	INT XPOS = 0;
	INT YPOS = 0;
	//INT XSIZE = GetSystemMetrics(SM_CXFULLSCREEN);
	//INT YSIZE = GetSystemMetrics(SM_CYFULLSCREEN);
	INT XSIZE = GetSystemMetrics(SM_CXSCREEN);
	INT YSIZE = GetSystemMetrics(SM_CYSCREEN);

	hwnd = CreateWindow(
		TEXT("BasicWorld"),		//Window Class name
		TEXT("Basic 2D World"),		//Window Caption/Name
		WS_OVERLAPPEDWINDOW,		//Window style
		XPOS,
		YPOS,
		XSIZE,
		YSIZE,
		NULL,						//parent window handle
		NULL,						//window menu handle
		hInstance,					//program instance handle
		NULL						//creation parameters
	);

	SetWindowLong(hwnd, GWL_STYLE, 0);

	ShowWindow(hwnd, iCmdShow);
	UpdateWindow(hwnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;

}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	RECT client_rect;
	GetClientRect(hwnd, &client_rect);

	switch (message)
	{
	case WM_CREATE:
	{
		InitializeTimer(hwnd);
		hdc = GetDC(hwnd);
		buildSimApp(hwnd, hdc);
		pApp->init();
	}
	return 0;
	break;
	case WM_TIMER:
	{	pApp->running_time += .333;
		//InvalidateRect(hwnd, (const RECT*)&client_rect, FALSE);

		BeginPaint(hwnd, &ps);
		pApp->Draw();

		EndPaint(hwnd, &ps);
		return 0;
		break;
	}
	case WM_LBUTTONDOWN:
	{
		int x, y;
		x = GET_X_LPARAM(lParam);
		y = GET_Y_LPARAM(lParam);
		POINT pt;
		pt.x = x;
		pt.y = y;
		cursor_down = 1;
		pApp->process_left_click(pt);
	}
	break;
	case WM_LBUTTONUP:
	{
		cursor_down = 0;
		dragging = 0;
		pApp->endDrag();
		pApp->process_left_mouse_up(wParam, lParam);
	}
	break;
	case WM_RBUTTONDOWN:
	{
		int x, y;
		x = GET_X_LPARAM(lParam);
		y = GET_Y_LPARAM(lParam);
		POINT pt;
		pt.x = x;
		pt.y = y;
	}
	return 0;
	break;
	case WM_MOUSEMOVE:
	{
		int x, y;
		x = GET_X_LPARAM(lParam);
		y = GET_Y_LPARAM(lParam);
		cursor_location.x = x;
		cursor_location.y = y;
		if (cursor_down && dragging == 0)
		{
			dragging = 1;
		}
		if (dragging)
		{
			pApp->process_drag(cursor_location);
		}
		pApp->process_mouse_move(wParam, lParam);
	}
	break;
	case WM_MOUSEWHEEL:
	{
		INT zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		UINT fwKeys = GET_KEYSTATE_WPARAM(wParam);

		int mult = 1;
		if (fwKeys == MK_SHIFT)
		{
			mult = 10;
		}
		pApp->process_scroll(cursor_location, zDelta, mult);
	}
	break;
	case WM_KEYDOWN:
	{
		//UINT fwKeys = GET_KEYSTATE_WPARAM(wParam);
		SHORT q_state = GetKeyState('Q');
		SHORT ctrl_state = GetKeyState(VK_CONTROL);

		if ((ctrl_state & 0x8000) && (q_state & 0x8000))
		{
			PostQuitMessage(0);
		}


		SHORT f_state = GetKeyState('F');
		if ((ctrl_state & 0x8000) && (f_state & 0x8000))
		{
			if (fullscreen)
			{
				fullscreen = !fullscreen;
				SetWindowLong(hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
				ShowWindow(hwnd, 10);
				UpdateWindow(hwnd);
			}
			else
			{
				fullscreen = !fullscreen;
				INT XSIZE = GetSystemMetrics(SM_CXSCREEN);
				INT YSIZE = GetSystemMetrics(SM_CYSCREEN);
				SetWindowLong(hwnd, GWL_STYLE, 0);
				SetWindowPos(hwnd, NULL, 0, 0, XSIZE, YSIZE, SWP_SHOWWINDOW);
				//ShowWindow(hwnd, 10);
				UpdateWindow(hwnd);
			}

		}


		if (wParam == 0x00000044)
		{
			// D key pressed
			pApp->key_d();
		}
		if (wParam == 0x00000041)
		{
			// A key pressed
			pApp->key_a();
		}
		if (wParam == 0x00000053)
		{
			// S key pressed
			pApp->key_s();
		}
		if (wParam == 0x00000057)
		{
			// W key pressed
			pApp->key_w();
		}
		if (wParam == VK_SPACE)
		{
			//spacebar is pressed
			pApp->key_space();
		}

	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
		break;
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}

}



VOID OnPaint(HDC hdc, HWND hwnd)
{
	RECT client_rect;
	GetClientRect(hwnd, &client_rect);
	HDC memdc = CreateCompatibleDC(hdc);
	HBITMAP hmem_map = CreateCompatibleBitmap(hdc, client_rect.right - client_rect.left, client_rect.bottom - client_rect.top);
	HBITMAP oldbm = (HBITMAP)SelectObject(memdc, hmem_map);

	COLORREF bg = 0xaa330f;
	HBRUSH bgBrush = CreateSolidBrush(bg);
	FillRect(memdc, &client_rect, bgBrush);

	BitBlt(hdc, client_rect.left, client_rect.top, client_rect.right - client_rect.left, client_rect.bottom - client_rect.top, memdc, client_rect.left, client_rect.top, SRCCOPY);
	SelectObject(memdc, oldbm);
	DeleteObject(hmem_map);
}


BOOL InitializeTimer(HWND hwnd)
{
	SetTimer(hwnd, 100, (1000 / 30), NULL);

	return TRUE;
}

void buildSimApp(HWND hwnd, HDC hdc)
{
	pApp = new world_sim_app(hwnd, hdc);
}

void CreateConsole()
{
	AllocConsole();		//Allocate a new console.
	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);	//Redirect stdout to the console
	freopen_s(&fp, "CONOUT$", "w", stderr);	//Redirect stderr to the console.
	freopen_s(&fp, "CONIN$", "r", stdin);	//Redirect stdin to the console.

	std::cout << "Console attached successfully!" << std::endl;
}