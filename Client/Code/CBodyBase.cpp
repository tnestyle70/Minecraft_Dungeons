#include "pch.h"
#include "CBodyBase.h"

CBodyBase::CBodyBase(LPDIRECT3DDEVICE9 pGraphicDev)
	: m_pGraphicDev(pGraphicDev), m_pAnim(nullptr)
{
}

CBodyBase::~CBodyBase()
{
}

HRESULT CBodyBase::Register_Part(EBodyPart iPartIndex, const BodyPart& part)
{
	//벡터 크기 확장
	if (iPartIndex >= (int)m_vecParts.size())
	{
		m_vecParts.resize(iPartIndex + 1);
	}

	CCubeBodyTex* pBuffer = dynamic_cast<CCubeBodyTex*>
		(CProtoMgr::GetInstance()->Clone_Prototype(part.szProtoName));

	if (!pBuffer)
	{
		MSG_BOX("Body Create Failed");
		return E_FAIL;
	}

	m_vecParts[iPartIndex].pBuffer = pBuffer;
	m_vecParts[iPartIndex].vAnchorPos = part.vAnchorPos;
	m_vecParts[iPartIndex].vPivotOffset = part.vPivotOffset;

	return S_OK;
}

_int CBodyBase::Update_Body(const _float& fTimeDelta, bool bMoving, bool bAttack)
{
	if (m_pAnim)
	{
		m_pAnim->Update(fTimeDelta, bMoving, bAttack);
	}
	return 0;
}

void CBodyBase::Render_Body(const _matrix* pParentWorld, Engine::CTexture* pTexture)
{
	if (!pParentWorld || !pTexture)
	{
		return;
	}

	m_pGraphicDev->SetRenderState(D3DRS_LIGHTING, FALSE);
	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	
	if (pTexture)
	{
		pTexture->Set_Texture(0);
	}

	const BodyPose& pose = m_pAnim->Get_Pose();
	//설정한 sway 값만큼 회전 행렬 만들어서 matWorld에 적용해주기
	_matrix matSway;
	D3DXMatrixTranslation(&matSway, 0.f, pose.fBodySwayY, 0.f);
	_matrix matWorld = matSway * (*pParentWorld);
	//등록된 파트 순회하면서 렌더링
	for (int i = 0; i < (int)m_vecParts.size(); ++i)
	{
		if (!m_vecParts[i].pBuffer)
			continue;
		_vec3 vRot = pose.GetRot(i);
		Render_Part(i, matWorld, vRot.x, vRot.y, vRot.z);
	}
	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
}

void CBodyBase::Render_Part(EBodyPart iPartIndex, const _matrix& matParent, 
	float fRotX, float fRotY, float fRotZ)
{
	//파트별 애니메이션 반영해서 렌더링
	_matrix matPartWorld = Calc_PartMatrix(iPartIndex, matParent, fRotX, fRotY, fRotZ);
	m_pGraphicDev->SetTransform(D3DTS_WORLD, &matPartWorld);
	m_vecParts[iPartIndex].pBuffer->Render_Buffer();
}

_matrix CBodyBase::Calc_PartMatrix(EBodyPart iPartIndex, const _matrix& matParent, 
	float fRotX, float fRotY, float fRotZ)
{
	const BodyPartData& part = m_vecParts[iPartIndex];
	//1단계 - 회전할 위치로 이동
	_matrix matToPivot;
	D3DXMatrixTranslation(&matToPivot, 
		part.vPivotOffset.x,
		part.vPivotOffset.y,
		part.vPivotOffset.z);
	//2단계 - 개별 회전 행렬 적용된 matrix 만들기
	_matrix matRotX, matRotY, matRotZ, matRot;
	D3DXMatrixRotationX(&matRotX, fRotX);
	D3DXMatrixRotationY(&matRotY, fRotY);
	D3DXMatrixRotationZ(&matRotZ, fRotZ);
	matRot = matRotX * matRotY * matRotZ;
	//3단계 - 회전 적용하고 원래 부모 기준 로컬 좌표로 다시 이동
	_matrix matToAnchor;
	D3DXMatrixTranslation(&matToAnchor,
		part.vAnchorPos.x - part.vPivotOffset.x,
		part.vAnchorPos.y - part.vPivotOffset.y,
		part.vAnchorPos.z - part.vPivotOffset.z);
	
	_matrix matLocal = matToPivot * matRot * matToAnchor;
	//부모 행렬에 적용
	return matLocal * matParent;
}

void CBodyBase::Free()
{
	for (auto& part : m_vecParts)
	{
		Safe_Release(part.pBuffer);
	}

	m_vecParts.clear();

	if (m_pAnim)
	{
		delete m_pAnim;
		m_pAnim = nullptr;
	}
}
