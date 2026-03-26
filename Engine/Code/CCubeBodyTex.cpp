#include "CCubeBodyTex.h"

CCubeBodyTex::CCubeBodyTex()
{
}

CCubeBodyTex::CCubeBodyTex(LPDIRECT3DDEVICE9 pGraphicDev)
    :CVIBuffer(pGraphicDev)
{
}

CCubeBodyTex::CCubeBodyTex(const CCubeBodyTex& rhs)
    :CVIBuffer(rhs)
{
}

CCubeBodyTex::~CCubeBodyTex()
{
}

HRESULT CCubeBodyTex::Ready_Buffer(const CUBE& cube)
{
    //개별면마다 버텍스를 다르게 설정해준다
    m_dwVtxSize = sizeof(VTXCUBEBODY);
    m_dwVtxCnt = 24;
    m_dwTriCnt = 12;
    m_dwFVF = FVF_CUBEBODY;
    m_dwIdxSize = sizeof(INDEX16);
    m_IdxFmt = D3DFMT_INDEX16;

    if (FAILED(CVIBuffer::Ready_Buffer()))
        return E_FAIL;

    VTXCUBEBODY* pVtx = NULL;
    m_pVB->Lock(0, 0, (void**)&pVtx, 0);

    INDEX16* pIdx = NULL;
    m_pIB->Lock(0, 0, (void**)&pIdx, 0);

    float hw = cube.fWidth * 0.5f;    
    float hh = cube.fHeight * 0.5f;   
    float hd = cube.fDepth * 0.5f;    

    // ---- 정면 (+Z) ----
    SetFace(pVtx, pIdx, 0, 0,
        { -hw,  hh, hd }, { hw,  hh, hd },
        { hw, -hh, hd }, { -hw, -hh, hd },
        { 0, 0, 1 }, cube.front);

    // ---- 후면 (-Z) ----
    SetFace(pVtx, pIdx, 4, 2,
        { hw,  hh, -hd }, { -hw,  hh, -hd },
        { -hw, -hh, -hd }, { hw, -hh, -hd },
        { 0, 0, -1 }, cube.back);

    // ---- 윗면 (+Y) ----
    SetFace(pVtx, pIdx, 8, 4,
        { -hw, hh, -hd }, { hw, hh, -hd },
        { hw, hh,  hd }, { -hw, hh,  hd },
        { 0, 1, 0 }, cube.top);

    // ---- 아랫면 (-Y) ----
    SetFace(pVtx, pIdx, 12, 6,
        { -hw, -hh,  hd }, { hw, -hh,  hd },
        { hw, -hh, -hd }, { -hw, -hh, -hd },
        { 0, -1, 0 }, cube.bottom);

    // ---- 오른쪽 (+X) ----
    SetFace(pVtx, pIdx, 16, 8,
        { hw,  hh, -hd }, { hw,  hh,  hd },
        { hw, -hh,  hd }, { hw, -hh, -hd },
        { 1, 0, 0 }, cube.right);

    // ---- 왼쪽 (-X) ----
    SetFace(pVtx, pIdx, 20, 10,
        { -hw,  hh,  hd }, { -hw,  hh, -hd },
        { -hw, -hh, -hd }, { -hw, -hh,  hd },
        { -1, 0, 0 }, cube.left);

    m_pVB->Unlock();
    m_pIB->Unlock();

    return S_OK;
}

void CCubeBodyTex::Render_Buffer()
{
    CVIBuffer::Render_Buffer();
}

void CCubeBodyTex::SetFace(VTXCUBEBODY* pVtx, INDEX16* pIndex,
    int vtxBase, int idxBase, _vec3 vPos0, _vec3 vPos1, _vec3 vPos2, _vec3 vPos3, _vec3 vNormal,
    const FACE_UV& uv)
{
    //한 면에 버텍스 4개, 삼각형 두 개 세팅
    //p0 - 좌상, p1 - 우상, p2 - 우하, p3 - 좌하

    //p0 - 좌상 세팅
    pVtx[vtxBase].vPosition = vPos0;
    pVtx[vtxBase].vNormal = vNormal;
    pVtx[vtxBase].vTexUV = { uv.u0, uv.v0 };
    //p1 - 우상 세팅
    pVtx[vtxBase + 1].vPosition = vPos1;
    pVtx[vtxBase + 1].vNormal = vNormal;
    pVtx[vtxBase + 1].vTexUV = { uv.u1, uv.v0 };
    //p2 - 우하 세팅
    pVtx[vtxBase + 2].vPosition = vPos2;
    pVtx[vtxBase + 2].vNormal = vNormal;
    pVtx[vtxBase + 2].vTexUV = { uv.u1, uv.v1 };
    //p3 - 좌하 세팅
    pVtx[vtxBase + 3].vPosition = vPos3;
    pVtx[vtxBase + 3].vNormal = vNormal;
    pVtx[vtxBase + 3].vTexUV = { uv.u0, uv.v1 };

    //삼각형 - 0 1 2
    pIndex[idxBase]._0 = (_ushort)(vtxBase + 0);
    pIndex[idxBase]._1 = (_ushort)(vtxBase + 1);
    pIndex[idxBase]._2 = (_ushort)(vtxBase + 2);
    //삼각형 - 0 2 3 
    pIndex[idxBase + 1]._0 = (_ushort)(vtxBase + 0);
    pIndex[idxBase + 1]._1 = (_ushort)(vtxBase + 2);
    pIndex[idxBase + 1]._2 = (_ushort)(vtxBase + 3);
}

CCubeBodyTex* CCubeBodyTex::Create(LPDIRECT3DDEVICE9 pGraphicDev,
    const CUBE& cube)
{
    CCubeBodyTex* pBody = new CCubeBodyTex(pGraphicDev);

    if (FAILED(pBody->Ready_Buffer(cube)))
    {
        Safe_Release(pBody);
        MSG_BOX("pBody Create Failed");
        return nullptr;
    }

    return pBody;
}

CComponent* CCubeBodyTex::Clone()
{
    return new CCubeBodyTex(*this);
}

void CCubeBodyTex::Free()
{
    CVIBuffer::Free();
}
