#include "pch.h"
#include "CPlayer.h"
#include "CRenderer.h"
#include "CManagement.h"
#include "CBlockMgr.h"
#include "CDInputMgr.h"
#include "CCollider.h"
#include "CParticleMgr.h"
#include "CEnvironmentMgr.h"
#include "CMonster.h"
#include "CMonsterMgr.h"
#include "CRedStoneGolem.h"
#include "CAncientGuardian.h"
#include "CSoundMgr.h"

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
	D3DXMatrixIdentity(&m_matLArmWorld);
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

	m_eArmorType = ARMOR_BARDSGARD;

#pragma region 파트별 크기, 오프셋
	m_vPartScale[PART_HEAD] = { 0.40f, 0.40f, 0.40f };
	m_vPartScale[PART_BODY] = { 0.50f, 0.50f, 0.25f };
	m_vPartScale[PART_LARM] = { 0.20f, 0.60f, 0.20f };
	m_vPartScale[PART_RARM] = { 0.20f, 0.60f, 0.20f };
	m_vPartScale[PART_LLEG] = { 0.20f, 0.60f, 0.20f };
	m_vPartScale[PART_RLEG] = { 0.20f, 0.60f, 0.20f };

	m_vPartOffset[PART_HEAD] = { 0.00f, 2.20f, 0.00f };
	m_vPartOffset[PART_BODY] = { 0.00f, 1.40f, 0.00f };
	m_vPartOffset[PART_LARM] = { 0.70f, 1.20f, 0.00f };
	m_vPartOffset[PART_RARM] = { -0.70f, 1.20f, 0.00f };
	m_vPartOffset[PART_LLEG] = { 0.26f, 0.45f, 0.00f };
	m_vPartOffset[PART_RLEG] = { -0.26f, 0.45f, 0.00f };
