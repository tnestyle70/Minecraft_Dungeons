#include "pch.h"
#include "CPlayer.h"
#include "CRenderer.h"
#include "CManagement.h"
#include "CBlockMgr.h"
#include "CDInputMgr.h"
#include "CCollider.h"

CPlayer::CPlayer(LPDIRECT3DDEVICE9 pGraphicDev)
	: CGameObject(pGraphicDev)
	, m_pBufferCom{}
	, m_pTextureCom(nullptr)
	, m_pTransformCom(nullptr)
	, m_pCalculatorCom(nullptr)
	, m_pColliderCom(nullptr)
	, m_vPartOffset{}
	, m_vPartScale{}
	, m_fWalkTime(0.f)
	, m_bMoving(false)
{
	D3DXMatrixIdentity(&m_matRArmWorld);
}

CPlayer::CPlayer(const CGameObject& rhs)
	: CGameObject(rhs)
	, m_pBufferCom{}
	, m_pTextureCom(nullptr)
	, m_pTransformCom(nullptr)
	, m_pCalculatorCom(nullptr)
	, m_pColliderCom(nullptr)
	, m_vPartOffset{}
	, m_vPartScale{}
	, m_fWalkTime(0.f)
	, m_bMoving(false)
{
}

CPlayer::~CPlayer()
{
}

HRESULT CPlayer::Ready_GameObject()
{
	if (FAILED(Add_Component()))
		return E_FAIL;

	m_pTransformCom->Set_Pos(0.f, 10.f, 0.f);

#pragma region 파트별 크기, 오프셋
	m_vPartScale[PART_HEAD] = { 0.20f, 0.20f, 0.20f };
	m_vPartScale[PART_BODY] = { 0.25f, 0.35f, 0.125f };
	m_vPartScale[PART_LARM] = { 0.10f, 0.30f, 0.10f };
	m_vPartScale[PART_RARM] = { 0.10f, 0.30f, 0.10f };
	m_vPartScale[PART_LLEG] = { 0.10f, 0.30f, 0.10f };
	m_vPartScale[PART_RLEG] = { 0.10f, 0.30f, 0.10f };

	m_vPartOffset[PART_HEAD] = { 0.00f, 1.10f, 0.00f };
	m_vPartOffset[PART_BODY] = { 0.00f, 0.60f, 0.00f };
	m_vPartOffset[PART_LARM] = { 0.35f, 0.60f, 0.00f };	
	m_vPartOffset[PART_RARM] = { -0.35f, 0.60f, 0.00f };
	m_vPartOffset[PART_LLEG] = { 0.13f, 0.30f, 0.00f };	
	m_vPartOffset[PART_RLEG] = { -0.13f, 0.30f, 0.00f };
#pragma endregion
	return S_OK;
}

_int CPlayer::Update_GameObject(const _float& fTimeDelta)
{
	_int iExit = CGameObject::Update_GameObject(fTimeDelta);

	// 공격
	if (m_iComboStep > 0)
	{
		m_fAtkTime += fTimeDelta;
		if (m_fAtkTime >= m_fAtkDuration)
		{
			m_fAtkTime = m_fAtkDuration;	// 끝자리 고정 (반복 방지)
			m_fComboTimer = m_fComboWindow;
			if (m_iComboStep == 3)
				m_iComboStep = 0;
		}
	}
	if (m_fComboTimer > 0.f)
	{
		m_fComboTimer -= fTimeDelta;
		if (m_fComboTimer <= 0.f)
			m_iComboStep = 0;
	}

	// 이동
	if (m_bMoving)
		m_fWalkTime += fTimeDelta * 8.f;

	// 피격
	if (m_bHit)
	{
		m_fHitTime += fTimeDelta;
		if (m_fHitTime >= m_fHitDuration)
		{
			m_bHit = false;
			m_fHitTime = 0.f;
		}
	}

	_vec3 vPos;
	m_pTransformCom->Get_Info(INFO_POS, &vPos);
	m_pColliderCom->Update_AABB(vPos);

	Attack_Collision();
	Apply_Gravity(fTimeDelta);
	Roll_Update(fTimeDelta);
	Resolve_BlockCollision();

	if (m_fRollCooldown > 0.f)
		m_fRollCooldown -= fTimeDelta;

	CRenderer::GetInstance()->Add_RenderGroup(RENDER_ALPHA, this);

	return iExit;
}

