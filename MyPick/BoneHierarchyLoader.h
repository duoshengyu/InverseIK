
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
	LPDIRECT3DTEXTURE9*  ppTextures;            //��������
	LPD3DXMESH           pWorkingMesh;          //work����
	DWORD                NumAttributeGroups;    //��������
	DWORD                NumInfl;               //ÿ������������Ӱ����ٹ���
	LPD3DXBUFFER         pBoneCombinationBuf;   //������ϱ�
	D3DXMATRIX**         ppBoneMatrixPtrs;      //Provides index to bone matrix lookup
	D3DXMATRIX*          pBoneOffsetMatrices;   //��Ź����ĳ�ʼ�任����
	DWORD                NumPaletteEntries;     //����ж��ٹ�������Ӱ��һ����
	//D3DXMATRIX*          currentBoneMatrices;   //��Ź�����ǰ����
};

class BoneHierarchyLoader : public ID3DXAllocateHierarchy
{
public:
	// �ص�������ͷ
	STDMETHOD(CreateFrame)(THIS_ LPCSTR Name, LPD3DXFRAME *ppNewFrame);
	// �ص�����������Ƥ��
	STDMETHOD(CreateMeshContainer)(THIS_ LPCSTR Name, CONST D3DXMESHDATA * pMeshData,
		CONST D3DXMATERIAL * pMaterials, CONST D3DXEFFECTINSTANCE * pEffectInstances,
		DWORD NumMaterials, CONST DWORD * pAdjacency, LPD3DXSKININFO pSkinInfo,
		LPD3DXMESHCONTAINER * ppNewMeshContainer);

	STDMETHOD(DestroyFrame)(THIS_ LPD3DXFRAME pFrameToFree);

	STDMETHOD(DestroyMeshContainer)(THIS_ LPD3DXMESHCONTAINER pMeshContainerToFree);

	BoneHierarchyLoader();

	//���÷���
	HRESULT SetMA(Character *pMA);

private:
	Character *m_pMA;
};
#endif