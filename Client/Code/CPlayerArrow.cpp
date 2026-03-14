#include "pch.h"
#include "CPlayerArrow.h"
#include "CRenderer.h"

CPlayerArrow::CPlayerArrow(LPDIRECT3DDEVICE9 pGraphicDev)
    : CGameObject(pGraphicDev)
    , m_pTransformCom(nullptr)
    , m_pBufferCom(nullptr)
    , m_pTextureCom(nullptr)
    , m_pColliderCom(nullptr)
    , m_vDir{}
    , m_fSpeed(20.f)
    , m_fDamage(0.f)
    , m_fLifeTime(0.f)
    , m_bDead(false)
{
}

CPlayerArrow::CPlayerArrow(const CGameObject& rhs)
    : CGameObject(rhs)
{
}

CPlayerArrow::~CPlayerArrow()
{
}

HRESULT CPlayerArrow::Ready_GameObject()
{
    if (FAILED(Add_Component()))
        return E_FAIL;

    return S_OK;
}

HRESULT CPlayerArrow::Add_Component()
{
    Engine::CComponent* pComponent = nullptr;

    pComponent = m_pBufferCom = dynamic_cast<Engine::CRcTex*>
        (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RcTex"));
    if (nullptr == pComponent)
        return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_Buffer", pComponent });

    pComponent = m_pTextureCom = dynamic_cast<Engine::CTexture*>
        (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_ArrowTexture"));
    if (nullptr == pComponent)
        return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_Texture", pComponent });

    pComponent = m_pTransformCom = dynamic_cast<Engine::CTransform*>
        (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));
    if (nullptr == pComponent)
        return E_FAIL;
    m_mapComponent[ID_DYNAMIC].insert({ L"Com_Transform", pComponent });

    m_pColliderCom = CCollider::Create(m_pGraphicDev,
        _vec3(0.1f, 0.1f, 0.5f),
        _vec3(0.f, 0.f, 0.f));
    if (nullptr == m_pColliderCom)
        return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_Collider", m_pColliderCom });

    m_pTransformCom->m_vScale = { 0.3f, 0.1f, 0.3f };

    return S_OK;
}

_int CPlayerArrow::Update_GameObject(const _float& fTimeDelta)
{
    _int iExit = CGameObject::Update_GameObject(fTimeDelta);

    if (m_bDead)
        return -1;

    // 이동
    m_fLifeTime += fTimeDelta;
    if (m_fLifeTime >= 5.f)
        m_bDead = true;

    _vec3 vPos;
    m_pTransformCom->Get_Info(INFO_POS, &vPos);
    vPos += m_vDir * m_fSpeed * fTimeDelta;
    m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);
    m_pColliderCom->Update_AABB(vPos);

    CRenderer::GetInstance()->Add_RenderGroup(RENDER_ALPHA, this);

    return iExit;
}

void CPlayerArrow::LateUpdate_GameObject(const _float& fTimeDelta)
{
    CGameObject::LateUpdate_GameObject(fTimeDelta);
}

void CPlayerArrow::Render_GameObject()
{
    _vec3 vPos;
    m_pTransformCom->Get_Info(INFO_POS, &vPos);

    float fAngleY = atan2f(m_vDir.x, m_vDir.z);

    if (nullptr == m_pTextureCom)
        return;

    _matrix matScale, matRotX90, matRotY, matTrans, matWorld;
    D3DXMatrixScaling(&matScale, 0.3f, 0.1f, 0.3f);
    D3DXMatrixRotationX(&matRotX90, D3DXToRadian(90.f));  // 평면으로 눕히기
    D3DXMatrixRotationY(&matRotY, fAngleY + D3DXToRadian(-60.f)); // 틀어져있는 이미지 1자로 세우기 
    D3DXMatrixTranslation(&matTrans, vPos.x, vPos.y, vPos.z);

    matWorld = matScale * matRotX90 * matRotY * matTrans;
    m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);

    m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    m_pGraphicDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    m_pGraphicDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    m_pGraphicDev->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
    m_pGraphicDev->SetRenderState(D3DRS_ALPHAREF, 128);
    m_pGraphicDev->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
    m_pTextureCom->Set_Texture(0);
    m_pBufferCom->Render_Buffer();
    m_pGraphicDev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    m_pGraphicDev->SetTexture(0, nullptr);
    m_pColliderCom->Render_Collider();
}

CPlayerArrow* CPlayerArrow::Create(LPDIRECT3DDEVICE9 pGraphicDev,
    const _vec3& vPos, const _vec3& vDir, float fCharge)
{
    CPlayerArrow* pArrow = new CPlayerArrow(pGraphicDev);
    pArrow->m_vDir = vDir;
    pArrow->m_fDamage = 10.f + 40.f * fCharge;

    if (FAILED(pArrow->Ready_GameObject()))
    {
        Safe_Release(pArrow);
        MSG_BOX("CPlayerArrow Create Failed");
        return nullptr;
    }

    pArrow->m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);

    return pArrow;
}

void CPlayerArrow::Free()
{
    CGameObject::Free();
}