void CPlayer::LateUpdate_GameObject(const _float& fTimeDelta)
{
	Key_Input(fTimeDelta);
	CGameObject::LateUpdate_GameObject(fTimeDelta);
}

void CPlayer::Render_GameObject()
{
	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	// 피격 빨간색 깜빡임
	if (m_bHit)
	{
		float fBlink = sinf(m_fHitTime * D3DX_PI * 8.f);
		if (fBlink > 0.f)
		{
			m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
			m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
			m_pGraphicDev->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_RGBA(255, 0, 0, 255));
		}
	}
	//디버깅용 공격 콜라이더 박스 보이기
	if (m_bAtkColliderActive && m_pAtkColliderCom)
		m_pAtkColliderCom->Render_Collider();

	// 180도 보정 (atan2 +180 제거로 인한 시각 보정)
	_matrix matRootWorld = *m_pTransformCom->Get_World();
	_matrix mat180;
	D3DXMatrixRotationY(&mat180, D3DXToRadian(180.f));
	matRootWorld = mat180 * matRootWorld;

	const _float fMaxAngle = D3DXToRadian(30.f);
	_float fSwing = (m_bMoving && !m_bRolling) ? sinf(m_fWalkTime) * fMaxAngle : 0.f;

	// 공격 모션
	float fAtkX = 0.f;
	float fAtkZ = 0.f;
	float fTorsoY = 0.f;

	if (m_iComboStep > 0)
	{
		float fRatio = min(m_fAtkTime / m_fAtkDuration, 1.f);
		float fDir = (m_iComboStep == 2) ? 1.f : -1.f;

		if (m_iComboStep == 1 || m_iComboStep == 2)
		{
			if (fRatio < 0.3f)	// 예비: 팔 올리고 옆으로 벌리고 허리 뒤틀기
			{
				float t = fRatio / 0.3f;
				fAtkX = D3DXToRadian(-90.f * t);
				fAtkZ = D3DXToRadian(-40.f * fDir * t);
				fTorsoY = D3DXToRadian(40.f * fDir * t);
			}
			else				// 휘두르기: 팔 내리치고 허리 반대로
			{
				float t = (fRatio - 0.3f) / 0.7f;
				fAtkX = D3DXToRadian(-90.f + 150.f * t);
				fAtkZ = D3DXToRadian(-40.f * fDir * (1.f - t));
				fTorsoY = D3DXToRadian(40.f * fDir - 80.f * fDir * t);
			}
		}
		else if (m_iComboStep == 3)	// 찌르기
		{
			fAtkX = D3DXToRadian(-30.f * fRatio);
			fAtkZ = 0.f;
			fTorsoY = 0.f;
		}

		_matrix matTorsoRot;
		D3DXMatrixRotationY(&matTorsoRot, fTorsoY);
		matRootWorld = matTorsoRot * matRootWorld;
	}

	// 몸체 6부위 렌더
	Render_Part(PART_RARM, m_iComboStep > 0 ? fAtkX : -fSwing, 0.f, m_iComboStep > 0 ? fAtkZ : 0.f, matRootWorld);
	Render_Part(PART_HEAD, 0.f, 0.f, 0.f, matRootWorld);
	Render_Part(PART_BODY, 0.f, 0.f, 0.f, matRootWorld);
	Render_Part(PART_LARM, fSwing, 0.f, 0.f, matRootWorld);
	Render_Part(PART_LLEG, -fSwing, 0.f, 0.f, matRootWorld);
	Render_Part(PART_RLEG, fSwing, 0.f, 0.f, matRootWorld);

	// 칼 - 오른손(matRArmWorld) 아래끝에 고정
	_vec3 vHandLocal(0.f, -1.f, 0.f);
	_vec3 vHandPos;
	D3DXVec3TransformCoord(&vHandPos, &vHandLocal, &m_matRArmWorld);

	float fSwordLen = m_vPartScale[PART_RARM].y * 2.f;

	_matrix matScale, matPivot, matTilt, matSwing, matStand, matRotY, matTrans;
	D3DXMatrixScaling(&matScale, fSwordLen * 0.25f, fSwordLen, 1.f);
	D3DXMatrixTranslation(&matPivot, 0.1f, fSwordLen * 0.7f, 0.f);
	D3DXMatrixRotationZ(&matTilt, D3DXToRadian(-70.f));
	D3DXMatrixRotationX(&matSwing, m_iComboStep > 0 ? fAtkX : fSwing);
	float fStabAngle = (m_iComboStep == 3) ?
		D3DXToRadian(90.f - 60.f * (m_fAtkTime / m_fAtkDuration)) :
		D3DXToRadian(90.f);
	D3DXMatrixRotationX(&matStand, fStabAngle);
	D3DXMatrixRotationY(&matRotY, D3DXToRadian(m_pTransformCom->m_vAngle.y) + D3DXToRadian(-90.f));
	D3DXMatrixTranslation(&matTrans, vHandPos.x, vHandPos.y, vHandPos.z);

	_matrix matSwordWorld = matScale * matPivot * matTilt * matSwing * matStand * matRotY * matTrans;

	m_pGraphicDev->SetTransform(D3DTS_WORLD, &matSwordWorld);
	m_pSwordTextureCom->Set_Texture(0);
	m_pGraphicDev->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	m_pGraphicDev->SetRenderState(D3DRS_ALPHAREF, 128);
	m_pGraphicDev->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	m_pSwordBufferCom->Render_Buffer();
	m_pGraphicDev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

	m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

	m_pColliderCom->Render_Collider();
}

