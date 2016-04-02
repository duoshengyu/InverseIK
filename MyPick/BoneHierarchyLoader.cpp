#include "Character.h"
#pragma warning(disable: 4995)
#pragma warning(default: 4995)
#pragma warning(disable:4996)
extern IDirect3DDevice9* g_pDevice;
//--------------------------------------------------------------------------------------
// Desc: allocate memory
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
// Constructor
//-----------------------------------------------------------------------------
BoneHierarchyLoader::BoneHierarchyLoader() : m_pMA(NULL)
{
}
//-----------------------------------------------------------------------------
// Desc: set a character point easy to access palette
//-----------------------------------------------------------------------------
HRESULT BoneHierarchyLoader::SetMA(THIS_ Character *pMA)
{
	m_pMA = pMA;

	return S_OK;
}
//--------------------------------------------------------------------------------------
// Desc: create framework , and allocate memory ,has no initiated to Bone
//--------------------------------------------------------------------------------------
HRESULT BoneHierarchyLoader::CreateFrame(LPCSTR Name, LPD3DXFRAME* ppNewFrame)
{
	HRESULT hr = S_OK;
	Bone *pFrame;

	*ppNewFrame = NULL;

	// set a name to frame
	pFrame = new Bone;  //create
	if (FAILED(AllocateName(Name, (LPSTR*)&pFrame->Name)))
	{
		delete pFrame;
		return hr;
	}

	// initiate to identity matrix
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
// Desc: read the skinning
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

	//if static mesh break;
	if (pSkinInfo == NULL)
	{
		hr = S_OK;
		goto e_Exit;
	}

	//only support mesh type
	if (pMeshData->Type != D3DXMESHTYPE_MESH)
	{
		hr = E_FAIL; goto e_Exit;
	}

	if (Name)
		AllocateName(Name, &pMC->Name);


	//acopy mesh first
	pMC->MeshData.Type = pMeshData->Type;
	pMC->MeshData.pMesh = pMeshData->pMesh;
	pMC->MeshData.pMesh->AddRef();

	//copy link information
	{
		DWORD dwNumFaces = pMC->MeshData.pMesh->GetNumFaces();//number of faces
		pMC->pAdjacency = new DWORD[3 * dwNumFaces];
		if (pMC->pAdjacency == NULL)
		{
			hr = E_OUTOFMEMORY; goto e_Exit;
		}

		CopyMemory(pMC->pAdjacency, pAdjacency, 3 * sizeof(DWORD)* dwNumFaces);
	}

	//set effects to NULL first
	pMC->pEffects = NULL;

	//copy material
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
				// get file name
				char sNewPath[MAX_PATH];
				{
					strcpy(sNewPath, "resources/meshes/");
					strcat(sNewPath, pMC->pMaterials[i].pTextureFilename);
					// create 3d texture
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
	else    //default material
	{
		ZeroMemory(&pMC->pMaterials[0].MatD3D, sizeof(D3DMATERIAL9));
		pMC->pMaterials[0].MatD3D.Diffuse.r = 0.5f;
		pMC->pMaterials[0].MatD3D.Diffuse.g = 0.5f;
		pMC->pMaterials[0].MatD3D.Diffuse.b = 0.5f;
		pMC->pMaterials[0].MatD3D.Specular = pMC->pMaterials[0].MatD3D.Diffuse;
		pMC->pMaterials[0].pTextureFilename = NULL;
	}

	//save skinning information
	pMC->pSkinInfo = pSkinInfo;
	pSkinInfo->AddRef();

	//get transform matrix from skinning information
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
	//decide the number of work set
	//
	{
		UINT iPaletteSize = 26;
		pMC->NumPaletteEntries = min(iPaletteSize, pMC->pSkinInfo->GetNumBones());
	}

	// changing vertex data structure, add weights and index
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

	// Determine the working set of character is large enough
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
	// determine vertex type
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

			//conform contain normal
			if (!(dwOldFVF & D3DFVF_NORMAL))
			{
				hr = D3DXComputeNormals(pMC->pWorkingMesh, NULL);
				if (FAILED(hr))
					goto e_Exit;
			}
		}
	}

	//Statement vertex type
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