#include "pch.h"
#include "CArrow.h"
#include "CRenderer.h"
#include "CManagement.h"
#include "CCollider.h"

CArrow::CArrow(LPDIRECT3DDEVICE9 pGraphicDev)
    : CGameObject(pGraphicDev)
{
}

CArrow::CArrow(const CGameObject& rhs)
    : CGameObject(rhs)
{
}

CArrow::~CArrow()
{
}

HRESULT CArrow::Ready_GameObject()
{
    if (FAILED(Add_Component()))
        return E_FAIL;
    return S_OK;
}

_int CArrow::Update_GameObject(const _float& fTimeDelta)
{
    if (m_bDead) return 0;

    _int iExit = CGameObject::Update_GameObject(fTimeDelta);

    // 수명 체크
    m_fLifeTime += fTimeDelta;
    if (m_fLifeTime >= m_fMaxLifeTime)
    {
        m_bDead = true;
        return 0;
    }

    // 방향으로 직진
    m_pTransformCom->Move_Pos(&m_vDir, m_fSpeed, fTimeDelta);

    // 콜라이더 위치 갱신
    _vec3 vPos;
    m_pTransformCom->Get_Info(INFO_POS, &vPos);
    m_pColliderCom->Update_AABB(vPos);

    // Y축 회전
    float fAngle = atan2f(m_vDir.x, m_vDir.z);
    m_pTransformCom->m_vAngle.y = D3DXToDegree(fAngle);

    // 플레이어 충돌 체크는 CMonster::Update_Arrow()에서 처리
    // CArrow는 이동과 콜라이더 갱신만 담당

    CRenderer::GetInstance()->Add_RenderGroup(RENDER_ALPHA, this);

    return iExit;
}

void CArrow::LateUpdate_GameObject(const _float& fTimeDelta)
{
    CGameObject::LateUpdate_GameObject(fTimeDelta);
}

void CArrow::Render_GameObject()
{
    if (m_bDead) return;

    m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    m_pGraphicDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    m_pGraphicDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

    m_pGraphicDev->SetTransform(D3DTS_WORLD, m_pTransformCom->Get_World());
    m_pTextureCom->Set_Texture(0);
    m_pBufferCom->Render_Buffer();

    m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

    if (m_pColliderCom)
        m_pColliderCom->Render_Collider();
}

HRESULT CArrow::Add_Component()
{
    Engine::CComponent* pComponent = nullptr;

    // Transform
    pComponent = m_pTransformCom = dynamic_cast<Engine::CTransform*>
        (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));
    if (!pComponent) return E_FAIL;
    m_mapComponent[ID_DYNAMIC].insert({ L"Com_Transform", pComponent });

    // 텍스처
    pComponent = m_pTextureCom = dynamic_cast<Engine::CTexture*>
        (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_ArrowTexture"));
    if (!pComponent) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_Texture", pComponent });

    // 버퍼
    pComponent = m_pBufferCom = dynamic_cast<Engine::CRcTex*>
        (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RcTex"));
    if (!pComponent) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_Buffer", pComponent });

    // 크기
    m_pTransformCom->m_vScale = { 0.3f, 0.3f, 0.3f };

    // 콜라이더
    m_pColliderCom = CCollider::Create(m_pGraphicDev,
        _vec3(0.3f, 0.3f, 0.3f),
        _vec3(0.f, 0.f, 0.f));
    if (!m_pColliderCom) return E_FAIL;

    return S_OK;
}

CArrow* CArrow::Create(LPDIRECT3DDEVICE9 pGraphicDev,
    const _vec3& vStartPos, const _vec3& vDir)
{
    CArrow* pArrow = new CArrow(pGraphicDev);

    if (FAILED(pArrow->Ready_GameObject()))
    {
        Safe_Release(pArrow);
        MSG_BOX("CArrow Create Failed");
        return nullptr;
    }

    pArrow->m_pTransformCom->Set_Pos(vStartPos.x, vStartPos.y, vStartPos.z);
    pArrow->Set_Direction(vDir);

    return pArrow;
}

void CArrow::Free()
{
    Safe_Release(m_pColliderCom);
    CGameObject::Free();
}