HRESULT CPlayer::Add_Component()
{
	Engine::CComponent* pComponent = nullptr;

#pragma region 큐브 6면 부위별
	FACE_UV uvHead[6] = {
		{0.125f, 0.125f, 0.25f,  0.25f },	// FRONT
		{0.375f, 0.125f, 0.5f,   0.25f },	// BACK
		{0.125f, 0.0f,   0.25f,  0.125f},	// TOP
		{0.25f,  0.0f,   0.375f, 0.125f},	// BOT
		{0.0f,   0.125f, 0.125f, 0.25f },	// LEFT
		{0.25f,  0.125f, 0.375f, 0.25f },	// RIGHT
	};
	FACE_UV uvBody[6] = {
		{0.3125f, 0.3125f, 0.4375f, 0.5f   },	// FRONT
		{0.5f,    0.3125f, 0.625f,  0.5f   },	// BACK
		{0.3125f, 0.25f,   0.4375f, 0.3125f},	// TOP
		{0.4375f, 0.25f,   0.5625f, 0.3125f},	// BOT
		{0.25f,   0.3125f, 0.3125f, 0.5f   },	// LEFT
		{0.4375f, 0.3125f, 0.5f,    0.5f   },	// RIGHT
	};
	FACE_UV uvRArm[6] = {
		{0.6875f, 0.3125f, 0.75f,   0.5f   },	// FRONT
		{0.8125f, 0.3125f, 0.875f,  0.5f   },	// BACK
		{0.6875f, 0.25f,   0.75f,   0.3125f},	// TOP
		{0.75f,   0.25f,   0.8125f, 0.3125f},	// BOT
		{0.625f,  0.3125f, 0.6875f, 0.5f   },	// LEFT
		{0.75f,   0.3125f, 0.8125f, 0.5f   },	// RIGHT
	};
	FACE_UV uvLArm[6] = {
		{0.5625f, 0.8125f, 0.625f,  1.0f   },	// FRONT
		{0.6875f, 0.8125f, 0.75f,   1.0f   },	// BACK
		{0.5625f, 0.75f,   0.625f,  0.8125f},	// TOP
		{0.625f,  0.75f,   0.6875f, 0.8125f},	// BOT
		{0.5f,    0.8125f, 0.5625f, 1.0f   },	// LEFT
		{0.625f,  0.8125f, 0.6875f, 1.0f   },	// RIGHT
	};
	FACE_UV uvRLeg[6] = {
		{0.0625f, 0.3125f, 0.125f,  0.5f   },	// FRONT
		{0.1875f, 0.3125f, 0.25f,   0.5f   },	// BACK
		{0.0625f, 0.25f,   0.125f,  0.3125f},	// TOP
		{0.125f,  0.25f,   0.1875f, 0.3125f},	// BOT
		{0.0f,    0.3125f, 0.0625f, 0.5f   },	// LEFT
		{0.125f,  0.3125f, 0.1875f, 0.5f   },	// RIGHT
	};
	FACE_UV uvLLeg[6] = {
		{0.3125f, 0.8125f, 0.375f,  1.0f   },	// FRONT
		{0.4375f, 0.8125f, 0.5f,    1.0f   },	// BACK
		{0.3125f, 0.75f,   0.375f,  0.8125f},	// TOP
		{0.375f,  0.75f,   0.4375f, 0.8125f},	// BOT
		{0.25f,   0.8125f, 0.3125f, 1.0f   },	// LEFT
		{0.375f,  0.8125f, 0.4375f, 1.0f   },	// RIGHT
	};
#pragma endregion

	FACE_UV* uvTable[PART_END] = { uvHead, uvBody, uvLArm, uvRArm, uvLLeg, uvRLeg };
	const wchar_t* tagTable[PART_END] = {
		L"Com_HeadBuf", L"Com_BodyBuf",
		L"Com_LArmBuf", L"Com_RArmBuf",
		L"Com_LLegBuf", L"Com_RLegBuf"
	};

	//부위별 생성
	for (_uint i = 0; i < PART_END; ++i)
	{
		m_pBufferCom[i] = CPlayerBody::Create(m_pGraphicDev, uvTable[i]);
		if (nullptr == m_pBufferCom[i])
		{
			MSG_BOX("BufferCom Failed");
			return E_FAIL;
		}
		m_mapComponent[ID_STATIC].insert({ tagTable[i], m_pBufferCom[i] });
	}

	pComponent = m_pTextureCom = dynamic_cast<Engine::CTexture*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_PlayerTexture"));
	if (nullptr == pComponent)
		return E_FAIL;
	m_mapComponent[ID_STATIC].insert({ L"Com_Texture", pComponent });


	//칼 버퍼
	pComponent = m_pSwordBufferCom = dynamic_cast<Engine::CRcTex*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RcTex"));
	if (nullptr == pComponent)
	{
		MSG_BOX("SwordBuffer Failed");
		return E_FAIL;
	}
	m_mapComponent[ID_STATIC].insert({ L"Com_SwordBuffer", pComponent });

	//칼 텍스쳐
	Engine::CComponent* pRaw = CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_SwordTexture");
	if (nullptr == pRaw)
		return E_FAIL;
	m_pSwordTextureCom = dynamic_cast<Engine::CTexture*>(pRaw);
	if (nullptr == m_pSwordTextureCom)
		return E_FAIL;
	pComponent = m_pSwordTextureCom;
	m_mapComponent[ID_STATIC].insert({ L"Com_SwordTexture", pComponent });

	pComponent = m_pTransformCom = dynamic_cast<Engine::CTransform*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));
	if (nullptr == pComponent)
		return E_FAIL;
	m_mapComponent[ID_DYNAMIC].insert({ L"Com_Transform", pComponent });


	//플레이어 콜라이더
	m_pColliderCom = CCollider::Create(m_pGraphicDev,
		_vec3(0.5f, 1.8f, 0.5f),
		_vec3(0.f, 0.9f, 0.f));
	if (nullptr == m_pColliderCom)
		return E_FAIL;
	m_mapComponent[ID_STATIC].insert({ L"Com_Collider", m_pColliderCom });

	//플레이어 공격 콜라이더
	m_pAtkColliderCom = CCollider::Create(m_pGraphicDev,
		_vec3(1.6f, 1.0f, 1.6f), //공격범위 크기
		_vec3(0.f, 0.f, 0.f)); //플레이어 기준 위치
	if (nullptr == m_pAtkColliderCom)
		return E_FAIL;
	m_mapComponent[ID_STATIC].insert({ L"Com_AtkCollider", m_pAtkColliderCom });

	pComponent = m_pCalculatorCom = dynamic_cast<Engine::CCalculator*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Calculator"));
	if (nullptr == pComponent)
		return E_FAIL;
	m_mapComponent[ID_STATIC].insert({ L"Com_Calculator", pComponent });

	return S_OK;
}

