#include "pch.h"
#include "CEnvironmentMgr.h"

IMPLEMENT_SINGLETON(CEnvironmentMgr)

CEnvironmentMgr::CEnvironmentMgr()
	: m_pGraphicDev(nullptr)
{
}

CEnvironmentMgr::~CEnvironmentMgr()
{
}

void CEnvironmentMgr::Add_Box(CBox* pBox)
{
	if (!pBox) return;

	m_vecBox.push_back(pBox);
}

void CEnvironmentMgr::Clear_Boxes()
{
	for (auto& pBox : m_vecBox)
	{
		Safe_Release(pBox);
	}
	m_vecBox.clear();
}

void CEnvironmentMgr::Free()
{
	m_vecBox.clear();

	Safe_Release(m_pGraphicDev);
}