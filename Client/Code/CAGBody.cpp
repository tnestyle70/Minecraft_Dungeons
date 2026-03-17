#include "pch.h"
#include "CAGBody.h"
#include "CAncientGuardianUV.h"

CAGBody::CAGBody(LPDIRECT3DDEVICE9 pGraphicDev)
    : CBodyBase(pGraphicDev)
{
    m_pAnim = nullptr;
}

HRESULT CAGBody::Ready_Body()
{
    struct AGPartData {
        int   eBody, eTop;
        const wchar_t* bodyTag;
        float z;
        float spikeY;
    };

    static const AGPartData TABLE[] = {
        { AG_HEAD,  AG_HEAD_SPIKE_TOP,  L"Proto_AG_Head",  0.00f, 2.0f  },
        { AG_TAIL1, AG_TAIL1_SPIKE_TOP, L"Proto_AG_Tail1", 2.5f,  1.25f },
        { AG_TAIL2, AG_TAIL2_SPIKE_TOP, L"Proto_AG_Tail2", 4.5f,  1.15f },
        { AG_TAIL3, AG_TAIL3_SPIKE_TOP, L"Proto_AG_Tail3", 6.5f,  1.05f },
    };

    for (auto& p : TABLE)
    {
        if (FAILED(Register_Part(p.eBody,
            { p.bodyTag, { 0.f, 0.f, p.z }, { 0.f, 0.f, 0.f } })))
            return E_FAIL;

        if (FAILED(Register_Part(p.eTop,
            { L"Proto_AG_Spike", { 0.f, p.spikeY, p.z }, { 0.f, 0.f, 0.f } })))
            return E_FAIL;
    }

    m_pAnim = new CAGAnim();

    return S_OK;
}

CAGBody* CAGBody::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
    CAGBody* pInstance = new CAGBody(pGraphicDev);

    if (FAILED(pInstance->Ready_Body()))
    {
        Safe_Release(pInstance);
        MSG_BOX("CAGBody Create Failed");
        return nullptr;
    }

    return pInstance;
}

void CAGBody::Free()
{
    CBodyBase::Free();
}