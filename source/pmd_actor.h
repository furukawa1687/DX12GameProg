﻿//---------------------------------------------------------------------------
//!	@file	pmd_actor.h
//!	@brief	PMDアクター
//---------------------------------------------------------------------------
#pragma once

#include <d3d12.h>
#include <DirectXMath.h>
#include <vector>
#include <wrl.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
class Dx12Wrapper;
class PMDRenderer;
class PMDActor
{
    friend PMDRenderer;

private:
    PMDRenderer& _renderer;
    Dx12Wrapper& _dx12;
    template<typename T>
    using com_ptr = Microsoft::WRL::ComPtr<T>;

    //頂点関連
    com_ptr<ID3D12Resource>   _vb     = nullptr;
    com_ptr<ID3D12Resource>   _ib     = nullptr;
    D3D12_VERTEX_BUFFER_VIEW _vbView = {};
    D3D12_INDEX_BUFFER_VIEW  _ibView = {};

    com_ptr<ID3D12Resource>       _transformMat  = nullptr;   //座標変換行列(今はワールドのみ)
    com_ptr<ID3D12DescriptorHeap> _transformHeap = nullptr;   //座標変換ヒープ

    //シェーダ側に投げられるマテリアルデータ
    struct MaterialForHlsl
    {
        DirectX::XMFLOAT3 diffuse;       //ディフューズ色
        float             alpha;         // ディフューズα
        DirectX::XMFLOAT3 specular;      //スペキュラ色
        float             specularity;   //スペキュラの強さ(乗算値)
        DirectX::XMFLOAT3 ambient;       //アンビエント色
    };
    //それ以外のマテリアルデータ
    struct AdditionalMaterial
    {
        std::string texPath;   //テクスチャファイルパス
        int         toonIdx;   //トゥーン番号
        bool        edgeFlg;   //マテリアル毎の輪郭線フラグ
    };
    //まとめたもの
    struct Material
    {
        unsigned int       indicesNum;   //インデックス数
        MaterialForHlsl    material;
        AdditionalMaterial additional;
    };

    struct Transform
    {
        //内部に持ってるXMMATRIXメンバが16バイトアライメントであるため
        //Transformをnewする際には16バイト境界に確保する
        void*             operator new(size_t size);
        DirectX::XMMATRIX world;
    };


    Transform              _transform;
    DirectX::XMMATRIX*     _mappedMatrices = nullptr;
    com_ptr<ID3D12Resource> _transformBuff   = nullptr;

    //マテリアル関連
    std::vector<Material>               _materials;
    com_ptr<ID3D12Resource>              _materialBuff = nullptr;
    std::vector<com_ptr<ID3D12Resource>> _textureResources;
    std::vector<com_ptr<ID3D12Resource>> _sphResources;
    std::vector<com_ptr<ID3D12Resource>> _spaResources;
    std::vector<com_ptr<ID3D12Resource>> _toonResources;

    // ボーン関連
    std::vector<DirectX::XMMATRIX> _boneMatrices; 

    struct BoneNode
    {
        u32 boneIdx;                        // ボーンインデックス
        DirectX::XMFLOAT3 startPos;         // ボーン基準点（回転中心）
        std::vector<BoneNode*> children;    // 子ノード
    };

    std::map<std::string, BoneNode> _boneNodeTable;
    std::vector<std::string>        _boneNameArray; // ボーン名を格納しておく
    std::vector<BoneNode*>          _boneNodeAddressArray;// ノードを格納しておく

    struct PMDIK
    {
        u16 boneIdx;
        u16 targetIdx;
        u16 iterations;
        f32 limit;
        std::vector<u16> nodeIdxes;
    };

    std::vector<PMDIK> _ikData;

    // キーフレーム構造体
    struct KeyFrame
    {
        u32 frameNo;
        DirectX::XMVECTOR quaternion;
        DirectX::XMFLOAT3 offset; // クォータニオン
        DirectX::XMFLOAT2 p1, p2;   //ベジェの中間コントロールポイント
        // コンストラクタ
        KeyFrame(u32 fno, DirectX::XMVECTOR& q, DirectX::XMFLOAT3& ofs, DirectX::XMFLOAT2& ip1, DirectX::XMFLOAT2& ip2)
        : frameNo(fno),
        quaternion(q), 
        offset(ofs),
        p1(ip1),
        p2(ip2){}
    };

    std::unordered_map<std::string, std::vector<KeyFrame>> _motionData;

    //読み込んだマテリアルをもとにマテリアルバッファを作成
    HRESULT CreateMaterialData();

    com_ptr<ID3D12DescriptorHeap> _materialHeap = nullptr;   //マテリアルヒープ(5個ぶん)
    //マテリアル＆テクスチャのビューを作成
    HRESULT CreateMaterialAndTextureView();

    //座標変換用ビューの生成
    HRESULT CreateTransformView();

    //PMDファイルのロード
    HRESULT LoadPMDFile(const char* path);
    void    RecursiveMatrixMultiply(BoneNode* node, DirectX::XMMATRIX& mat);
    f32 _angle;   //テスト用Y軸回転

	f32 GetYFromXOnBezier(f32 x, const DirectX::XMFLOAT2& a, const DirectX::XMFLOAT2& b, uint8_t n = 12);

    DWORD _startTime;
    void MotionUpdate();

public:
    PMDActor(const char* filepath, PMDRenderer& renderer);
    ~PMDActor();
    ///クローンは頂点およびマテリアルは共通のバッファを見るようにする
    PMDActor* Clone();
    void      LoadVMDFile(const char* filepath, const char* name);
    void      Update();
    void      Draw();
    void PlayAnimation();
};
