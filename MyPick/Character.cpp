#include "Character.h"
#include <fstream>
#include "D3DUtil.h"
using namespace std;
extern IDirect3DDevice9* g_pDevice;
extern ofstream g_debug;
//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
Character::Character()
{
	m_pRootBone = NULL;
	m_pAnimControl = NULL;
	m_pDevice = NULL;
	m_pEffect = NULL;

}

Character::~Character()
{
	BoneHierarchyLoader boneHierarchy;
	boneHierarchy.DestroyFrame(m_pRootBone);
	if (m_pAnimControl)m_pAnimControl->Release();
}
//-----------------------------------------------------------------------------
//Load character from filename
//-----------------------------------------------------------------------------
void Character::Load(char fileName[])
{
	BoneHierarchyLoader boneHierarchy;
	boneHierarchy.SetMA(this);

	if (FAILED(D3DXLoadMeshHierarchyFromX(fileName, D3DXMESH_MANAGED,
		m_pDevice, &boneHierarchy,
		NULL, &m_pRootBone, &m_pAnimControl)))
		g_debug << "load - FAILED \n";

	SetupBoneMatrixPointers((Bone*)m_pRootBone);

	D3DXVECTOR3 vCenter;
	D3DXFrameCalculateBoundingSphere(m_pRootBone, &vCenter, &m_fBoundingRadius);

	//update all bone
	D3DXMATRIX i;
	D3DXMatrixIdentity(&i);
	UpdateMatrices((Bone*)m_pRootBone, &i);

	//InitBoneCylinder((Bone*)m_pRootBone, NULL);
}
//-----------------------------------------------------------------------------
// Name: CreateBox
// Desc: create a fake bone(a box) rotate and move to the center of bone 
//-----------------------------------------------------------------------------
void Character::CreateBoneBox(Bone* parent, Bone *bone, D3DXVECTOR3 size)
{
	HR(D3DXCreateBox(g_pDevice, size.x, size.y, size.z, &parent->pCylinder, NULL));
	D3DXMATRIX w1 = bone->CombinedTransformationMatrix;
	D3DXMATRIX w2 = parent->CombinedTransformationMatrix;

	//Extraction Trandformation
	D3DXVECTOR3 thisBone = D3DXVECTOR3(w1._41, w1._42, w1._43);
	D3DXVECTOR3 ParentBone = D3DXVECTOR3(w2._41, w2._42, w2._43);
	//D3DXCreateCylinder(g_pDevice, D3DXVec3Length(&(thisBone - ParentBone)) / 8, D3DXVec3Length(&(thisBone - ParentBone)) / 10, D3DXVec3Length(&(thisBone - ParentBone)), 4, 1, &parent->pCylinder, 0);
	D3DXVECTOR3 all = thisBone + ParentBone;
	D3DXVECTOR3 sub = thisBone - ParentBone;
	D3DXVECTOR3 zAxis = D3DXVECTOR3(0, 0, 1);
	D3DXVECTOR3 three = sub - zAxis;
	float wantedJointAngle = 0.0f;
	float distSub = D3DXVec3Length(&sub);
	float distZAxis = D3DXVec3Length(&zAxis);
	float distThree = D3DXVec3Length(&three);
	float cosAngle = (distSub * distSub + distZAxis * distZAxis - distThree * distThree) / (2.0f * distSub * distZAxis);
	wantedJointAngle = acosf(cosAngle);

	D3DXVECTOR3 rotAxis;
	D3DXVec3Cross(&rotAxis, &sub, &zAxis);
	D3DXMATRIX w3, w4;
	D3DXMatrixTranslation(&w3, all.x / 2, all.y / 2, all.z / 2);
	D3DXMatrixRotationAxis(&w4, &rotAxis, -wantedJointAngle);
	parent->curWorld = w4 * w3;
}

