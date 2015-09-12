#include "D3DApp.h"

namespace {
	D3DApp *g_d3dApp = nullptr;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	if ( GetAsyncKeyState(VK_ESCAPE) )
		message = WM_DESTROY;
	switch ( message ) {
	case (WM_DESTROY) : {
		PostQuitMessage(0);
		break;
	}
	}
	return g_d3dApp->MsgProc(hWnd, message, wParam, lParam);
}

D3DApp::D3DApp(HINSTANCE hinst) :
	m_hinsApp(hinst),
	m_mainWindTitle(L"FS Graphic Project"),
	m_d3dDriverType(D3D_DRIVER_TYPE_HARDWARE),
	m_clientWidth(BACKBUFFER_WIDTH),
	m_clientHeight(BACKBUFFER_HEIGHT),
	m_enable4xMsaa(true),
	m_appPaused(false),
	m_minimized(false),
	m_maximized(false),
	m_resizing(false),
	m_timerStop(false),
	m_4xMsaaQuality(1),

	m_mutex(nullptr),

	m_d3dDevice(nullptr),
	m_d3dImmediateContext(nullptr),
	m_swapChain(nullptr),

	m_depthStencilBuffer(nullptr),
	m_renderTargetView(nullptr),
	m_depthStencilView(nullptr),
	m_d3dDebug(nullptr),

	m_camView(XMMatrixIdentity()),
	m_camProjection(XMMatrixIdentity()),
	m_camUp(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)),

	m_constDefaultForward(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f)),
	m_camForward(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f)),
	m_constDefaultRight(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f)),
	m_camRight(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f)),

	m_moveLeftRight(0.0f),
	m_moveBackForward(-5.0f),
	m_camYaw(0.0f),
	m_camPitch(0.0f),

	m_mouseAplha(XM_PI),
	m_mouseBeta(XM_PI)

{
	ZeroMemory(&m_screenViewport, sizeof(D3D11_VIEWPORT));
	m_lastMousePos.x = 0;
	m_lastMousePos.y = 0;

	// default camera initalize
	m_camPosition = XMVectorSet(0.0f, 1.0f, -0.5f, 1.0f);
	m_camTarget = XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f);
	m_camView = XMMatrixLookAtLH(m_camPosition, m_camTarget, m_camUp);

	// multithreading
	m_mutex = new mutex;

	g_d3dApp = this;
}

D3DApp::~D3DApp() {
	SafeRelease(m_renderTargetView);
	SafeRelease(m_swapChain);
	SafeRelease(m_depthStencilView);
	SafeRelease(m_depthStencilBuffer);

	if ( m_d3dImmediateContext )
		m_d3dImmediateContext->ClearState();

	SafeRelease(m_d3dImmediateContext);
	if ( SUCCEEDED(m_d3dDevice->QueryInterface(__uuidof(ID3D11Debug), (void**)&m_d3dDebug)) ) {
		ID3D11InfoQueue *d3dInfoQueue = nullptr;
		if ( SUCCEEDED(m_d3dDebug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&d3dInfoQueue)) ) {
#if defined(DEBUG) || defined(_DEBUG)
			d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
			d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
#endif
			D3D11_MESSAGE_ID hide[] = {
				D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
				// Add more message IDs here as needed
			};
			D3D11_INFO_QUEUE_FILTER filter;
			memset(&filter, 0, sizeof(filter));
			filter.DenyList.NumIDs = _countof(hide);
			filter.DenyList.pIDList = hide;
			d3dInfoQueue->AddStorageFilterEntries(&filter);
			d3dInfoQueue->Release();
		}
	}
	m_d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY | D3D11_RLDO_DETAIL);
	SafeRelease(m_d3dDevice);
	SafeRelease(m_d3dDebug);

	SafeDelete(m_mutex);

	UnregisterClass(L"DirectXApplication", m_hinsApp);
}

bool D3DApp::Init() {
	if ( !InitMainWindow() ) return false;
	if ( !InitDirect3D() ) return false;
	return true;
}

