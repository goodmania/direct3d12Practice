﻿//-----------------------------------------------------------------------------
// File : SampleApp.h
// Desc : Sample Application.
// Copyright(c) Pocol. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <App.h>
#include <Camera.h>
#include <ConstantBuffer.h>
#include <Material.h>
#include <SphereMapConverter.h>
#include <IBLBaker.h>
#include <SkyBox.h>
#include <Camera.h>
#include <RootSignature.h>
#include <array>


///////////////////////////////////////////////////////////////////////////////
// SampleApp class
///////////////////////////////////////////////////////////////////////////////
class SampleApp : public App
{
    //=========================================================================
    // list of friend classes and methods.
    //=========================================================================
    /* NOTHING */

public:
    //=========================================================================
    // public variables.
    //=========================================================================
    /* NOTHING */

    //=========================================================================
    // public methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //!
    //! @param[in]      width       ウィンドウの横幅です.
    //! @param[in]      height      ウィンドウの縦幅です.
    //-------------------------------------------------------------------------
    SampleApp(uint32_t width, uint32_t height);

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    virtual ~SampleApp();

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    ComPtr<ID3D12PipelineState>     m_pScenePSO;                    //!< シーン用パイプラインステートです.
    RootSignature                   m_SceneRootSig;                 //!< シーン用ルートシグニチャです.
    ComPtr<ID3D12PipelineState>     m_pTonemapPSO;                  //!< トーンマップ用パイプラインステートです.
    RootSignature                   m_TonemapRootSig;               //!< トーンマップ用ルートシグニチャです.
    ColorTarget                     m_SceneColorTarget;             //!< シーン用レンダーターゲットです.
    DepthTarget                     m_SceneDepthTarget;             //!< シーン用深度ターゲットです.
    VertexBuffer                    m_QuadVB;                       //!< 頂点バッファです.
    ConstantBuffer                  m_TonemapCB[FrameCount];        //!< 定数バッファです.
    ConstantBuffer                  m_LightCB[FrameCount];          //!< ライトバッファです.
    ConstantBuffer                  m_CameraCB[FrameCount];         //!< カメラバッファです.
    ConstantBuffer                  m_TransformCB[FrameCount];      //!< 変換用バッファです.
    ConstantBuffer                  m_MeshCB[FrameCount * 16];      //!< メッシュ用バッファです.
    std::vector<Mesh*>              m_pMesh;                        //!< メッシュです.
    Material                        m_Material[16];                 //!< マテリアルです.
    float                           m_RotateAngle;                  //!< ライトの回転角です.
    int                             m_TonemapType;                  //!< トーンマップタイプ.
    int                             m_ColorSpace;                   //!< 出力色空間
    float                           m_BaseLuminance;                //!< 基準輝度値.
    float                           m_MaxLuminance;                 //!< 最大輝度値.
    float                           m_Exposure;                     //!< 露光値.
    Texture                         m_SphereMap;                    //!< スフィアマップです.
    SphereMapConverter              m_SphereMapConverter;           //!< スフィアマップコンバータ.
    IBLBaker                        m_IBLBaker;                     //!< IBLベイク.
    SkyBox                          m_SkyBox;                       //!< スカイボックスです.
    DirectX::SimpleMath::Matrix     m_View;                         //!< ビュー行列.
    DirectX::SimpleMath::Matrix     m_Proj;                         //!< 射影行列.
    Camera                          m_Camera;                       //!< カメラ.
    int                             m_PrevCursorX;                  //!< 前回のカーソル位置X.
    int                             m_PrevCursorY;                  //!< 前回のカーソル位置Y.

    //=========================================================================
    // private methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      初期化時の処理です.
    //!
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //-------------------------------------------------------------------------
    bool OnInit() override;

    //-------------------------------------------------------------------------
    //! @brief      終了時の処理です.
    //-------------------------------------------------------------------------
    void OnTerm() override;

    //-------------------------------------------------------------------------
    //! @brief      描画時の処理です.
    //-------------------------------------------------------------------------
    void OnRender() override;

    //-------------------------------------------------------------------------
    //! @brief      メッセージプロシージャです.
    //-------------------------------------------------------------------------
    void OnMsgProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) override;

    //-------------------------------------------------------------------------
    //! @brief      ディスプレイモードを変更します.
    //!
    //! @param[in]      hdr     trueであればHDRディスプレイ用の設定に変更します.
    //-------------------------------------------------------------------------
    void ChangeDisplayMode(bool hdr);

    //-------------------------------------------------------------------------
    //! @brief      シーンを描画します.
    //-------------------------------------------------------------------------
    void DrawScene(ID3D12GraphicsCommandList* pCmdList);

    //-------------------------------------------------------------------------
    //! @brief      トーンマップを適用します.
    //-------------------------------------------------------------------------
    void DrawTonemap(ID3D12GraphicsCommandList* pCmdList);

    //-------------------------------------------------------------------------
    //! @brief      メッシュを描画します.
    //-------------------------------------------------------------------------
    void DrawMesh(ID3D12GraphicsCommandList* pCmdList, int material_index);

#if 0
    std::array<ComPtr<ID3D12Resource>, 2> m_BloomBuffers;//ブルーム用バッファ
    ComPtr<ID3D12Resource> m_pShrinkBuffer;//被写界深度用ぼかしバッファ


     // bloom用のバッファの生成.
    {
        // ヒーププロパティ.
        D3D12_HEAP_PROPERTIES prop = {};
        prop.Type = D3D12_HEAP_TYPE_UPLOAD;
        prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        prop.CreationNodeMask = 1;
        prop.VisibleNodeMask = 1;

        // カラーターゲットのdesc
        auto& desc = m_ColorTarget->GetDesc();

        D3D12_CLEAR_VALUE clearValue = {};
        clearValue.Color[0] = 0.0f;
        clearValue.Color[1] = 0.0f;
        clearValue.Color[2] = 0.0f;
        clearValue.Color[3] = 1.0f;
        clearValue.Format = desc.Format;

        for (auto& res : m_BloomBuffers)
        {
            auto hr = m_pDevice->CreateCommittedResource(
                &prop,
                D3D12_HEAP_FLAG_NONE,
                &desc,
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                &clearValue,
                IID_PPV_ARGS(res.ReleaseAndGetAddressOf()));

            // 2枚目は縮小バッファなので幅を半分にする
            desc.Width >>= 1;

            if (FAILED(hr))
            {
                return false;
            }
        }
    }
#endif

};
