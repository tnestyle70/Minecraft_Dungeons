#include "CParticleMgr.h"

IMPLEMENT_SINGLETON(CParticleMgr)

CParticleMgr::CParticleMgr()
{
}

CParticleMgr::~CParticleMgr()
{
    Free();
}

void CParticleMgr::Add_Emitter(CParticleEmitter* pEmitter)
{
    if (!pEmitter)
        return;
    pEmitter->AddRef();
    m_EmitterList.push_back(pEmitter);
}

void CParticleMgr::Update(const _float& fTimeDelta)
{
    if (m_EmitterList.empty())
        return;

    auto iter = m_EmitterList.begin();

    while (iter != m_EmitterList.end())
    {
        (*iter)->Update_GameObject(fTimeDelta);

        if ((*iter)->Is_Dead())
        {
            Safe_Release(*iter);
            iter = m_EmitterList.erase(iter);
        }
        else
        {
            ++iter;
        }
    }
}

void CParticleMgr::Render()
{
    if (m_EmitterList.empty())
        return;

    for (auto* pEmitter : m_EmitterList)
        pEmitter->Render_GameObject();
}

void CParticleMgr::Clear_Emitters()
{
    for (auto* pEmitter : m_EmitterList)
        Safe_Release(pEmitter);

    m_EmitterList.clear();
}

void CParticleMgr::Free()
{
    Clear_Emitters();
}
