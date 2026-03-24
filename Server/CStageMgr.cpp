#include "CStageMgr.h"
#include "ServerLog.h"

CStageMgr* CStageMgr::m_pInstance = nullptr;

CStageMgr* CStageMgr::GetInstance()
{
    if (!m_pInstance)
        m_pInstance = new CStageMgr();
    return m_pInstance;
}

void CStageMgr::DestroyInstance()
{
    delete m_pInstance;
    m_pInstance = nullptr;
    LOG_INFO("CStageMgr destroyed");
}
