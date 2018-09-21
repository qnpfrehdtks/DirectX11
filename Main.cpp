//--------------------------------------------------------------------------------------
// File: EmptyProject11.cpp
//
// Empty starting point for new Direct3D 11 Win32 desktop applications
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "DXUT.h"
#include <vector>
#include"Main.h"
#include "EngineScene.h"
#include <assert.h>
#include <windowsx.h>
#include "TimeMGR.h"
#include "cMouseInput.h"
#include "Camera.h"

#include "DXUT.h"
#include "DXUTgui.h"
#include "DXUTmisc.h"
#include "DXUTCamera.h"
#include "DXUTSettingsDlg.h"
#include "SDKmisc.h"
#include "SDKmesh.h"
#include "SDKMesh.h"
#include <WindowsX.h>
#include <sstream>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"


#pragma comment(lib, "d3d11.lib")

#pragma warning( disable : 4100 )

using namespace DirectX;
using namespace Vertexs;
//--------------------------------------------------------------------------------------
// Global Function
//--------------------------------------------------------------------------------------


void    CaptionUpdate();
BOOL    InitAni();
BOOL    InitDevice();
BOOL    InitWnd();
void    OnResize();
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
//CDXUTDialogResourceManager          g_DialogResourceManager; // manager for shared resources of dialogs
//CD3DSettingsDlg                     g_SettingsDlg;          // Device settings dialog
//CDXUTTextHelper*                    g_pTxtHelper = nullptr;
//CDXUTDialog                         g_HUD;                  // manages the 3D UI
//CDXUTDialog                         g_SampleUI;             // dialog for sample specific controls


//--------------------------------------------------------------------------------------
// D3D Variables
//--------------------------------------------------------------------------------------
UINT    g_idxSize = 0;
UINT    g_VertexSize = 0;

ID3D11Device*          g_pDevice = nullptr;
ID3D11DeviceContext*   g_pDeviceContext = nullptr;
IDXGISwapChain*        g_pSwapChain = nullptr;

CDXUTTextHelper*            g_pTxtHelper = NULL;
CDXUTDialogResourceManager  g_DialogResourceManager; // manager for shared resources of dialogs

D3D_DRIVER_TYPE        g_DriverType = D3D_DRIVER_TYPE_HARDWARE;
D3D_FEATURE_LEVEL      g_featureLevel = D3D_FEATURE_LEVEL_11_0;

BOOL                   m_Enable4xMsaa;

UINT                   g_4xMsaaQuality = 4;
UINT                   g_Width = 1024;
UINT                   g_Height = 768;

HWND                   g_hWnd;
HINSTANCE              g_hInst;

ID3D11RenderTargetView*  g_pRTV;
ID3D11ShaderResourceView*  g_pHDRSRV;

ID3D11DepthStencilView*  g_pDSV;
ID3D11Texture2D*         g_DepthStencilTexture;

D3D11_VIEWPORT          g_ViewPort;
std::wstring            g_WndCaption = L"D3D11 Application";



void RenderText()
{
	g_pTxtHelper->Begin();
	g_pTxtHelper->SetInsertionPos(5, 5);
	g_pTxtHelper->SetForegroundColor(XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f));
	g_pTxtHelper->DrawTextLine(DXUTGetFrameStats(DXUTIsVsyncEnabled()));
	g_pTxtHelper->DrawTextLine(DXUTGetDeviceStats());
	g_pTxtHelper->End();
}

