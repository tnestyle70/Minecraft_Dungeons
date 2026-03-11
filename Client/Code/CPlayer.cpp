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

	m_pTransformCom->Set_Pos(0.f, 5.f, 0.f);
	// ========== 파트별 스케일 (CCubeTex는 -1~1 단위 → 실제크기 = scale * 2) ==========
	// 머리: 0.4 x 0.4 x 0.4
	m_vPartScale[PART_HEAD] = { 0.20f, 0.20f, 0.20f };
	// 몸통: 0.5 x 0.7 x 0.25
	m_vPartScale[PART_BODY] = { 0.25f, 0.35f, 0.125f };
	// 왼팔: 0.2 x 0.6 x 0.2
	m_vPartScale[PART_LARM] = { 0.10f, 0.30f, 0.10f };
	// 오른팔
	m_vPartScale[PART_RARM] = { 0.10f, 0.30f, 0.10f };
	// 왼다리: 0.2 x 0.6 x 0.2
	m_vPartScale[PART_LLEG] = { 0.10f, 0.30f, 0.10f };
	// 오른다리
	m_vPartScale[PART_RLEG] = { 0.10f, 0.30f, 0.10f };

	// ========== 파트별 오프셋 (루트 = 발 중심 기준) ==========
	// 머리: 몸통 위 (몸통 높이 0.7 + 머리 반높이 0.2)
	m_vPartOffset[PART_HEAD] = { 0.00f,  1.10f,  0.00f };
	// 몸통 중심
	m_vPartOffset[PART_BODY] = { 0.00f,  0.60f,  0.00f };
	// 왼팔: 몸통 왼쪽 (몸통 반너비 0.25 + 팔 반너비 0.10)
	m_vPartOffset[PART_LARM] = { -0.35f, 0.60f,  0.00f };
	// 오른팔
	m_vPartOffset[PART_RARM] = { 0.35f, 0.60f,  0.00f };
	// 왼다리: 몸통 아래 절반 (다리 반높이 0.30)
	m_vPartOffset[PART_LLEG] = { -0.13f, 0.30f,  0.00f };
	// 오른다리
	m_vPartOffset[PART_RLEG] = { 0.13f, 0.30f,  0.00f };

	return S_OK;
}

_int CPlayer::Update_GameObject(const _float& fTimeDelta)
{
	_int iExit = CGameObject::Update_GameObject(fTimeDelta);

	if (m_bMoving)
		m_fWalkTime += fTimeDelta * 8.f;

	_vec3 vPos;
	m_pTransformCom->Get_Info(INFO_POS, &vPos);
	//AABB 업데이트
	m_pColliderCom->Update_AABB(vPos);

	Apply_Gravity(fTimeDelta);
	Resolve_BlockCollision();
	//Set_OnTerrain();

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
	m_pGraphicDev->SetTransform(D3DTS_WORLD, m_pTransformCom->Get_World());

	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	const _float fMaxAngle = D3DXToRadian(30.f);
	_float fSwing = m_bMoving ? sinf(m_fWalkTime) * fMaxAngle : 0.f;

	Render_Part(PART_HEAD, 0.f);
	Render_Part(PART_BODY, 0.f);
	Render_Part(PART_LARM, fSwing);	// 왼팔 앞
	Render_Part(PART_RARM, -fSwing);	// 오른팔 뒤
	Render_Part(PART_LLEG, -fSwing);	// 왼다리 뒤
	Render_Part(PART_RLEG, fSwing);	// 오른다리 앞

	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

	//collider rendering
	m_pColliderCom->Render_Collider();
}

