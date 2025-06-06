#include "render.h"
#include <iostream>

#define HANDLE_MESSAGES_AND_OCCLUSION()                          \
    MSG msg;                                                     \
    while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))      \
    {                                                            \
        ::TranslateMessage(&msg);                                \
        ::DispatchMessage(&msg);                                 \
        if (msg.message == WM_QUIT)                              \
            done = true;                                         \
    }                                                            \
    if (done) break;                                             \
                                                                 \
    if (g_SwapChainOccluded &&                                   \
        g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED) \
    {                                                            \
        Sleep(10);                                               \
        continue;                                                \
    }

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool Render::CreateDeviceD3D(HWND hWnd)
{
	// Setup swap chain
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;
	//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
	if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
		res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
	if (res != S_OK)
		return false;

	CreateRenderTarget();
	return true;
}
void Render::CleanupDeviceD3D()
{
	CleanupRenderTarget();
	if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
	if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}
void Render::CreateRenderTarget()
{
	ID3D11Texture2D* pBackBuffer;
	g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
	pBackBuffer->Release();
}
void Render::CleanupRenderTarget()
{
	if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_NCHITTEST:
	{
		POINT pt;
		pt.x = GET_X_LPARAM(lParam);
		pt.y = GET_Y_LPARAM(lParam);
		ScreenToClient(hWnd, &pt);

		if (pt.y < 25 && pt.x <= g_ResizeWidth - 23)
			return HTCAPTION;

		break;
	}
	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED)
			return 0;
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

std::wstring utf8_to_wstring(const char* utf8Str)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, nullptr, 0);
	std::wstring wstr(len, L'\0');
	MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, &wstr[0], len);
	// Remove trailing null char added by MultiByteToWideChar
	wstr.resize(len - 1);
	return wstr;
}


bool Render::CreateFrontendWindow(const char* name, UINT width, UINT height)
{
	g_ResizeWidth = width;
	g_ResizeHeight = height;
	aname = name;

	WNDCLASSEXW wc = { 0 };
	std::wstring wname = utf8_to_wstring(name);
	wc.cbSize = sizeof(wc);
	wc.style = CS_CLASSDC;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = GetModuleHandle(nullptr);
	wc.lpszClassName = L"MyWindowClass";

	RegisterClassExW(&wc);

	hwnd = CreateWindowExW(
		0,
		wc.lpszClassName,
		wname.c_str(),
		WS_POPUP | WS_VISIBLE,
		100, 100, g_ResizeWidth, g_ResizeHeight,
		nullptr, nullptr, wc.hInstance, nullptr
	);
	// Initialize Direct3D
	if (!CreateDeviceD3D(hwnd))
	{
		CleanupDeviceD3D();
		::UnregisterClassW(wc.lpszClassName, wc.hInstance);
		return false;
	}

	// Show the window
	::ShowWindow(hwnd, SW_SHOWDEFAULT);
	::UpdateWindow(hwnd);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
	io.ConfigViewportsNoAutoMerge = true;
	//io.ConfigViewportsNoTaskBarIcon = true;
	//io.ConfigViewportsNoDefaultParent = true;
	io.ConfigDockingAlwaysTabBar = true;
	io.ConfigDockingTransparentPayload = true;
	//io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;     // FIXME-DPI: Experimental. THIS CURRENTLY DOESN'T WORK AS EXPECTED. DON'T USE IN USER APP!
	//io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports; // FIXME-DPI: Experimental.

	ImFontConfig cfg;
	cfg.SizePixels = 13.0f;
	font20 = io.Fonts->AddFontDefault(&cfg); // index 0
	cfg.SizePixels = 26.0f;
	font32 = io.Fonts->AddFontDefault(&cfg); // index 1

	io.FontDefault = font20;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

	return true;
}
void Render::FrontendRenderStuff1()
{
	g_SwapChainOccluded = false;

	// Handle window resize (we don't resize directly in the WM_SIZE handler)
	if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
	{
		CleanupRenderTarget();
		g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
		//g_ResizeWidth = g_ResizeHeight = 0;
		CreateRenderTarget();
	}

	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}
