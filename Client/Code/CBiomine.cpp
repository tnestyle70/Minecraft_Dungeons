#include "pch.h"
#include "CBiomine.h"
#include "CRenderer.h"

CBiomine::CBiomine(LPDIRECT3DDEVICE9 pGraphicDev)
    : CGameObject(pGraphicDev)
{
}

CBiomine::CBiomine(const CGameObject& rhs)
    : CGameObject(rhs)
{
}

CBiomine::~CBiomine()
{
}

HRESULT CBiomine::Ready_GameObject()
{
    if (FAILED(Add_Component()))
        return E_FAIL;

    return S_OK;
}

_int CBiomine::Update_GameObject(const _float& fTimeDelta)
{
    if (m_bDead) return 0;

    _int iExit = CGameObject::Update_GameObject(fTimeDelta);

    // 1단계 - 낙하 중
    if (!m_bOnGround)
    {
        Apply_Gravity(fTimeDelta);     // 중력 적용
        Resolve_BlockCollision();      // 블록 충돌 체크 - 땅에 박히면 m_bOnGround = true
    }
    // 2단계 - 땅에 박힌 후 폭발 타이머
    else
    {
        m_fExplodeTimer += fTimeDelta; // 폭발 타이머 누적

        // 폭발 직전 깜빡임 (0.5초 전부터)
        if (m_fExplodeTimer >= m_fExplodeMax - 0.5f)
        {
            m_fFlashTimer += fTimeDelta;
            if (m_fFlashTimer >= 0.1f) // 0.1초마다 깜빡임
            {
                m_bFlash = !m_bFlash;
                m_fFlashTimer = 0.f;
            }
        }

        // 폭발 시간 도달
        if (m_fExplodeTimer >= m_fExplodeMax && !m_bExploded)
        {
            Explode(); // 폭발 처리
        }

        // 폭발 후 0.2초 뒤 삭제
        if (m_bExploded && m_fExplodeTimer >= m_fExplodeMax + 0.2f)
        {
            m_bDead = true;
            return 0;
        }
    }

    // 콜라이더 위치 갱신
    _vec3 vPos;
    m_pTransformCom->Get_Info(INFO_POS, &vPos);
    m_pColliderCom->Update_AABB(vPos);

    CRenderer::GetInstance()->Add_RenderGroup(RENDER_NONALPHA, this);

    return iExit;
}

void CBiomine::LateUpdate_GameObject(const _float& fTimeDelta)
{
    CGameObject::LateUpdate_GameObject(fTimeDelta);
}

void CBiomine::Render_GameObject()
{
    if (m_bDead) return;
    if (m_bFlash) return; // 깜빡임 - 이 프레임은 안 그림

    m_pGraphicDev->SetTransform(D3DTS_WORLD, m_pTransformCom->Get_World());
    m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    m_pTextureCom->Set_Texture(0);
    m_pBufferCom->Render_Buffer();
    m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

    m_pColliderCom->Render_Collider();

    // 폭발 범위 콜라이더도 디버그용 렌더
    if (m_bExploded && m_pExplosionColliderCom)
        m_pExplosionColliderCom->Render_Collider();
}

HRESULT CBiomine::Add_Component()
{
    Engine::CComponent* pComponent = nullptr;

    // Transform
    pComponent = m_pTransformCom = dynamic_cast<Engine::CTransform*>
        (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));
    if (!pComponent) return E_FAIL;
    m_mapComponent[ID_DYNAMIC].insert({ L"Com_Transform", pComponent });

    // 텍스처 - AG 텍스처 재사용
    pComponent = m_pTextureCom = dynamic_cast<Engine::CTexture*>
        (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_AncientGuardianTexture"));
    if (!pComponent) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_Texture", pComponent });

    // 버퍼 - AG_Spike 재사용
    pComponent = m_pBufferCom = dynamic_cast<CCubeBodyTex*>
        (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_AG_Spike"));
    if (!pComponent) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_Buffer", pComponent });

    // 자신 콜라이더
    m_pColliderCom = CCollider::Create(m_pGraphicDev,
        _vec3(0.5f, 0.5f, 0.5f),
        _vec3(0.f, 0.f, 0.f));
    if (!m_pColliderCom) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_Collider", m_pColliderCom });

    // 폭발 범위 콜라이더 - 평소엔 비활성, 폭발 시 활성화
    m_pExplosionColliderCom = CCollider::Create(m_pGraphicDev,
        _vec3(3.f, 3.f, 3.f), // 폭발 반경 3
        _vec3(0.f, 0.f, 0.f));
    if (!m_pExplosionColliderCom) return E_FAIL;

    // 크기
    m_pTransformCom->m_vScale = { 0.5f, 0.5f, 0.5f };

    return S_OK;
}

void CBiomine::Apply_Gravity(const _float& fTimeDelta)
{
    // CMonster 중력 코드 동일하게 재사용
    m_fVelocityY += m_fGravity * fTimeDelta;
    if (m_fVelocityY < m_fMaxFall)
        m_fVelocityY = m_fMaxFall;

    _vec3 vPos;
    m_pTransformCom->Get_Info(INFO_POS, &vPos);
    vPos.y += m_fVelocityY * fTimeDelta;
    m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);
}

void CBiomine::Resolve_BlockCollision()
{
    // CMonster 블록 충돌 코드 동일하게 재사용
    _vec3 vPos;
    m_pTransformCom->Get_Info(INFO_POS, &vPos);

    AABB tMyAABB = m_pColliderCom->Get_AABB();

    const auto& mapBlocks = CBlockMgr::GetInstance()->Get_Blocks();
    for (const auto& pair : mapBlocks)
    {
        AABB tBlockAABB = CBlockMgr::GetInstance()->Get_BlockAABB(pair.first);
        if (m_pColliderCom->IsColliding(tBlockAABB))
        {
            _vec3 vResolve = m_pColliderCom->Resolve(tBlockAABB);

            // Y축 충돌 - 땅에 박힘
            if (vResolve.y > 0.f)
            {
                vPos.y += vResolve.y;
                m_fVelocityY = 0.f;
                m_bOnGround = true; // 땅에 박힘 - 폭발 타이머 시작
            }
            else
            {
                vPos.x += vResolve.x;
                vPos.z += vResolve.z;
            }
            m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);
        }
    }
}

void CBiomine::Explode()
{
    m_bExploded = true;

    // 폭발 범위 콜라이더 위치 갱신 - 플레이어 팀원이 충돌 체크에 사용
    _vec3 vPos;
    m_pTransformCom->Get_Info(INFO_POS, &vPos);
    m_pExplosionColliderCom->Update_AABB(vPos);
}

CBiomine* CBiomine::Create(LPDIRECT3DDEVICE9 pGraphicDev,
    const _vec3& vStartPos)
{
    CBiomine* pMine = new CBiomine(pGraphicDev);

    if (FAILED(pMine->Ready_GameObject()))
    {
        Safe_Release(pMine);
        MSG_BOX("CBiomine Create Failed");
        return nullptr;
    }

    pMine->m_pTransformCom->Set_Pos(vStartPos.x, vStartPos.y, vStartPos.z);

    return pMine;
}

void CBiomine::Free()
{
    Safe_Release(m_pExplosionColliderCom); // 맵에 안 넣은 콜라이더라 직접 해제
    CGameObject::Free();
}