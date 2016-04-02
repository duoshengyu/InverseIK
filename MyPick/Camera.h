//------------------------------------------------------------------------------
//simple camera class
//------------------------------------------------------------------------------
#ifndef CAMERA_H
#define CAMERA_H

#include <d3dx9.h>
#include "d3dUtil.h"

class Camera
{
public:
	Camera();

	const D3DXMATRIX& view() const;					//get view matrix
	const D3DXMATRIX& proj() const;					//get project matrix
	const D3DXMATRIX& viewProj() const;				//get proj * view
	const D3DXMATRIX& world() const;				//get world matrix
	const D3DXVECTOR3& right() const;
	const D3DXVECTOR3& up() const;
	const D3DXVECTOR3& look() const;

	D3DXVECTOR3& pos();

	void lookAt(D3DXVECTOR3& pos, D3DXVECTOR3& target, D3DXVECTOR3& up);
	void setLens(float fov, float aspect, float nearZ, float farZ);
	void setSpeed(float s);

	void update(float dt);

protected:
	void buildView();

protected:
	D3DXMATRIX mWorld;
	D3DXMATRIX mView;
	D3DXMATRIX mProj;
	D3DXMATRIX mViewProj;

	//camera vectors
	D3DXVECTOR3 mPosW;
	D3DXVECTOR3 mRightW;
	D3DXVECTOR3 mUpW;
	D3DXVECTOR3 mLookW;

	float mSpeed;


};

#endif 