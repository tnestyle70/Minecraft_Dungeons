#pragma once
#include "CProtoMgr.h"
#include "COceanTypes.h"

// ============================================================
//  COceanBuffer : CVIBuffer
//  역할: 매 프레임 CPU에서 계산한 파도 높이를
//        Dynamic VB에 Lock → Write → Unlock 으로 반영
//
//  CTerrainTex 와 다른 점:
//   - VB를 D3DUSAGE_DYNAMIC | D3DPOOL_DEFAULT 로 생성
//     (매 프레임 Lock 성능 때문에 MANAGED 불가)
//   - IB는 격자 위상 고정이므로 MANAGED 유지
//   - m_pPos[] 배열에 최종 월드 높이 캐시 (픽킹용)
// ============================================================

class COceanBuffer : public CVIBuffer
{
public:
    explicit COceanBuffer(LPDIRECT3DDEVICE9 pGraphicDev);
    COceanBuffer(const COceanBuffer& rhs);
    virtual ~COceanBuffer();

public:
    // ─── 초기화 ──────────────────────────────────────────────
    HRESULT Ready_Buffer(const OCEAN_DESC& desc);

    // ─── 매 프레임: 외부에서 계산된 높이/노멀 배열을 VB에 반영
    // pHeight : [N*N] 높이값
    // pDisplX : [N*N] 수평 변위 X (nullptr = Choppy 미사용)
    // pDisplZ : [N*N] 수평 변위 Z
    // pNormals: [N*N] 법선 벡터
    void Update_Vertices(const float* pHeight,
        const float* pDisplX,
        const float* pDisplZ,
        const D3DXVECTOR3* pNormals);

    // ─── 렌더 ────────────────────────────────────────────────
    virtual void Render_Buffer() override;

    // ─── 격자 기본 위치 접근 (픽킹 등) ──────────────────────
    const D3DXVECTOR3* Get_BasePos() const { return m_pBasePos; }
    _ulong Get_VtxCntX() const { return m_dwVtxCntX; }
    _ulong Get_VtxCntZ() const { return m_dwVtxCntZ; }

public:
    static COceanBuffer* Create(LPDIRECT3DDEVICE9 pGraphicDev,
        const OCEAN_DESC& desc);
    virtual CComponent* Clone() override;
    virtual void Free() override;

private:
    HRESULT Create_DynamicVB();     // D3DUSAGE_DYNAMIC VB 생성
    HRESULT Build_IndexBuffer();    // 격자 IB 구성 (불변)
    void    Compute_BaseGrid();     // XZ 기본 격자 좌표 계산

private:
    _ulong          m_dwVtxCntX = 0;
    _ulong          m_dwVtxCntZ = 0;
    _float          m_fPatchSize = 0.f;

    D3DXVECTOR3* m_pBasePos = nullptr;   // 기본 XZ 격자 (Y=0)
    D3DXVECTOR2* m_pBaseUV = nullptr;   // 기본 UV
};
