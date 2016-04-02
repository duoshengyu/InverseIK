#include "Camera.h"
#include "DirectInputClass.h"
#include "D3DUtil.h"

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
Camera::Camera()
{
	//Set identity matrix
	D3DXMatrixIdentity(&mWorld);
	D3DXMatrixIdentity(&mView);
	D3DXMatrixIdentity(&mProj);
	D3DXMatrixIdentity(&mViewProj);
	mPosW = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	mRightW = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
	mUpW = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	mLookW = D3DXVECTOR3(0.0f, 0.0f, 1.0f);

	//change Sensitivity
	mSpeed = 50.0f;
}
const D3DXMATRIX& Camera::world() const
{
	return mWorld;
}
const D3DXMATRIX& Camera::view() const
{
	return mView;
}

const D3DXMATRIX& Camera::proj() const
{
	return mProj;
}

const D3DXMATRIX& Camera::viewProj() const
{
	return mViewProj;
}

const D3DXVECTOR3& Camera::right() const
{
	return mRightW;
}

const D3DXVECTOR3& Camera::up() const
{
	return mUpW;
}

const D3DXVECTOR3& Camera::look() const
{
	return mLookW;
}

D3DXVECTOR3& Camera::pos()
{
	return mPosW;
}
//-----------------------------------------------------------------------------
// LookAt implement input(eye, target, up)
//-----------------------------------------------------------------------------
void Camera::lookAt(D3DXVECTOR3& pos, D3DXVECTOR3& target, D3DXVECTOR3& up)
{
	D3DXVECTOR3 L = target - pos;
	D3DXVec3Normalize(&L, &L);

	D3DXVECTOR3 R;
	D3DXVec3Cross(&R, &up, &L);
	D3DXVec3Normalize(&R, &R);

	D3DXVECTOR3 U;
	D3DXVec3Cross(&U, &L, &R);
	D3DXVec3Normalize(&U, &U);

	mPosW = pos;
	mRightW = R;
	mUpW = U;
	mLookW = L;

	buildView();


	mViewProj = mView * mProj;
}
//-----------------------------------------------------------------------------
// project matrix
//-----------------------------------------------------------------------------
void Camera::setLens(float fov, float aspect, float nearZ, float farZ)
{
	D3DXMatrixPerspectiveFovLH(&mProj, fov, aspect, nearZ, farZ);
	mViewProj = mView * mProj;
}

void Camera::setSpeed(float s)
{
	mSpeed = s;
}

void Camera::update(float dt)
{
	//press w s a d change diffrent vector
	D3DXVECTOR3 dir(0.0f, 0.0f, 0.0f);
	if (gDInput->keyDown(DIK_W))
		dir += mLookW;
	if (gDInput->keyDown(DIK_S))
		dir -= mLookW;
	if (gDInput->keyDown(DIK_D))
		dir += mRightW;
	if (gDInput->keyDown(DIK_A))
		dir -= mRightW;

	//change position of camera
	D3DXVec3Normalize(&dir, &dir);
	D3DXVECTOR3 newPos = mPosW + dir*mSpeed*dt;

	mPosW = newPos;
	//change orientation when push the right button of mouse
	float pitch = 0.0f;
	float yAngle = 0.0f;
	if (gDInput->mouseButtonDown(1))
	{
		pitch = gDInput->mouseDY() / 150.0f;
		yAngle = gDInput->mouseDX() / 150.0f;
	}


	// rotate camera with right vector
	D3DXMATRIX R;
	D3DXMatrixRotationAxis(&R, &mRightW, pitch);
	D3DXVec3TransformCoord(&mLookW, &mLookW, &R);
	D3DXVec3TransformCoord(&mUpW, &mUpW, &R);


	//rotate with y axis(turn left or right)
	D3DXMatrixRotationY(&R, yAngle);
	D3DXVec3TransformCoord(&mRightW, &mRightW, &R);
	D3DXVec3TransformCoord(&mUpW, &mUpW, &R);
	D3DXVec3TransformCoord(&mLookW, &mLookW, &R);


	// rebuild view matrix
	buildView();
	mViewProj = mView * mProj;
}
//--------------------------------------------------------------------------------------
// Desc: calculate the view matrix
//--------------------------------------------------------------------------------------
void Camera::buildView()
{
	// normalrize and cross product to get right vector
	D3DXVec3Normalize(&mLookW, &mLookW);

	D3DXVec3Cross(&mUpW, &mLookW, &mRightW);
	D3DXVec3Normalize(&mUpW, &mUpW);

	D3DXVec3Cross(&mRightW, &mUpW, &mLookW);
	D3DXVec3Normalize(&mRightW, &mRightW);

	// write view matrix

	float x = -D3DXVec3Dot(&mPosW, &mRightW);
	float y = -D3DXVec3Dot(&mPosW, &mUpW);
	float z = -D3DXVec3Dot(&mPosW, &mLookW);

	mView(0, 0) = mRightW.x;
	mView(1, 0) = mRightW.y;
	mView(2, 0) = mRightW.z;
	mView(3, 0) = x;

	mView(0, 1) = mUpW.x;
	mView(1, 1) = mUpW.y;
	mView(2, 1) = mUpW.z;
	mView(3, 1) = y;

	mView(0, 2) = mLookW.x;
	mView(1, 2) = mLookW.y;
	mView(2, 2) = mLookW.z;
	mView(3, 2) = z;

	mView(0, 3) = 0.0f;
	mView(1, 3) = 0.0f;
	mView(2, 3) = 0.0f;
	mView(3, 3) = 1.0f;
}