//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
		return true;

	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	
	case WM_SIZE:
		if (g_pDevice != NULL && wParam != SIZE_MINIMIZED)
		{
			ImGui_ImplDX11_InvalidateDeviceObjects();
			ImGui_ImplDX11_CreateDeviceObjects();
		}
		return 0;
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			TimeMGR::GetInstance()->Stop();
		}
		else
		{
			TimeMGR::GetInstance()->Start();
		}
		return 0;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_MENUCHAR:
		return MAKELRESULT(0, MNC_CLOSE);

	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = g_Width;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = g_Height;

		break;

		// Mouse 입력
	case WM_LBUTTONDOWN : case WM_MBUTTONDOWN : case WM_RBUTTONDOWN : case WM_LBUTTONUP : case WM_MBUTTONUP : case WM_RBUTTONUP : case WM_MOUSEMOVE :
			D3DMain::GetInstance()->OnMouse(message, wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

//--------------------------------------------------------------------------------------
// Create Wnd and swap chain
//--------------------------------------------------------------------------------------
BOOL InitWnd()
{
	WNDCLASS wnd;
	wnd.style = CS_HREDRAW | CS_VREDRAW;
	wnd.lpfnWndProc = WndProc;
	wnd.cbClsExtra = 0;
	wnd.cbWndExtra = 0;
	wnd.hInstance = g_hInst;
	wnd.hIcon = LoadIcon(0, IDI_APPLICATION);
	wnd.hCursor = LoadCursor(0, IDC_ARROW);
	wnd.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wnd.lpszMenuName = 0;
	wnd.lpszClassName = L"D3DWndClassName";

	if (!RegisterClass(&wnd))
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}

	
	// Compute window rectangle dimensions based on requested client area dimensions.
	RECT rect = { 0, 0, g_Width, g_Height };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	g_hWnd = CreateWindow(L"D3DWndClassName", g_WndCaption.c_str(),
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, g_hInst,0);
	if(!g_hWnd)
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	ShowWindow(g_hWnd, SW_SHOW);
	UpdateWindow(g_hWnd);

	return true;

}





BOOL InitAni()
{
	return 0;
}



//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
BOOL InitDevice()
{
	UINT createDeviceFlags = 0;

#if defined(DEBUG) || defined(_DEBUG)  
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevel;
	HRESULT hr = D3D11CreateDevice(
		0,                 // default adapter
		g_DriverType,
		0,                 // no software device
		createDeviceFlags,
		0, 0,              // default feature level array
		D3D11_SDK_VERSION,
		&g_pDevice,
		&featureLevel,
		&g_pDeviceContext);

	if (FAILED(hr))
	{
		MessageBox(0, L"D3D11CreateDevice Failed.", 0, 0);
		return false;
	}

	if (featureLevel != D3D_FEATURE_LEVEL_11_0)
	{
		MessageBox(0, L"Direct3D Feature Level 11 unsupported.", 0, 0);
		return false;
	}

	// Check 4X MSAA quality support for our back buffer format.
	// All Direct3D 11 capable devices support 4X MSAA for all render 
	// target formats, so we only need to check quality support.

	HR(g_pDevice->CheckMultisampleQualityLevels(
		DXGI_FORMAT_R16G16B16A16_FLOAT, 4, &g_4xMsaaQuality));
	assert(g_4xMsaaQuality > 0);

	// m_4xMsaaQuality 는 품질 수준 개수가 저장되는 변수.



	// Fill out a DXGI_SWAP_CHAIN_DESC to describe our swap chain.

	DXGI_SWAP_CHAIN_DESC scd;

	//	ZeroMemory(&sd, sizeof(DXGI_SWAP_CHAIN_DESC));

	scd.BufferDesc.Width = g_Width;
	scd.BufferDesc.Height = g_Height;
	scd.BufferDesc.RefreshRate.Numerator = 60;
	scd.BufferDesc.RefreshRate.Denominator = 1;
	scd.BufferDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	scd.BufferUsage = DXGI_USAGE_SHADER_INPUT | DXGI_USAGE_BACK_BUFFER;
	// Use 4X MSAA? 
	if (m_Enable4xMsaa)
	{
		scd.SampleDesc.Count = 4;
		scd.SampleDesc.Quality = g_4xMsaaQuality - 1;
	}
	// No MSAA
	else
	{
		scd.SampleDesc.Count = 1;
		scd.SampleDesc.Quality = 0;
	}

	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.BufferCount = 1;
	scd.OutputWindow = g_hWnd;
	scd.Windowed = true;
	scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scd.Flags = 0;

	// To correctly create the swap chain, we must use the IDXGIFactory that was
	// used to create the device.  If we tried to use a different IDXGIFactory instance
	// (by calling CreateDXGIFactory), we get an error: "IDXGIFactory::CreateSwapChain: 
	// This function is being called with a device from a different IDXGIFactory."

	IDXGIDevice* dxgiDevice = 0;
	HR(g_pDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice));

	IDXGIAdapter* dxgiAdapter = 0;
	HR(dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter));

	IDXGIFactory* dxgiFactory = 0;
	HR(dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory));


	HR(dxgiFactory->CreateSwapChain(g_pDevice, &scd, &g_pSwapChain));

	SAFE_RELEASE(dxgiDevice);
	SAFE_RELEASE(dxgiAdapter);
	SAFE_RELEASE(dxgiFactory);

	// Create Render Target View , ViewPort
	OnResize();
	// The remaining steps that need to be carried out for d3d creation
	// also need to be executed every time the window is resized.  So
	// just call the OnResize method here to avoid code duplication.
		return S_OK;
}

