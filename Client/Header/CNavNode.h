#pragma once

struct NavNode
{
    int     x, z;           // XZ 격자 좌표 (Y는 지형이 처리)
    float   g = 0.f;        // 시작점 
    float   h = 0.f;        // 현재 목표 추정 비용 (휴리스틱)
    float   f = 0.f;        // g + h
    NavNode* pParent = nullptr;

    // 우선순위큐에서 f 기준 정렬 (작은 게 우선)
    bool operator>(const NavNode& rhs) const { return f > rhs.f; }
};