#pragma endregion


	Safe_Release(m_pAttackEmitter);

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
			m_fAtkTime = m_fAtkDuration;
			if (m_fComboTimer <= 0.f)
				m_fComboTimer = m_fComboWindow;
		}
		//========Attack Particle============//
		if (m_iComboStep > 0 && m_pAttackEmitter)
		{
			_vec3 vPos, vLook;
			m_pTransformCom->Get_Info(INFO_POS, &vPos);
			m_pTransformCom->Get_Info(INFO_LOOK, &vLook);
			D3DXVec3Normalize(&vLook, &vLook);
			vPos -= vLook * 1.5f;
			vPos.y += 2.5f;
			m_pAttackEmitter->Set_Position(vPos);
		}
	}

	if (m_fComboTimer > 0.f)
	{
		m_fComboTimer -= fTimeDelta;
		if (m_fComboTimer <= 0.f)
		{
			m_iComboStep = 0;
			m_fAtkTime = 0.f;
		}
	}

	if (m_iComboStep == 3 && m_fAtkTime >= m_fAtkDuration)
	{
		m_iComboStep = 0;
		m_fAtkTime = 0.f;
		m_fComboTimer = 0.f;
		m_pAtkColliderCom->Update_AABB(_vec3(0.f, -9999.f, 0.f));
	}

	// 이동
	if (m_bMoving)
		m_fWalkTime += fTimeDelta * 8.f;

	//발걸음 사운드
	if (m_bMoving)
	{
		m_fStepTimer += fTimeDelta;
		if (m_fStepTimer >= m_fStepInterval)
		{
			m_fStepTimer = 0.f;
			int iIdx = rand() % 6 + 1;
			TCHAR szKey[MAX_PATH];
			wsprintf(szKey, L"Player/__cutfast_convert_sfx_player_stepStone-%03d_soundWave.wav", iIdx);
			CSoundMgr::GetInstance()->PlayEffect(szKey, 0.5f);
		}
	}
	else
		m_fStepTimer = 0.f;

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

	if (m_fBowCooldown > 0.f)
		m_fBowCooldown -= fTimeDelta;

	_vec3 vPos;
	m_pTransformCom->Get_Info(INFO_POS, &vPos);
	m_pColliderCom->Update_AABB(vPos);

	Attack_Collision();

	//화살
	for (auto& pArrow : m_vecArrows)
		pArrow->Update_GameObject(fTimeDelta);

	if (m_pHeldTNT)
	{
		_vec3 vPos;
		m_pTransformCom->Get_Info(INFO_POS, &vPos);
		vPos.y += 3.f;
		m_pHeldTNT->Get_Transform()->Set_Pos(vPos.x, vPos.y, vPos.z);
	}

	m_vecArrows.erase(
		remove_if(m_vecArrows.begin(), m_vecArrows.end(),
			[](CPlayerArrow* p) {
				if (p->Is_Dead() && !p->Is_Exploding()) { Safe_Release(p); return true; }
				return false;
			}),
		m_vecArrows.end());

	m_vecTNTs.erase(
		remove_if(m_vecTNTs.begin(), m_vecTNTs.end(),
			[](CTNT* p) { return p->Is_Dead(); }),
		m_vecTNTs.end());


	//넉백
	if (m_bKnockback)
	{
		m_fKnockbackTime += fTimeDelta;
		_vec3 vPos;
		m_pTransformCom->Get_Info(INFO_POS, &vPos);
		vPos.x += m_vKnockbackDir.x * m_fKnockbackSpeed * fTimeDelta;
		vPos.z += m_vKnockbackDir.z * m_fKnockbackSpeed * fTimeDelta;
		m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);
		if (m_fKnockbackTime >= m_fKnockbackDuration)
		{
			m_bKnockback = false;
			m_fKnockbackTime = 0.f;
		}
	}

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

	// 월드 행렬
	_matrix matRootWorld = *m_pTransformCom->Get_World();

	const _float fMaxAngle = D3DXToRadian(30.f);
	_float fSwing = (m_bMoving && !m_bRolling) ? sinf(m_fWalkTime) * fMaxAngle : 0.f;

	// 공격 모션 각도 계산
	float fAtkX = 0.f, fAtkY = 0.f, fTorsoY = 0.f;
	Calc_AttackMotion(fAtkX, fAtkY, fTorsoY);

	if (m_iComboStep > 0)
	{
		_matrix matTorsoRot;
		D3DXMatrixRotationY(&matTorsoRot, fTorsoY);
		matRootWorld = matTorsoRot * matRootWorld;
	}



	//활시위 당기기
	float fLArmX = 0.f, fLArmY = 0.f;
	float fRArmX = 0.f, fRArmY = 0.f;

	if (m_bCharging)
	{
		_matrix matBodyTurn;
		D3DXMatrixRotationY(&matBodyTurn, D3DXToRadian(-90.f));  // 오른쪽으로
		matRootWorld = matBodyTurn * matRootWorld;

		float fRatio = m_fCharge / m_fMaxCharge;

		fLArmX = D3DXToRadian(-90.f);
		fLArmY = D3DXToRadian(-90.f);   // 화살 방향

		fRArmX = D3DXToRadian(-90.f);
		fRArmY = D3DXToRadian(-90.f);   // 화살 방향
	}
	_matrix matRArmRoot = matRootWorld;	
	if (m_bCharging)
	{
		float fRatio = m_fCharge / m_fMaxCharge;
		_vec3 vLook, vRight;
		m_pTransformCom->Get_Info(INFO_LOOK, &vLook);
		m_pTransformCom->Get_Info(INFO_RIGHT, &vRight);
		D3DXVec3Normalize(&vLook, &vLook);
		D3DXVec3Normalize(&vRight, &vRight);
		matRArmRoot._41 += vLook.x * fRatio * 0.3f;
		matRArmRoot._43 += vLook.z * fRatio * 0.3f;
		matRArmRoot._41 -= vRight.x * 0.4f;
		matRArmRoot._43 -= vRight.z * 0.4f;
	}


	//아머 렌더링
	Render_Part(PART_HEAD, 0.f, m_bCharging ? D3DXToRadian(90.f) : 0.f, 0.f, matRootWorld);
	Render_Part(PART_BODY, 0.f, 0.f, 0.f, matRootWorld);
	Render_Part(PART_LARM, m_pHeldTNT ? D3DXToRadian(-150.f) : (m_bCharging ? fLArmX : fSwing),
		m_pHeldTNT ? 0.f : (m_bCharging ? fLArmY : 0.f), 0.f, matRArmRoot);

	Render_Part(PART_RARM, m_pHeldTNT ? D3DXToRadian(-150.f) : (m_bCharging ? fRArmX : (m_iComboStep > 0 ? fAtkX : -fSwing)),
		m_pHeldTNT ? 0.f : (m_bCharging ? fRArmY : (m_iComboStep > 0 ? fAtkY : 0.f)), 0.f, matRootWorld);
	Render_Part(PART_LLEG, -fSwing, 0.f, 0.f, matRootWorld);
	Render_Part(PART_RLEG, fSwing, 0.f, 0.f, matRootWorld);

	if (m_eArmorType != ARMOR_NONE && m_pArmorTextureCom)
	{
		m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		m_pGraphicDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		m_pGraphicDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		m_pGraphicDev->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
		m_pGraphicDev->SetRenderState(D3DRS_ALPHAREF, 10);
		m_pGraphicDev->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);

		for (int i = 0; i < PART_END; ++i)
		{
			_matrix matScale, matArmor;
			D3DXMatrixScaling(&matScale, 1.15f, 1.15f, 1.15f);
			matArmor = matScale * m_matPartWorld[i];
			m_pGraphicDev->SetTransform(D3DTS_WORLD, &matArmor);
			m_pArmorTextureCom->Set_Texture(0);
			m_pArmorBufferCom[i]->Render_Buffer();
		}

		m_pGraphicDev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
		m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		m_pTextureCom->Set_Texture(0);
	}


	// 칼 - 활 스위칭
	if (m_bCharging || m_bBowEquipped)
		Render_Bow();
	else if(m_bSwordEquipped)
		Render_Sword(fAtkX, fAtkY, fSwing);

	m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

	//화살 렌더링
	for (auto& pArrow : m_vecArrows)
		pArrow->Render_GameObject();

	m_pColliderCom->Render_Collider();
}