//-----------------------------------------------------------------------------
// Name: DestroyFrame()
// Desc: Create a pseudo-bones throughout the skeleton
//-----------------------------------------------------------------------------
HRESULT Character::InitBoneCylinder(Bone* m_pRootBone, Bone *parent)
{
	//bone reference
	Bone* Head_End = (Bone*)D3DXFrameFind(m_pRootBone, "Bone05");
	Bone* Head = (Bone*)D3DXFrameFind(m_pRootBone, "Head");
	Bone* Neck = (Bone*)D3DXFrameFind(m_pRootBone, "Bone03");
	Bone* Pelvis = (Bone*)D3DXFrameFind(m_pRootBone, "Bone01");
	Bone* Spine = (Bone*)D3DXFrameFind(m_pRootBone, "Bone02");
	Bone* R_Shoulder = (Bone*)D3DXFrameFind(m_pRootBone, "Bone16");
	Bone* L_Shoulder = (Bone*)D3DXFrameFind(m_pRootBone, "Bone19");
	Bone* U_R_Arm = (Bone*)D3DXFrameFind(m_pRootBone, "Bone17");
	Bone* U_L_Arm = (Bone*)D3DXFrameFind(m_pRootBone, "Bone20");
	Bone* L_R_Arm = (Bone*)D3DXFrameFind(m_pRootBone, "Bone18");
	Bone* L_L_Arm = (Bone*)D3DXFrameFind(m_pRootBone, "Bone21");
	Bone* R_Hand = (Bone*)D3DXFrameFind(m_pRootBone, "Bone22");
	Bone* L_Hand = (Bone*)D3DXFrameFind(m_pRootBone, "Bone27");

	Bone* R_Pelvis = (Bone*)D3DXFrameFind(m_pRootBone, "Bone06");
	Bone* L_Pelvis = (Bone*)D3DXFrameFind(m_pRootBone, "Bone09");
	Bone* R_Thigh = (Bone*)D3DXFrameFind(m_pRootBone, "Bone07");
	Bone* L_Thigh = (Bone*)D3DXFrameFind(m_pRootBone, "Bone10");
	Bone* R_Calf = (Bone*)D3DXFrameFind(m_pRootBone, "Bone08");
	Bone* L_Calf = (Bone*)D3DXFrameFind(m_pRootBone, "Bone11");
	Bone* R_Foot = (Bone*)D3DXFrameFind(m_pRootBone, "Bone14");
	Bone* L_Foot = (Bone*)D3DXFrameFind(m_pRootBone, "Bone12");
	Bone* R_Foot_End = (Bone*)D3DXFrameFind(m_pRootBone, "Bone15");
	Bone* L_Foot_End = (Bone*)D3DXFrameFind(m_pRootBone, "Bone13");

	//Head
	CreateBoneBox(Head, Head_End, D3DXVECTOR3(0.2f, 0.2f, 0.3f));
	//CreateBoneBox(Head, Head_End, D3DXVECTOR3(1.0f, 1.0f, 1.0f));
	//Right Arm
	CreateBoneBox(U_R_Arm, L_R_Arm, D3DXVECTOR3(0.12f, 0.12f, 0.3f));
	CreateBoneBox(L_R_Arm, R_Hand, D3DXVECTOR3(0.12f, 0.12f, 0.3f));

	//Left Arm
	CreateBoneBox(U_L_Arm, L_L_Arm, D3DXVECTOR3(0.12f, 0.12f, 0.3f));
	CreateBoneBox(L_L_Arm, L_Hand, D3DXVECTOR3(0.12f, 0.12f, 0.3f));

	//Spine
	CreateBoneBox(Spine, Neck, D3DXVECTOR3(0.15f, 0.35f, 0.5f));

	//Right Leg
	CreateBoneBox(R_Thigh, R_Calf, D3DXVECTOR3(0.15f, 0.15f, 0.4f));
	CreateBoneBox(R_Calf, R_Foot, D3DXVECTOR3(0.14f, 0.14f, 0.4f));
	CreateBoneBox(R_Foot, R_Foot_End, D3DXVECTOR3(0.06f, 0.12f, 0.15f));

	//Left Leg
	CreateBoneBox(L_Thigh, L_Calf, D3DXVECTOR3(0.15f, 0.15f, 0.4f));
	CreateBoneBox(L_Calf, L_Foot, D3DXVECTOR3(0.14f, 0.14f, 0.4f));
	CreateBoneBox(L_Foot, L_Foot_End, D3DXVECTOR3(0.06f, 0.12f, 0.15f));
	return S_OK;
}
//--------------------------------------------------------------------------------------
// Desc: ÉèÖÃ¹Ç÷À¾ØÕó
//--------------------------------------------------------------------------------------
void Character::SetupBoneMatrixPointers(Bone *bone)
{
	if (bone->pMeshContainer != NULL)
	{
		BoneMesh *boneMesh = (BoneMesh*)bone->pMeshContainer;

		if (boneMesh->pSkinInfo != NULL)
		{
			int NumBones = boneMesh->pSkinInfo->GetNumBones();
			boneMesh->ppBoneMatrixPtrs = new D3DXMATRIX*[NumBones];

			for (int i = 0; i < NumBones; i++)
			{
				Bone *b = (Bone*)D3DXFrameFind(m_pRootBone, boneMesh->pSkinInfo->GetBoneName(i));
				if (b != NULL)boneMesh->ppBoneMatrixPtrs[i] = &b->CombinedTransformationMatrix;
				else boneMesh->ppBoneMatrixPtrs[i] = NULL;
			}
		}
	}

	if (bone->pFrameSibling != NULL)SetupBoneMatrixPointers((Bone*)bone->pFrameSibling);
	if (bone->pFrameFirstChild != NULL)SetupBoneMatrixPointers((Bone*)bone->pFrameFirstChild);
}

