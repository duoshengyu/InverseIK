
#ifndef BoneHierarchyLoader_H
#define BoneHierarchyLoader_H


#include <vector>


#include <d3d9.h>
#include <d3dx9.h>

class Character;

struct Bone : public D3DXFRAME
{
	D3DXMATRIX CombinedTransformationMatrix;
	D3DXMATRIX curWorld;
	LPD3DXMESH pCylinder = NULL;
	bool isPick = false;
};

struct BoneMesh : public D3DXMESHCONTAINER
{
	LPDIRECT3DTEXTURE9*  ppTextures;            //纹理数组
	LPD3DXMESH           pWorkingMesh;          //work网格
	DWORD                NumAttributeGroups;    //骨骼数量
	DWORD                NumInfl;               //每个顶点最多可以影响多少骨骼
	LPD3DXBUFFER         pBoneCombinationBuf;   //骨骼结合表
	D3DXMATRIX**         ppBoneMatrixPtrs;      //Provides index to bone matrix lookup
	D3DXMATRIX*          pBoneOffsetMatrices;   //存放骨骼的初始变换矩阵
	DWORD                NumPaletteEntries;     //最多有多少骨骼可以影响一个点
	//D3DXMATRIX*          currentBoneMatrices;   //存放骨骼当前矩阵
};

class BoneHierarchyLoader : public ID3DXAllocateHierarchy
{
public:
	// 回调创建骨头
	STDMETHOD(CreateFrame)(THIS_ LPCSTR Name, LPD3DXFRAME *ppNewFrame);
	// 回调创建纹理（蒙皮）
	STDMETHOD(CreateMeshContainer)(THIS_ LPCSTR Name, CONST D3DXMESHDATA * pMeshData,
		CONST D3DXMATERIAL * pMaterials, CONST D3DXEFFECTINSTANCE * pEffectInstances,
		DWORD NumMaterials, CONST DWORD * pAdjacency, LPD3DXSKININFO pSkinInfo,
		LPD3DXMESHCONTAINER * ppNewMeshContainer);

	STDMETHOD(DestroyFrame)(THIS_ LPD3DXFRAME pFrameToFree);

	STDMETHOD(DestroyMeshContainer)(THIS_ LPD3DXMESHCONTAINER pMeshContainerToFree);

	BoneHierarchyLoader();

	//设置方法
	HRESULT SetMA(Character *pMA);

private:
	Character *m_pMA;
};
#endif