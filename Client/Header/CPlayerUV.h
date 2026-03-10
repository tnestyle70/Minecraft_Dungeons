#pragma once
#include "CCubeBodyTex.h"

//개별 헤더에서 UV값 세팅해주기
/*
namespace CPlayerUV
{
    // 머리 (8x8x8)
    static const CUBE HEAD = {
        0.5f, 0.5f, 0.5f,
        MakeUV(8,  8,  8, 8),    // 정면
        MakeUV(24, 8,  8, 8),    // 후면
        MakeUV(8,  0,  8, 8),    // 윗면
        MakeUV(16, 0,  8, 8),    // 아랫면
        MakeUV(0,  8,  8, 8),    // 오른쪽
        MakeUV(16, 8,  8, 8),    // 왼쪽
    };

    // 몸통 (8x12x4)
    static const CUBE BODY = {
        0.5f, 0.75f, 0.25f,
        MakeUV(20, 20, 8, 12),   // 정면
        MakeUV(32, 20, 8, 12),   // 후면
        MakeUV(20, 16, 8,  4),   // 윗면
        MakeUV(28, 16, 8,  4),   // 아랫면
        MakeUV(16, 20, 4, 12),   // 오른쪽
        MakeUV(28, 20, 4, 12),   // 왼쪽
    };

    // 오른팔 (4x12x4)
    static const CUBE R_ARM = {
        0.25f, 0.75f, 0.25f,
        MakeUV(44, 20, 4, 12),   // 정면
        MakeUV(52, 20, 4, 12),   // 후면
        MakeUV(44, 16, 4,  4),   // 윗면
        MakeUV(48, 16, 4,  4),   // 아랫면
        MakeUV(40, 20, 4, 12),   // 오른쪽
        MakeUV(48, 20, 4, 12),   // 왼쪽
    };

    // 왼팔
    static const CUBE L_ARM = {
        0.25f, 0.75f, 0.25f,
        MakeUV(44, 20, 4, 12),
        MakeUV(52, 20, 4, 12),
        MakeUV(44, 16, 4,  4),
        MakeUV(48, 16, 4,  4),
        MakeUV(40, 20, 4, 12),
        MakeUV(48, 20, 4, 12),
    };

    // 오른다리 (4x12x4)
    static const CUBE R_LEG = {
        0.25f, 0.75f, 0.25f,
        MakeUV(4,  20, 4, 12),   // 정면
        MakeUV(12, 20, 4, 12),   // 후면
        MakeUV(4,  16, 4,  4),   // 윗면
        MAKEUV(8,  16, 4,  4),   // 아랫면
        MAKEUV(0,  20, 4, 12),   // 오른쪽
        MAKEUV(8,  20, 4, 12),   // 왼쪽
    };

    // 왼다리
    static const CUBE L_LEG = {
        0.25f, 0.75f, 0.25f,
        MAKEUV(4,  20, 4, 12),
        MAKEUV(12, 20, 4, 12),
        MAKEUV(4,  16, 4,  4),
        MAKEUV(8,  16, 4,  4),
        MAKEUV(0,  20, 4, 12),
        MAKEUV(8,  20, 4, 12),
    };
}
*/