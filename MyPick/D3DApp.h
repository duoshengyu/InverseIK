//------------------------------------------------------------------------------
//Simple directx demo application class.  
//You can create your application simply derive from App class
//You need to overwrite framework methods.
//------------------------------------------------------------------------------

#ifndef D3DAPP_H
#define D3DAPP_H

#include "d3dUtil.h"
#include <string>

class D3DApp
{
public:
	D3DApp(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	virtual ~D3DApp();

	HINSTANCE getAppInst();
	HWND      getMainWnd();

	// frame functions
	virtual bool checkDeviceCaps()     { return true; }
	virtual void onLostDevice()        {}
	virtual void onResetDevice()       {}
	virtual void updateScene(float dt) {}
	virtual void drawScene()           {}

	// initial functions you also can rewrite it
	virtual void initMainWindow();
	virtual void initDirect3D();
	virtual int run();
	virtual LRESULT msgProc(UINT msg, WPARAM wParam, LPARAM lParam);

	void enableFullScreenMode(bool enable);
	bool isDeviceLost();

protected:

	std::string		mMainWndCaption;//window name
	D3DDEVTYPE		mDevType;//device type
	DWORD			mRequestedVP;
	
	HINSTANCE             mhAppInst;//process instant
	HWND                  mhMainWnd;//handle of window
	IDirect3D9*           md3dObject;//Direct3D9
	bool                  mAppPaused;
	D3DPRESENT_PARAMETERS md3dPP;//device describe
};

// global variable to easy access
extern D3DApp* g_d3dApp;
extern IDirect3DDevice9* g_pDevice;

#endif 