void Character::SetPose(D3DXMATRIX world)
{
	UpdateMatrices((Bone*)m_pRootBone, &world);
}

void Character::UpdateMatrices(Bone* bone, D3DXMATRIX *parentMatrix)
{
	if (bone == NULL)return;

	D3DXMatrixMultiply(&bone->CombinedTransformationMatrix,
		&bone->TransformationMatrix,
		parentMatrix);

	if (bone->pFrameSibling)UpdateMatrices((Bone*)bone->pFrameSibling, parentMatrix);
	if (bone->pFrameFirstChild)UpdateMatrices((Bone*)bone->pFrameFirstChild, &bone->CombinedTransformationMatrix);
}

//--------------------------------------------------------------------------------------
// Desc: draw fake bone(geometry)
//--------------------------------------------------------------------------------------
void Character::DrawBoneCylinder(Bone* bone, Bone *parent, D3DXMATRIX world)
{
	if (parent != NULL && parent->pCylinder != NULL)
	{
		{
			LPD3DXMESH pMesh;
			parent->pCylinder->CloneMeshFVF(D3DXMESH_MANAGED, D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1, g_pDevice, &pMesh);
			parent->curWorld = world * parent->curWorld;
			m_pEffect->SetTechnique("RenderScene");
			m_pEffect->SetMatrix("g_mWorld", &(parent->curWorld));
			m_pDevice->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1);
			//m_pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);

			UINT cPasses;
			m_pEffect->Begin(&cPasses, 0);
			for (UINT p = 0; p < cPasses; ++p)
			{
				m_pEffect->BeginPass(p);
				pMesh->DrawSubset(0);
				m_pEffect->EndPass();
			}
			m_pEffect->End();
			SAFE_RELEASE(pMesh);
		}
	}

	if (bone->pFrameSibling)DrawBoneCylinder((Bone*)bone->pFrameSibling, parent, world);
	if (bone->pFrameFirstChild)DrawBoneCylinder((Bone*)bone->pFrameFirstChild, bone, world);
}
//--------------------------------------------------------------------------------------
// Desc: draw character
//--------------------------------------------------------------------------------------
void Character::Render(Bone* pFrame)
{
	Bone *pF = pFrame;
	if (pF == NULL)pF = (Bone*)m_pRootBone;
	if (pF->pMeshContainer)
	{
		BoneMesh* pMC = (BoneMesh*)pF->pMeshContainer;
		D3DXMATRIX mx;

		if (pMC->pSkinInfo == NULL)
			return;

		//Get skeleton combination information
		LPD3DXBONECOMBINATION pBC = (LPD3DXBONECOMBINATION)(pMC->pBoneCombinationBuf->GetBufferPointer());
		DWORD dwAttrib, dwPalEntry;

		//For each palette
		for (dwAttrib = 0; dwAttrib < pMC->NumAttributeGroups; ++dwAttrib)
		{
			//Apply of the parent transform matrix 
			for (dwPalEntry = 0; dwPalEntry < pMC->NumPaletteEntries; ++dwPalEntry)
			{
				DWORD dwMatrixIndex = pBC[dwAttrib].BoneId[dwPalEntry];
				if (dwMatrixIndex != UINT_MAX)
					D3DXMatrixMultiply(&m_amxWorkingPalette[dwPalEntry],
					&(pMC->pBoneOffsetMatrices[dwMatrixIndex]),
					pMC->ppBoneMatrixPtrs[dwMatrixIndex]);
			}

			//sent to effect
			m_pEffect->SetMatrixArray("amPalette",
				m_amxWorkingPalette,
				pMC->NumPaletteEntries);

			m_pEffect->SetTexture("g_txScene", pMC->ppTextures[pBC[dwAttrib].AttribId]);

			m_pEffect->SetInt("CurNumBones", pMC->NumInfl - 1);

			// set technique
			if (FAILED(m_pEffect->SetTechnique("Skinning20")))
				return;

			UINT uiPasses, uiPass;

			//draw
			m_pEffect->Begin(&uiPasses, 0 /*D3DXFX_DONOTSAVESTATE*/);
			for (uiPass = 0; uiPass < uiPasses; ++uiPass)
			{
				m_pEffect->BeginPass(uiPass);
				pMC->pWorkingMesh->DrawSubset(dwAttrib);
				m_pEffect->EndPass();
			}
			m_pEffect->End();
		}
	}

	if (pF->pFrameSibling)
		Render((Bone*)pF->pFrameSibling);

	if (pF->pFrameFirstChild)
		Render((Bone*)pF->pFrameFirstChild);
}

