#pragma once

#include "D3DUtils.h"
#include "XTime.h"
#include "Camera.h"

class D3DApp {
public:
	D3DApp(HINSTANCE hinst);
	virtual ~D3DApp();

	HINSTANCE	AppInstance() const { return m_hinsApp; }
	HWND		MainWnd() const { return m_hWindow; }
	float		AspectRatio() const { return static_cast<float>(m_clientWidth) / m_clientHeight; }

	virtual bool Init();
	virtual void OnResize();
	virtual void UpdateKeyboardInput(double _dt) = 0;
	virtual void UpdateScene(double _dt) = 0;
	virtual void DrawScene() = 0;
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	virtual void OnMouseDown(WPARAM _btnState, int _x, int _y) {}
	virtual void OnMouseUp(WPARAM _btnState, int _x, int _y) {}
	virtual void OnMouseMove(WPARAM _btnState, int _x, int _y) {}

	bool Run();

protected:
	bool InitMainWindow();
	bool InitDirect3D();
	void ShowFPS();

protected:
	// window related
	HINSTANCE						m_hinsApp;
	HWND							m_hWindow;
	bool							m_appPaused;
	bool							m_minimized;
	bool							m_maximized;
	bool							m_resizing;
	UINT							m_4xMsaaQuality;

	// timer
	XTime							m_timer;
	bool							m_timerStop;

	// properties
	D3D_DRIVER_TYPE					m_d3dDriverType;
	wstring							m_mainWindTitle;
	wstring							m_deviceName;
	int								m_clientWidth;
	int								m_clientHeight;
	bool							m_enable4xMsaa;

	// D3D stuff
	ID3D11Device					*m_d3dDevice;
	ID3D11DeviceContext				*m_d3dImmediateContext;
	IDXGISwapChain					*m_swapChain;
	ID3D11Texture2D					*m_depthStencilBuffer;
	ID3D11RenderTargetView			*m_renderTargetView;
	ID3D11DepthStencilView			*m_depthStencilView;
	D3D11_VIEWPORT					 m_screenViewport;	// non-pointer
	ID3D11Debug						*m_d3dDebug;

	Camera							m_camera;
	// user control 
	POINT							m_lastMousePos;

};