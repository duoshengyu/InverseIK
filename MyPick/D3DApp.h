//=============================================================================
// Direct3DӦ�����ṩһ������Ļ�����ܺͽӿ�.
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

	// ��ܷ�����������Ҫ��д
	virtual bool checkDeviceCaps()     { return true; }
	virtual void onLostDevice()        {}
	virtual void onResetDevice()       {}
	virtual void updateScene(float dt) {}
	virtual void drawScene()           {}

	// Ĭ�ϵĴ��ڴ������̣�����д
	// direct3D�豸�Ĵ���, ������Ϣ������, ��Ϣѭ��.
	virtual void initMainWindow();
	virtual void initDirect3D();
	virtual int run();
	virtual LRESULT msgProc(UINT msg, WPARAM wParam, LPARAM lParam);

	void enableFullScreenMode(bool enable);
	bool isDeviceLost();

protected:

	std::string mMainWndCaption;//��������
	D3DDEVTYPE  mDevType;//�豸����
	DWORD       mRequestedVP;//�豸����Ҫ��
	
	HINSTANCE             mhAppInst;//����ʵ��
	HWND                  mhMainWnd;//���ھ��
	IDirect3D9*           md3dObject;//Direct3D9
	bool                  mAppPaused;
	D3DPRESENT_PARAMETERS md3dPP;//�豸����
};

// ȫ�ֱ����Ա��ȡ
extern D3DApp* g_d3dApp;
extern IDirect3DDevice9* g_pDevice;

#endif 