bool D3DApp::InitMainWindow() {
	WNDCLASSEX  wndClass;
	ZeroMemory(&wndClass, sizeof(wndClass));
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.lpfnWndProc = WndProc;	// changed
	wndClass.lpszClassName = L"DirectXApplication";
	wndClass.hInstance = m_hinsApp;
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOWFRAME);
	wndClass.hIcon = LoadIcon(0, IDI_APPLICATION);

	if ( !RegisterClassEx(&wndClass) ) {
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}
	RegisterClassEx(&wndClass);

	RECT window_size = { 0, 0, m_clientWidth, m_clientHeight };

	AdjustWindowRect(&window_size, WS_OVERLAPPEDWINDOW, false);

	m_hWindow = CreateWindow(
		L"DirectXApplication", m_mainWindTitle.c_str(),
		WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX),
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		window_size.right - window_size.left,
		window_size.bottom - window_size.top,
		NULL, NULL, m_hinsApp, 0
	);	// this

	ShowWindow(m_hWindow, SW_SHOW);
	UpdateWindow(m_hWindow);	// added
	return true;
}

bool D3DApp::InitDirect3D() {

	// Begin to create device and swap chains
	UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL  FeatureLevelsRequested = D3D_FEATURE_LEVEL_11_0;
	D3D_FEATURE_LEVEL  FeatureLevelsSupported;
	UINT               numLevelsRequested = 1;

	// create a struct to hold information about the swap chain
	DXGI_SWAP_CHAIN_DESC swapChainDesc;

	// clear out the struct for use
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	// fill the swap chain description struct
	swapChainDesc.BufferDesc.Width						= m_clientWidth;
	swapChainDesc.BufferDesc.Height						= m_clientHeight;
	swapChainDesc.BufferDesc.RefreshRate.Numerator		= 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator	= 1;
	swapChainDesc.BufferDesc.Format						= DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.ScanlineOrdering			= DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling					= DXGI_MODE_SCALING_UNSPECIFIED;
	if ( m_enable4xMsaa ) {
		swapChainDesc.SampleDesc.Count					= 4;
		swapChainDesc.SampleDesc.Quality				= m_4xMsaaQuality - 1;
	} else {
		swapChainDesc.SampleDesc.Count					= 1;
		swapChainDesc.SampleDesc.Quality				= 0;
	}
	swapChainDesc.BufferUsage							= DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount							= 1;
	swapChainDesc.OutputWindow							= m_hWindow;
	swapChainDesc.Windowed								= true;
	swapChainDesc.SwapEffect							= DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags									= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	IDXGIFactory *factoryPtr = nullptr;
	IDXGIAdapter *adapterPtr = nullptr;
	vector<IDXGIAdapter *> adapters;

	CreateDXGIFactory(__uuidof(IDXGIFactory), (void **)&factoryPtr);

	UINT bestAdapterIndex = 0;
	size_t bestMemSize = 0;
	DXGI_ADAPTER_DESC adapterDesc;
	ZeroMemory(&adapterDesc, sizeof(adapterDesc));

	for ( UINT i = 0; factoryPtr->EnumAdapters(i, &adapterPtr) != DXGI_ERROR_NOT_FOUND; i++ ) {
		adapters.push_back(adapterPtr);
		adapterPtr->GetDesc(&adapterDesc);

		if ( adapterDesc.DedicatedVideoMemory > bestMemSize ) {
			bestAdapterIndex = 0;
			bestMemSize = adapterDesc.DedicatedVideoMemory;
		}
	}
	adapters[bestAdapterIndex]->GetDesc(&adapterDesc);
	m_deviceName = adapterDesc.Description;
	SafeRelease(factoryPtr);

	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		adapters[bestAdapterIndex],				// Multiple adapters
		D3D_DRIVER_TYPE_UNKNOWN,				// Driver Type If you specify the adapter, you cannot specify the driver type
		NULL,									// Software
		createDeviceFlags,						// Flags
		&FeatureLevelsRequested,				// Feature Levels Requested Pointer
		1,										// Number of Feature Levels
		D3D11_SDK_VERSION,						// D3D11_SDK_VERSION
		&swapChainDesc,							// Swap Chain Desciptions
		&m_swapChain,							// Swap Chain Pointer
		&m_d3dDevice,							// D3D Device
		&FeatureLevelsSupported,				// Return supported levels
		&m_d3dImmediateContext					// Device Context Pointer
		);

	if ( FAILED(hr) ) {
		MessageBox(0, L"D3D11CreateDevice Failed.", 0, 0);
		return false;
	}

	if ( FeatureLevelsSupported != D3D_FEATURE_LEVEL_11_0 ) {
		MessageBox(0, L"Direct3D Feature Level 11 unsupported.", 0, 0);
		return false;
	}

	HR(m_d3dDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m_4xMsaaQuality));
	assert(m_4xMsaaQuality > 0);

	OnResize();

	return true;
}

