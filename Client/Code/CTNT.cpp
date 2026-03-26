#include "pch.h"
#include "CTNT.h"
#include "CRenderer.h"
#include "CProtoMgr.h"
#include "CBlockMgr.h"
#include "CParticleMgr.h"
#include "CParticleEmitter.h"
#include "CSoundMgr.h"
#include "CExplosionLight.h"

CTNT::CTNT(LPDIRECT3DDEVICE9 pGraphicDev)
    : CGameObject(pGraphicDev)
{
}

CTNT::~CTNT()
{
}

HRESULT CTNT::Ready_GameObject(_vec3 vPos)
{
    if (FAILED(Add_Component()))
        return E_FAIL;

    m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);
    return S_OK;
}

HRESULT CTNT::Add_Component()
{
    m_pTransformCom = dynamic_cast<Engine::CTransform*>
        (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));
    if (!m_pTransformCom) return E_FAIL;
    m_mapComponent[ID_DYNAMIC].insert({ L"Com_Transform", m_pTransformCom });

    m_pTopTex = dynamic_cast<Engine::CTexture*>
        (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_TNT_Top"));
    if (!m_pTopTex) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_TopTex", m_pTopTex });

    m_pColliderCom = CCollider::Create(m_pGraphicDev,
        _vec3(1.f, 1.f, 1.f), _vec3(0.f, 0.f, 0.f));
    if (!m_pColliderCom) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_Collider", m_pColliderCom });

    CUBE tCube;
    ZeroMemory(&tCube, sizeof(CUBE));
    tCube.fWidth = 1.f; tCube.fHeight = 1.f; tCube.fDepth = 1.f;
    tCube.front = { 0.25000f, 0.25000f, 0.50000f, 0.50000f };
    tCube.back = { 0.75000f, 0.25000f, 1.00000f, 0.50000f };
    tCube.top = { 0.25000f, 0.00000f, 0.50000f, 0.25000f };
    tCube.bottom = { 0.25000f, 0.50000f, 0.50000f, 0.75000f };
    tCube.right = { 0.50000f, 0.25000f, 0.75000f, 0.50000f };
    tCube.left = { 0.00000f, 0.25000f, 0.25000f, 0.50000f };

    m_pTopCubeCom = Engine::CCubeBodyTex::Create(m_pGraphicDev, tCube);
    if (!m_pTopCubeCom) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_TopCube", m_pTopCubeCom });

    m_pExplodeColliderCom = CCollider::Create(m_pGraphicDev,
        _vec3(5.f, 5.f, 5.f), _vec3(0.f, 0.f, 0.f));
    if (!m_pExplodeColliderCom) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_ExplodeCollider", m_pExplodeColliderCom });

    return S_OK;
}

_int CTNT::Update_GameObject(const _float& fTimeDelta)
{
    // 설명 : 조명 업데이트
    if (m_pExplosionLight)
    {
        m_pExplosionLight->Update(fTimeDelta);
        if (m_pExplosionLight->Is_Done())
        {
            delete m_pExplosionLight;
            m_pExplosionLight = nullptr;
        }
    }

    // 설명 : 조명이 살아있는 동안은 Dead여도 렌더 그룹에 추가
    if (m_pExplosionLight)
        CRenderer::GetInstance()->Add_RenderGroup(RENDER_NONALPHA, this);

    if (m_bDead)
        return 0;

    _int iExit = CGameObject::Update_GameObject(fTimeDelta);

    _vec3 vPos;
    m_pTransformCom->Get_Info(INFO_POS, &vPos);

    if (m_bThrown)
    {
        m_vVelocity.y += m_fGravity * fTimeDelta;
        vPos += m_vVelocity * fTimeDelta;
        m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);

        BlockPos tBlockPos = { (int)vPos.x, (int)(vPos.y - 0.5f), (int)vPos.z };
        if (CBlockMgr::GetInstance()->HasBlock(tBlockPos))
        {
            m_vVelocity = {};
            m_bThrown = false;
            m_bFusing = true;
        }
    }

    if (m_bFusing)
    {
        m_fFuseTimer -= fTimeDelta;
        if (m_fFuseTimer <= 0.f)
        {
            m_pExplodeColliderCom->Update_AABB(vPos);
            m_bExploding = true;
            m_fExplodeTimer = 0.3f;
            m_bDead = true;

            // 설명 : 폭발 조명 + 사운드
            m_pExplosionLight = new CExplosionLight(m_pGraphicDev, vPos);
            CSoundMgr::GetInstance()->PlayEffect(L"Monster/explode.wav", 1.f);
            return 0;
        }

        if (m_bExploding)
        {
            m_fExplodeTimer -= fTimeDelta;
            if (m_fExplodeTimer <= 0.f)
                m_bExploding = false;
        }
    }

    m_pColliderCom->Update_AABB(vPos);
    CRenderer::GetInstance()->Add_RenderGroup(RENDER_NONALPHA, this);
    return 0;
}

void CTNT::LateUpdate_GameObject(const _float& fTimeDelta)
{
    CGameObject::LateUpdate_GameObject(fTimeDelta);
}

void CTNT::Render_GameObject()
{
    // 설명 : 폭발 플래시는 Dead 여부 무관하게 렌더
    if (m_pExplosionLight)
        m_pExplosionLight->Render();

    if (m_bDead) return;

    if (m_bFusing)
    {
        float fBlinkRate = 2.f + (1.f - m_fFuseTimer / m_fMaxFuseTime) * 10.f;
        float fBlink = sinf(m_fFuseTimer * D3DX_PI * fBlinkRate);
        if (fBlink > 0.f)
        {
            m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
            m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
            m_pGraphicDev->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_RGBA(255, 0, 0, 255));
        }
    }

    _vec3 vPos;
    m_pTransformCom->Get_Info(INFO_POS, &vPos);

    _matrix matWorld = *m_pTransformCom->Get_World();
    m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);
    m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

    m_pTopTex->Set_Texture(0);
    m_pTopCubeCom->Render_Buffer();

    m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
    m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

    m_pColliderCom->Render_Collider();
}

void CTNT::Throw(_vec3 vPos, _vec3 vDir, float fPower)
{
    m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);
    m_vVelocity = vDir * fPower;
    m_vVelocity.y += 5.f;
    m_bThrown = true;
    m_bPickedUp = false;
}

CTNT* CTNT::Create(LPDIRECT3DDEVICE9 pGraphicDev, _vec3 vPos)
{
    CTNT* pTNT = new CTNT(pGraphicDev);
    if (FAILED(pTNT->Ready_GameObject(vPos)))
    {
        Safe_Release(pTNT);
        MSG_BOX("CTNT Create Failed");
        return nullptr;
    }
    return pTNT;
}

void CTNT::Free()
{
    // 설명 : 폭발 조명 강제 삭제
    if (m_pExplosionLight)
    {
        delete m_pExplosionLight;
        m_pExplosionLight = nullptr;
    }

    CGameObject::Free();
}