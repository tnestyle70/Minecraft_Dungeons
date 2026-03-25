#include "CScene.h"
#include "CDynamicCamera.h"
#include "CNetworkPlayer.h"
#include "CDragon.h"

class COcean;

class CNetworkStage : public CScene
{
protected:
	explicit CNetworkStage(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual ~CNetworkStage();
public:
	virtual HRESULT Ready_Scene();
	virtual _int Update_Scene(const _float& fTimeDelta);
	virtual void LateUpdate_Scene(const _float& fTimeDelta);
	virtual void Render_Scene();
	virtual void Render_UI() override;
private:
	HRESULT Ready_Environment_Layer(const _tchar* pLayerTag);
	HRESULT Ready_GameLogic_Layer(const _tchar* pLayerTag);
	HRESULT Ready_UI_Layer(const _tchar* pLayerTag);
	HRESULT Ready_Light();
	HRESULT Ready_StageData(const _tchar* szPath);
public:
	static CNetworkStage* Create(LPDIRECT3DDEVICE9 pGraphicDev);
private:
	virtual void Free();

	CDynamicCamera* m_pDynamicCamera = nullptr;
	COcean* m_pOcean = nullptr;
	CNetworkPlayer* m_pLocalPlayer = nullptr;  // 로컬 플레이어 참조 (입력 추출용)
	_vec3           m_vPrevPlayerPos = {};        // 이전 프레임 위치 (방향 계산용)
	float           m_fInputTimer = 0.f;       // 전송 주기 제어 (20TPS)

	CDragon* m_pDragon[4] = {};        // 드래곤 4마리
};