void Render::FrontendRenderStuff2()
{
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	// Rendering
	ImGui::Render();
	const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
	g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
	g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	// Update and Render additional Platform Windows
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	// Present
	HRESULT hr = g_pSwapChain->Present(1, 0);   // Present with vsync
	//HRESULT hr = g_pSwapChain->Present(0, 0); // Present without vsync
	g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
}

void Render::FrontendRender()
{
	bool open = true;
	static char installingApiText[128] = "Installing API's...";

	std::thread([]()
		{
			int dotsCount = 0;

			while (true)
			{
				std::this_thread::sleep_for(std::chrono::seconds(1));
				if (!isApiInstalled)
				{
					if (!ytdlpInstalled)
					{
						std::snprintf(installingApiText, sizeof(installingApiText),
							"Wait for API yt-dlp to be installed%s",
							std::string(dotsCount, '.').c_str());
					}
					else if (!ffmpegInstalled)
					{
						std::snprintf(installingApiText, sizeof(installingApiText),
							"Wait for API ffmpeg to be installed%s",
							std::string(dotsCount, '.').c_str());
					}
					dotsCount = (dotsCount + 1) % 4;
				}
			}
		}
	).detach();

	while (!done)
	{
		HANDLE_MESSAGES_AND_OCCLUSION();

		FrontendRenderStuff1();

		if (!open)
		{
			FrontendCleanup();
			break;
		}

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(g_ResizeWidth, g_ResizeHeight));
		ImGui::Begin(aname, &open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

		if (ImGui::BeginTabBar("##main"))
		{
			if (ImGui::BeginTabItem("Downloader"))
			{
				if (isApiInstalled)
				{
					static char youtubeUrl[256];
					static bool mp4 = true;

					ImGui::InputTextWithHint("##idk", "Enter the Youtube video url", youtubeUrl, sizeof(youtubeUrl));
					if (ImGui::Button("Download video"))
					{
						std::thread([]()
							{
								if (!isApiInstalled)
								{
									//MessageBoxA(0, "The Api's are not ready yet", "Error", MB_ICONERROR | MB_OK);
								}
								else
								{
									//std::thread([]() {MessageBoxA(0, "Video is now being downloaded.", "Success", MB_ICONINFORMATION | MB_OK); }).detach();
									backend::DownloadVideo(youtubeUrl, mp4);
								}
							}).detach();
					}
					ImGui::SameLine();
					ImGui::Checkbox("Download mp4", &mp4);
					ImGui::EndTabItem();
				}
				else
				{
					ImGui::PushFont(io.Fonts->Fonts[1]);

					ImVec2 windowSize = ImGui::GetWindowSize();
					ImVec2 textSize = ImGui::CalcTextSize(installingApiText);

					ImVec2 pos;
					pos.x = (windowSize.x - textSize.x) * 0.5f;
					pos.y = (windowSize.y - textSize.y) * 0.5f;

					ImGui::SetCursorPos(pos);
					ImGui::Text(installingApiText);

					ImGui::PopFont();

					ImGui::EndTabItem();
				}
			}
			if (ImGui::BeginTabItem("Utils"))
			{
				static bool showConsole = false;

				ImGui::Checkbox("Show Console", &showConsole);
				backend::console::ToggleConsole(showConsole);

				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}

		ImGui::End();

		FrontendRenderStuff2();
	}

	FrontendCleanup();
}

void Render::FrontendCleanup()
{
	done = true;
	//Sleep(200);
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	CleanupDeviceD3D();
	::DestroyWindow(hwnd);
	::UnregisterClassA(wc.lpszClassName, wc.hInstance);
	//Sleep(200);
	exit(0);
}