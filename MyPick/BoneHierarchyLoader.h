//------------------------------------------------------------------------------
//BoneHierarchyLoader from D3D samples, a little modified by Yu Duosheng
//------------------------------------------------------------------------------
#ifndef BoneHierarchyLoader_H
#define BoneHierarchyLoader_H


#include <vector>


#include <d3d9.h>
#include <d3dx9.h>

class Character;

struct Bone : public D3DXFRAME
{
	D3DXMATRIX CombinedTransformationMatrix; //transform matrix
	D3DXMATRIX curWorld;
	LPD3DXMESH pCylinder = NULL;		//geometry point
	bool isPick = false;
};

struct BoneMesh : public D3DXMESHCONTAINER
{
	LPDIRECT3DTEXTURE9*  ppTextures;            //Texture array
	LPD3DXMESH           pWorkingMesh;          //work mesh
	DWORD                NumAttributeGroups;    //number of bone
	DWORD                NumInfl;               //how much a joint can influence bones
	LPD3DXBUFFER         pBoneCombinationBuf;   //bone link list
	D3DXMATRIX**         ppBoneMatrixPtrs;      //Provides index to bone matrix lookup
	D3DXMATRIX*          pBoneOffsetMatrices;   //bone`s first transform matrix
	DWORD                NumPaletteEntries;     //how much bones can influence a joint
	//D3DXMATRIX*          currentBoneMatrices;   //curren matrix of bone
};

class BoneHierarchyLoader : public ID3DXAllocateHierarchy
{
public:
	// 回调创建骨头
	//call back function to create bone
	STDMETHOD(CreateFrame)(THIS_ LPCSTR Name, LPD3DXFRAME *ppNewFrame);
	// 回调创建纹理（蒙皮）
	//call back function to create textrue(skining)
	STDMETHOD(CreateMeshContainer)(THIS_ LPCSTR Name, CONST D3DXMESHDATA * pMeshData,
		CONST D3DXMATERIAL * pMaterials, CONST D3DXEFFECTINSTANCE * pEffectInstances,
		DWORD NumMaterials, CONST DWORD * pAdjacency, LPD3DXSKININFO pSkinInfo,
		LPD3DXMESHCONTAINER * ppNewMeshContainer);

	STDMETHOD(DestroyFrame)(THIS_ LPD3DXFRAME pFrameToFree);

	STDMETHOD(DestroyMeshContainer)(THIS_ LPD3DXMESHCONTAINER pMeshContainerToFree);

	BoneHierarchyLoader();

	//设置方法
	//set method
	HRESULT SetMA(Character *pMA);

private:
	Character *m_pMA;
};
#endif