#pragma once

#include <d3d11.h>
#include "Resource.h"
#include "Vertex.h"
#include "singletonBase.h"

//#include "DXUTcamera.h"

#include <d3dx11effect.h>

#include <DirectXMath.h>

using namespace Vertexs;
using namespace DirectX;

#ifndef MAIN_H
#define MAIN_H

class cBaseScene;
class cMouseInput;
class cCamera;


class D3DMain : public SingletonBase<D3DMain>
{

public:
	D3DMain();
	~D3DMain();


private:
	cMouseInput *          m_MouseInput;
	cCamera *               m_Cam;

	HINSTANCE               g_hInst = nullptr;
	HWND                    m_hWnd = nullptr;
	D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
	ID3D11Device*           m_pDevice = nullptr;
	ID3D11DeviceContext*    m_pDC = nullptr;
	IDXGISwapChain*         m_pSwapChain = nullptr;
	ID3D11RenderTargetView* m_pRenderTargetView = nullptr;
	ID3D11DepthStencilView* m_pDepthStencilView = nullptr;
	D3D11_VIEWPORT          m_ViewPort;

	UINT m_Width;
	UINT m_Height;

	bool m_isDown = false;


protected:
	cBaseScene* m_CurScene;

public:


	BOOL            InitMain();

	void OnResize();
	void OnMouse(UINT msg, WPARAM btnState, int X, int Y);

	int  Run();
	void FrameUpdate(float dt);
	void SceneRender();
	void CaptionUpdate();

	void ResizeSwapChain();

	// ==================================================================
	//                             Get, Set
	// ==================================================================
public:
	void SetDevice(ID3D11Device* device)                  { m_pDevice = device; }
	void SetDC(ID3D11DeviceContext* dc)                   { m_pDC = dc; }
	void SetSwapChain(IDXGISwapChain* swapChain)          { m_pSwapChain = swapChain; }
	void SetRTV(ID3D11RenderTargetView* rtv)              { m_pRenderTargetView = rtv; }
	void SetViewPort(D3D11_VIEWPORT vp)                   { m_ViewPort = vp; }
	void SetDepthStencilView(ID3D11DepthStencilView* dsv) { m_pDepthStencilView = dsv; }
	void SetWidth(UINT width)                             { m_Width = width; }
	void SetHeight(UINT height)                           { m_Height = height; }

	ID3D11Device *           GetDevice() const             { return m_pDevice; }
	ID3D11DeviceContext*     GetDC() const                 { return m_pDC; }
	IDXGISwapChain*          GetSwapChain() const          { return m_pSwapChain; }
	ID3D11RenderTargetView*  GetRTV() const                { return m_pRenderTargetView; }
	D3D11_VIEWPORT           GetViewPort() const           { return m_ViewPort; }
	ID3D11DepthStencilView* GetDSV() const                 { return m_pDepthStencilView;  }

	HWND                     GetHWND()                     { return m_hWnd; }
	UINT                     GetWidth()                    { return m_Width; }
	UINT                     GetHeight()                   { return m_Height; }
	float                    GetAspect()                   { return (float)m_Width / (float)m_Height; }
	const cMouseInput*       GetMouse()                    { return m_MouseInput; }
	 cCamera*                GetCam()                      { return m_Cam; }


	//CDXUTDialogResourceManager GetDialogResourceManager() { return g_DialogResourceManager; } // manager for shared resources of dialogs
	//CD3DSettingsDlg GetSettingsDlg() { return g_SettingsDlg; }         // Device settings dialog
	//CDXUTTextHelper* GetTextHelper() {return g_pTxtHelper; }
	//CDXUTDialog GetDialog() { return g_HUD; }                  // manages the 3D UI
	//CDXUTDialog GetSampleUI() { g_SampleUI; }             // dialog for sample specific controls

};

#endif