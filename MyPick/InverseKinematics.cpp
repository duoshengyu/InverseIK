#include "InverseKinematics.h"
#include "Character.h"
#include "DirectInputClass.h"
#include "D3DApp.h"
#include <fstream>
using namespace std;
extern ofstream g_debug;
extern D3DApp * g_d3dApp;
extern DirectInput* gDInput;
//get the direction of head looking to
InverseKinematics::InverseKinematics(Character* pCharacter)
{
	m_pCharacter = pCharacter;

	//get head bone
	m_pHeadBone = (Bone*)D3DXFrameFind(m_pCharacter->getPBone(), "Head");

	if (m_pHeadBone != NULL)
	{
		// calculate forward direction 
		// remove translate 
		D3DXMATRIX headMatrix = m_pHeadBone->CombinedTransformationMatrix;
		headMatrix._41 = 0.0f;
		headMatrix._42 = 0.0f;
		headMatrix._43 = 0.0f;
		headMatrix._44 = 1.0f;

		D3DXMATRIX toHeadSpace;
		if (D3DXMatrixInverse(&toHeadSpace, NULL, &headMatrix) == NULL)
			return;

		//transform -z to head space
		D3DXVECTOR4 vec;
		D3DXVec3Transform(&vec, &D3DXVECTOR3(0, 0, -1), &toHeadSpace);
		m_headForward = D3DXVECTOR3(vec.x, vec.y, vec.z);
	}

	//get arm, leg, and their position 
	U_R_Arm = (Bone*)D3DXFrameFind(m_pCharacter->getPBone(), "Bone17");
	U_L_Arm = (Bone*)D3DXFrameFind(m_pCharacter->getPBone(), "Bone20");
	L_R_Arm = (Bone*)D3DXFrameFind(m_pCharacter->getPBone(), "Bone18");
	L_L_Arm = (Bone*)D3DXFrameFind(m_pCharacter->getPBone(), "Bone21");
	R_Hand = (Bone*)D3DXFrameFind(m_pCharacter->getPBone(), "Bone22");
	L_Hand = (Bone*)D3DXFrameFind(m_pCharacter->getPBone(), "Bone27");
	R_Thigh = (Bone*)D3DXFrameFind(m_pCharacter->getPBone(), "Bone07");
	L_Thigh = (Bone*)D3DXFrameFind(m_pCharacter->getPBone(), "Bone10");
	R_Calf = (Bone*)D3DXFrameFind(m_pCharacter->getPBone(), "Bone08");
	L_Calf = (Bone*)D3DXFrameFind(m_pCharacter->getPBone(), "Bone11");
	R_Foot = (Bone*)D3DXFrameFind(m_pCharacter->getPBone(), "Bone14");
	L_Foot = (Bone*)D3DXFrameFind(m_pCharacter->getPBone(), "Bone12");

	LHandTarget.x = L_Hand->CombinedTransformationMatrix._41;
	LHandTarget.y = L_Hand->CombinedTransformationMatrix._42;
	LHandTarget.z = L_Hand->CombinedTransformationMatrix._43;
	RHandTarget.x = R_Hand->CombinedTransformationMatrix._41;
	RHandTarget.y = R_Hand->CombinedTransformationMatrix._42;
	RHandTarget.z = R_Hand->CombinedTransformationMatrix._43;
	LLegTarget.x = L_Foot->CombinedTransformationMatrix._41;
	LLegTarget.y = L_Foot->CombinedTransformationMatrix._42;
	LLegTarget.z = L_Foot->CombinedTransformationMatrix._43;
	RLegTarget.x = R_Foot->CombinedTransformationMatrix._41;
	RLegTarget.y = R_Foot->CombinedTransformationMatrix._42;
	RLegTarget.z = R_Foot->CombinedTransformationMatrix._43;
}

