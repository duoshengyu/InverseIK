#include "Character.h"
#pragma warning(disable: 4995)
#pragma warning(default: 4995)
#pragma warning(disable:4996)
extern IDirect3DDevice9* g_pDevice;
//--------------------------------------------------------------------------------------
// Desc: 为骨骼或网格名称的字符串分配内存
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
// 构造函数
//-----------------------------------------------------------------------------
BoneHierarchyLoader::BoneHierarchyLoader() : m_pMA(NULL)
{
}
//-----------------------------------------------------------------------------
// Desc: 设置m_pMA便于获取调色板
//-----------------------------------------------------------------------------
HRESULT BoneHierarchyLoader::SetMA(THIS_ Character *pMA)
{
	m_pMA = pMA;

	return S_OK;
}
//--------------------------------------------------------------------------------------
// Desc: 创建框架, 仅仅是分配内存和初始化,还没有对其成员赋予合适的值
//--------------------------------------------------------------------------------------
HRESULT BoneHierarchyLoader::CreateFrame(LPCSTR Name, LPD3DXFRAME* ppNewFrame)
{
	HRESULT hr = S_OK;
	Bone *pFrame;

	*ppNewFrame = NULL;

	// 为框架指定名称
	pFrame = new Bone;  // 创建框架结构对象
	if (FAILED(AllocateName(Name, (LPSTR*)&pFrame->Name)))
	{
		delete pFrame;
		return hr;
	}

	// 初始化D3DXFRAME_DERIVED结构其它成员变量
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
// Desc: 被D3DX调用当读取一个骨架的蒙皮网格
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

	//如果是一个静态网格退出
	if (pSkinInfo == NULL)
	{
		hr = S_OK;
		goto e_Exit;
	}

	// 只支持mesh type
	if (pMeshData->Type != D3DXMESHTYPE_MESH)
	{
		hr = E_FAIL; goto e_Exit;
	}

	if (Name)
		AllocateName(Name, &pMC->Name);


	//先复制网格
	pMC->MeshData.Type = pMeshData->Type;
	pMC->MeshData.pMesh = pMeshData->pMesh;
	pMC->MeshData.pMesh->AddRef();

	//复制邻接信息
	{
		DWORD dwNumFaces = pMC->MeshData.pMesh->GetNumFaces();//面数
		pMC->pAdjacency = new DWORD[3 * dwNumFaces];
		if (pMC->pAdjacency == NULL)
		{
			hr = E_OUTOFMEMORY; goto e_Exit;
		}

		CopyMemory(pMC->pAdjacency, pAdjacency, 3 * sizeof(DWORD)* dwNumFaces);
	}

	// 不管Effect指针
	pMC->pEffects = NULL;

	// 复值材质
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
				// 获取文件名
				char sNewPath[MAX_PATH];
				{
					strcpy(sNewPath, "resources/meshes/");
					strcat(sNewPath, pMC->pMaterials[i].pTextureFilename);
					// 创建3d纹理
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
	else    //默认的材质
	{
		ZeroMemory(&pMC->pMaterials[0].MatD3D, sizeof(D3DMATERIAL9));
		pMC->pMaterials[0].MatD3D.Diffuse.r = 0.5f;
		pMC->pMaterials[0].MatD3D.Diffuse.g = 0.5f;
		pMC->pMaterials[0].MatD3D.Diffuse.b = 0.5f;
		pMC->pMaterials[0].MatD3D.Specular = pMC->pMaterials[0].MatD3D.Diffuse;
		pMC->pMaterials[0].pTextureFilename = NULL;
	}

	//保存蒙皮信息
	pMC->pSkinInfo = pSkinInfo;
	pSkinInfo->AddRef();

	// 从蒙皮信息中获取骨头的偏离矩阵
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
	//决定工作集大小
	//
	{
		UINT iPaletteSize = 26;
		pMC->NumPaletteEntries = min(iPaletteSize, pMC->pSkinInfo->GetNumBones());
	}

	// 获取蒙皮网格- 改变顶点的数据结构，加入权值和索引
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

	// 确定角色的工作集大小足够大
	// 一个骨头数组，在渲染角色是用作工作集
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
	// 确定顶点格式
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

			//确定包含法向量
			if (!(dwOldFVF & D3DFVF_NORMAL))
			{
				hr = D3DXComputeNormals(pMC->pWorkingMesh, NULL);
				if (FAILED(hr))
					goto e_Exit;
			}
		}
	}

	//声明顶点类型
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