#include "d3dApp.h"

D3DApp* g_d3dApp = 0;
IDirect3DDevice9* g_pDevice = 0;

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// ÏûÏ¢´¦ÀúÖ¯Êý
	if (g_d3dApp != 0)
		return g_d3dApp->msgProc(msg, wParam, lParam);
	else
		return DefWindowProc(hwnd, msg, wParam, lParam);
}

D3DApp::D3DApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP)
{
	mMainWndCaption = winCaption;
	mDevType = devType;
	mRequestedVP = requestedVP;

	mhAppInst = hInstance;
	mhMainWnd = 0;
	md3dObject = 0;
	mAppPaused = false;
	ZeroMemory(&md3dPP, sizeof(md3dPP));

	initMainWindow();
	initDirect3D();
}

D3DApp::~D3DApp()
{
	SAFE_RELEASE(md3dObject);
	SAFE_RELEASE(g_pDevice);
}

HINSTANCE D3DApp::getAppInst()
{
	return mhAppInst;
}

HWND D3DApp::getMainWnd()
{
	return mhMainWnd;
}

void D3DApp::initMainWindow()
{
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = mhAppInst;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = "D3DWndClassName";

	if (!RegisterClass(&wc))
	{
		MessageBox(0, "RegisterClass FAILED", 0, 0);
		PostQuitMessage(0);
	}

	// ´´½¨´°¿Ú

	RECT R = { 0, 0, 800, 600 };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	mhMainWnd = CreateWindow("D3DWndClassName", mMainWndCaption.c_str(),
		WS_OVERLAPPEDWINDOW, 100, 100, R.right, R.bottom,
		0, 0, mhAppInst, 0);

	if (!mhMainWnd)
	{
		MessageBox(0, "CreateWindow FAILED", 0, 0);
		PostQuitMessage(0);
	}

	ShowWindow(mhMainWnd, SW_SHOW);
	UpdateWindow(mhMainWnd);
}

void D3DApp::initDirect3D()
{
	// 1: ´´½¨IDirect3D9 ¶ÔÏE

	md3dObject = Direct3DCreate9(D3D_SDK_VERSION);
	if (!md3dObject)
	{
		MessageBox(0, "Direct3DCreate9 FAILED", 0, 0);
		PostQuitMessage(0);
	}


	//  2: ÑéÖ¤Ó²¼þÊÇ·ñÖ§³ÖÌØ¶¨µÄÊý¾Ý¸ñÊ½ÒÔ¼°È«ÆÁÄ£Ê½

	D3DDISPLAYMODE mode;
	md3dObject->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &mode);
	HR(md3dObject->CheckDeviceType(D3DADAPTER_DEFAULT, mDevType, mode.Format, mode.Format, true));
	HR(md3dObject->CheckDeviceType(D3DADAPTER_DEFAULT, mDevType, D3DFMT_X8R8G8B8, D3DFMT_X8R8G8B8, false));

	// 3: ¼EéÒªÇóµÄ´¦ÀúàÜÁ¦

	D3DCAPS9 caps;
	HR(md3dObject->GetDeviceCaps(D3DADAPTER_DEFAULT, mDevType, &caps));

	DWORD devBehaviorFlags = 0;
	if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
		devBehaviorFlags |= mRequestedVP;
	else
		devBehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;


	if (caps.DevCaps & D3DDEVCAPS_PUREDEVICE &&
		devBehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING)
		devBehaviorFlags |= D3DCREATE_PUREDEVICE;

	// 4: Ìûì´D3DPRESENT_PARAMETERS ½á¹¹ÌE

	md3dPP.BackBufferWidth = 0;
	md3dPP.BackBufferHeight = 0;
	md3dPP.BackBufferFormat = D3DFMT_UNKNOWN;
	md3dPP.BackBufferCount = 1;
	md3dPP.MultiSampleType = D3DMULTISAMPLE_NONE;
	md3dPP.MultiSampleQuality = 0;
	md3dPP.SwapEffect = D3DSWAPEFFECT_DISCARD;
	md3dPP.hDeviceWindow = mhMainWnd;
	md3dPP.Windowed = true;
	md3dPP.EnableAutoDepthStencil = true;
	md3dPP.AutoDepthStencilFormat = D3DFMT_D24S8;
	md3dPP.Flags = 0;
	md3dPP.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	md3dPP.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;


	// 5£º´´½¨Direct3DÉè±¸

	HR(md3dObject->CreateDevice(
		D3DADAPTER_DEFAULT, 
		mDevType,           
		mhMainWnd,          
		devBehaviorFlags,   
		&md3dPP,            
		&g_pDevice));      
}

int D3DApp::run()
{
	MSG  msg;
	msg.message = WM_NULL;

	__int64 cntsPerSec = 0;
	QueryPerformanceFrequency((LARGE_INTEGER*)&cntsPerSec);
	float secsPerCnt = 1.0f / (float)cntsPerSec;

	__int64 prevTimeStamp = 0;
	QueryPerformanceCounter((LARGE_INTEGER*)&prevTimeStamp);

	while (msg.message != WM_QUIT)
	{
		// ÓÐÏûÏ¢Ôò´¦ÀE
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			if (mAppPaused)
			{
				Sleep(20);
				continue;
			}

			if (!isDeviceLost())
			{
				__int64 currTimeStamp = 0;
				QueryPerformanceCounter((LARGE_INTEGER*)&currTimeStamp);
				float dt = (currTimeStamp - prevTimeStamp)*secsPerCnt;

				updateScene(dt);
				drawScene();

				prevTimeStamp = currTimeStamp;
			}
		}
	}
	return (int)msg.wParam;
}