//-----------------------------------------------------------------------------
// IK of head, look to mouse position
// This has something wrong, shound get backbuffer width and height from d3d 
//-----------------------------------------------------------------------------
void InverseKinematics::UpdateHeadIK()
{
	if (m_pHeadBone == NULL)
		return;

	//look at cursor
	POINT p;
	GetCursorPos(&p);
	p.x = max(min(p.x, 800), 0);
	p.y = max(min(p.y, 600), 0);
	float x = (p.x - 400) / 800.0f;
	float y = (p.y - 300) / 600.0f;
	D3DXVECTOR3 lookAt(x, 1.5f - y, -1.0f);

	// apply IK
	ApplyLookAtIK(lookAt, D3DXToRadian(60.0f));
}

//-----------------------------------------------------------------------------
// this is just a test function, test arm IK
//-----------------------------------------------------------------------------
void InverseKinematics::UpdateArmIK()
{
	if (U_L_Arm == NULL || L_L_Arm == NULL || L_Hand == NULL)
		return;
	POINT p;
	GetCursorPos(&p);
	p.x = max(min(p.x, 800), 0);
	p.y = max(min(p.y, 600), 0);
	float x = (p.x - 400) / 800.0f;
	float y = (p.y - 300) / 600.0f;
	D3DXVECTOR3 target(x , 1.5f-y, -0.25f);

	ApplyArmIK(D3DXVECTOR3(0, 0, -1), target);
}
//-----------------------------------------------------------------------------
// leg,	hand IK
//-----------------------------------------------------------------------------
void InverseKinematics::UpdateIK()
{
	if (U_L_Arm == NULL || L_L_Arm == NULL || L_Hand == NULL || U_R_Arm == NULL || L_R_Arm == NULL || R_Hand == NULL ||
		L_Calf == NULL || L_Foot == NULL || L_Thigh == NULL || R_Calf == NULL || R_Foot == NULL || R_Thigh == NULL)
		return;

	ApplyIK(U_L_Arm, L_L_Arm, L_Hand, D3DXVECTOR3(0, 0, -1), LHandTarget);
	ApplyIK(U_R_Arm, L_R_Arm, R_Hand, D3DXVECTOR3(0, 0, -1), RHandTarget);
	ApplyIK(L_Thigh, L_Calf, L_Foot, D3DXVECTOR3(0, 0, 1), LLegTarget);
	ApplyIK(R_Thigh, R_Calf, R_Foot, D3DXVECTOR3(0, 0, 1), RLegTarget);
}
//-----------------------------------------------------------------------------
// Head IK
//-----------------------------------------------------------------------------
void InverseKinematics::ApplyLookAtIK(D3DXVECTOR3 &lookAtTarget, float maxAngle)
{
	//do this in local space
	D3DXMATRIX mtxToLocal;
	D3DXMatrixInverse(&mtxToLocal, NULL, &m_pHeadBone->CombinedTransformationMatrix);

	//transform target to head local space 
	D3DXVECTOR3 localLookAt;
	D3DXVec3TransformCoord(&localLookAt, &lookAtTarget, &mtxToLocal);

	D3DXVec3Normalize(&localLookAt, &localLookAt);

	//calculate axis and rotate angle
	D3DXVECTOR3 localRotationAxis;
	D3DXVec3Cross(&localRotationAxis, &m_headForward, &localLookAt);
	D3DXVec3Normalize(&localRotationAxis, &localRotationAxis);

	float localAngle = acosf(D3DXVec3Dot(&m_headForward, &localLookAt));

	//limit angle	
	localAngle = min(localAngle, maxAngle);

	// apply to head bone
	D3DXMATRIX rotation;
	D3DXMatrixRotationAxis(&rotation, &localRotationAxis, localAngle);
	m_pHeadBone->CombinedTransformationMatrix = rotation * m_pHeadBone->CombinedTransformationMatrix;

	// update to children bone
	if (m_pHeadBone->pFrameFirstChild)
		m_pCharacter->UpdateMatrices((Bone*)m_pHeadBone->pFrameFirstChild, &m_pHeadBone->CombinedTransformationMatrix);
}

