#include "pch.h"
#include "CRemotePlayer.h"
#include "CPlayer.h"   // BODYPART enum (PART_HEAD … PART_END)
#include "CFontMgr.h"
#include <cstring>
#include <cmath>

CRemotePlayer::CRemotePlayer(LPDIRECT3DDEVICE9 pGraphicDev)
    : Engine::CGameObject(pGraphicDev)
{}

CRemotePlayer::~CRemotePlayer()
{}

// =====================================================================
//  Ready_GameObject  —  버퍼 / 텍스처 생성
// =====================================================================
HRESULT CRemotePlayer::Ready_GameObject()
{
    // ── UV 테이블 (CPlayer::Add_Component 와 동일) ─────────────────────
    FACE_UV uvHead[6] = {
        {0.125f, 0.125f, 0.25f,  0.25f },   // FRONT
        {0.375f, 0.125f, 0.5f,   0.25f },   // BACK
        {0.125f, 0.0f,   0.25f,  0.125f},   // TOP
        {0.25f,  0.0f,   0.375f, 0.125f},   // BOT
        {0.0f,   0.125f, 0.125f, 0.25f },   // LEFT
        {0.25f,  0.125f, 0.375f, 0.25f },   // RIGHT
    };
    FACE_UV uvBody[6] = {
        {0.3125f, 0.3125f, 0.4375f, 0.5f   },  // FRONT
        {0.5f,    0.3125f, 0.625f,  0.5f   },  // BACK
        {0.3125f, 0.25f,   0.4375f, 0.3125f},  // TOP
        {0.4375f, 0.25f,   0.5625f, 0.3125f},  // BOT
        {0.25f,   0.3125f, 0.3125f, 0.5f   },  // LEFT
        {0.4375f, 0.3125f, 0.5f,    0.5f   },  // RIGHT
    };
    FACE_UV uvRArm[6] = {
        {0.6875f, 0.3125f, 0.75f,   0.5f   },  // FRONT
        {0.8125f, 0.3125f, 0.875f,  0.5f   },  // BACK
        {0.6875f, 0.25f,   0.75f,   0.3125f},  // TOP
        {0.75f,   0.25f,   0.8125f, 0.3125f},  // BOT
        {0.625f,  0.3125f, 0.6875f, 0.5f   },  // LEFT
        {0.75f,   0.3125f, 0.8125f, 0.5f   },  // RIGHT
    };
    FACE_UV uvLArm[6] = {
        {0.5625f, 0.8125f, 0.625f,  1.0f   },  // FRONT
        {0.6875f, 0.8125f, 0.75f,   1.0f   },  // BACK
        {0.5625f, 0.75f,   0.625f,  0.8125f},  // TOP
        {0.625f,  0.75f,   0.6875f, 0.8125f},  // BOT
        {0.5f,    0.8125f, 0.5625f, 1.0f   },  // LEFT
        {0.625f,  0.8125f, 0.6875f, 1.0f   },  // RIGHT
    };
    FACE_UV uvRLeg[6] = {
        {0.0625f, 0.3125f, 0.125f,  0.5f   },  // FRONT
        {0.1875f, 0.3125f, 0.25f,   0.5f   },  // BACK
        {0.0625f, 0.25f,   0.125f,  0.3125f},  // TOP
        {0.125f,  0.25f,   0.1875f, 0.3125f},  // BOT
        {0.0f,    0.3125f, 0.0625f, 0.5f   },  // LEFT
        {0.125f,  0.3125f, 0.1875f, 0.5f   },  // RIGHT
    };
    FACE_UV uvLLeg[6] = {
        {0.3125f, 0.8125f, 0.375f,  1.0f   },  // FRONT
        {0.4375f, 0.8125f, 0.5f,    1.0f   },  // BACK
        {0.3125f, 0.75f,   0.375f,  0.8125f},  // TOP
        {0.375f,  0.75f,   0.4375f, 0.8125f},  // BOT
        {0.25f,   0.8125f, 0.3125f, 1.0f   },  // LEFT
        {0.375f,  0.8125f, 0.4375f, 1.0f   },  // RIGHT
    };

    FACE_UV* uvTable[PART_END] = { uvHead, uvBody, uvLArm, uvRArm, uvLLeg, uvRLeg };

    for (int i = 0; i < PART_END; ++i)
    {
        m_pBufferCom[i] = CPlayerBody::Create(m_pGraphicDev, uvTable[i]);
        if (!m_pBufferCom[i])
            return E_FAIL;
    }

    m_pTextureCom = dynamic_cast<Engine::CTexture*>(
        CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_PlayerTexture"));

    if (!m_pTextureCom)
        return E_FAIL;

    // ── 방어구(로브) UV 테이블 — CPlayer::Add_Component 와 동일 ──────────
    FACE_UV uvArmorHead[6] = {
        {0.31250f, 0.00000f, 0.46875f, 0.14063f},  // FRONT
        {0.00000f, 0.00000f, 0.15625f, 0.14063f},  // BACK
        {0.15625f, 0.00000f, 0.31250f, 0.14063f},  // TOP
        {0.15625f, 0.00000f, 0.31250f, 0.14063f},  // BOT
        {0.15625f, 0.00000f, 0.31250f, 0.14063f},  // LEFT
        {0.15625f, 0.00000f, 0.31250f, 0.14063f},  // RIGHT
    };
    FACE_UV uvArmorBody[6] = {
        {0.09375f, 0.50000f, 0.25000f, 0.68750f},  // FRONT
        {0.34375f, 0.50000f, 0.50000f, 0.68750f},  // BACK
        {0.25000f, 0.42188f, 0.09375f, 0.50000f},  // TOP
        {0.09375f, 0.50000f, 0.25000f, 0.68750f},  // BOT
        {0.00000f, 0.50000f, 0.09375f, 0.68750f},  // LEFT
        {0.25000f, 0.50000f, 0.34375f, 0.68750f},  // RIGHT
    };
    FACE_UV uvArmorLArm[6] = {
        {0.46875f, 0.18750f, 0.60938f, 0.31250f},  // FRONT
        {0.75000f, 0.18750f, 0.89063f, 0.31250f},  // BACK
        {0.31250f, 0.14063f, 0.45313f, 0.28125f},  // TOP
        {0.31250f, 0.14063f, 0.45313f, 0.28125f},  // BOT
        {0.00000f, 0.00000f, 0.00000f, 0.00000f},  // LEFT
        {0.60938f, 0.18750f, 0.75000f, 0.31250f},  // RIGHT
    };
    FACE_UV uvArmorRArm[6] = {
        {0.46875f, 0.00000f, 0.60938f, 0.12500f},  // FRONT
        {0.75000f, 0.00000f, 0.89063f, 0.12500f},  // BACK
        {0.31250f, 0.14063f, 0.45313f, 0.28125f},  // TOP
        {0.31250f, 0.14063f, 0.45313f, 0.28125f},  // BOT
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
        {0.50000f, 0.68750f, 0.59375f, 0.81250f},  // LEFT
        {0.00000f, 0.68750f, 0.09375f, 0.81250f},  // RIGHT
    };

    FACE_UV* uvArmorTable[PART_END] = {
        uvArmorHead, uvArmorBody, uvArmorLArm, uvArmorRArm, uvArmorLLeg, uvArmorRLeg
    };

    for (int i = 0; i < PART_END; ++i)
    {
        m_pArmorBufferCom[i] = CPlayerBody::Create(m_pGraphicDev, uvArmorTable[i]);
        if (!m_pArmorBufferCom[i])
            return E_FAIL;
    }

    m_pArmorTextureCom = dynamic_cast<Engine::CTexture*>(
        CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_ArmorTexture"));
    if (!m_pArmorTextureCom)
        return E_FAIL;

    // ── 파트별 크기 / 오프셋 (CPlayer 와 동일) ────────────────────────
    m_vPartScale[PART_HEAD] = { 0.40f, 0.40f, 0.40f };
    m_vPartScale[PART_BODY] = { 0.50f, 0.50f, 0.25f };
    m_vPartScale[PART_LARM] = { 0.20f, 0.60f, 0.20f };
    m_vPartScale[PART_RARM] = { 0.20f, 0.60f, 0.20f };
    m_vPartScale[PART_LLEG] = { 0.20f, 0.60f, 0.20f };
    m_vPartScale[PART_RLEG] = { 0.20f, 0.60f, 0.20f };

    m_vPartOffset[PART_HEAD] = { 0.00f,  2.20f, 0.00f };
    m_vPartOffset[PART_BODY] = { 0.00f,  1.40f, 0.00f };
    m_vPartOffset[PART_LARM] = { 0.70f,  1.20f, 0.00f };
    m_vPartOffset[PART_RARM] = { -0.70f, 1.20f, 0.00f };
    m_vPartOffset[PART_LLEG] = { 0.26f,  0.45f, 0.00f };
    m_vPartOffset[PART_RLEG] = { -0.26f, 0.45f, 0.00f };

    return S_OK;
}

// =====================================================================
//  InitSpawn  —  S2C_Spawn 수신 시 최초 설정
// =====================================================================
void CRemotePlayer::InitSpawn(int iPlayerId,
    float fX, float fY, float fZ,
    float fRotY, const char* szNickname)
{
    m_iPlayerId = iPlayerId;
    strncpy_s(m_szNickname, szNickname, _TRUNCATE);

    m_fTargetX = fX;
    m_fTargetY = fY;
    m_fTargetZ = fZ;
    m_fTargetRotY = fRotY;
    m_fCurX = fX;
    m_fCurY = fY;
    m_fCurZ = fZ;
    m_fCurRotY = fRotY;
}

// =====================================================================
//  SetTargetState  —  S2C_StateSnapshot 수신 시 목표값 갱신
// =====================================================================
void CRemotePlayer::SetTargetState(float fX, float fY, float fZ,
    float fRotY, int iState, int iSequence, bool bOnDragon)
{
    // iSequence != -1: 역전된 오래된 스냅샷이면 무시 (패킷 재정렬 방지)
    if (iSequence != -1 && iSequence <= m_iLastSequence)
        return;
    if (iSequence != -1)
        m_iLastSequence = iSequence;

    m_fTargetX = fX;
    m_fTargetY = fY;
    m_fTargetZ = fZ;
    m_fTargetRotY = fRotY;
    m_iTargetState = iState;
    m_bMoving = (iState == 1);
    m_bOnDragon = bOnDragon;
}

// =====================================================================
//  Update_GameObject  —  현재 위치를 목표 위치로 Lerp
// =====================================================================
_int CRemotePlayer::Update_GameObject(const _float& fTimeDelta)
{
    float fT = LERP_SPEED * fTimeDelta;
    if (fT > 1.f) fT = 1.f;

    m_fCurX += (m_fTargetX - m_fCurX) * fT;
    m_fCurY += (m_fTargetY - m_fCurY) * fT;
    m_fCurZ += (m_fTargetZ - m_fCurZ) * fT;

    // 각도 Lerp: 차이를 [-PI, +PI] 범위로 정규화해서 항상 짧은 방향으로 회전
    float fRotDiff = m_fTargetRotY - m_fCurRotY;
    while (fRotDiff > D3DX_PI) fRotDiff -= D3DX_PI * 2.f;
    while (fRotDiff < -D3DX_PI) fRotDiff += D3DX_PI * 2.f;
    m_fCurRotY += fRotDiff * fT;

    if (m_bMoving)
        m_fWalkTime += fTimeDelta * 8.f;

    return 0;
}

// =====================================================================
//  LateUpdate_GameObject
// =====================================================================
void CRemotePlayer::LateUpdate_GameObject(const _float& fTimeDelta)
{
    // 렌더러에 등록하지 않고 CNetworkMgr::Render()에서 직접 호출
}

// =====================================================================
//  Render_Part  —  CPlayer::Render_Part 와 동일한 행렬 구성
// =====================================================================
void CRemotePlayer::Render_Part(int ePart, float fAngleX, float fAngleY, float fAngleZ,
    const _matrix& matRootWorld)
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

    _matrix matPartWorld = matScale * matPivotDown * matRotX * matRotY * matJoint * matRootWorld;

    m_matPartWorld[ePart] = matPartWorld;   // 방어구 2패스에서 재사용

    m_pGraphicDev->SetTransform(D3DTS_WORLD, &matPartWorld);
    m_pTextureCom->Set_Texture(0);
    m_pBufferCom[ePart]->Render_Buffer();
}

// =====================================================================
//  Render_GameObject  —  6파트 렌더링 (걷기 애니메이션 포함)
// =====================================================================
void CRemotePlayer::Render_GameObject()
{
    if (!m_pTextureCom) return;
    for (int i = 0; i < PART_END; ++i)
        if (!m_pBufferCom[i]) return;

    m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

    // ── 루트 월드 행렬: Y축 회전 + 위치 이동 ─────────────────────────
    _matrix matRotY, matTrans, matRootWorld;
    D3DXMatrixRotationY(&matRotY, m_fCurRotY);
    D3DXMatrixTranslation(&matTrans, m_fCurX, m_fCurY, m_fCurZ);
    matRootWorld = matRotY * matTrans;

    // ── 걷기 스윙 각도 ─────────────────────────────────────────────────
    const float fMaxAngle = D3DXToRadian(30.f);
    float fSwing = m_bMoving ? sinf(m_fWalkTime) * fMaxAngle : 0.f;

    // ── 파트별 렌더 ────────────────────────────────────────────────────
    Render_Part(PART_HEAD, 0.f, 0.f, 0.f, matRootWorld);
    Render_Part(PART_BODY, 0.f, 0.f, 0.f, matRootWorld);
    Render_Part(PART_LARM, fSwing, 0.f, 0.f, matRootWorld);
    Render_Part(PART_RARM, -fSwing, 0.f, 0.f, matRootWorld);
    Render_Part(PART_LLEG, -fSwing, 0.f, 0.f, matRootWorld);
    Render_Part(PART_RLEG, fSwing, 0.f, 0.f, matRootWorld);

    m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

    // ── 2패스: 방어구(로브) 오버레이 — CPlayer 방어구 렌더 블록과 동일 ──
    if (m_pArmorTextureCom)
    {
        m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        m_pGraphicDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        m_pGraphicDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

        for (int i = 0; i < PART_END; ++i)
        {
            _matrix matScale, matArmor;
            D3DXMatrixScaling(&matScale, 1.15f, 1.15f, 1.15f);
            matArmor = matScale * m_matPartWorld[i];
            m_pGraphicDev->SetTransform(D3DTS_WORLD, &matArmor);
            m_pArmorTextureCom->Set_Texture(0);
            m_pArmorBufferCom[i]->Render_Buffer();
        }

        m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    }

    Render_NameTag();
}

// =====================================================================
//  Render_NameTag  —  머리 위 닉네임 표시 (WorldToScreen)
// =====================================================================
void CRemotePlayer::Render_NameTag()
{
    if (m_szNickname[0] == '\0')
        return;

    // 머리 위 월드 좌표 (PART_HEAD offset Y≈2.2 + 여유 0.8)
    _vec3 vPos = { m_fCurX, m_fCurY + 3.0f, m_fCurZ };

    // View / Projection (D3DXMATRIX: typedef 충돌·인식 문제 방지)
    D3DXMATRIX matView;
    D3DXMATRIX matProj;
    m_pGraphicDev->GetTransform(D3DTS_VIEW, &matView);
    m_pGraphicDev->GetTransform(D3DTS_PROJECTION, &matProj);

    D3DVIEWPORT9 vp;
    m_pGraphicDev->GetViewport(&vp);

    // 월드 → 클립 공간
    D3DXVECTOR4 vClip;
    D3DXVECTOR4 vW(vPos.x, vPos.y, vPos.z, 1.f);
    D3DXMATRIX matVP;
    D3DXMatrixMultiply(&matVP, &matView, &matProj);
    D3DXVec4Transform(&vClip, &vW, &matVP);

    if (fabsf(vClip.w) < 1e-5f)
        return;

    // 카메라 뒤에 있으면 스킵
    float fNdcZ = vClip.z / vClip.w;
    if (fNdcZ < 0.f || fNdcZ > 1.f)
        return;

    // NDC → 스크린 픽셀
    float fSx = (vClip.x / vClip.w + 1.f) * 0.5f * (float)vp.Width;
    float fSy = (1.f - vClip.y / vClip.w) * 0.5f * (float)vp.Height;

    // 닉네임 char → wchar
    _tchar szNick[32];
    MultiByteToWideChar(CP_ACP, 0, m_szNickname, -1, szNick, 32);

    // 글자 수 기반 수평 중앙 정렬 근사 (Font_Minecraft: 글자 폭 약 10px)
    int iLen = (int)wcslen(szNick);
    fSx -= iLen * 5.f;

    _vec2 vScreen(fSx, fSy);

    // 노란색으로 렌더 (데미지 흰색과 구분)
    CFontMgr::GetInstance()->Render_Font(
        L"Font_Minecraft", szNick, &vScreen, D3DXCOLOR(1.f, 1.f, 0.f, 1.f));
}

// =====================================================================
CRemotePlayer* CRemotePlayer::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
    CRemotePlayer* pObj = new CRemotePlayer(pGraphicDev);
    if (FAILED(pObj->Ready_GameObject()))
    {
        pObj->Release();
        return nullptr;
    }
    return pObj;
}

void CRemotePlayer::Free()
{
    for (int i = 0; i < PART_END; ++i)
        Engine::Safe_Release(m_pBufferCom[i]);

    Engine::Safe_Release(m_pTextureCom);

    for (int i = 0; i < PART_END; ++i)
        Engine::Safe_Release(m_pArmorBufferCom[i]);

    Engine::Safe_Release(m_pArmorTextureCom);

    CGameObject::Free();
}