//--------------------------------------------------------------------------------------
// Resise or Wnd Set Up
//--------------------------------------------------------------------------------------
void OnResize()
{
	assert(g_pDeviceContext);
	assert(g_pDevice);
	assert(g_pSwapChain);

	// Release the old views, as they hold references to the buffers we
	// will be destroying.  Also release the old depth/stencil buffer.

	SAFE_RELEASE(g_pRTV);
	SAFE_RELEASE(g_pDSV);
	SAFE_RELEASE(g_DepthStencilTexture);

	HR(g_pSwapChain->ResizeBuffers(1, g_Width, g_Height, DXGI_FORMAT_R16G16B16A16_FLOAT, 0));
	ID3D11Texture2D* backBuffer;
	HR(g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)));
	HR(g_pDevice->CreateRenderTargetView(backBuffer, 0, &g_pRTV));

	/*D3D11_SHADER_RESOURCE_VIEW_DESC dsrvd =	
	{
		DXGI_FORMAT_R16G16B16A16_FLOAT,
		D3D11_SRV_DIMENSION_TEXTURE2D,
		0,
		0
	};

	dsrvd.Texture2D.MipLevels = 1;
	HR(g_pDevice->CreateShaderResourceView(backBuffer, 0, &g_pHDRSRV));

	
	SAFE_RELEASE(backBuffer);*/

	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = g_Width;
	depthStencilDesc.Height = g_Height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// Use 4X MSAA? --must match swap chain MSAA values.
	if (m_Enable4xMsaa)
	{
		depthStencilDesc.SampleDesc.Count = 4;
		depthStencilDesc.SampleDesc.Quality = g_4xMsaaQuality - 1;
	}
	// No MSAA
	else
	{
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
	}

	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	// 깊이 스텐실로 플래그를 세운다.
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	// Texture 생성., 깊이 버퍼로 생성.
	HR(g_pDevice->CreateTexture2D(&depthStencilDesc, 0, &g_DepthStencilTexture));
	HR(g_pDevice->CreateDepthStencilView(g_DepthStencilTexture, 0, &g_pDSV));

	// Bind the render target view and depth/stencil view to the pipeline.
	// a render target is a feature of modern graphics processing units 
	// (GPUs) that allows a 3D scene to be rendered to an intermediate memory buffer, 
	// or Render Target Texture (RTT), instead of the frame buffer or back buffer.
	// from Wiki

	// RenderTarget is a memory Buffer for rendering pixel
	// 그래픽 파이프라인은 후면 버퍼라는 기본 Render Target을 가지고 있는데, 
	// 후면 버퍼는 다음에 그릴 프레임을 가진 비디오 메모리이다.
	// 만약 렌더 탁세 없이 스크린을 그리게 되면, 후면 버퍼를 기본값으로 사용하겠다는 뜻이다.
	// 그래서 추가적인 렌더 타겟을 만들어야 한다.
	// 그래서 offscreen에 여러 렌더 타겟을 모아서 후면버퍼에 그려서 해당 프렝미에 보여 줄 수 있다.

	g_pDeviceContext->OMSetRenderTargets(1, &g_pRTV, g_pDSV);


	
	// Set the viewport transform.

	g_ViewPort.TopLeftX = 0;
	g_ViewPort.TopLeftY = 0;
	g_ViewPort.Width = static_cast<float>(g_Width);
	g_ViewPort.Height = static_cast<float>(g_Height);
	g_ViewPort.MinDepth = 0.0f;
	g_ViewPort.MaxDepth = 1.0f;

	g_pDeviceContext->RSSetViewports(1, &g_ViewPort);
}