//-----------------------------------------------------------------------------
// this is just a test function, test left hand IK. You can ignore this function
//-----------------------------------------------------------------------------
void InverseKinematics::ApplyArmIK(D3DXVECTOR3 &hingeAxis, D3DXVECTOR3 &target)
{

	D3DXVECTOR3 startPosition = D3DXVECTOR3(U_L_Arm->CombinedTransformationMatrix._41, U_L_Arm->CombinedTransformationMatrix._42, U_L_Arm->CombinedTransformationMatrix._43);
	D3DXVECTOR3 jointPosition = D3DXVECTOR3(L_L_Arm->CombinedTransformationMatrix._41, L_L_Arm->CombinedTransformationMatrix._42, L_L_Arm->CombinedTransformationMatrix._43);
	D3DXVECTOR3 endPosition = D3DXVECTOR3(L_Hand->CombinedTransformationMatrix._41, L_Hand->CombinedTransformationMatrix._42, L_Hand->CombinedTransformationMatrix._43);
	//g_debug << endPosition.x <<" "<< endPosition.y<<" " << endPosition.z << endl;
	D3DXVECTOR3 startToTarget = target - startPosition;
	D3DXVECTOR3 startToJoint = jointPosition - startPosition;
	D3DXVECTOR3 jointToEnd = endPosition - jointPosition;

	float distStartToTarget = D3DXVec3Length(&startToTarget);
	float distStartToJoint = D3DXVec3Length(&startToJoint);
	float distJointToEnd = D3DXVec3Length(&jointToEnd);


	float wantedJointAngle = 0.0f;

	if (distStartToTarget >= distStartToJoint + distJointToEnd)
	{
		wantedJointAngle = D3DXToRadian(180.0f);
	}
	else
	{
		float cosAngle = (distStartToJoint * distStartToJoint + distJointToEnd * distJointToEnd - distStartToTarget * distStartToTarget) / (2.0f * distStartToJoint * distJointToEnd);
		wantedJointAngle = acosf(cosAngle);
	}

	D3DXVECTOR3 nmlStartToJoint = startToJoint;
	D3DXVECTOR3 nmlJointToEnd = jointToEnd;
	D3DXVec3Normalize(&nmlStartToJoint, &nmlStartToJoint);
	D3DXVec3Normalize(&nmlJointToEnd, &nmlJointToEnd);

	float currentJointAngle = acosf(D3DXVec3Dot(&(-nmlStartToJoint), &nmlJointToEnd));

	float diffJointAngle = wantedJointAngle - currentJointAngle;
	D3DXMATRIX rotation;
	D3DXMatrixRotationAxis(&rotation, &hingeAxis, diffJointAngle);

	L_L_Arm->TransformationMatrix = rotation * L_L_Arm->TransformationMatrix;

	D3DXMATRIX tempMatrix = L_L_Arm->CombinedTransformationMatrix;
	tempMatrix._41 = 0.0f;
	tempMatrix._42 = 0.0f;
	tempMatrix._43 = 0.0f;
	tempMatrix._44 = 1.0f;

	D3DXVECTOR3 worldHingeAxis;
	D3DXVECTOR3 newJointToEnd;
	D3DXVec3TransformCoord(&worldHingeAxis, &hingeAxis, &tempMatrix);
	D3DXMatrixRotationAxis(&rotation, &worldHingeAxis, diffJointAngle);
	D3DXVec3TransformCoord(&newJointToEnd, &jointToEnd, &rotation);

	D3DXVECTOR3 newEndPosition;
	D3DXVec3Add(&newEndPosition, &newJointToEnd, &jointPosition);

	D3DXMATRIX mtxToLocal;
	D3DXMatrixInverse(&mtxToLocal, NULL, &U_L_Arm->CombinedTransformationMatrix);

	D3DXVECTOR3 localNewEnd;
	D3DXVECTOR3 localTarget;
	D3DXVec3TransformCoord(&localNewEnd, &newEndPosition, &mtxToLocal);
	D3DXVec3TransformCoord(&localTarget, &target, &mtxToLocal);
	D3DXVec3Normalize(&localNewEnd, &localNewEnd);
	D3DXVec3Normalize(&localTarget, &localTarget);

	D3DXVECTOR3 localAxis;
	D3DXVec3Cross(&localAxis, &localNewEnd, &localTarget);
	if (D3DXVec3Length(&localAxis) == 0.0f)
		return;

	D3DXVec3Normalize(&localAxis, &localAxis);
	float localAngle = acosf(D3DXVec3Dot(&localNewEnd, &localTarget));

	D3DXMatrixRotationAxis(&rotation, &localAxis, localAngle);
	U_L_Arm->CombinedTransformationMatrix = rotation * U_L_Arm->CombinedTransformationMatrix;
	U_L_Arm->TransformationMatrix = rotation * U_L_Arm->TransformationMatrix;

	if (U_L_Arm->pFrameFirstChild)
		m_pCharacter->UpdateMatrices((Bone*)U_L_Arm->pFrameFirstChild,
		&U_L_Arm->CombinedTransformationMatrix);
	UpdateCurMat(U_L_Arm,L_L_Arm);
	UpdateCurMat(L_L_Arm, L_Hand);
}
//-----------------------------------------------------------------------------
//Leg, Hand IK function, note: just support 2 bone IK
//First, calculate middle joint angle(Elbow and knee), and target angle then rotate
//-----------------------------------------------------------------------------
void InverseKinematics::ApplyIK(Bone *Start, Bone *Joint, Bone *End, D3DXVECTOR3 &hingeAxis, D3DXVECTOR3 &target)
{
	// get 3 joint position(start, joint, end)
	D3DXVECTOR3 startPosition = D3DXVECTOR3(Start->CombinedTransformationMatrix._41, Start->CombinedTransformationMatrix._42, Start->CombinedTransformationMatrix._43);
	D3DXVECTOR3 jointPosition = D3DXVECTOR3(Joint->CombinedTransformationMatrix._41, Joint->CombinedTransformationMatrix._42, Joint->CombinedTransformationMatrix._43);
	D3DXVECTOR3 endPosition = D3DXVECTOR3(End->CombinedTransformationMatrix._41, End->CombinedTransformationMatrix._42, End->CombinedTransformationMatrix._43);
	//g_debug << endPosition.x <<" "<< endPosition.y<<" " << endPosition.z << endl;
	D3DXVECTOR3 startToTarget = target - startPosition;
	D3DXVECTOR3 startToJoint = jointPosition - startPosition;
	D3DXVECTOR3 jointToEnd = endPosition - jointPosition;

	float distStartToTarget = D3DXVec3Length(&startToTarget);
	float distStartToJoint = D3DXVec3Length(&startToJoint);
	float distJointToEnd = D3DXVec3Length(&jointToEnd);

	// calculate current angle and target angle
	float wantedJointAngle = 0.0f;

	if (distStartToTarget >= distStartToJoint + distJointToEnd)
	{
		// biggest angle is 180
		wantedJointAngle = D3DXToRadian(180.0f);
	}
	else
	{
		//Law of cosines
		float cosAngle = (distStartToJoint * distStartToJoint + distJointToEnd * distJointToEnd - distStartToTarget * distStartToTarget) / (2.0f * distStartToJoint * distJointToEnd);
		wantedJointAngle = acosf(cosAngle);
	}

	//Normalize
	D3DXVECTOR3 nmlStartToJoint = startToJoint;
	D3DXVECTOR3 nmlJointToEnd = jointToEnd;
	D3DXVec3Normalize(&nmlStartToJoint, &nmlStartToJoint);
	D3DXVec3Normalize(&nmlJointToEnd, &nmlJointToEnd);

	//current angle
	float currentJointAngle = acosf(D3DXVec3Dot(&(-nmlStartToJoint), &nmlJointToEnd));

	//rotate matrix
	float diffJointAngle = wantedJointAngle - currentJointAngle;
	D3DXMATRIX rotation;
	D3DXMatrixRotationAxis(&rotation, &hingeAxis, diffJointAngle);

	//apply to bone
	Joint->TransformationMatrix = rotation * Joint->TransformationMatrix;


	// calculate new end point position
	// calculate in world space and then transform to local space
	
	D3DXMATRIX tempMatrix = Joint->CombinedTransformationMatrix;
	tempMatrix._41 = 0.0f;
	tempMatrix._42 = 0.0f;
	tempMatrix._43 = 0.0f;
	tempMatrix._44 = 1.0f;

	D3DXVECTOR3 worldHingeAxis;
	D3DXVECTOR3 newJointToEnd;
	D3DXVec3TransformCoord(&worldHingeAxis, &hingeAxis, &tempMatrix);
	D3DXMatrixRotationAxis(&rotation, &worldHingeAxis, diffJointAngle);
	D3DXVec3TransformCoord(&newJointToEnd, &jointToEnd, &rotation);

	D3DXVECTOR3 newEndPosition;
	D3DXVec3Add(&newEndPosition, &newJointToEnd, &jointPosition);

	// total rotation
	D3DXMATRIX mtxToLocal;
	D3DXMatrixInverse(&mtxToLocal, NULL, &Start->CombinedTransformationMatrix);

	D3DXVECTOR3 localNewEnd;
	D3DXVECTOR3 localTarget;
	D3DXVec3TransformCoord(&localNewEnd, &newEndPosition, &mtxToLocal);
	D3DXVec3TransformCoord(&localTarget, &target, &mtxToLocal);
	D3DXVec3Normalize(&localNewEnd, &localNewEnd);
	D3DXVec3Normalize(&localTarget, &localTarget);

	D3DXVECTOR3 localAxis;
	D3DXVec3Cross(&localAxis, &localNewEnd, &localTarget);
	if (D3DXVec3Length(&localAxis) == 0.0f)
		return;

	D3DXVec3Normalize(&localAxis, &localAxis);
	float localAngle = acosf(D3DXVec3Dot(&localNewEnd, &localTarget));

	// apply to bone
	D3DXMatrixRotationAxis(&rotation, &localAxis, localAngle);
	Start->CombinedTransformationMatrix = rotation * Start->CombinedTransformationMatrix;
	Start->TransformationMatrix = rotation * Start->TransformationMatrix;

	// update children bone
	if (Start->pFrameFirstChild)
		m_pCharacter->UpdateMatrices((Bone*)Start->pFrameFirstChild,
		&Start->CombinedTransformationMatrix);
	UpdateCurMat(Start, Joint);
	UpdateCurMat(Joint, End);
}
void InverseKinematics::UpdateCurMat(Bone* parent, Bone *bone)
{
	D3DXMATRIX w1 = bone->CombinedTransformationMatrix;
	D3DXMATRIX w2 = parent->CombinedTransformationMatrix;

	//Extract translation
	D3DXVECTOR3 thisBone = D3DXVECTOR3(w1._41, w1._42, w1._43);
	D3DXVECTOR3 ParentBone = D3DXVECTOR3(w2._41, w2._42, w2._43);
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
void InverseKinematics::getAllName(Bone *bone)
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
void InverseKinematics::addLHMove(D3DXVECTOR3 &move)
{
	LHandTarget = LHandTarget + move;
}
void InverseKinematics::addRHMove(D3DXVECTOR3 &move)
{
	RHandTarget = RHandTarget + move;
}
void InverseKinematics::addLLMove(D3DXVECTOR3 &move)
{
	LLegTarget = LLegTarget + move;
}
void InverseKinematics::addRLMove(D3DXVECTOR3 &move)
{
	RLegTarget = RLegTarget + move;
}