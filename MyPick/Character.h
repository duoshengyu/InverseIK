//------------------------------------------------------------------------------
//My character class inherit from BoneHierarchyLoader.h
//------------------------------------------------------------------------------

#ifndef Character_H
#define Character_H

#include <vector>
#include <d3d9.h>
#include <d3dx9.h>
#include <string>
#include <windows.h>
#include "D3DUtil.h"
#include "BoneHierarchyLoader.h"
using namespace std;

class Character
{
	friend class BoneHierarchyLoader;

public:
	Character();
	~Character();
	void Load(char fileName[]);
	void Render(Bone *bone);

	void SetPose(D3DXMATRIX world);
	void GetAnimations(vector<string> &animations);

	ID3DXAnimationController* GetController();

	D3DXFRAME* GetBone(string name);
	//get root bone
	Bone * getPBone(){ return (Bone*)m_pRootBone; }

	LPDIRECT3DDEVICE9   GetDevice();
	LPD3DXEFFECT        GetEffect();

	HRESULT     Setup(LPDIRECT3DDEVICE9 pDevice);

	D3DXMATRIX      GetWorldTransform();
	void            SetWorldTransform(const D3DXMATRIX* pmxWorld);

	float               GetBoundingRadius();
	void DrawBoneCylinder(Bone* bone, Bone *parent, D3DXMATRIX world);

	HRESULT InitBoneCylinder(Bone* bone, Bone *parent);
	void CreateBoneBox(Bone* parent, Bone *bone, D3DXVECTOR3 size);
	void UpdateMatrices(Bone* bone, D3DXMATRIX *parentMatrix);

private:

	void SetupBoneMatrixPointers(Bone *bone);

	D3DXFRAME *m_pRootBone;
	ID3DXAnimationController *m_pAnimControl;

	LPDIRECT3DDEVICE9 m_pDevice;
	//effect point
	LPD3DXEFFECT m_pEffect;

	D3DXMATRIX m_mxWorld;

	//get diameter of bounding ball
	float m_fBoundingRadius;
protected:
	DWORD m_dwWorkingPaletteSize;
	D3DXMATRIX* m_amxWorkingPalette;
};

#endif