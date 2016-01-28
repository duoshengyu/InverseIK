#include "d3dUtil.h"
#include "DirectInputClass.h"
#include "D3DApp.h"
extern D3DApp * g_d3dApp;
DirectInput* gDInput = 0;

DirectInput::DirectInput(DWORD keyboardCoopFlags, DWORD mouseCoopFlags)
{
	ZeroMemory(mKeyboardState, sizeof(mKeyboardState));
	ZeroMemory(&mMouseState, sizeof(mMouseState));
	//��ʼ��һ��IDirectInput8�ӿڶ���
	HR(DirectInput8Create(g_d3dApp->getAppInst(), DIRECTINPUT_VERSION,
		IID_IDirectInput8, (void**)&mDInput, 0));
	//���м����豸�ĳ�ʼ��
	HR(mDInput->CreateDevice(GUID_SysKeyboard, &mKeyboard, 0));
	HR(mKeyboard->SetDataFormat(&c_dfDIKeyboard));
	HR(mKeyboard->SetCooperativeLevel(g_d3dApp->getMainWnd(), keyboardCoopFlags));
	HR(mKeyboard->Acquire());
	//��������豸�ĳ�ʼ��
	HR(mDInput->CreateDevice(GUID_SysMouse, &mMouse, 0));
	HR(mMouse->SetDataFormat(&c_dfDIMouse2));
	HR(mMouse->SetCooperativeLevel(g_d3dApp->getMainWnd(), mouseCoopFlags));
	HR(mMouse->Acquire());
}

DirectInput::~DirectInput()
{
	SAFE_RELEASE(mDInput);
	mKeyboard->Unacquire();
	mMouse->Unacquire();
	SAFE_RELEASE(mKeyboard);
	SAFE_RELEASE(mMouse);
}

void DirectInput::poll()
{
	// ��ѯ����
	HRESULT hr = mKeyboard->GetDeviceState(sizeof(mKeyboardState), (void**)&mKeyboardState);
	if (FAILED(hr))
	{
		// ��ʧ���̣��ü���״̬����Ϊ��
		ZeroMemory(mKeyboardState, sizeof(mKeyboardState));

		// ��ͼ��ȡ����
		hr = mKeyboard->Acquire();
	}

	// ��ѯ���
	hr = mMouse->GetDeviceState(sizeof(DIMOUSESTATE2), (void**)&mMouseState);
	if (FAILED(hr))
	{
		ZeroMemory(&mMouseState, sizeof(mMouseState));
		hr = mMouse->Acquire();
	}
}

bool DirectInput::keyDown(char key)
{
	return (mKeyboardState[key] & 0x80) != 0;
}

bool DirectInput::mouseButtonDown(int button)
{
	return (mMouseState.rgbButtons[button] & 0x80) != 0;
}

float DirectInput::mouseDX()
{
	return (float)mMouseState.lX;
}

float DirectInput::mouseDY()
{
	return (float)mMouseState.lY;
}

float DirectInput::mouseDZ()
{
	return (float)mMouseState.lZ;
}