void CPlayer::Key_Input(const _float& fTimeDelta)
{
	m_bMoving = false;

	// 공격
	bool bAtkKey = (GetAsyncKeyState('F') & 0x8000);
	if (bAtkKey && !m_bAtkKeyPrev)
	{
		if (m_iComboStep == 0 || m_fComboTimer > 0.f)
		{
			m_iComboStep = (m_iComboStep % 3) + 1;
			m_fAtkTime = 0.f;
			m_fComboTimer = m_fComboWindow;
		}
	}
	m_bAtkKeyPrev = bAtkKey;

	// 임시 피격 테스트
	if (GetAsyncKeyState('R') & 0x8000)
	{
		m_fHp -= 10.f;
		if (m_fHp < 0.f) m_fHp = 0.f;
		Hit();
	}

	// 화살표 회전/이동
	if (GetAsyncKeyState(VK_LEFT))
		m_pTransformCom->Rotation(ROT_Y, 180.f * fTimeDelta);
	if (GetAsyncKeyState(VK_RIGHT))
		m_pTransformCom->Rotation(ROT_Y, -180.f * fTimeDelta);

	_vec3 vLook;
	m_pTransformCom->Get_Info(INFO_LOOK, &vLook);

	if (GetAsyncKeyState(VK_UP))
	{
		m_pTransformCom->Move_Pos(D3DXVec3Normalize(&vLook, &vLook), 10.f, fTimeDelta);
		m_bMoving = true;
	}
	if (GetAsyncKeyState(VK_DOWN))
	{
		m_pTransformCom->Move_Pos(D3DXVec3Normalize(&vLook, &vLook), -10.f, fTimeDelta);
		m_bMoving = true;
	}

	// 마우스 클릭 이동
	if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
	{
		m_vTargetPos = Picking_OnBlock();
		m_vTargetPos.y = 0.f;
		m_bHasTarget = true;
	}

	if (m_bHasTarget)
	{
		_vec3 vPos;
		m_pTransformCom->Get_Info(INFO_POS, &vPos);

		_vec3 vDir = m_vTargetPos - vPos;
		vDir.y = 0.f;

		float fDist = D3DXVec3Length(&vDir);

		if (fDist > 0.3f)
		{
			D3DXVec3Normalize(&vDir, &vDir);
			m_pTransformCom->m_vAngle.y = D3DXToDegree(atan2f(vDir.x, vDir.z));	// +180 제거
			m_pTransformCom->Move_Pos(&vDir, 5.f, fTimeDelta);
			m_bMoving = true;
		}
		else
		{
			m_bHasTarget = false;
			m_bMoving = false;
		}
	}

	// 구르기
	if (CDInputMgr::GetInstance()->Get_DIKeyState(DIK_SPACE))
	{
		if (!m_bRolling && m_fRollCooldown <= 0.f)
		{
			_vec3 vLook;
			m_pTransformCom->Get_Info(INFO_LOOK, &vLook);

			if (m_bMoving && m_bHasTarget)
			{
				_vec3 vPos;
				m_pTransformCom->Get_Info(INFO_POS, &vPos);
				m_vRollDir = m_vTargetPos - vPos;
				m_vRollDir.y = 0.f;
				D3DXVec3Normalize(&m_vRollDir, &m_vRollDir);
			}
			else
			{
				m_vRollDir = vLook;		// 부호 제거
				m_vRollDir.y = 0.f;
				D3DXVec3Normalize(&m_vRollDir, &m_vRollDir);
			}

			m_bRolling = true;
			m_fRollTime = 0.f;
			m_bHasTarget = false;
		}
	}
}

