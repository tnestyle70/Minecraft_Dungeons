#include "pch.h"
#include "CMonsterBody.h"


CMonsterBody::CMonsterBody(LPDIRECT3DDEVICE9 pGraphicDev, EMonsterType eType)
    : CBodyBase(pGraphicDev), m_eType(eType)
{
}

CMonsterBody::~CMonsterBody()
{
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

    // 타입에 맞는 애니메이션 연결
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