void D3DApp::InitCamera() {
	// set up projection mat
	m_camProjection = XMMatrixPerspectiveFovLH(0.4f*3.14f, AspectRatio(), 1.0f, 1000.0f);

}

void D3DApp::UpdateCamera() {
	m_camRotationMatrix = XMMatrixRotationRollPitchYaw(m_camPitch, m_camYaw, 0);
	m_camTarget = XMVector3TransformCoord(m_constDefaultForward, m_camRotationMatrix);

	m_camTarget = XMVector3Normalize(m_camTarget);

	XMMATRIX RotateYTempMatrix;

	RotateYTempMatrix = XMMatrixRotationY(m_camYaw);

	//camForward = XMVector3TransformCoord(DefaultForward, RotateYTempMatrix);	// walk on ground

	m_camForward = m_camTarget;

	m_camRight = XMVector3TransformCoord(m_constDefaultRight, RotateYTempMatrix);

	m_camUp = XMVector3TransformCoord(m_camUp, RotateYTempMatrix);
	m_camPosition += m_moveLeftRight*m_camRight + m_moveBackForward*m_camForward;
	m_camTarget += m_camPosition;

	m_camView = XMMatrixLookAtLH(m_camPosition, m_camTarget, m_camUp);

	m_moveLeftRight = 0.0f;
	m_moveBackForward = 0.0f;
}

void D3DApp::OnResize() {
	assert(m_d3dImmediateContext);
	assert(m_d3dDevice);
	assert(m_swapChain);

	// Release the old views, as they hold references to the buffers we
	// will be destroying.  Also release the old depth/stencil buffer.
	SafeRelease(m_renderTargetView);
	SafeRelease(m_depthStencilView);
	SafeRelease(m_depthStencilBuffer);

	// Resize the swap chain and recreate the render target view.
	HR(m_swapChain->ResizeBuffers(1, m_clientWidth, m_clientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));

	// Backbuffer and Render Terget View Creation
	ID3D11Texture2D *backBuffer;
	HR(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)));
	HR(m_d3dDevice->CreateRenderTargetView(backBuffer, 0, &m_renderTargetView));
	SafeRelease(backBuffer);

	// Create the depth/stencil buffer and view.
	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = m_clientWidth;
	depthStencilDesc.Height = m_clientHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;	//	A 32-bit z-buffer format that supports 24 bits for depth and 8 bits for stencil.
	//depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;	//	A single-component, 32-bit floating-point format that supports 32 bits for depth.

	if ( m_enable4xMsaa ) {
		depthStencilDesc.SampleDesc.Count = 4;
		depthStencilDesc.SampleDesc.Quality = m_4xMsaaQuality - 1;
	} else {
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
	}
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	HR(m_d3dDevice->CreateTexture2D(&depthStencilDesc, 0, &m_depthStencilBuffer));
	HR(m_d3dDevice->CreateDepthStencilView(m_depthStencilBuffer, 0, &m_depthStencilView));

	// Set the viewport transform.
	m_screenViewport.TopLeftX = 0;
	m_screenViewport.TopLeftY = 0;
	m_screenViewport.Width = static_cast<float>(m_clientWidth);
	m_screenViewport.Height = static_cast<float>(m_clientHeight);
	m_screenViewport.MinDepth = 0.0f;
	m_screenViewport.MaxDepth = 1.0f;

	m_d3dImmediateContext->RSSetViewports(1, &m_screenViewport);

	InitCamera();
}

