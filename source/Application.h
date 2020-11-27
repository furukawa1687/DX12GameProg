﻿//---------------------------------------------------------------------------
//!	@file	application.h
//!	@brief	アプリケーション
//---------------------------------------------------------------------------
#pragma once
class Dx12Wrapper;
class PMDRenderer;
class PMDActor;
///シングルトンクラス
class Application
{
private:
    //ここに必要な変数(バッファやヒープなど)を書く
    //ウィンドウ周り
    WNDCLASSEX                   _windowClass;
    HWND                         _hwnd;
    std::shared_ptr<Dx12Wrapper> _dx12;
    std::shared_ptr<PMDRenderer> _pmdRenderer;
    std::shared_ptr<PMDActor>    _pmdActor;

    //ゲーム用ウィンドウの生成
    void CreateGameWindow(HWND& hwnd, WNDCLASSEX& windowClass);

    //↓シングルトンのためにコンストラクタをprivateに
    //さらにコピーと代入を禁止に
    Application();
    Application(const Application&) = delete;
    void operator=(const Application&) = delete;

public:
    ///Applicationのシングルトンインスタンスを得る
    static Application& Instance();

    ///初期化
    bool Init();

    ///ループ起動
    void Run();

    ///後処理
    void Terminate();
    SIZE GetWindowSize() const;
    ~Application();
};