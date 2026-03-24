#pragma once

#include "CTestStage.h"

// =====================================================================
//  CStageMgr  —  스테이지 싱글턴 관리자
//  현재는 TestStage 단일 운영
//  향후 다중 스테이지 전환 시 map<int, CStageBase*> 구조로 확장
// =====================================================================
class CStageMgr
{
private:
    CStageMgr()  = default;
    ~CStageMgr() = default;
    CStageMgr(const CStageMgr&)            = delete;
    CStageMgr& operator=(const CStageMgr&) = delete;

    static CStageMgr* m_pInstance;

public:
    static CStageMgr* GetInstance();
    static void       DestroyInstance();

    CTestStage* GetTestStage() { return &m_testStage; }

private:
    CTestStage m_testStage;
};