void D3DApp::ShowFPS() {
	static int frameCnt = 0;
	static float timeElapsed = 0.0f;
	frameCnt++;
	// Compute averages over one second period.
	if ( (m_timer.TotalTime() - timeElapsed) >= 1.0f ) {
		float fps = (float)frameCnt; // fps = frameCnt / 1
		float mspf = 1000.0f / fps;
		std::wostringstream outs;
		outs.precision(6);
		outs << m_mainWindTitle << L" - " << m_deviceName << L" - FPS: " << fps << L", Time: " << mspf << L" (ms)"
			 << " - Total thread(s): " << m_threads.size() + 1
			 << " - Cam Position: (" 
			 << XMVectorGetX(m_camPosition) << ", "
			 << XMVectorGetY(m_camPosition) << ", "
			 << XMVectorGetZ(m_camPosition) << ")";
		SetWindowText(m_hWindow, outs.str().c_str());
		// Reset for next average.
		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}


bool D3DApp::Run() {
	if ( !m_timerStop ) m_timer.Signal();

	if ( !m_appPaused ) {
		ShowFPS();
		UpdateKeyboardInput(m_timer.Delta());
		UpdateCamera();
		UpdateScene(m_timer.Delta());
		DrawScene();
	} else {
		Sleep(100);
	}

	return true;
}

LRESULT D3DApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	int wmId = 0;
	switch ( msg ) {
	case WM_ACTIVATE:
		if ( LOWORD(wParam) == WA_INACTIVE ) {
			m_appPaused = true;
			m_timerStop = true;
		} else {
			m_appPaused = false;
			m_timerStop = false;
		}
		return 0;
		// WM_SIZE is sent when the user resizes the m_hWindow. 
	case WM_COMMAND: {
		wmId = LOWORD(wParam);
		switch (wmId) {
			case ID_FILE_OPEN_MENU:

				break;
			case ID_FILE_EXIT:

				break;
			}
		return 0;
	}
	case WM_SIZE:
		// Save the new client area dimensions.
		m_clientWidth = LOWORD(lParam);
		m_clientHeight = HIWORD(lParam);
		if ( m_d3dDevice ) {
			if ( wParam == SIZE_MINIMIZED ) {
				m_appPaused = true;
				m_minimized = true;
				m_maximized = false;
			} else if ( wParam == SIZE_MAXIMIZED ) {
				m_appPaused = false;
				m_minimized = false;
				m_maximized = true;
				OnResize();
			} else if ( wParam == SIZE_RESTORED ) {
				// Restoring from minimized state?
				if ( m_minimized ) {
					m_appPaused = false;
					m_minimized = false;
					OnResize();
				}
				// Restoring from maximized state?
				else if ( m_maximized ) {
					m_appPaused = false;
					m_maximized = false;
					OnResize();
				} else if ( m_resizing ) {
					// If user is dragging the resize bars, we do not resize 
					// the buffers here because as the user continuously 
					// drags the resize bars, a stream of WM_SIZE messages are
					// sent to the m_hWindow, and it would be pointless (and slow)
					// to resize for each WM_SIZE message received from dragging
					// the resize bars.  So instead, we reset after the user is 
					// done resizing the m_hWindow and releases the resize bars, which 
					// sends a WM_EXITSIZEMOVE message.
				} else {
					// API call such as SetWindowPos or mSwapChain->SetFullscreenState.
					OnResize();
				}
			}
		}
		return 0;

		// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
	case WM_ENTERSIZEMOVE:
		m_appPaused = true;
		m_resizing = true;
		m_timerStop = true;
		return 0;

		// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
		// Here we reset everything based on the new m_hWindow dimensions.
	case WM_EXITSIZEMOVE:
		m_appPaused = false;
		m_resizing = false;
		m_timerStop = false;
		OnResize();
		return 0;

		// WM_DESTROY is sent when the m_hWindow is being destroyed.
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

		// The WM_MENUCHAR message is sent when a menu is active and the user presses 
		// a key that does not correspond to any mnemonic or accelerator key. 
	case WM_MENUCHAR:
		// Don't beep when we alt-enter.
		return MAKELRESULT(0, MNC_CLOSE);

		// Catch this message so to prevent the m_hWindow from becoming too small.
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_MOUSEMOVE:
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

