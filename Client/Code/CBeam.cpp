#include "pch.h"
#include "CBeam.h"
#include "CRenderer.h"
#include "CCollider.h"

CBeam::CBeam(LPDIRECT3DDEVICE9 pGraphicDev)
    : CGameObject(pGraphicDev)
{
}

CBeam::CBeam(const CGameObject& rhs)
    : CGameObject(rhs)
{
}

CBeam::~CBeam()
{
}

HRESULT CBeam::Ready_GameObject()
{
    if (FAILED(Add_Component()))
        return E_FAIL;
    return S_OK;
}

_int CBeam::Update_GameObject(const _float fTimeDelta)
{
    if (m_bDead) return 0;

    _int iExit = CGameObject::Update_GameObject(fTimeDelta);

    m_fLifeTime += fTimeDelta;
    if (m_fLifeTime >= m_fMaxLifeTime)
    {
        m_bDead = true;
        return 0;
    }

    _vec3 vPos;
    m_pTransformCom->Get_Info(INFO_POS, &vPos);
    vPos += m_vDir * m_fSpeed * fTimeDelta;
    m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);

    // 콜라이더 위치 갱신 // 추가
    m_pColliderCom->Update_AABB(vPos); // 추가

    float fAngle = atan2f(m_vDir.x, m_vDir.z);
    m_pTransformCom->m_vAngle.y = D3DXToDegree(fAngle);

    CRenderer::GetInstance()->Add_RenderGroup(RENDER_ALPHA, this);

    return 0;
}

void CBeam::LateUpdate_GameObject(const _float& fTimeDelta)
{
    if (m_bDead) return;

    _matrix matView, matBill, matWorld;
    matWorld = *m_pTransformCom->Get_World();
    m_pGraphicDev->GetTransform(D3DTS_VIEW, &matView);

    // Y축만이 아닌 전체 회전 역행렬 적용 // 수정
    matBill._11 = matView._11; matBill._12 = matView._21; matBill._13 = matView._31;
    matBill._21 = matView._12; matBill._22 = matView._22; matBill._23 = matView._32;
    matBill._31 = matView._13; matBill._32 = matView._23; matBill._33 = matView._33;
    matBill._14 = matBill._24 = matBill._34 = 0.f;
    matBill._41 = matBill._42 = matBill._43 = 0.f;
    matBill._44 = 1.f;

    matWorld = matBill * matWorld;
    m_pTransformCom->Set_World(&matWorld);

    CGameObject::LateUpdate_GameObject(fTimeDelta);
}

void CBeam::Render_GameObject()
{
    if (m_bDead) return;

    m_pGraphicDev->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);        // 추가 - Z쓰기 비활성화
    m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);     // 추가
    m_pGraphicDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA); // 추가
    m_pGraphicDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);    // 추가 - Additive Blending

    m_pGraphicDev->SetTransform(D3DTS_WORLD, m_pTransformCom->Get_World());
    m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    m_pTextureCom->Set_Texture(0);
    m_pBufferCom->Render_Buffer();
    m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

    m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);    // 추가 - 복구
    m_pGraphicDev->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);         // 추가 - Z쓰기 복구

    m_pColliderCom->Render_Collider();
}

HRESULT CBeam::Add_Component()
{
    Engine::CComponent* pComponent = nullptr;

    pComponent = m_pTransformCom = dynamic_cast<Engine::CTransform*>
        (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));
    if (!pComponent) return E_FAIL;
    m_mapComponent[ID_DYNAMIC].insert({ L"Com_Transform", pComponent });

    pComponent = m_pTextureCom = dynamic_cast<Engine::CTexture*>
        (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_AncientGuardianTexture"));
    if (!pComponent) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_Texture", pComponent });

    // 버퍼 - AG_Spike 재사용 (가시 모양 큐브)
    pComponent = m_pBufferCom = dynamic_cast<CCubeBodyTex*>
        (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_AG_Spike"));
    if (!pComponent) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_Buffer", pComponent });

    // 콜라이더
    m_pColliderCom = CCollider::Create(m_pGraphicDev,
        _vec3(0.2f, 1.0f, 0.2f),
        _vec3(0.f, 0.f, 0.f));
    if (!m_pColliderCom) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_Collider", m_pColliderCom });

    // 크기 설정 - 가시 크기 키우기 // 수정
    m_pTransformCom->m_vScale = { 0.5f, 2.0f, 0.5f }; // 수정 - 길고 얇게

    return S_OK;
}

CBeam* CBeam::Create(LPDIRECT3DDEVICE9 pGraphicDev,
    const _vec3& vStartPos, const _vec3& vDir)
{
    CBeam* pBeam = new CBeam(pGraphicDev);
    if (FAILED(pBeam->Ready_GameObject()))
    {
        Safe_Release(pBeam);
        MSG_BOX("CBeam Create Failed");
        return nullptr;
    }
    pBeam->m_pTransformCom->Set_Pos(vStartPos.x, vStartPos.y, vStartPos.z);
    pBeam->Set_Direction(vDir);
    return pBeam;
}

void CBeam::Free()
{
    CGameObject::Free();
}