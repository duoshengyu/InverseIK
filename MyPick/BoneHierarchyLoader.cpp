#include "Character.h"
#pragma warning(disable: 4995)
#pragma warning(default: 4995)
#pragma warning(disable:4996)
extern IDirect3DDevice9* g_pDevice;
//--------------------------------------------------------------------------------------
// Desc: Ϊ�������������Ƶ��ַ��������ڴ�
//--------------------------------------------------------------------------------------
HRESULT AllocateName(LPCSTR Name, LPSTR* pNewName)
{
	UINT cbLength;

	if (Name != NULL)
	{
		cbLength = (UINT)strlen(Name) + 1;
		*pNewName = new CHAR[cbLength];
		memcpy(*pNewName, Name, cbLength * sizeof(CHAR));
	}
	else
	{
		*pNewName = NULL;
	}

	return S_OK;
}

//-----------------------------------------------------------------------------
// ���캯��
//-----------------------------------------------------------------------------
BoneHierarchyLoader::BoneHierarchyLoader() : m_pMA(NULL)
{
}
//-----------------------------------------------------------------------------
// Desc: ����m_pMA���ڻ�ȡ��ɫ��
//-----------------------------------------------------------------------------
HRESULT BoneHierarchyLoader::SetMA(THIS_ Character *pMA)
{
	m_pMA = pMA;

	return S_OK;
}
//--------------------------------------------------------------------------------------
// Desc: �������, �����Ƿ����ڴ�ͳ�ʼ��,��û�ж����Ա������ʵ�ֵ
//--------------------------------------------------------------------------------------
HRESULT BoneHierarchyLoader::CreateFrame(LPCSTR Name, LPD3DXFRAME* ppNewFrame)
{
	HRESULT hr = S_OK;
	Bone *pFrame;

	*ppNewFrame = NULL;

	// Ϊ���ָ������
	pFrame = new Bone;  // ������ܽṹ����
	if (FAILED(AllocateName(Name, (LPSTR*)&pFrame->Name)))
	{
		delete pFrame;
		return hr;
	}

	// ��ʼ��D3DXFRAME_DERIVED�ṹ������Ա����
	D3DXMatrixIdentity(&pFrame->TransformationMatrix);
	D3DXMatrixIdentity(&pFrame->CombinedTransformationMatrix);

	pFrame->pMeshContainer = NULL;
	pFrame->pFrameSibling = NULL;
	pFrame->pFrameFirstChild = NULL;
	pFrame->pCylinder = NULL;

	*ppNewFrame = pFrame;
	pFrame = NULL;

	return hr;
}