void CPlayer::Set_OnTerrain()
{
	_vec3 vPos;
	m_pTransformCom->Get_Info(INFO_POS, &vPos);

	Engine::CTerrainTex* pTerrainVtxCom = dynamic_cast<Engine::CTerrainTex*>
		(CManagement::GetInstance()->Get_Component(ID_STATIC, L"GameLogic_Layer", L"Terrain", L"Com_Buffer"));

	if (nullptr == pTerrainVtxCom)
		return;

	_float fY = m_pCalculatorCom->Compute_HeightOnTerrain(&vPos, pTerrainVtxCom->Get_VtxPos(), VTXCNTX, VTXCNTZ);
	m_pTransformCom->Set_Pos(vPos.x, fY + 2.f, vPos.z);
}

_vec3 CPlayer::Picking_OnBlock()
{
	POINT ptMouse;
	GetCursorPos(&ptMouse);
	ScreenToClient(g_hWnd, &ptMouse);

	D3DVIEWPORT9 vp;
	m_pGraphicDev->GetViewport(&vp);

	_vec3 vMousePos;
	vMousePos.x = ptMouse.x / (vp.Width * 0.5f) - 1.f;
	vMousePos.y = ptMouse.y / -(vp.Height * 0.5f) + 1.f;
	vMousePos.z = 0.f;

	_matrix matInvProj;
	m_pGraphicDev->GetTransform(D3DTS_PROJECTION, &matInvProj);
	D3DXMatrixInverse(&matInvProj, 0, &matInvProj);
	D3DXVec3TransformCoord(&vMousePos, &vMousePos, &matInvProj);

	_matrix matInvView;
	m_pGraphicDev->GetTransform(D3DTS_VIEW, &matInvView);
	D3DXMatrixInverse(&matInvView, 0, &matInvView);

	_vec3 vRayPos = { 0.f, 0.f, 0.f };
	_vec3 vRayDir = vMousePos - vRayPos;
	D3DXVec3TransformCoord(&vRayPos, &vRayPos, &matInvView);
	D3DXVec3TransformNormal(&vRayDir, &vRayDir, &matInvView);
	D3DXVec3Normalize(&vRayDir, &vRayDir);

	float fMinT = FLT_MAX;
	_vec3 vHit = _vec3(0.f, 0.f, 0.f);
	bool bHit = false;

	for (auto& pair : CBlockMgr::GetInstance()->Get_Blocks())
	{
		AABB tAABB = CBlockMgr::GetInstance()->Get_BlockAABB(pair.first);

		float tMin = 0.f, tMax = FLT_MAX;
		float bounds[2][3] = {
			{ tAABB.vMin.x, tAABB.vMin.y, tAABB.vMin.z },
			{ tAABB.vMax.x, tAABB.vMax.y, tAABB.vMax.z }
		};
		float rayOrigin[3] = { vRayPos.x, vRayPos.y, vRayPos.z };
		float rayDir[3] = { vRayDir.x, vRayDir.y, vRayDir.z };

		bool bMiss = false;
		for (int i = 0; i < 3; ++i)
		{
			if (fabsf(rayDir[i]) < 1e-6f)
			{
				if (rayOrigin[i] < bounds[0][i] || rayOrigin[i] > bounds[1][i])
				{
					bMiss = true; break;
				}
			}
			else
			{
				float t1 = (bounds[0][i] - rayOrigin[i]) / rayDir[i];
				float t2 = (bounds[1][i] - rayOrigin[i]) / rayDir[i];
				if (t1 > t2) swap(t1, t2);
				tMin = max(tMin, t1);
				tMax = min(tMax, t2);
				if (tMin > tMax) { bMiss = true; break; }
			}
		}

		if (!bMiss && tMin < fMinT && tMin > 0.f)
		{
			fMinT = tMin;
			vHit = vRayPos + vRayDir * tMin;
			vHit.y = tAABB.vMax.y;
			bHit = true;
		}
	}

	if (!bHit)
	{
		if (fabsf(vRayDir.y) > 0.0001f)
		{
			float t = -vRayPos.y / vRayDir.y;
			if (t > 0.f)
			{
				vHit.x = vRayPos.x + vRayDir.x * t;
				vHit.y = 0.f;
				vHit.z = vRayPos.z + vRayDir.z * t;
				return vHit;
			}
		}
		_vec3 vPos;
		m_pTransformCom->Get_Info(INFO_POS, &vPos);
		return vPos;
	}

	return vHit;
}