//-----------------------------------------------------------------------------
// Desc: get effect point
//-----------------------------------------------------------------------------
LPD3DXEFFECT Character::GetEffect()
{
	if (m_pEffect)
		m_pEffect->AddRef();

	return m_pEffect;
}
//-----------------------------------------------------------------------------
// Desc:get device
//-----------------------------------------------------------------------------
LPDIRECT3DDEVICE9 Character::GetDevice()
{
	m_pDevice->AddRef();
	return m_pDevice;
}
HRESULT Character::Setup(LPDIRECT3DDEVICE9 pDevice)
{
	//set device
	m_pDevice = pDevice;
	m_pDevice->AddRef();

	//read Effect framework from file
	ID3DXBuffer *pErrorMsgs = NULL;
	HR(D3DXCreateEffectFromFile(m_pDevice, "resources/fx/MultiAnimation.fx", NULL, NULL, D3DXSHADER_DEBUG, NULL, &m_pEffect, &pErrorMsgs));
	return S_OK;
}

D3DXMATRIX Character::GetWorldTransform()
{
	return m_mxWorld;
}


void Character::SetWorldTransform(const D3DXMATRIX* pmxWorld)
{
	m_mxWorld = *pmxWorld;
}

float Character::GetBoundingRadius()
{
	return m_fBoundingRadius;
}

void Character::GetAnimations(vector<string> &animations)
{
	ID3DXAnimationSet *anim = NULL;

	for (int i = 0; i<(int)m_pAnimControl->GetMaxNumAnimationSets(); i++)
	{
		anim = NULL;
		m_pAnimControl->GetAnimationSet(i, &anim);

		if (anim != NULL)
		{
			animations.push_back(anim->GetName());
			anim->Release();
		}
	}
}

ID3DXAnimationController* Character::GetController()
{
	ID3DXAnimationController* newAnimController = NULL;

	if (m_pAnimControl != NULL)
	{
		m_pAnimControl->CloneAnimationController(m_pAnimControl->GetMaxNumAnimationOutputs(),
			m_pAnimControl->GetMaxNumAnimationSets(),
			m_pAnimControl->GetMaxNumTracks(),
			m_pAnimControl->GetMaxNumEvents(),
			&newAnimController);
	}

	return newAnimController;
}

D3DXFRAME* Character::GetBone(string name)
{
	return D3DXFrameFind(m_pRootBone, name.c_str());
}