#include "pch.h"
#include "CAncientGuardian.h"
#include "CRenderer.h"
#include "CManagement.h"

CAncientGuardian::CAncientGuardian(LPDIRECT3DDEVICE9 pGraphicDev)
    : CDLCBoss(pGraphicDev)
{
}

CAncientGuardian::~CAncientGuardian()
{
}

HRESULT CAncientGuardian::Ready_GameObject()
{
    if (FAILED(CDLCBoss::Ready_GameObject()))
        return E_FAIL;

    return S_OK;
}

_int CAncientGuardian::Update_GameObject(const _float& fTimeDelta)
{
    
    _int iExit = CDLCBoss::Update_GameObject(fTimeDelta); 
     
    if (!m_pBodyCom) return iExit;
    m_pBodyCom->Update_Body(fTimeDelta, false, false); 

    CRenderer::GetInstance()->Add_RenderGroup(RENDER_NONALPHA, this);

    return iExit;
}

void CAncientGuardian::LateUpdate_GameObject(const _float& fTimeDelta)
{
    CDLCBoss::LateUpdate_GameObject(fTimeDelta);
}

HRESULT CAncientGuardian::Add_Component()
{ 
    Engine::CComponent* pComponent = nullptr;

    pComponent = m_pTransformCom = dynamic_cast<Engine::CTransform*> (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));
    if (!pComponent) return E_FAIL;
    m_mapComponent[ID_DYNAMIC].insert({ L"Com_Transform", pComponent });

    pComponent = m_pTextureCom = dynamic_cast<Engine::CTexture*> (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_AncientGuardianTexture"));
    if (!pComponent) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_Texture",pComponent });

    m_pBodyCom = CAGBody::Create(m_pGraphicDev);
    if (!m_pBodyCom) return E_FAIL;

    return S_OK;
}

void CAncientGuardian::Update_AI(const _float& fTimeDelta)
{
    if (!m_pTransformCom) return;

    Engine::CTransform* pPlayerTrans = dynamic_cast<Engine::CTransform*>(
        CManagement::GetInstance()->Get_Component(
            ID_DYNAMIC, L"GameLogic_Layer", L"Player", L"Com_Transform"));

    if (!pPlayerTrans) return;

    _vec3 vMyPos, vPlayerPos;
    m_pTransformCom->Get_Info(INFO_POS, &vMyPos);
    pPlayerTrans->Get_Info(INFO_POS, &vPlayerPos);

    // ── 수평 이동 - 플레이어 따라가기 ──────────────────
    _vec3 vDir = vPlayerPos - vMyPos;
    vDir.y = 0.f;
    float fDist = D3DXVec3Length(&vDir);

    if (fDist > 2.f)
    {
        D3DXVec3Normalize(&vDir, &vDir);

        // Y축 - 부드러운 좌우 회전
        float fTargetAngleY = D3DXToDegree(atan2f(vDir.x, vDir.z)) + 180.f;
        float fCurAngleY = m_pTransformCom->m_vAngle.y;
        float fDiffY = fTargetAngleY - fCurAngleY;
        while (fDiffY > 180.f) fDiffY -= 360.f;
        while (fDiffY < -180.f) fDiffY += 360.f;

        float fMaxRot = 120.f * fTimeDelta;
        if (fabsf(fDiffY) < fMaxRot)
            m_pTransformCom->m_vAngle.y = fTargetAngleY;
        else
            m_pTransformCom->m_vAngle.y += (fDiffY > 0.f ? fMaxRot : -fMaxRot);

        // 이동
        vMyPos.x += vDir.x * 3.f * fTimeDelta;
        vMyPos.z += vDir.z * 3.f * fTimeDelta;
    }

    // ── 위아래 헤엄치는 움직임 ──────────────────────────
    m_fHoverTime += fTimeDelta;

    // X축 각도 위아래로 번갈아 - 고개를 위로 들었다 내렸다
    float fPitchAngle = sinf(m_fHoverTime * 1.5f) * 20.f; // ±20도
    m_pTransformCom->m_vAngle.x = fPitchAngle;

    // 고개 방향대로 Y위치도 같이 움직임 (위를 보면 올라가고 아래를 보면 내려가고)
    float fTargetY = 12.5f + sinf(m_fHoverTime * 1.5f) * 2.5f; // 10~15 사이
    vMyPos.y += (fTargetY - vMyPos.y) * 3.f * fTimeDelta;

    // Y 범위 클램프
    if (vMyPos.y < 10.f) vMyPos.y = 10.f;
    if (vMyPos.y > 15.f) vMyPos.y = 15.f;

    m_pTransformCom->Set_Pos(vMyPos.x, vMyPos.y, vMyPos.z);
}

void CAncientGuardian::Render_GameObject()
{ 

    if (!m_pBodyCom || !m_pTransformCom || !m_pTextureCom) return;
    m_pBodyCom->Render_Body(m_pTransformCom->Get_World(), m_pTextureCom); 

  
}

void CAncientGuardian::Update_Orbit(const _float& fTimeDelta)
{
}

void CAncientGuardian::Update_Charge(const _float& fTimeDelta)
{
}

void CAncientGuardian::Fire_Beam()
{
}

void CAncientGuardian::Update_Beams(const _float& fTimeDelta)
{
}

void CAncientGuardian::Drop_Biomine()
{
}

void CAncientGuardian::Update_Biomines(const _float& fTimeDelta)
{
}

CAncientGuardian* CAncientGuardian::Create(LPDIRECT3DDEVICE9 pGraphicDev, _vec3 vPos)
{ 
    
    CAncientGuardian* pInstance = new CAncientGuardian(pGraphicDev);

    if (FAILED(pInstance->Ready_GameObject()))
    {
        Safe_Release(pInstance);
        MSG_BOX("CAncientGuardian Create Failed");
        return nullptr;
    }

    pInstance->m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);

    return pInstance;
}

void CAncientGuardian::Free()
{ 
    Safe_Release(m_pBodyCom);
    CDLCBoss::Free();
}