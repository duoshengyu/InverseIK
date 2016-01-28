//=============================================================================
// Direct3D应用类提供一个程序的基本框架和接口.
//=============================================================================

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

	// 框架方法，根据需要重写
	virtual bool checkDeviceCaps()     { return true; }
	virtual void onLostDevice()        {}
	virtual void onResetDevice()       {}
	virtual void updateScene(float dt) {}
	virtual void drawScene()           {}

	// 默认的窗口创建过程，可重写
	// direct3D设备的创建, 窗口消息处理函数, 消息循环.
	virtual void initMainWindow();
	virtual void initDirect3D();
	virtual int run();
	virtual LRESULT msgProc(UINT msg, WPARAM wParam, LPARAM lParam);

	void enableFullScreenMode(bool enable);
	bool isDeviceLost();

protected:

	std::string mMainWndCaption;//窗口名称
	D3DDEVTYPE  mDevType;//设备类型
	DWORD       mRequestedVP;//设备功能要求
	
	HINSTANCE             mhAppInst;//程序实例
	HWND                  mhMainWnd;//窗口句柄
	IDirect3D9*           md3dObject;//Direct3D9
	bool                  mAppPaused;
	D3DPRESENT_PARAMETERS md3dPP;//设备描述
};

// 全局变量以便获取
extern D3DApp* g_d3dApp;
extern IDirect3DDevice9* g_pDevice;

#endif 