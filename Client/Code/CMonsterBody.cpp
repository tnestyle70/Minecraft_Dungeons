#include "pch.h"
#include "CMonsterBody.h"


CMonsterBody::CMonsterBody(LPDIRECT3DDEVICE9 pGraphicDev, EMonsterType eType)
    : CBodyBase(pGraphicDev), m_eType(eType)
{
}

CMonsterBody::~CMonsterBody()
{
}

_matrix CMonsterBody::Get_PartWorld(EBodyPart iPartIndex, const _matrix* pParentWorld)
{
    if (!m_pAnim)
    {
        _matrix matIdentity;
        D3DXMatrixIdentity(&matIdentity);
        return matIdentity;
    }

    _matrix matSway;
    D3DXMatrixTranslation(&matSway, 0.f, m_pAnim->Get_Pose().fBodySwayY, 0.f);
    _matrix matWorld = matSway * (*pParentWorld);

    _vec3 vRot = m_pAnim->Get_Pose().GetRot(iPartIndex);
    return Calc_PartMatrix(iPartIndex, matWorld, vRot.x, vRot.y, vRot.z);
}

void CMonsterBody::Render_PartsWithOffset(const _matrix* pParentWorld,
    Engine::CTexture* pTexture, const _vec3* pOffsets)
{
    if (!pParentWorld || !pTexture) return;

    m_pGraphicDev->SetRenderState(D3DRS_LIGHTING, FALSE);
    m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    pTexture->Set_Texture(0);

    const BodyPose& pose = m_pAnim->Get_Pose();

    _matrix matSway;
    D3DXMatrixTranslation(&matSway, 0.f, pose.fBodySwayY, 0.f);
    _matrix matWorld = matSway * (*pParentWorld);

    for (int i = 0; i < (int)m_vecParts.size(); ++i)
    {
        if (!m_vecParts[i].pBuffer) continue;

        _vec3 vRot = pose.GetRot(i);

        
        _matrix matPartWorld = Calc_PartMatrix(i, matWorld, vRot.x, vRot.y, vRot.z);

       
        if (pOffsets)
        {
            matPartWorld._41 += pOffsets[i].x;
            matPartWorld._42 += pOffsets[i].y;
            matPartWorld._43 += pOffsets[i].z;
        }

        m_pGraphicDev->SetTransform(D3DTS_WORLD, &matPartWorld);
        m_vecParts[i].pBuffer->Render_Buffer();
    }

    m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
}

HRESULT CMonsterBody::Ready_Body()
{
    // 발바닥(Y=0) 기준 위치 계산 
    float fLegCenterY = m_fLegHeight * 0.5f;
    float fBodyBottom = m_fLegHeight;
    float fBodyCenterY = fBodyBottom + m_fBodyHeight * 0.5f;
    float fBodyTop = fBodyBottom + m_fBodyHeight;
    float fHeadCenterY = fBodyTop + m_fHeadHeight * 0.5f;
    float fArmX = (m_fBodyWidth + m_fArmWidth) * 0.5f;
    float fArmCenterY = fBodyTop - m_fArmHeight * 0.5f;
    float fLegX = m_fBodyWidth * 0.25f;

    // 타입에 따라 프로토타입 태그 선택
    const _tchar* pHead = nullptr;
    const _tchar* pBody = nullptr;
    const _tchar* pRArm = nullptr;
    const _tchar* pLArm = nullptr;
    const _tchar* pRLeg = nullptr;
    const _tchar* pLLeg = nullptr;

    switch (m_eType)
    {
    case EMonsterType::ZOMBIE:
        pHead = L"Proto_Zombie_Head";
        pBody = L"Proto_Zombie_Body";
        pRArm = L"Proto_Zombie_RArm";
        pLArm = L"Proto_Zombie_LArm";
        pRLeg = L"Proto_Zombie_RLeg";
        pLLeg = L"Proto_Zombie_LLeg";
        break;
    case EMonsterType::SKELETON:
        pHead = L"Proto_Skeleton_Head";
        pBody = L"Proto_Skeleton_Body";
        pRArm = L"Proto_Skeleton_RArm";
        pLArm = L"Proto_Skeleton_LArm";
        pRLeg = L"Proto_Skeleton_RLeg";
        pLLeg = L"Proto_Skeleton_LLeg";
        break;

    default:
        return E_FAIL;
    }

    // 파츠 등록
    if (FAILED(Register_Part(MonsterPart::HEAD,
        { pHead, { 0.f,   fHeadCenterY, 0.f }, { 0.f, -m_fHeadHeight * 0.5f, 0.f } })))
        return E_FAIL;

    if (FAILED(Register_Part(MonsterPart::BODY,
        { pBody, { 0.f,   fBodyCenterY, 0.f }, { 0.f, 0.f, 0.f } })))
        return E_FAIL;

    if (FAILED(Register_Part(MonsterPart::RIGHT_ARM,
        { pRArm, { fArmX,  fArmCenterY, 0.f }, { 0.f, -m_fArmHeight * 0.5f, 0.f } })))
        return E_FAIL;

    if (FAILED(Register_Part(MonsterPart::LEFT_ARM,
        { pLArm, { -fArmX, fArmCenterY, 0.f }, { 0.f, -m_fArmHeight * 0.5f, 0.f } })))
        return E_FAIL;

    if (FAILED(Register_Part(MonsterPart::RIGHT_LEG,
        { pRLeg, { fLegX,  fLegCenterY, 0.f }, { 0.f, -m_fLegHeight * 0.5f, 0.f } })))
        return E_FAIL;

    if (FAILED(Register_Part(MonsterPart::LEFT_LEG,
        { pLLeg, { -fLegX, fLegCenterY, 0.f }, { 0.f, -m_fLegHeight * 0.5f, 0.f } })))
        return E_FAIL;

    
    m_pAnim = new CMonsterAnim(m_eType);

    return S_OK;
}

CMonsterBody* CMonsterBody::Create(LPDIRECT3DDEVICE9 pGraphicDev, EMonsterType eType)
{
    CMonsterBody* pInstance = new CMonsterBody(pGraphicDev, eType);

    if (FAILED(pInstance->Ready_Body()))
    {
        Safe_Release(pInstance);
        MSG_BOX("CMonsterBody Create Failed");
        return nullptr;
    }

    return pInstance;
}

void CMonsterBody::Free()
{
    CBodyBase::Free();
}