//--------------------------------------------------------------------------------------
// Initialize everything and go into a render loop
//--------------------------------------------------------------------------------------
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	g_hInst = hInstance;

	

	if (!D3DMain::GetInstance()->InitMain()) return 0;

	return D3DMain::GetInstance()->Run();

}

D3DMain::D3DMain() : m_CurScene(nullptr), m_hWnd(g_hWnd)
{

}

D3DMain::~D3DMain()
{
}

BOOL D3DMain::InitMain()
{
	if (FAILED(InitWnd())) return false;
	if (FAILED(InitDevice())) return false;

	m_Width = g_Width;
	m_Height = g_Height;

	// Setup Dear ImGui binding
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui_ImplWin32_Init(g_hWnd);
	ImGui_ImplDX11_Init(g_pDevice, g_pDeviceContext);

	ImGui::StyleColorsDark();


	m_Cam = new cCamera();
	m_Cam->SetPos(10, 15, 10);

	m_ViewPort = g_ViewPort;
	m_MouseInput = new cMouseInput();

	OnResize();

	m_CurScene = new cEngineScene(g_pDevice, g_pDeviceContext, g_pRTV, g_pDSV,g_pHDRSRV);
	m_CurScene->Init();
	m_MouseInput->SetCamera(m_Cam);
	


	return true;
}

void D3DMain::OnResize()
{
	float fAspectRatio = GetAspect();
	m_Cam->SetProjection(XM_PI / 4, fAspectRatio, 0.1f, 3000.0f);

	if (m_CurScene != nullptr)
		m_CurScene->OnResize();


}

void D3DMain::OnMouse(UINT msg, WPARAM btnState, int X, int Y)
{
	if (m_CurScene)
	{
		switch (msg)
		{
		case WM_RBUTTONDOWN:
			m_MouseInput->OnMouseDown(btnState, X, Y);
			return;
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
				m_MouseInput->OnMouseUp(btnState, X, Y);
			return;
		case WM_MOUSEMOVE:
			if(!m_isDown)
				m_MouseInput->OnMouseMove(btnState, X, Y);
			return;
		}
	}

}

int D3DMain::Run()
{
	MSG msg = { 0 };

	TimeMGR::GetInstance()->Reset();

	while (msg.message != WM_QUIT)
	{
		// If there are Window messages then process them.
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		// Otherwise, do animation/game stuff.
		else
		{
			TimeMGR::GetInstance()->Tick();
			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			// Start the Dear ImGui frame
		
			if (m_CurScene != nullptr)
			{
				FrameUpdate(TimeMGR::GetInstance()->DeltaTime());
				SceneRender();
			}

     
		}

		
	}

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();


	return (int)msg.wParam;
}

void D3DMain::FrameUpdate(float dt)
{

	m_CurScene->UpdateScene(dt);
	m_Cam->InputKey(dt);
	m_Cam->UpdateViewMatrix();
	CaptionUpdate();
	
}

void D3DMain::SceneRender()
{
	
	if (m_CurScene != nullptr)
	m_CurScene->DrawScene();

	g_pSwapChain->Present(0, 0);




}

void D3DMain::CaptionUpdate()
{
	static int frameCnt = 0;
	static float timeElapsed = 0.0f;
	XMFLOAT3 camPos = m_Cam->GetPos();

	frameCnt++;

	// Compute averages over one second period.
	if ((TimeMGR::GetInstance()->TotalTime() - timeElapsed) >= 1.0f)
	{
		float fps = (float)frameCnt; // fps = frameCnt / 1
		float mspf = 1000.0f / fps;

		std::wostringstream outs;
		outs.precision(6);
		outs << g_WndCaption << L"    "
			<< L"FPS: " << fps << L"    "
			<< L"Frame Time: " << mspf << L" (ms)" << L"X : " << camPos.x << L" Y : " << camPos.y << L" Z : " << camPos.z;
			SetWindowText(g_hWnd, outs.str().c_str());

			// Reset for next average.
		frameCnt = 0;
		timeElapsed += 1.0f;
	}

}

