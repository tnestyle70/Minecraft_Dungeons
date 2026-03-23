#include "pch.h"
#include "CTNT.h"
#include "CRenderer.h"
#include "CProtoMgr.h"
#include "CBlockMgr.h"
#include "CParticleMgr.h"
#include "CParticleEmitter.h"

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

    m_pBottomTex = dynamic_cast<Engine::CTexture*>
        (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_TNT_Bottom"));
    if (!m_pBottomTex) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_BottomTex", m_pBottomTex });

    m_pSideTex = dynamic_cast<Engine::CTexture*>
        (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_TNT_Side"));
    if (!m_pSideTex) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_SideTex", m_pSideTex });

    m_pColliderCom = CCollider::Create(m_pGraphicDev,
        _vec3(1.f, 1.f, 1.f), _vec3(0.f, 0.f, 0.f));
    if (!m_pColliderCom) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_Collider", m_pColliderCom });

    CUBE tCube;
    ZeroMemory(&tCube, sizeof(CUBE));
    tCube.fWidth = 1.f; tCube.fHeight = 1.f; tCube.fDepth = 1.f;
    tCube.front = { 0.f, 0.f, 1.f, 1.f };
    tCube.back = { 0.f, 0.f, 1.f, 1.f };
    tCube.top = { 0.f, 0.f, 1.f, 1.f };
    tCube.bottom = { 0.f, 0.f, 1.f, 1.f };
    tCube.right = { 0.f, 0.f, 1.f, 1.f };
    tCube.left = { 0.f, 0.f, 1.f, 1.f };

    m_pSideCubeCom = Engine::CCubeBodyTex::Create(m_pGraphicDev, tCube);
    if (!m_pSideCubeCom) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_SideCube", m_pSideCubeCom });

    m_pTopCubeCom = Engine::CCubeBodyTex::Create(m_pGraphicDev, tCube);
    if (!m_pTopCubeCom) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_TopCube", m_pTopCubeCom });

    m_pBottomCubeCom = Engine::CCubeBodyTex::Create(m_pGraphicDev, tCube);
    if (!m_pBottomCubeCom) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_BottomCube", m_pBottomCubeCom });

    m_pExplodeColliderCom = CCollider::Create(m_pGraphicDev,
        _vec3(5.f, 5.f, 5.f),
        _vec3(0.f, 0.f, 0.f));
    if (!m_pExplodeColliderCom) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_ExplodeCollider", m_pExplodeColliderCom });

    return S_OK;
}

_int CTNT::Update_GameObject(const _float& fTimeDelta)
{
    if (m_bDead)
        return 0;

    _int iExit = CGameObject::Update_GameObject(fTimeDelta);

    _vec3 vPos;
    m_pTransformCom->Get_Info(INFO_POS, &vPos);

    if (m_bThrown)
    {
        // 중력
        m_vVelocity.y += m_fGravity * fTimeDelta;
        vPos += m_vVelocity * fTimeDelta;
        m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);

        // 바닥 착지 체크
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
            CParticleMgr::GetInstance()->Add_Emitter(
                CParticleEmitter::Create(m_pGraphicDev, PARTICLE_FIREWORK, vPos, nullptr));
            m_pExplodeColliderCom->Update_AABB(vPos);
            m_bExploding = true;
            m_fExplodeTimer = 0.3f;
            m_bDead = true;
            return 0;
        }

        // 폭발 타이머
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
    if (m_bDead) return;

    // 깜빡임
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

    _matrix matScale, matRot, matTrans, matWorld;
    D3DXMatrixScaling(&matScale, 1.f, 1.f, 1.f);
    D3DXMatrixTranslation(&matTrans, vPos.x, vPos.y, vPos.z);

    _matrix matOffset;

     matWorld = *m_pTransformCom->Get_World();
    m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);

    m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

    // Side 4면
    m_pSideTex->Set_Texture(0);
    m_pSideCubeCom->Render_Buffer();

  //  // Top
  //  m_pTopTex->Set_Texture(0);
  //  m_pTopCubeCom->Render_Buffer();
  //
  //  // Bottom
  //  m_pBottomTex->Set_Texture(0);
  //  m_pBottomCubeCom->Render_Buffer();

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
    CGameObject::Free();
}