void CPlayer::Render_Part(BODYPART ePart, _float fAngleX, _float fAngleY, _float fAngleZ, const _matrix& matRootWorld)
{
	_matrix matScale;
	D3DXMatrixScaling(&matScale,
		m_vPartScale[ePart].x,
		m_vPartScale[ePart].y,
		m_vPartScale[ePart].z);

	_matrix matPivotDown;
	D3DXMatrixTranslation(&matPivotDown, 0.f, -m_vPartScale[ePart].y, 0.f);

	_matrix matRotX, matRotY, matRotZ;
	D3DXMatrixRotationX(&matRotX, fAngleX);
	D3DXMatrixRotationY(&matRotY, fAngleY);
	D3DXMatrixRotationZ(&matRotZ, fAngleZ);

	_matrix matJoint;
	D3DXMatrixTranslation(&matJoint,
		m_vPartOffset[ePart].x,
		m_vPartOffset[ePart].y + m_vPartScale[ePart].y,
		m_vPartOffset[ePart].z);

	_matrix matRollLocal;
	D3DXMatrixIdentity(&matRollLocal);
	if (m_bRolling)
	{
		float fRatio = m_fRollTime / m_fRollDuration;
		float fRollAngle = -fRatio * D3DX_PI * 2.f;

		_matrix matToCenter, matRollX, matFromCenter;
		D3DXMatrixTranslation(&matToCenter, 0.f, -0.6f, 0.f);
		D3DXMatrixRotationX(&matRollX, fRollAngle);
		D3DXMatrixTranslation(&matFromCenter, 0.f, 0.6f, 0.f);

		matRollLocal = matToCenter * matRollX * matFromCenter;
	}

	// matRotZ를 matJoint 뒤에 배치 → 어깨 기준 옆으로 벌리기
	_matrix matPartWorld = matScale * matPivotDown * matRotX * matRotZ * matRotY * matJoint * matRollLocal * matRootWorld;

	if (ePart == PART_RARM)
		m_matRArmWorld = matPartWorld;

	m_pGraphicDev->SetTransform(D3DTS_WORLD, &matPartWorld);
	m_pTextureCom->Set_Texture(0);
	m_pBufferCom[ePart]->Render_Buffer();
}