HRESULT CPlayer::Add_Component()
{
	Engine::CComponent* pComponent = nullptr;

#pragma region 큐브 6면 부위별 (플레이어)
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


#pragma region 큐브6면 부위별 (갑옷)
	// 아머용 버퍼
	FACE_UV uvArmorHead[6] = {
	{0.31250f, 0.00000f, 0.46875f, 0.14063f},  // FRONT
	{0.00000f, 0.00000f, 0.15625f, 0.14063f},  // BACK
	{0.15625f, 0.00000f, 0.31250f, 0.14063f},  // TOP
	{0.00000f, 0.00000f, 0.00000f, 0.00000f},  // BOT
	{0.15625f, 0.00000f, 0.31250f, 0.14063f},  // LEFT
	{0.15625f, 0.00000f, 0.31250f, 0.14063f},  // RIGHT
	};
	FACE_UV uvArmorBody[6] = {
		{0.09375f, 0.50000f, 0.25000f, 0.68750f},  // FRONT
		{0.34375f, 0.50000f, 0.50000f, 0.68750f},  // BACK
		{0.25000f, 0.42188f, 0.09375f, 0.50000f},  // TOP
		{0.00000f, 0.00000f, 0.00000f, 0.00000f},  // BOT
		{0.00000f, 0.50000f, 0.09375f, 0.68750f},  // LEFT
		{0.25000f, 0.50000f, 0.34375f, 0.68750f},  // RIGHT
	};
	FACE_UV uvArmorLArm[6] = {
		{0.46875f, 0.18750f, 0.60938f, 0.31250f},  // FRONT
		{0.75000f, 0.18750f, 0.89063f, 0.31250f},  // BACK
		{0.31250f, 0.14063f, 0.45313f, 0.28125f},  // TOP
		{0.00000f, 0.00000f, 0.00000f, 0.00000f},  // BOT
		{0.00000f, 0.00000f, 0.00000f, 0.00000f},  // LEFT 
		{0.60938f, 0.18750f, 0.75000f, 0.31250f},  // RIGHT
	};
	FACE_UV uvArmorRArm[6] = {
		{0.46875f, 0.00000f, 0.60938f, 0.12500f},  // FRONT
		{0.75000f, 0.00000f, 0.89063f, 0.12500f},  // BACK
		{0.31250f, 0.14063f, 0.45313f, 0.28125f},  // TOP
		{0.00000f, 0.00000f, 0.00000f, 0.00000f},  // BOT
		{0.60938f, 0.00000f, 0.75000f, 0.12500f},  // LEFT 
		{0.00000f, 0.00000f, 0.00000f, 0.00000f},  // RIGHT
	};
	FACE_UV uvArmorLLeg[6] = {
		{0.34375f, 0.68750f, 0.42188f, 0.81250f},  // FRONT
		{0.17188f, 0.68750f, 0.25000f, 0.81250f},  // BACK
		{0.00000f, 0.00000f, 0.00000f, 0.00000f},  // TOP
		{0.00000f, 0.00000f, 0.00000f, 0.00000f},  // BOT
		{0.00000f, 0.00000f, 0.00000f, 0.00000f},  // LEFT
		{0.25000f, 0.68750f, 0.34375f, 0.81250f},  // RIGHT
	};
	FACE_UV uvArmorRLeg[6] = {
		{0.42188f, 0.68750f, 0.50000f, 0.81250f},  // FRONT
		{0.09375f, 0.68750f, 0.17188f, 0.81250f},  // BACK
		{0.00000f, 0.00000f, 0.00000f, 0.00000f},  // TOP
		{0.00000f, 0.00000f, 0.00000f, 0.00000f},  // BOT
		{0.00000f, 0.00000f, 0.00000f, 0.00000f},  // LEFT
		{0.00000f, 0.68750f, 0.09375f, 0.81250f},  // RIGHT
	};

	FACE_UV* uvArmorTable[PART_END] = {
		uvArmorHead, uvArmorBody, uvArmorLArm, uvArmorRArm, uvArmorLLeg, uvArmorRLeg
	};
	const wchar_t* armorTagTable[PART_END] = {
		L"Com_ArmorHeadBuf", L"Com_ArmorBodyBuf",
		L"Com_ArmorLArmBuf", L"Com_ArmorRArmBuf",
		L"Com_ArmorLLegBuf", L"Com_ArmorRLegBuf"
	};

	for (_uint i = 0; i < PART_END; ++i)
	{
		m_pArmorBufferCom[i] = CPlayerBody::Create(m_pGraphicDev, uvArmorTable[i]);
		if (nullptr == m_pArmorBufferCom[i])
			return E_FAIL;
		m_mapComponent[ID_STATIC].insert({ armorTagTable[i], m_pArmorBufferCom[i] });
	}


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


	// 활 버퍼
	pComponent = m_pBowBufferCom = dynamic_cast<Engine::CRcTex*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RcTex"));
	if (nullptr == pComponent)
		return E_FAIL;
	m_mapComponent[ID_STATIC].insert({ L"Com_BowBuffer", pComponent });

	// 활 텍스처
	const _tchar* bowTags[4] = {
		L"Proto_BowStandby",
		L"Proto_BowPulling0",
		L"Proto_BowPulling1",
		L"Proto_BowPulling2"
	};
	for (int i = 0; i < 4; ++i)
	{
		m_pBowTexture[i] = dynamic_cast<Engine::CTexture*>
			(CProtoMgr::GetInstance()->Clone_Prototype(bowTags[i]));
		if (nullptr == m_pBowTexture[i])
			return E_FAIL;
	}
	m_mapComponent[ID_STATIC].insert({ L"Com_BowTex0", m_pBowTexture[0] });
	m_mapComponent[ID_STATIC].insert({ L"Com_BowTex1", m_pBowTexture[1] });
	m_mapComponent[ID_STATIC].insert({ L"Com_BowTex2", m_pBowTexture[2] });
	m_mapComponent[ID_STATIC].insert({ L"Com_BowTex3", m_pBowTexture[3] });

	//아머 텍스쳐
	m_pArmorTextureCom = dynamic_cast<Engine::CTexture*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_ArmorTexture"));
	if (nullptr == m_pArmorTextureCom)
		return E_FAIL;
	m_mapComponent[ID_STATIC].insert({ L"Com_ArmorTexture", m_pArmorTextureCom });


	//플레이어 콜라이더
	m_pColliderCom = CCollider::Create(m_pGraphicDev,
										_vec3(1.2f, 3.2f, 1.2f),
										_vec3(0.f, 1.3f, 0.f));

	if (nullptr == m_pColliderCom)
		return E_FAIL;
	m_mapComponent[ID_STATIC].insert({ L"Com_Collider", m_pColliderCom });



	//플레이어 공격 콜라이더
	m_pAtkColliderCom = CCollider::Create(m_pGraphicDev,
		_vec3(3.5f, 2.0f, 3.5f), //공격범위 크기
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

	// 화살 / TNT 던지기
	bool bRClick = (GetAsyncKeyState(VK_RBUTTON) & 0x8000);

	if (m_pHeldTNT)
	{
		if (bRClick)
		{
			_vec3 vPos, vLook;
			m_pTransformCom->Get_Info(INFO_POS, &vPos);
			m_pTransformCom->Get_Info(INFO_LOOK, &vLook);
			D3DXVec3Normalize(&vLook, &vLook);
			vPos.y += 3.f;
			m_pHeldTNT->Throw(vPos, -vLook, 10.f);
			m_pHeldTNT = nullptr;
		}
	}
	else
	{
		if (bRClick && m_fBowCooldown <= 0.f)
		{
			//시위 당기기 사운드
			if (!m_bCharging)
			{
				int iIdx = rand() % 3 + 1;
				TCHAR szKey[MAX_PATH];
				wsprintf(szKey, L"Player/sfx_item_CrossBowLoadTwang-%03d.wav", iIdx);
				CSoundMgr::GetInstance()->PlayEffect(szKey, 1.f);
			}
			m_bCharging = true;
			m_fCharge += fTimeDelta;
			if (m_fCharge > m_fMaxCharge)
				m_fCharge = m_fMaxCharge;

			_vec3 vPos;
			m_pTransformCom->Get_Info(INFO_POS, &vPos);
			_vec3 vPickPos = Picking_OnBlock();

			bool bAimedGuardian = false;
			if (m_pTargetGuardian && !m_pTargetGuardian->Is_Dead())
			{
				CCollider* pCol = dynamic_cast<CCollider*>(
					m_pTargetGuardian->Get_Component(ID_STATIC, L"Com_Collider"));
				if (pCol)
				{
					AABB tAABB = pCol->Get_AABB();
					_vec3 vCenter = (tAABB.vMin + tAABB.vMax) * 0.5f;

					_vec3 vToGuardian = { vCenter.x - vPos.x, 0.f, vCenter.z - vPos.z };
					_vec3 vToPick = { vPickPos.x - vPos.x, 0.f, vPickPos.z - vPos.z };
					D3DXVec3Normalize(&vToGuardian, &vToGuardian);
					D3DXVec3Normalize(&vToPick, &vToPick);

					float fDot = D3DXVec3Dot(&vToGuardian, &vToPick);
					if (fDot > 0.5f)
					{
						_vec3 vDir = vCenter - vPos;
						if (D3DXVec3Length(&vDir) > 0.1f)
						{
							D3DXVec3Normalize(&vDir, &vDir);
							m_vBowDir = vDir;
							_vec3 vDirH = { vDir.x, 0.f, vDir.z };
							if (D3DXVec3Length(&vDirH) > 0.01f)
								m_pTransformCom->m_vAngle.y = D3DXToDegree(atan2f(vDirH.x, vDirH.z)) + 180.f;
							bAimedGuardian = true;
						}
					}
				}
			}

			if (!bAimedGuardian)
			{
				_vec3 vDir = vPickPos - vPos;
				vDir.y = 0.f;
				if (D3DXVec3Length(&vDir) > 0.1f)
				{
					D3DXVec3Normalize(&vDir, &vDir);
					m_vBowDir = vDir;
					m_pTransformCom->m_vAngle.y = D3DXToDegree(atan2f(vDir.x, vDir.z)) + 180.f;
				}
			}
		}
		else if (m_bCharging)
		{
			_vec3 vPos;
			m_pTransformCom->Get_Info(INFO_POS, &vPos);
			vPos.y += 1.0f;
			m_fLastChargeRatio = m_fCharge / m_fMaxCharge;

			//화살 발사 사운드
			int iIdx = rand() % 3 + 1;
			TCHAR szKey[MAX_PATH];
			wsprintf(szKey, L"Player/sfx_item_bowShoot-%03d_soundWave.wav", iIdx);
			CSoundMgr::GetInstance()->PlayEffect(szKey, 1.f);

			CPlayerArrow* pArrow = CPlayerArrow::Create(m_pGraphicDev, vPos, m_vBowDir, Get_BowDmg());
			if (pArrow)
			{
				pArrow->Set_Firework(m_bFireworkArrow);
				m_vecArrows.push_back(pArrow);
			}
			m_fBowCooldown = 1.f;
			m_fCharge = 0.f;
			m_bCharging = false;
			m_bFireworkArrow = false;
		}
	}



	if (GetAsyncKeyState('R') & 0x8000)
	{
		if (!m_bRKeyPrev)
		{
			m_bFireworkArrow = !m_bFireworkArrow;
			m_bRKeyPrev = true;
		}
	}
	else
	{
		m_bRKeyPrev = false;
	}

	// 마우스 클릭 이동
	if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
	{
		_vec3 vPickPos = Picking_OnBlock();
		_vec3 vPos;
		m_pTransformCom->Get_Info(INFO_POS, &vPos);

		// 1. 박스 오픈
		for (auto& pBox : CEnvironmentMgr::GetInstance()->Get_Box())
		{
			if (!pBox)
				continue;

			Engine::CTransform* pTrans = dynamic_cast<Engine::CTransform*>
				(pBox->Get_Component(ID_DYNAMIC, L"Com_Transform"));
			
			if (!pTrans)
				continue;

			_vec3 vBoxPos;
			pTrans->Get_Info(INFO_POS, &vBoxPos);
			_vec3 vPlayerDiff = vBoxPos - vPos;
			vPlayerDiff.y = 0.f;
			if (D3DXVec3Length(&vPlayerDiff) >= 2.f)
				continue;

			_vec3 vPickDiff = vPickPos - vBoxPos;
			vPickDiff.y = 0.f;
			if (D3DXVec3Length(&vPickDiff) >= 2.f)
				continue;

			pBox->Open_Box();
			break;
		}
		//NPC 상호작용
		for (auto& pNPC : m_vecNPCs)
		{
			if (!pNPC)
				continue;

			_vec3 vNPCPos;
			pNPC->Get_Transform()->Get_Info(INFO_POS, &vNPCPos);

			_vec3 vPlayerDiff = vNPCPos - vPos;
			vPlayerDiff.y = 0.f;
			if (D3DXVec3Length(&vPlayerDiff) >= 3.f)
				continue;

			_vec3 vPickDiff = vPickPos - vNPCPos;
			vPickDiff.y = 0.f;
			if (D3DXVec3Length(&vPickDiff) >= 3.f)
				continue;

			pNPC->Interact();

			//범위밖으로 나가면 대화창 사라지기.
			break;
		}

		// 2. TNT 줍기
		if (!m_pHeldTNT)
		{
			for (auto& pTNT : m_vecTNTs)
			{
				if (pTNT->Is_Dead() || pTNT->Is_PickedUp())
					continue;

				Engine::CTransform* pTrans = dynamic_cast<Engine::CTransform*>
					(pTNT->Get_Component(ID_DYNAMIC, L"Com_Transform"));

				if (!pTrans)
					continue;
				_vec3 vTNTPos;
				pTrans->Get_Info(INFO_POS, &vTNTPos);
				_vec3 vPlayerDiff = vTNTPos - vPos;
				vPlayerDiff.y = 0.f;

				if (D3DXVec3Length(&vPlayerDiff) >= 2.f)
					continue;

				_vec3 vPickDiff = vPickPos - vTNTPos;
				vPickDiff.y = 0.f;
				if (D3DXVec3Length(&vPickDiff) >= 2.f)
					continue;

				pTNT->PickUp();
				m_pHeldTNT = pTNT;
				break;
			}
		}

		// 3. 몬스터 피킹하면 이동
		bool bMonsterPicked = false;
		for (auto& pair : CMonsterMgr::GetInstance()->Get_MonsterGroups())
		{
			for (auto& pMonster : pair.second.vecMonsters)
			{
				if (!pMonster->IsActive())
					continue;
				CCollider* pCol = pMonster->Get_Collider();
				if (!pCol)
					continue;
				AABB tAABB = pCol->Get_AABB();
				_vec3 vMonsterCenter = (tAABB.vMin + tAABB.vMax) * 0.5f;
				_vec3 vPickDiff = vPickPos - vMonsterCenter;
				vPickDiff.y = 0.f;
				if (D3DXVec3Length(&vPickDiff) >= 2.f)
					continue;

				_vec3 vPlayerDiff = vMonsterCenter - vPos;
				vPlayerDiff.y = 0.f;
				bool bInRange = D3DXVec3Length(&vPlayerDiff) < 2.f;

				if (bInRange && (m_iComboStep == 0 ||
					(m_fAtkTime >= m_fAtkDuration && m_fComboTimer > 0.f)))
				{
					//바로 공격
					m_iComboStep = (m_iComboStep % 3) + 1;
					m_fMeleeDmg = 10.f + (float)(rand() % 6);
					if (m_iComboStep == 1 || m_iComboStep == 2 || m_iComboStep == 3)
						m_fAtkTime = 0.f;
					m_fComboTimer = m_fComboWindow;
					m_bHasTarget = false;

					// 타격사운드
					int iIdx = rand() % 3 + 2;  
					TCHAR szKey[MAX_PATH];
					wsprintf(szKey, L"Player/__cutfast_convert_sfx_misc_swordHit-%03d_soundWave.wav", iIdx);
					CSoundMgr::GetInstance()->PlayEffect(szKey, 1.f);
				}
				else
				{
					//  이동
					m_vTargetPos = vMonsterCenter;
					m_vTargetPos.y = 0.f;
					m_bHasTarget = true;
					m_pTargetMonster = pMonster;
				}
				bMonsterPicked = true;
				break;
			}
			if (bMonsterPicked)
				break;
		}

		// 보스 피킹
		if (!bMonsterPicked && m_pTargetBoss)
		{
			CCollider* pCol = dynamic_cast<CCollider*>(
				m_pTargetBoss->Get_Component(ID_STATIC, L"Com_Collider"));

			if (pCol)
			{
				AABB tAABB = pCol->Get_AABB();
				_vec3 vBossCenter = (tAABB.vMin + tAABB.vMax) * 0.5f;

				_vec3 vPickDiff = vPickPos - vBossCenter;
				vPickDiff.y = 0.f;

				if (D3DXVec3Length(&vPickDiff) < 3.f)
				{
					_vec3 vPlayerDiff = vBossCenter - vPos;
					vPlayerDiff.y = 0.f;
					bool bInRange = D3DXVec3Length(&vPlayerDiff) < 3.f;

					if (bInRange && (m_iComboStep == 0 ||
						(m_fAtkTime >= m_fAtkDuration && m_fComboTimer > 0.f)))
					{
						// 바로 공격
						m_iComboStep = (m_iComboStep % 3) + 1;
						m_fMeleeDmg = 10.f + (float)(rand() % 6);
						m_fAtkTime = 0.f;
						m_fComboTimer = m_fComboWindow;
						m_bHasTarget = false;
						//타격 사운드
						int iIdx = rand() % 3 + 2;
						TCHAR szKey[MAX_PATH];
						wsprintf(szKey, L"Player/__cutfast_convert_sfx_misc_swordHit-%03d_soundWave.wav", iIdx);
						CSoundMgr::GetInstance()->PlayEffect(szKey, 1.f);
					}
					else
					{
						// 이동
						m_vTargetPos = vBossCenter;
						m_vTargetPos.y = 0.f;
						m_bHasTarget = true;
						m_pTargetMonster = nullptr;
					}
					bMonsterPicked = true;
				}
			}
		}


		// 4. 일반 이동
		if (!bMonsterPicked)
		{
			m_vTargetPos = vPickPos;
			m_vTargetPos.y = 0.f;
			m_bHasTarget = true;
			m_pTargetMonster = nullptr;
		}
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
			if(!m_bCharging)
			m_pTransformCom->m_vAngle.y = D3DXToDegree(atan2f(vDir.x, vDir.z)) + 180.f;
			m_pTransformCom->Move_Pos(&vDir, m_fMoveSpeed, fTimeDelta);
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
				m_vRollDir = -vLook;		// 부호 복구
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

void CPlayer::Render_Part(BODYPART ePart, _float fAngleX, _float fAngleY, _float fAngleZ,
							const _matrix& matRootWorld, Engine::CTexture* pTex, CPlayerBody* pBuf)
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

	// 어깨 고정, 팔 수평 회전
	_matrix matPartWorld = matScale * matPivotDown * matRotX * matRotY * matJoint * matRollLocal * matRootWorld;

	if (ePart == PART_RARM)
		m_matRArmWorld = matPartWorld;

	if (ePart == PART_LARM)
		m_matLArmWorld = matPartWorld;

	m_matPartWorld[ePart] = matPartWorld;

	m_pGraphicDev->SetTransform(D3DTS_WORLD, &matPartWorld);
	if (pTex)
		pTex->Set_Texture(0);
	else
		m_pTextureCom->Set_Texture(0);

	if (pBuf)
		pBuf->Render_Buffer();	
	else
		m_pBufferCom[ePart]->Render_Buffer();
}

void CPlayer::Calc_AttackMotion(float& fAtkX, float& fAtkY, float& fTorsoY)
{
	if (m_iComboStep == 0)
		return;

	float fRatio = min(m_fAtkTime / m_fAtkDuration, 1.f);
	float fDir = (m_iComboStep == 2) ? 1.f : -1.f;

	if (m_iComboStep == 1)
	{
		if (fRatio < 0.3f)
		{
			m_pAtkColliderCom->Update_AABB(_vec3(0.f, -9999.f, 0.f));
			m_bAtkColliderActive = false;
			float t = fRatio / 0.3f;
			fAtkX = D3DXToRadian(-90.f * t);
			fAtkY = D3DXToRadian(5.f * fDir * t);
			fTorsoY = D3DXToRadian(-40.f * fDir * t);
		}
		else
		{
			m_bAtkColliderActive = true;
			float t = (fRatio - 0.3f) / 0.7f;
			fAtkX = D3DXToRadian(-90.f);
			fAtkY = D3DXToRadian(25.f * fDir + 180.f * fDir * t);
			fTorsoY = D3DXToRadian(-40.f * fDir + 80.f * fDir * t);
		}
	}
	else if (m_iComboStep == 2)
	{
		if (fRatio < 0.3f)
		{
			m_pAtkColliderCom->Update_AABB(_vec3(0.f, -9999.f, 0.f));
			m_bAtkColliderActive = false;
			float t = fRatio / 0.3f;
			fAtkX = D3DXToRadian(-90.f * t);
			fAtkY = D3DXToRadian(25.f * fDir * t);
			fTorsoY = D3DXToRadian(-40.f * fDir * t);
		}
		else
		{
			m_bAtkColliderActive = true;
			float t = (fRatio - 0.3f) / 0.7f;
			fAtkX = D3DXToRadian(-90.f);
			fAtkY = D3DXToRadian(180.f * fDir + 100.f * fDir * t);
			fTorsoY = D3DXToRadian(-40.f * fDir + 80.f * fDir * t);
		}
	}
	else if (m_iComboStep == 3)
	{
		m_bAtkColliderActive = true;
		fAtkX = D3DXToRadian(90.f * fRatio);
		fAtkY = 0.f;
		fTorsoY = 0.f;
	}
}

void CPlayer::Render_Sword(float fAtkX, float fAtkY, float fSwing)
{
	// 손끝 위치
	_vec3 vHandLocal(0.f, -1.f, 0.f);
	_vec3 vHandPos;
	D3DXVec3TransformCoord(&vHandPos, &vHandLocal, &m_matRArmWorld);

	float fSwordLen = m_vPartScale[PART_RARM].y * 2.f;

	// 팔 회전 추출 (스케일 제거)
	_matrix matArmRot = m_matRArmWorld;
	D3DXVec3Normalize((_vec3*)&matArmRot._11, (_vec3*)&matArmRot._11);
	D3DXVec3Normalize((_vec3*)&matArmRot._21, (_vec3*)&matArmRot._21);
	D3DXVec3Normalize((_vec3*)&matArmRot._31, (_vec3*)&matArmRot._31);
	matArmRot._41 = 0.f; matArmRot._42 = 0.f; matArmRot._43 = 0.f; matArmRot._44 = 1.f;

	// 칼 tilt: 대기 -70도(직각), 1·2타 공격 중 180도(1자), 3타 서서히 1자
	float fTiltAngle = D3DXToRadian(-70.f);
	if (m_iComboStep == 1 || m_iComboStep == 2)
	{
		float fRatio = min(m_fAtkTime / m_fAtkDuration, 1.f);
		fTiltAngle = (fRatio < 0.3f) ? D3DXToRadian(110.f) + fAtkX : D3DXToRadian(180.f);
	}
	else if (m_iComboStep == 3)
	{
		float fRatio = min(m_fAtkTime / m_fAtkDuration, 1.f);
		fTiltAngle = D3DXToRadian(-70.f + 260.f * fRatio);
	}

	_matrix matScale, matPivot, matTilt, matSwing, matStand, matRotY, matTrans;
	D3DXMatrixScaling(&matScale, fSwordLen * 0.25f, fSwordLen, 1.f);
	D3DXMatrixTranslation(&matPivot, 0.1f, fSwordLen * 0.7f, 0.f);
	D3DXMatrixRotationZ(&matTilt, fTiltAngle);
	D3DXMatrixRotationX(&matSwing, m_iComboStep > 0 ? fAtkX : fSwing);
	D3DXMatrixRotationX(&matStand, D3DXToRadian(90.f));
	D3DXMatrixRotationY(&matRotY, D3DXToRadian(m_pTransformCom->m_vAngle.y) + D3DXToRadian(90.f) + fAtkY);
	D3DXMatrixTranslation(&matTrans, vHandPos.x, vHandPos.y, vHandPos.z);

	_matrix matSwordWorld;
	if (m_iComboStep == 1 || m_iComboStep == 2)
	{
		_matrix matPivotArm;
		D3DXMatrixTranslation(&matPivotArm, 0.f, fSwordLen * 0.5f, 0.f);
		matSwordWorld = matScale * matPivotArm * matTilt * matArmRot * matTrans;
	}
	else if (m_iComboStep == 3)
	{
		_matrix matPivotArm;
		D3DXMatrixTranslation(&matPivotArm, 0.f, fSwordLen * 0.7f, 0.f);
		matSwordWorld = matScale * matPivotArm * matTilt * matArmRot * matTrans;
	}
	else
		matSwordWorld = matScale * matPivot * matTilt * matSwing * matStand * matRotY * matTrans;

	m_pGraphicDev->SetTransform(D3DTS_WORLD, &matSwordWorld);
	m_pSwordTextureCom->Set_Texture(0);
	m_pGraphicDev->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	m_pGraphicDev->SetRenderState(D3DRS_ALPHAREF, 128);
	m_pGraphicDev->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	m_pSwordBufferCom->Render_Buffer();
	m_pGraphicDev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
}

void CPlayer::Render_Bow()
{
	// 왼손 위치 추출
	_vec3 vHandLocal(0.f, -1.f, 0.f);
	_vec3 vLArmPos;
	D3DXVec3TransformCoord(&vLArmPos, &vHandLocal, &m_matRArmWorld);

	float fBowSize = m_vPartScale[PART_LARM].y * 2.f;

	// 차징 단계에 따라 텍스처 선택
	int iTexIdx = 0;
	if (m_bCharging)
	{
		float fRatio = m_fCharge / m_fMaxCharge;
		if (fRatio < 0.33f)      iTexIdx = 1;
		else if (fRatio < 0.66f) iTexIdx = 2;
		else                     iTexIdx = 3;
	}

	_matrix matScale, matRot, matTrans, matBowWorld, matRotZ;
	D3DXMatrixScaling(&matScale, fBowSize*0.7f, fBowSize*0.7f, 1.f);
	D3DXMatrixRotationZ(&matRotZ, D3DXToRadian(40.f));
	D3DXMatrixRotationY(&matRot, D3DXToRadian(m_pTransformCom->m_vAngle.y) + D3DXToRadian(270.f));
	D3DXMatrixTranslation(&matTrans, vLArmPos.x, vLArmPos.y, vLArmPos.z);
	matBowWorld = matScale * matRotZ * matRot * matTrans;

	m_pGraphicDev->SetTransform(D3DTS_WORLD, &matBowWorld);
	m_pBowTexture[iTexIdx]->Set_Texture(0);
	m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_pGraphicDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_pGraphicDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	m_pGraphicDev->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	m_pGraphicDev->SetRenderState(D3DRS_ALPHAREF, 128);
	m_pGraphicDev->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	m_pBowBufferCom->Render_Buffer();
	m_pGraphicDev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
}

void CPlayer::Attack_Collision()
{
	if (m_iComboStep > 0 && m_bAtkColliderActive)
	{
		_vec3 vPos, vLook;
		m_pTransformCom->Get_Info(INFO_POS, &vPos);
		m_pTransformCom->Get_Info(INFO_LOOK, &vLook);
		D3DXVec3Normalize(&vLook, &vLook);
		_vec3 vAtkPos = vPos - vLook * 0.8f;
		vAtkPos.y += 0.9f;
		m_pAtkColliderCom->Update_AABB(vAtkPos);
	}
}

void CPlayer::Equip(eEquipType eType)
{
	switch (eType)
	{
	case eEquipType::MELEE: m_bSwordEquipped = true;            
		break;
	case eEquipType::ARMOR: m_eArmorType = ARMOR_BARDSGARD;     
		break;
	case eEquipType::RANGED:   m_bBowEquipped = true;              
		break;
	}
}

void CPlayer::UnEquip(eEquipType eType)
{
	switch (eType)
	{
	case eEquipType::MELEE: m_bSwordEquipped = false;            
		break;
	case eEquipType::ARMOR: m_eArmorType = ARMOR_NONE;		
		break;
	case eEquipType::RANGED:   m_bBowEquipped = false;              
		break;
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

				// 계단 올라가기
				if (fabsf(vResolve.y) == 0.f && (fabsf(vResolve.x) > 0.f || fabsf(vResolve.z) > 0.f))
				{
					BlockPos tAbove = { tBlockPos.x, tBlockPos.y + 1, tBlockPos.z };
					BlockPos tAbove2 = { tBlockPos.x, tBlockPos.y + 2, tBlockPos.z };
					if (!CBlockMgr::GetInstance()->HasBlock(tAbove) &&
						!CBlockMgr::GetInstance()->HasBlock(tAbove2))
					{
						vPos.y += 1.f;
						m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);
						m_pColliderCom->Update_AABB(vPos);
						tPlayerAABB = m_pColliderCom->Get_AABB();
					}
				}
			}
		}
	}
}

void CPlayer::Hit(float fDamage)
{
	if (m_bHit)
		return;
	m_bHit = true;
	m_fHitTime = 0.f;
	m_fHp -= fDamage;
	if (m_fHp < 0.f) m_fHp = 0.f;

	// 넉백값 이상이면 넉백
	if (fDamage >= m_fKnockbackThreshold)
	{
		_vec3 vLook;
		m_pTransformCom->Get_Info(INFO_LOOK, &vLook);
		D3DXVec3Normalize(&vLook, &vLook);
		m_vKnockbackDir = vLook;  // 바라보는 반대로 날아감
		m_bKnockback = true;
		m_fKnockbackTime = 0.f;
	}
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
	//Effect Release
	Safe_Release(m_pFootStepEmitter);
	Safe_Release(m_pAttackEmitter);

	for (auto& pArrow : m_vecArrows)
		Safe_Release(pArrow);
	m_vecArrows.clear();
	CGameObject::Free();
}