HRESULT CPlayer::Add_Component()
{
	Engine::CComponent* pComponent = nullptr;

	// ===== 마인크래프트 스킨 UV 정의 =====
	// 면 순서: [0]FRONT(Z-) [1]BACK(Z+) [2]TOP(Y+) [3]BOT(Y-) [4]LEFT(X-) [5]RIGHT(X+)
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

	FACE_UV* uvTable[PART_END] = { uvHead, uvBody, uvLArm, uvRArm, uvLLeg, uvRLeg };
	const wchar_t* tagTable[PART_END] = {
		L"Com_HeadBuf", L"Com_BodyBuf",
		L"Com_LArmBuf", L"Com_RArmBuf",
		L"Com_LLegBuf", L"Com_RLegBuf"
	};

	for (_uint i = 0; i < PART_END; ++i)
	{
		m_pBufferCom[i] = CPlayerBody::Create(m_pGraphicDev, uvTable[i]);
		if (nullptr == m_pBufferCom[i])
			return E_FAIL;
		m_mapComponent[ID_STATIC].insert({ tagTable[i], m_pBufferCom[i] });
	}

	// Texture (Proto_PlayerTexture → 스킨 파일 경로로 교체 필요)
	pComponent = m_pTextureCom = dynamic_cast<Engine::CTexture*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_PlayerTexture"));
	if (nullptr == pComponent)
		return E_FAIL;
	m_mapComponent[ID_STATIC].insert({ L"Com_Texture", pComponent });

	// Transform
	pComponent = m_pTransformCom = dynamic_cast<Engine::CTransform*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));
	if (nullptr == pComponent)
		return E_FAIL;
	m_mapComponent[ID_DYNAMIC].insert({ L"Com_Transform", pComponent });

	// Collider (플레이어 크기에 맞게 scale, offset 설정)
	m_pColliderCom = CCollider::Create(m_pGraphicDev,
		_vec3(0.5f, 1.8f, 0.5f),	// scale: 너비 x 높이 x 깊이
		_vec3(0.f, 0.9f, 0.f));		// offset: 발 기준이므로 Y 절반 위로
	if (nullptr == m_pColliderCom)
		return E_FAIL;
	m_mapComponent[ID_STATIC].insert({ L"Com_Collider", m_pColliderCom });

	// Calculator
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


	if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
	{
		m_vTargetPos = Picking_OnBlock();
		m_vTargetPos.y = 0.f;  // Y는 중력으로 처리
		m_bHasTarget = true;
	}

	// 목적지를 향해 이동
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
			m_pTransformCom->m_vAngle.y = D3DXToDegree(atan2f(vDir.x, vDir.z)) + 180.f;
			m_pTransformCom->Move_Pos(&vDir, 5.f, fTimeDelta);
			m_bMoving = true;
		}
		else
		{
			m_bHasTarget = false;
			m_bMoving = false;
		}
	}

	//플레이어 점프 적용
	if (CDInputMgr::GetInstance()->Get_DIKeyState(DIK_SPACE))
	{
		if (m_bOnGround)
		{
			m_fVelocityY = m_fJumpPower;
			m_bOnGround = false;
		}
	}
	//if (GetAsyncKeyState(VK_SPACE) & 0x8000)
	//{
	//	if (m_bOnGround)
	//	{
	//		m_fVelocityY = m_fJumpPower;
	//		m_bOnGround = false;
	//	}
	//}
}

void CPlayer::Set_OnTerrain()
{
	_vec3	vPos;
	m_pTransformCom->Get_Info(INFO_POS, &vPos);

	Engine::CTerrainTex* pTerrainVtxCom = dynamic_cast<Engine::CTerrainTex*>
		(CManagement::GetInstance()->Get_Component(ID_STATIC, L"GameLogic_Layer", L"Terrain", L"Com_Buffer"));

	if (nullptr == pTerrainVtxCom)
		return;

	_float fY = m_pCalculatorCom->Compute_HeightOnTerrain(&vPos, pTerrainVtxCom->Get_VtxPos(), VTXCNTX, VTXCNTZ);

	m_pTransformCom->Set_Pos(vPos.x, fY + 1.f, vPos.z);
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

	// 가장 가까운 블럭 AABB와 교차 검사
	float fMinT = FLT_MAX;
	_vec3 vHit = _vec3(0.f, 0.f, 0.f);
	bool bHit = false;

	for (auto& pair : CBlockMgr::GetInstance()->Get_Blocks())
	{
		AABB tAABB = CBlockMgr::GetInstance()->Get_BlockAABB(pair.first);

		// 레이 vs AABB (slab method)
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
			vHit.y = tAABB.vMax.y;  // 블럭 윗면으로 스냅
			bHit = true;
		}
	}

	// 블럭 못 찾으면 현재 위치 반환
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

