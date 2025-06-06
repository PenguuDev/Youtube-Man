#pragma once
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>
#include <Windowsx.h>
#include <locale>
#include <codecvt>
#include <string>

#include <backend/backend.h>

inline static UINT g_ResizeWidth = 0, g_ResizeHeight;
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
class Render
{
	ID3D11Device* g_pd3dDevice;
	ID3D11DeviceContext* g_pd3dDeviceContext;
	IDXGISwapChain* g_pSwapChain;
	bool g_SwapChainOccluded;
	ID3D11RenderTargetView* g_mainRenderTargetView;

	const char* aname;

	ImGuiIO io;
	HWND hwnd;
	WNDCLASSEXA wc;
	bool done;

	ImFont* font32;
	ImFont* font20;

	bool CreateDeviceD3D(HWND hWnd);
	void CleanupDeviceD3D();
	void CreateRenderTarget();
	void CleanupRenderTarget();
	void FrontendRenderStuff1();
	void FrontendRenderStuff2();

public:
	bool CreateFrontendWindow(const char* name, UINT width, UINT height);
	void FrontendRender();
	void FrontendCleanup();

	// notification
};