LRESULT D3DApp::msgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	//³ÌÐòÔÚ×ûÐó»¯»¹ÊÇ×ûì¡»¯µÄ×´Ì¬
	static bool minOrMaxed = false;

	RECT clientRect = { 0, 0, 0, 0 };
	switch (msg)
	{

		// WM_ACTIVE µ±´°¿Ú¼¤»ûÖÍ·Ç¼¤»ûæ±²úÉE
		// ·Ç¼¤»ûæ±ÔÝÍ£
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
			mAppPaused = true;
		else
			mAppPaused = false;
		return 0;


		// WM_SIZE µ±¸Ä±ä´°¿Ú´óÐ¡Ê±²úÉE
	case WM_SIZE:
		if (g_pDevice)
		{//¸Ä±äºóÌ¨»º³åÇE
			md3dPP.BackBufferWidth = LOWORD(lParam);
			md3dPP.BackBufferHeight = HIWORD(lParam);

			if (wParam == SIZE_MINIMIZED)
			{
				mAppPaused = true;
				minOrMaxed = true;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				mAppPaused = false;
				minOrMaxed = true;
				onLostDevice();
				HR(g_pDevice->Reset(&md3dPP));
				onResetDevice();
			}
			// Restored ÊÇ·Ç×ûÐóºÍ·Ç×ûì¡µÄ×´Ì¬.
			else if (wParam == SIZE_RESTORED)
			{
				mAppPaused = false;

				if (minOrMaxed && md3dPP.Windowed)
				{
					onLostDevice();
					HR(g_pDevice->Reset(&md3dPP));
					onResetDevice();
				}
				else
				{
					//±úæ¾ÓÃ»§ÊÇÍÏ¶¯´°¿Ú±ßÔµ¸Ä±ä£¬ÎÒÃÇÔÚWM_EXITSIZEMOVE´¦ÀE
				}
				minOrMaxed = false;
			}
		}
		return 0;


		// WM_EXITSIZEMOVE µ±ÓÃ»§ÊÍ·Å±ßÔµÊ±²úÉE
	case WM_EXITSIZEMOVE:
		GetClientRect(mhMainWnd, &clientRect);
		md3dPP.BackBufferWidth = clientRect.right;
		md3dPP.BackBufferHeight = clientRect.bottom;
		onLostDevice();
		HR(g_pDevice->Reset(&md3dPP));
		onResetDevice();

		return 0;

		// WM_CLOSE µ±µã»÷ÓÒÉÏ½ÇµÄx²úÉE
	case WM_CLOSE:
		DestroyWindow(mhMainWnd);
		return 0;

		// WM_DESTROY µ±´°¿Ú±»´Ý»ÙÊ±²úÉE
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
			enableFullScreenMode(false);
		else if (wParam == 'F')
			enableFullScreenMode(true);
		return 0;
	}
	return DefWindowProc(mhMainWnd, msg, wParam, lParam);
}

void D3DApp::enableFullScreenMode(bool enable)
{
	// ÇÐ»»µ½È«ÆÁ
	if (enable)
	{

		if (!md3dPP.Windowed)
			return;

		int width = GetSystemMetrics(SM_CXSCREEN);
		int height = GetSystemMetrics(SM_CYSCREEN);

		md3dPP.BackBufferFormat = D3DFMT_X8R8G8B8;
		md3dPP.BackBufferWidth = width;
		md3dPP.BackBufferHeight = height;
		md3dPP.Windowed = false;
		SetWindowLongPtr(mhMainWnd, GWL_STYLE, WS_POPUP);
		SetWindowPos(mhMainWnd, HWND_TOP, 0, 0, width, height, SWP_NOZORDER | SWP_SHOWWINDOW);
	}
	//´°¿ÚÄ£Ê½
	else
	{
		if (md3dPP.Windowed)
			return;

		RECT R = { 0, 0, 800, 600 };
		AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
		md3dPP.BackBufferFormat = D3DFMT_UNKNOWN;
		md3dPP.BackBufferWidth = 800;
		md3dPP.BackBufferHeight = 600;
		md3dPP.Windowed = true;

		//¸Ä±ä´°¿ÚÀàÐÍµ½¸EÑºÃµÄÀàÐÍ
		SetWindowLongPtr(mhMainWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
		// SetWindowPosÓEetWindowLongPtrÅäºÏÊ¹¸Ä±äÉúÐ§£¬¸EÂ´°¿Ú³ß´E
		SetWindowPos(mhMainWnd, HWND_TOP, 100, 100, R.right, R.bottom, SWP_NOZORDER | SWP_SHOWWINDOW);
	}

	// ÖØÖÃÉè±¸
	onLostDevice();
	HR(g_pDevice->Reset(&md3dPP));
	onResetDevice();
}

bool D3DApp::isDeviceLost()
{
	//»ñÈ¡Éè±¸×´Ì¬
	HRESULT hr = g_pDevice->TestCooperativeLevel();

	//Èç¹û¶ªÊ§
	if (hr == D3DERR_DEVICELOST)
	{
		Sleep(20);
		return true;
	}
	//´úêó£¬ÍË³E
	else if (hr == D3DERR_DRIVERINTERNALERROR)
	{
		MessageBox(0, "Internal Driver Error...Exiting", 0, 0);
		PostQuitMessage(0);
		return true;
	}
	//ÖØÖÃ
	else if (hr == D3DERR_DEVICENOTRESET)
	{
		onLostDevice();
		HR(g_pDevice->Reset(&md3dPP));
		onResetDevice();
		return false;
	}
	else
		return false;
}