//_vec3 CPlayer::Picking_OnTerrain()
//{
//	Engine::CTerrainTex* pTerrainVtxCom = dynamic_cast<Engine::CTerrainTex*>
//		(CManagement::GetInstance()->Get_Component(ID_STATIC, L"GameLogic_Layer", L"Terrain", L"Com_Buffer"));
//
//	if (nullptr == pTerrainVtxCom)
//		return _vec3();
//
//	Engine::CTransform* pTerrainTransformCom = dynamic_cast<Engine::CTransform*>
//		(CManagement::GetInstance()->Get_Component(ID_DYNAMIC, L"GameLogic_Layer", L"Terrain", L"Com_Transform"));
//
//	if (nullptr == pTerrainTransformCom)
//		return _vec3();
//
//	return m_pCalculatorCom->Picking_OnTerrain(g_hWnd, pTerrainVtxCom, pTerrainTransformCom);
//}

void CPlayer::Render_Part(BODYPART ePart, _float fAngle)
{
	// 스케일 행렬
	_matrix matScale;
	D3DXMatrixScaling(&matScale,
		m_vPartScale[ePart].x,
		m_vPartScale[ePart].y,
		m_vPartScale[ePart].z);

	// 피벗 = 관절을 원점으로 내림 (파트 상단 = 관절)
	_matrix matPivotDown;
	D3DXMatrixTranslation(&matPivotDown, 0.f, -m_vPartScale[ePart].y, 0.f);

	// X축 회전 (앞뒤 흔들기)
	_matrix matRot;
	D3DXMatrixRotationX(&matRot, fAngle);

	// 관절 위치로 이동 (오프셋 + 파트 반높이)
	_matrix matJoint;
	D3DXMatrixTranslation(&matJoint,
		m_vPartOffset[ePart].x,
		m_vPartOffset[ePart].y + m_vPartScale[ePart].y,
		m_vPartOffset[ePart].z);

	// Scale → 피벗 → 회전 → 관절위치 → 루트월드
	_matrix matPartWorld = matScale * matPivotDown * matRot * matJoint * (*m_pTransformCom->Get_World());

	m_pGraphicDev->SetTransform(D3DTS_WORLD, &matPartWorld);
	m_pTextureCom->Set_Texture(0);
	m_pBufferCom[ePart]->Render_Buffer();
}

void CPlayer::Apply_Gravity(const _float& fTimeDelta)
{
	if (m_bOnGround)
		return;
	//중력 누적
	m_fVelocityY += m_fGravity * fTimeDelta;
	//최대 낙하 속도 제한
	if (m_fVelocityY < m_fMaxFall)
	{
		m_fVelocityY = m_fMaxFall;
	}
	//Y 이동 적용
	_vec3 vPos;
	m_pTransformCom->Get_Info(INFO_POS, &vPos);
	vPos.y += m_fVelocityY * fTimeDelta;
	m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);
}

void CPlayer::Resolve_BlockCollision()
{
	_vec3 vPos;
	m_pTransformCom->Get_Info(INFO_POS, &vPos);
	//콜라이더를 현재 위치로 이동
	m_pColliderCom->Update_AABB(vPos);
	//플레이어 AABB 가지고 오기
	AABB tPlayerAABB = m_pColliderCom->Get_AABB();

	m_bOnGround = false;

	//플레이어 주변 블럭 체크
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
				//blockpos 기준으로 충돌 resolve
				BlockPos tBlockPos = { x, y, z };
				//블럭 없으면 스킵
				if (!CBlockMgr::GetInstance()->HasBlock(tBlockPos))
				{
					OutputDebugString(L"not block \n");
					continue;
				}
				//븝럭 AABB 가져오기
				AABB tBlockAABB = CBlockMgr::GetInstance()->Get_BlockAABB(tBlockPos);
				//충돌 체크
				if (!m_pColliderCom->IsColliding(tBlockAABB))
				{
					OutputDebugString(L"not colliding \n");
					continue;
				}
				OutputDebugString(L"colliding \n");
				//충돌 보정값 vector 받아오기
				_vec3 vResolve = m_pColliderCom->Resolve(tBlockAABB);
				//위치 보정
				vPos += vResolve;
				m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);
				//Colldier 위치도 같이 변경
				m_pColliderCom->Update_AABB(vPos);
				tPlayerAABB = m_pColliderCom->Get_AABB();

				if (fabsf(vResolve.y) > 0.f)
				{
					//위에서 내려왔을 경우 바닥 충돌
					if (vResolve.y > 0.f)
					{
						m_bOnGround = true;
						m_fVelocityY = 0.f;
					}
					//아래에서 올라왔을 경우 천장 충돌
					else
					{
						m_fVelocityY = 0.f;
					}
				}
			}
		}
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