void CPlayer::Attack_Collision()
{
	if (m_iComboStep > 0)
	{
		_vec3 vPos, vLook;
		m_pTransformCom->Get_Info(INFO_POS, &vPos);
		m_pTransformCom->Get_Info(INFO_LOOK, &vLook);
		D3DXVec3Normalize(&vLook, &vLook);
		_vec3 vAtkPos = vPos;
		vAtkPos.y += 0.9f;
		m_pAtkColliderCom->Update_AABB(vAtkPos);
		m_bAtkColliderActive = true;
	}
	else
	{
		m_bAtkColliderActive = false;
	}
}

void CPlayer::Apply_Gravity(const _float& fTimeDelta)
{
	if (m_bOnGround)
		return;

	m_fVelocityY += m_fGravity * fTimeDelta;
	if (m_fVelocityY < m_fMaxFall)
		m_fVelocityY = m_fMaxFall;

	_vec3 vPos;
	m_pTransformCom->Get_Info(INFO_POS, &vPos);
	vPos.y += m_fVelocityY * fTimeDelta;
	m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);
}

void CPlayer::Resolve_BlockCollision()
{
	_vec3 vPos;
	m_pTransformCom->Get_Info(INFO_POS, &vPos);
	m_pColliderCom->Update_AABB(vPos);
	AABB tPlayerAABB = m_pColliderCom->Get_AABB();

	//공격시 콜라이더
	if (m_iComboStep > 0)
	{
		_vec3 vLook;
		m_pTransformCom->Get_Info(INFO_LOOK, &vLook);
		D3DXVec3Normalize(&vLook, &vLook);
		_vec3 vAtkPos = vPos + vLook * 0.8f;
		vAtkPos.y += 0.9f;
		m_pAtkColliderCom->Update_AABB(vAtkPos);
		m_bAtkColliderActive = true;
	}
	else
	{
		m_bAtkColliderActive = false;
	}

	m_bOnGround = false;

	int iMinX = (int)floorf(tPlayerAABB.vMin.x);
	int iMaxX = (int)ceilf(tPlayerAABB.vMax.x);
	int iMinY = (int)floorf(tPlayerAABB.vMin.y) - 3;
	int iMaxY = (int)ceilf(tPlayerAABB.vMax.y);
	int iMinZ = (int)floorf(tPlayerAABB.vMin.z);
	int iMaxZ = (int)ceilf(tPlayerAABB.vMax.z);

	for (int y = iMinY; y <= iMaxY; ++y)
	{
		for (int x = iMinX; x <= iMaxX; ++x)
		{
			for (int z = iMinZ; z <= iMaxZ; ++z)
			{
				BlockPos tBlockPos = { x, y, z };
				if (!CBlockMgr::GetInstance()->HasBlock(tBlockPos))
					continue;

				AABB tBlockAABB = CBlockMgr::GetInstance()->Get_BlockAABB(tBlockPos);
				if (!m_pColliderCom->IsColliding(tBlockAABB))
					continue;

				_vec3 vResolve = m_pColliderCom->Resolve(tBlockAABB);
				vPos += vResolve;
				m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);
				m_pColliderCom->Update_AABB(vPos);
				tPlayerAABB = m_pColliderCom->Get_AABB();

				if (fabsf(vResolve.y) > 0.f)
				{
					if (vResolve.y > 0.f)
					{
						m_bOnGround = true;
						m_fVelocityY = 0.f;
					}
					else
					{
						m_fVelocityY = 0.f;
					}
				}
			}
		}
	}
}

void CPlayer::Hit()
{
	m_bHit = true;
	m_fHitTime = 0.f;
}

void CPlayer::Roll_Update(const _float& fTimeDelta)
{
	if (!m_bRolling)
		return;

	m_fRollTime += fTimeDelta;

	float fRatio = m_fRollTime / m_fRollDuration;
	float fSpeed = m_fRollSpeed * (1.f - fRatio);
	m_pTransformCom->Move_Pos(&m_vRollDir, fSpeed, fTimeDelta);

	if (m_fRollTime >= m_fRollDuration)
	{
		m_bRolling = false;
		m_fRollTime = 0.f;
		m_fRollCooldown = m_fRollCoolMax;
	}
}

CPlayer* CPlayer::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
	CPlayer* pPlayer = new CPlayer(pGraphicDev);

	if (FAILED(pPlayer->Ready_GameObject()))
	{
		Safe_Release(pPlayer);
		MSG_BOX("pPlayer Create Failed");
		return nullptr;
	}

	return pPlayer;
}

void CPlayer::Free()
{
	CGameObject::Free();
}