//-----------------------------------------------------------------------------
// Desc: ��D3DX���õ���ȡһ���Ǽܵ���Ƥ����
//-----------------------------------------------------------------------------
HRESULT BoneHierarchyLoader::CreateMeshContainer(THIS_
	LPCSTR Name,
	CONST D3DXMESHDATA *pMeshData,
	CONST D3DXMATERIAL *pMaterials,
	CONST D3DXEFFECTINSTANCE *pEffectInstances,
	DWORD NumMaterials,
	CONST DWORD *pAdjacency,
	LPD3DXSKININFO pSkinInfo,
	LPD3DXMESHCONTAINER *ppNewMeshContainer)
{

	*ppNewMeshContainer = NULL;

	LPDIRECT3DDEVICE9 pd3dDevice = NULL;
	LPD3DXMESH pMesh = NULL;
	pMesh = pMeshData->pMesh;
	pMesh->GetDevice(&pd3dDevice);

	HRESULT hr = S_OK;

	BoneMesh * pMC = new BoneMesh;
	if (pMC == NULL)
	{
		hr = E_OUTOFMEMORY; goto e_Exit;
	}

	ZeroMemory(pMC, sizeof(BoneMesh));

	//�����һ����̬�����˳�
	if (pSkinInfo == NULL)
	{
		hr = S_OK;
		goto e_Exit;
	}

	// ֻ֧��mesh type
	if (pMeshData->Type != D3DXMESHTYPE_MESH)
	{
		hr = E_FAIL; goto e_Exit;
	}

	if (Name)
		AllocateName(Name, &pMC->Name);


	//�ȸ�������
	pMC->MeshData.Type = pMeshData->Type;
	pMC->MeshData.pMesh = pMeshData->pMesh;
	pMC->MeshData.pMesh->AddRef();

	//�����ڽ���Ϣ
	{
		DWORD dwNumFaces = pMC->MeshData.pMesh->GetNumFaces();//����
		pMC->pAdjacency = new DWORD[3 * dwNumFaces];
		if (pMC->pAdjacency == NULL)
		{
			hr = E_OUTOFMEMORY; goto e_Exit;
		}

		CopyMemory(pMC->pAdjacency, pAdjacency, 3 * sizeof(DWORD)* dwNumFaces);
	}

	// ����Effectָ��
	pMC->pEffects = NULL;

	// ��ֵ����
	pMC->NumMaterials = max(1, NumMaterials);
	pMC->pMaterials = new D3DXMATERIAL[pMC->NumMaterials];
	pMC->ppTextures = new LPDIRECT3DTEXTURE9[pMC->NumMaterials];
	if (pMC->pMaterials == NULL || pMC->ppTextures == NULL)
	{
		hr = E_OUTOFMEMORY; goto e_Exit;
	}

	if (NumMaterials > 0)
	{
		CopyMemory(pMC->pMaterials, pMaterials, NumMaterials * sizeof(D3DXMATERIAL));
		for (DWORD i = 0; i < NumMaterials; ++i)
		{
			if (pMC->pMaterials[i].pTextureFilename)
			{
				// ��ȡ�ļ���
				char sNewPath[MAX_PATH];
				{
					strcpy(sNewPath, "resources/meshes/");
					strcat(sNewPath, pMC->pMaterials[i].pTextureFilename);
					// ����3d����
					if (FAILED(D3DXCreateTextureFromFile(pd3dDevice,
						sNewPath,
						&pMC->ppTextures[i])))
						pMC->ppTextures[i] = NULL;
				}
			}
			else
				pMC->ppTextures[i] = NULL;
		}
	}
	else    //Ĭ�ϵĲ���
	{
		ZeroMemory(&pMC->pMaterials[0].MatD3D, sizeof(D3DMATERIAL9));
		pMC->pMaterials[0].MatD3D.Diffuse.r = 0.5f;
		pMC->pMaterials[0].MatD3D.Diffuse.g = 0.5f;
		pMC->pMaterials[0].MatD3D.Diffuse.b = 0.5f;
		pMC->pMaterials[0].MatD3D.Specular = pMC->pMaterials[0].MatD3D.Diffuse;
		pMC->pMaterials[0].pTextureFilename = NULL;
	}

	//������Ƥ��Ϣ
	pMC->pSkinInfo = pSkinInfo;
	pSkinInfo->AddRef();

	// ����Ƥ��Ϣ�л�ȡ��ͷ��ƫ�����
	pMC->pBoneOffsetMatrices = new D3DXMATRIX[pSkinInfo->GetNumBones()];
	if (pMC->pBoneOffsetMatrices == NULL)
	{
		hr = E_OUTOFMEMORY; goto e_Exit;
	}
	{
		for (DWORD i = 0; i < pSkinInfo->GetNumBones(); ++i)
			pMC->pBoneOffsetMatrices[i] = *(D3DXMATRIX *)pSkinInfo->GetBoneOffsetMatrix(i);
	}

	//
	//������������С
	//
	{
		UINT iPaletteSize = 26;
		pMC->NumPaletteEntries = min(iPaletteSize, pMC->pSkinInfo->GetNumBones());
	}

	// ��ȡ��Ƥ����- �ı䶥������ݽṹ������Ȩֵ������
	hr = pMC->pSkinInfo->ConvertToIndexedBlendedMesh(pMC->MeshData.pMesh,
		D3DXMESH_MANAGED | D3DXMESHOPT_VERTEXCACHE,
		pMC->NumPaletteEntries,
		pMC->pAdjacency,
		NULL,
		NULL,
		NULL,
		&pMC->NumInfl,
		&pMC->NumAttributeGroups,
		&pMC->pBoneCombinationBuf,
		&pMC->pWorkingMesh);
	if (FAILED(hr))
		goto e_Exit;

	// ȷ����ɫ�Ĺ�������С�㹻��
	// һ����ͷ���飬����Ⱦ��ɫ������������
	if (m_pMA->m_dwWorkingPaletteSize < pMC->NumPaletteEntries)
	{
		if (m_pMA->m_amxWorkingPalette)
			delete[] m_pMA->m_amxWorkingPalette;

		m_pMA->m_dwWorkingPaletteSize = pMC->NumPaletteEntries;
		m_pMA->m_amxWorkingPalette = new D3DXMATRIX[m_pMA->m_dwWorkingPaletteSize];
		if (m_pMA->m_amxWorkingPalette == NULL)
		{
			m_pMA->m_dwWorkingPaletteSize = 0;
			hr = E_OUTOFMEMORY;
			goto e_Exit;
		}
	}
	// ȷ�������ʽ
	{
		DWORD dwOldFVF = pMC->pWorkingMesh->GetFVF();
		DWORD dwNewFVF = (dwOldFVF & D3DFVF_POSITION_MASK) | D3DFVF_NORMAL | D3DFVF_TEX1 | D3DFVF_LASTBETA_UBYTE4;
		if (dwNewFVF != dwOldFVF)
		{
			LPD3DXMESH pMesh;
			hr = pMC->pWorkingMesh->CloneMeshFVF(pMC->pWorkingMesh->GetOptions(),
				dwNewFVF,
				g_pDevice,
				&pMesh);
			if (FAILED(hr))
				goto e_Exit;

			pMC->pWorkingMesh->Release();
			pMC->pWorkingMesh = pMesh;

			//ȷ������������
			if (!(dwOldFVF & D3DFVF_NORMAL))
			{
				hr = D3DXComputeNormals(pMC->pWorkingMesh, NULL);
				if (FAILED(hr))
					goto e_Exit;
			}
		}
	}

	//������������
	D3DVERTEXELEMENT9 pDecl[MAX_FVF_DECL_SIZE];
	D3DVERTEXELEMENT9 * pDeclCur;
	hr = pMC->pWorkingMesh->GetDeclaration(pDecl);
	if (FAILED(hr))
		goto e_Exit;

	pDeclCur = pDecl;
	while (pDeclCur->Stream != 0xff)
	{
		if ((pDeclCur->Usage == D3DDECLUSAGE_BLENDINDICES) && (pDeclCur->UsageIndex == 0))
			pDeclCur->Type = D3DDECLTYPE_D3DCOLOR;
		pDeclCur++;
	}

	hr = pMC->pWorkingMesh->UpdateSemantics(pDecl);
	if (FAILED(hr))
		goto e_Exit;

e_Exit:

	if (FAILED(hr))
	{
		if (pMC)
			DestroyMeshContainer(pMC);
	}
	else
		* ppNewMeshContainer = pMC;

	return hr;
}


