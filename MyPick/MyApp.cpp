//------------------------------------------------------------------------------
//Simple inverse kinematic implement.  
//------------------------------------------------------------------------------
#include "D3DApp.h"
#include "windows.h"
#include "DirectInputClass.h"
#include <ctime>
#include "Character.h"
#include "InverseKinematics.h"
#include <fstream>
#include "Camera.h"
using namespace std;

ofstream g_debug("debug.txt");

extern D3DApp * g_d3dApp;
Camera* g_Camera = 0;


struct DirLight
{
	D3DXCOLOR ambient;
	D3DXCOLOR diffuse;
	D3DXCOLOR spec;
	D3DXVECTOR3 dirW;
};
struct D3DVERTEX
{
	D3DXVECTOR3 p;
	D3DXVECTOR3 n;
	FLOAT tu, tv;

	static const DWORD FVF;
};
const DWORD                 D3DVERTEX::FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;
class TriPickDemo : public D3DApp
{
public:
	TriPickDemo(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
	~TriPickDemo();
	void getAllName(Bone *bone);
	bool checkDeviceCaps();
	void onLostDevice();
	void onResetDevice();
	void updateScene(float dt);
	void drawScene();
	void getWorldPickingRay(D3DXVECTOR3& originW, D3DXVECTOR3& dirW, D3DXMATRIX matWorld);
	void checkPick(LPD3DXMESH mesh, D3DXMATRIX matWorld);
	bool checkBonePick(LPD3DXMESH mesh, D3DXMATRIX matWorld);
	D3DXVECTOR3 getMouseMoveVector(float boneZ);
	D3DXMATRIX getWorld(){ return m_matWorld; }

private:
	ID3DXEffect * m_FX;
	LPD3DXMESH  m_Mesh;
	D3DXMATRIX m_matWorld;
	DirLight m_Light;
	Character *m_pCharacter;
	InverseKinematics *m_pIK;
	ID3DXAnimationController* m_animController;
	Bone* L_Hand;
	Bone* R_Hand;
	Bone* L_Leg;
	Bone* R_Leg;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	srand(time(0));
	//create camera
	Camera camera;
	g_Camera = &camera;
	TriPickDemo app(hInstance, "MyInverseKinematicsDemo", D3DDEVTYPE_HAL, D3DCREATE_HARDWARE_VERTEXPROCESSING);
	g_d3dApp = &app;
	DirectInput di(DISCL_NONEXCLUSIVE | DISCL_FOREGROUND, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
	gDInput = &di;

	return g_d3dApp->run();
}

TriPickDemo::TriPickDemo(HINSTANCE hInstance, std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP)
	: D3DApp(hInstance, winCaption, devType, requestedVP)
{
	if (!checkDeviceCaps())
	{
		MessageBox(0, "checkDeviceCaps() Failed", 0, 0);
		PostQuitMessage(0);
	}
	//initiate our objects
	HR(D3DXCreateCylinder(g_pDevice, 1, 1, 1, 10, 5, &m_Mesh, 0));
	m_pCharacter = new Character();
	m_pCharacter->Setup(g_pDevice);
	m_pCharacter->Load("resources/meshes/soldier.x");
	m_pCharacter->InitBoneCylinder(m_pCharacter->getPBone(), NULL);
	//set static anime
	m_animController = m_pCharacter->GetController();
	ID3DXAnimationSet *anim = NULL;
	m_animController->GetAnimationSet(2, &anim);
	m_animController->SetTrackAnimationSet(0, anim);
	anim->Release();
	//initiate IK
	m_pIK = new InverseKinematics(m_pCharacter);
	//initiate camera
	D3DXVECTOR3 vFromPt = D3DXVECTOR3(0.0f, 2.0f, -3.0f);
	D3DXVECTOR3 vLookatPt = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	D3DXVECTOR3 up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	g_Camera->lookAt(vFromPt, vLookatPt, up);
	g_Camera->setSpeed(10.0f);

	D3DXMatrixIdentity(&m_matWorld);
	//read shader
	ID3DXBuffer *pErrorMsgs = NULL;
	HR(D3DXCreateEffectFromFile(g_pDevice, "Pick.fx", NULL, NULL, D3DXSHADER_DEBUG, NULL, &m_FX, &pErrorMsgs));

	//initiate lighting
	m_Light.dirW = D3DXVECTOR3(0.0f, 2.0f, -3.0f);
	D3DXVec3Normalize(&m_Light.dirW, &m_Light.dirW);
	m_Light.ambient = D3DXCOLOR(0.5f, 0.5f, 0.5f, 1.0f);
	m_Light.diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	m_Light.spec = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

	L_Hand = (Bone*)D3DXFrameFind(m_pCharacter->getPBone(), "Bone21");
	R_Hand = (Bone*)D3DXFrameFind(m_pCharacter->getPBone(), "Bone18");
	L_Leg = (Bone*)D3DXFrameFind(m_pCharacter->getPBone(), "Bone11");
	R_Leg = (Bone*)D3DXFrameFind(m_pCharacter->getPBone(), "Bone08");
	onResetDevice();
}

bool TriPickDemo::checkDeviceCaps()
{
	D3DCAPS9 caps;
	HR(g_pDevice->GetDeviceCaps(&caps));

	//check device is support vertex shader version2.0
	if (caps.VertexShaderVersion < D3DVS_VERSION(2, 0))
		return false;

	//check device is support pixel shader version2.0
	if (caps.PixelShaderVersion < D3DPS_VERSION(2, 0))
		return false;

	return true;
}

void TriPickDemo::onLostDevice()
{
	HR(m_FX->OnLostDevice());
	HR(m_pCharacter->GetEffect()->OnLostDevice());
}

void TriPickDemo::onResetDevice()
{
	HR(m_FX->OnResetDevice());
	HR(m_pCharacter->GetEffect()->OnResetDevice());
	//Aspect ratio rely on the size of back buffer 
	//Back buffer will change when user change window size, so we need to reset project matrix.
	float w = (float)md3dPP.BackBufferWidth;
	float h = (float)md3dPP.BackBufferHeight;

	g_Camera->setLens(D3DX_PI * 0.25f, w / h, 0.1f, 1000.0f);

}

void TriPickDemo::updateScene(float dt)
{
	//get input
	gDInput->poll();
	g_Camera->update(dt);
	if (gDInput->keyDown(DIK_ESCAPE))
	{
		::DestroyWindow(mhMainWnd);
		::PostQuitMessage(0);
	}
	if (m_animController != NULL)
	{
		m_animController->AdvanceTime(dt, NULL);
	}
}

void TriPickDemo::drawScene()
{

	HR(g_pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0));

	HR(g_pDevice->BeginScene());

	//rendering characters
	{
		ID3DXEffect* pMAEffect = m_pCharacter->GetEffect();

		if (pMAEffect)
		{
			pMAEffect->SetMatrix("g_mViewProj", &(g_Camera->viewProj()));
		}
		//set lighting
		D3DXVECTOR4 v(m_Light.dirW.x, m_Light.dirW.y, m_Light.dirW.z, 1.0f);
		if (pMAEffect)
		{
			pMAEffect->SetVector("lhtDir", &v);
		}
		SAFE_RELEASE(pMAEffect);
		/*/ compute reorientation matrix based on default orientation and bounding radius
		D3DXMATRIX mx, Orientation;
		//float fScale = 1.f / g_Character.GetBoundingRadius() / 7.f;
		//D3DXMatrixScaling(&mx, fScale, fScale, fScale);
		D3DXMatrixScaling(&mx, 1.0f, 1.0f, 1.0f);
		Orientation = mx;
		//D3DXMatrixRotationX(&mx, -D3DX_PI / 2.0f);
		D3DXMatrixRotationX(&mx, 0.0f);
		D3DXMatrixMultiply(&Orientation, &Orientation, &mx);
		//D3DXMatrixRotationY(&mx, D3DX_PI);
		D3DXMatrixRotationY(&mx, 0.0f);
		D3DXMatrixMultiply(&Orientation, &Orientation, &mx);

		D3DXMATRIX mxWorld;

		// compute world matrix based on pos/face
		D3DXMatrixRotationY(&mxWorld, 0.0f);
		D3DXMatrixTranslation(&mx, 0.0f, 0.0f, 0.0f);
		D3DXMatrixMultiply(&mxWorld, &mxWorld, &mx);
		D3DXMatrixMultiply(&mxWorld, &Orientation, &mxWorld);*/
		m_pCharacter->SetPose(m_matWorld);
		//check seclection
		D3DXVECTOR3  mouseMV;
		if (gDInput->mouseButtonDown(0))
		{
			if (L_Hand->isPick == false)
				L_Hand->isPick = checkBonePick(L_Hand->pCylinder, L_Hand->curWorld);

			if (R_Hand->isPick == false)
				R_Hand->isPick = checkBonePick(R_Hand->pCylinder, R_Hand->curWorld);

			if (L_Leg->isPick == false)
				L_Leg->isPick = checkBonePick(L_Leg->pCylinder, L_Leg->curWorld);

			if (R_Leg->isPick == false)
				R_Leg->isPick = checkBonePick(R_Leg->pCylinder, R_Leg->curWorld);
			//get move vector
			if (L_Hand->isPick)
			{
				mouseMV = getMouseMoveVector(1.0f);
				m_pIK->addLHMove(mouseMV);
			}
			else
				if (R_Hand->isPick)
				{
					mouseMV = getMouseMoveVector(1.0f);
					m_pIK->addRHMove(mouseMV);
				}
				else
					if (L_Leg->isPick)
					{
						mouseMV = getMouseMoveVector(1.0f);
						m_pIK->addLLMove(mouseMV);
					}
					else
						if (R_Leg->isPick)
						{
							mouseMV = getMouseMoveVector(1.0f);
							m_pIK->addRLMove(mouseMV);
						}
		}
		else
		{
			L_Hand->isPick = false;
			R_Hand->isPick = false;
			L_Leg->isPick = false;
			R_Leg->isPick = false;
		}
		// wireframe
		//g_pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		//Update IK
		if (m_pIK)
		{
			m_pIK->UpdateHeadIK();
			m_pIK->UpdateIK();
		}
		//m_pCharacter->DrawBoneCylinder(m_pCharacter->getPBone(), NULL, m_matWorld);
		m_pCharacter->Render(NULL);
	}

	HR(g_pDevice->EndScene());

	HR(g_pDevice->Present(0, 0, 0, 0));
}

//------------------------------------------------------------------------------
//get a ray
//------------------------------------------------------------------------------

void TriPickDemo::getWorldPickingRay(D3DXVECTOR3& originW, D3DXVECTOR3& dirW, D3DXMATRIX matWorld)
{
	//get coordinates in screen
	POINT s;
	GetCursorPos(&s);

	//converts to client-area coordinates
	ScreenToClient(mhMainWnd, &s);

	// get width and height

	float w = (float)md3dPP.BackBufferWidth;
	float h = (float)md3dPP.BackBufferHeight;

	D3DXMATRIX proj = g_Camera->proj();

	float x = (2.0f*s.x / w - 1.0f) / proj(0, 0);
	float y = (-2.0f*s.y / h + 1.0f) / proj(1, 1);
	//g_debug << w << h << endl;
	//start point and end point
	D3DXVECTOR3 origin(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 dir(x, y, 1.0f);

	D3DXMATRIX mWorldView = matWorld * g_Camera->view();
	D3DXMATRIX m;
	D3DXMatrixInverse(&m, NULL, &mWorldView);

	//convert to original space
	dirW.x = dir.x * m._11 + dir.y * m._21 + dir.z * m._31;
	dirW.y = dir.x * m._12 + dir.y * m._22 + dir.z * m._32;
	dirW.z = dir.x * m._13 + dir.y * m._23 + dir.z * m._33;
	originW.x = m._41;
	originW.y = m._42;
	originW.z = m._43;
}
//------------------------------------------------------------------------------
//this is a check function
//------------------------------------------------------------------------------
void TriPickDemo::checkPick(LPD3DXMESH mesh, D3DXMATRIX matWorld)
{
	HRESULT hr;

	D3DXMATRIX mWorldViewProjection;
	mWorldViewProjection = matWorld * g_Camera->viewProj();

	HR(m_FX->SetTechnique("RenderScene"));

	// send matrix to shader
	HR(m_FX->SetMatrix("g_mWorldViewProjection", &mWorldViewProjection));
	HR(m_FX->SetMatrix("g_mWorld", &matWorld));
	UINT uPasses;
	V(m_FX->Begin(&uPasses, 0));

	g_pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

	V(m_FX->BeginPass(0));
	//get select ray
	D3DXVECTOR3 originW(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 dirW(0.0f, 0.0f, 0.0f);
	if (gDInput->mouseButtonDown(0))
	{
		getWorldPickingRay(originW, dirW, matWorld);
		LPD3DXMESH pMesh;

		mesh->CloneMeshFVF(D3DXMESH_MANAGED, D3DVERTEX::FVF, g_pDevice, &pMesh);
		BOOL hit = 0;
		DWORD faceIndex = -1;
		float u = 0.0f;
		float v = 0.0f;
		float dist = 0.0f;
		ID3DXBuffer* allhits = 0;
		DWORD numHits = 0;
		HR(D3DXIntersect(pMesh, &originW, &dirW, &hit,
			&faceIndex, &u, &v, &dist, &allhits, &numHits));
		SAFE_RELEASE(allhits);
		//if hit
		if (hit)
		{
			IDirect3DVertexBuffer9* vb = 0;
			IDirect3DIndexBuffer9* ib = 0;
			HR(pMesh->GetVertexBuffer(&vb));
			HR(pMesh->GetIndexBuffer(&ib));

			HR(g_pDevice->SetIndices(ib));
			HR(g_pDevice->SetFVF(D3DVERTEX::FVF));
			HR(g_pDevice->SetStreamSource(0, vb, 0, sizeof(D3DVERTEX)));

			//render hit surface
			HR(g_pDevice->DrawIndexedPrimitive(
				D3DPT_TRIANGLELIST, 0, 0, pMesh->GetNumVertices(), faceIndex * 3, 1))

				g_pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
			SAFE_RELEASE(vb);
			SAFE_RELEASE(ib);
			SAFE_RELEASE(pMesh);
		}
	}

	HR(m_FX->EndPass());
	HR(m_FX->End());
}
bool TriPickDemo::checkBonePick(LPD3DXMESH mesh, D3DXMATRIX matWorld)
{
	D3DXVECTOR3 originW(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 dirW(0.0f, 0.0f, 0.0f);
	getWorldPickingRay(originW, dirW, matWorld);
	LPD3DXMESH pMesh;

	mesh->CloneMeshFVF(D3DXMESH_MANAGED, D3DVERTEX::FVF, g_pDevice, &pMesh);
	BOOL hit = 0;
	DWORD faceIndex = -1;
	float u = 0.0f;
	float v = 0.0f;
	float dist = 0.0f;
	ID3DXBuffer* allhits = 0;
	DWORD numHits = 0;
	HR(D3DXIntersect(pMesh, &originW, &dirW, &hit,
		&faceIndex, &u, &v, &dist, &allhits, &numHits));
	SAFE_RELEASE(allhits);

	if (hit)
	{
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
//calculate move vector, use to move our select object
//------------------------------------------------------------------------------
D3DXVECTOR3 TriPickDemo::getMouseMoveVector(float boneZ)
{
	D3DXVECTOR3 vPickRayDir;
	D3DXVECTOR3 vPickRayDir2;
	D3DXVECTOR3 vPickRayOrig;

	//is similar to check pick,but a point more

	POINT so, sd;
	GetCursorPos(&so);
	GetCursorPos(&sd);
	//add mouse move
	sd.x += gDInput->mouseDX();
	sd.y += gDInput->mouseDY();

	ScreenToClient(mhMainWnd, &so);
	ScreenToClient(mhMainWnd, &sd);

	float w = (float)md3dPP.BackBufferWidth;
	float h = (float)md3dPP.BackBufferHeight;

	D3DXMATRIX proj = g_Camera->proj();

	D3DXVECTOR3 v, vd;
	v.x = (2.0f*so.x / w - 1.0f) / proj(0, 0);
	v.y = (-2.0f*so.y / h + 1.0f) / proj(1, 1);
	v.z = 1.0f;
	v.z = boneZ;
	vd.x = (2.0f*sd.x / w - 1.0f) / proj(0, 0);
	vd.y = (-2.0f*sd.y / h + 1.0f) / proj(1, 1);
	vd.z = 1.0f;
	vd.z = boneZ;

	const D3DXMATRIX matView = g_Camera->view();
	const D3DXMATRIX matWorld = m_matWorld;
	D3DXMATRIX mWorldView = matWorld * matView;
	D3DXMATRIX m;
	D3DXMatrixInverse(&m, NULL, &mWorldView);

	vPickRayDir.x = v.x * m._11 + v.y * m._21 + v.z * m._31;
	vPickRayDir.y = v.x * m._12 + v.y * m._22 + v.z * m._32;
	vPickRayDir.z = v.x * m._13 + v.y * m._23 + v.z * m._33;
	vPickRayDir2.x = vd.x * m._11 + vd.y * m._21 + vd.z * m._31;
	vPickRayDir2.y = vd.x * m._12 + vd.y * m._22 + vd.z * m._32;
	vPickRayDir2.z = vd.x * m._13 + vd.y * m._23 + vd.z * m._33;
	vPickRayOrig.x = m._41;
	vPickRayOrig.y = m._42;
	vPickRayOrig.z = m._43;
	D3DXVECTOR3 vOrigToDir;
	D3DXVECTOR3 vOrigToDir2;
	vOrigToDir = vPickRayDir - vPickRayOrig;
	vOrigToDir2 = vPickRayDir2 - vPickRayOrig;
	return (vOrigToDir2 - vOrigToDir);
}
TriPickDemo::~TriPickDemo()
{
	SAFE_RELEASE(m_FX);
	SAFE_RELEASE(m_Mesh);
	SAFE_DELETE(m_pCharacter);
	SAFE_DELETE(m_pIK);
}
void TriPickDemo::getAllName(Bone *bone)
{
	if (bone->pFrameFirstChild)
		g_debug << bone->pFrameFirstChild->Name << endl;
	if (bone->pFrameSibling)
		g_debug << bone->pFrameSibling->Name << endl;
	if (bone->pFrameFirstChild)
		getAllName((Bone*)bone->pFrameFirstChild);
	if (bone->pFrameSibling)
		getAllName((Bone*)bone->pFrameSibling);
}