//-----------------------------------------------------------------------------
// Name: DestroyFrame()
//-----------------------------------------------------------------------------
HRESULT BoneHierarchyLoader::DestroyFrame(THIS_ LPD3DXFRAME pFrameToFree)
{
	Bone * pFrame = (Bone *)pFrameToFree;

	if (pFrame->Name)
		delete[] pFrame->Name;

	delete pFrame;

	return S_OK;
}




//-----------------------------------------------------------------------------
// Name: DestroyMeshContainer()
//-----------------------------------------------------------------------------
HRESULT BoneHierarchyLoader::DestroyMeshContainer(THIS_ LPD3DXMESHCONTAINER pMeshContainerToFree)
{
	BoneMesh * pMC = (BoneMesh *)pMeshContainerToFree;

	if (pMC->Name)
		delete[] pMC->Name;

	if (pMC->MeshData.pMesh)
		pMC->MeshData.pMesh->Release();

	if (pMC->pAdjacency)
		delete[] pMC->pAdjacency;

	if (pMC->pMaterials)
		delete[] pMC->pMaterials;

	for (DWORD i = 0; i < pMC->NumMaterials; ++i)
	{
		if (pMC->ppTextures[i])
			pMC->ppTextures[i]->Release();
	}

	if (pMC->ppTextures)
		delete[] pMC->ppTextures;

	if (pMC->pSkinInfo)
		pMC->pSkinInfo->Release();

	if (pMC->pBoneOffsetMatrices)
		delete[] pMC->pBoneOffsetMatrices;

	if (pMC->pWorkingMesh)
	{
		pMC->pWorkingMesh->Release();
		pMC->pWorkingMesh = NULL;
	}

	pMC->NumPaletteEntries = 0;
	pMC->NumInfl = 0;
	pMC->NumAttributeGroups = 0;

	if (pMC->pBoneCombinationBuf)
	{
		pMC->pBoneCombinationBuf->Release();
		pMC->pBoneCombinationBuf = NULL;
	}

	if (pMC->ppBoneMatrixPtrs)
		delete[] pMC->ppBoneMatrixPtrs;

	delete pMeshContainerToFree;

	return S_OK;
}