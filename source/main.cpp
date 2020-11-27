//---------------------------------------------------------------------------
//!	@file	main.cpp
//!	@brief	アプリケーションメイン
//---------------------------------------------------------------------------

// 初期状態のフルスクリーンモード
constexpr bool FULLSCREEN_MODE = false;

namespace {
HWND hwnd_          = nullptr;   //!< ウィンドウハンドル
u32  mouseDragging_ = 0;         //!< ウィンドウ上でマウスをドラッグしているかどうか　bit0:左ボタン bit1:右ボタン bit2:中央ボタン
}   // namespace

//---------------------------------------------------------------------------
//! ウィンドウプロシージャ
//!	@param	[in]	hwnd	ウィンドウハンドル
//!	@param	[in]	message	ウィンドウメッセージ
//!	@param	[in]	wparam	パラメーター1
//!	@param	[in]	lparam	パラメーター2
//!	@return ウィンドウプロシージャのそれぞれのメッセージ処理後の値
//---------------------------------------------------------------------------
LRESULT CALLBACK windowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    switch(message) {
        case WM_CREATE:   // ウィンドウ生成時(コンストラクタ相当)
        {
            // ここでGWLP_USERDATAにthisポインタを保存
            auto* cs = reinterpret_cast<CREATESTRUCT*>(lparam);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
            return 0;
        }

        case WM_SYSKEYDOWN:
        {
            auto vk = wparam;          // 現在押されている仮想キーコード
            if(lparam & (1 << 29)) {   // Altキーが押されているかどうか
                // Alt+Enter
                if(vk == VK_RETURN) {
                    // フルスクリーンモードを切り替え
                    auto* gpuSwapChain = gpu::render()->swapChain();
                    gpuSwapChain->toggleFullScreen();
                    return 0;
                }
            }
            break;
        }

        // マウス左ボタンを押した
        case WM_LBUTTONDOWN:
            mouseDragging_ |= 1 << 0;
            SetCapture(hwnd);
            break;
        // マウス右ボタンを押した
        case WM_RBUTTONDOWN:
            mouseDragging_ |= 1 << 1;
            SetCapture(hwnd);
            break;
        // マウス中央ボタンを押した
        case WM_MBUTTONDOWN:
            mouseDragging_ |= 1 << 2;
            SetCapture(hwnd);
            break;

            // マウス左ボタンを離した
        case WM_LBUTTONUP:
            mouseDragging_ &= ~(1 << 0);
            if(mouseDragging_ == 0) {
                ReleaseCapture();
            }
            break;
            // マウス右ボタンを離した
        case WM_RBUTTONUP:
            mouseDragging_ &= ~(1 << 1);
            if(mouseDragging_ == 0) {
                ReleaseCapture();
            }
            break;
            // マウス中央ボタンを離した
        case WM_MBUTTONUP:
            mouseDragging_ &= ~(1 << 2);
            if(mouseDragging_ == 0) {
                ReleaseCapture();
            }
            break;

        case WM_PAINT:   // 再描画
            gpu::render()->update();
            gpu::render()->render();
            return 0;

        case WM_DESTROY:   // ウィンドウ破棄時(デストラクタ相当)
            PostQuitMessage(0);
            return 0;
    }

    //-------------------------------------------------------------
    // デフォルトのウィンドウプロシージャ
    // ※注意※ UNICODE版CreateWindowWを利用する場合はこの関数もUNICODE版にする必要あり
    //-------------------------------------------------------------
    return DefWindowProcW(hwnd, message, wparam, lparam);
}

//---------------------------------------------------------------------------
// メイン処理
//---------------------------------------------------------------------------
int run()
{
    // 高DPIモード対応
    // ウィンドウUIのみDPIスケーリングされ、クライアント領域はdot-by-dot
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    //----------------------------------------------------------
    // ウィンドウクラスの登録
    //----------------------------------------------------------
    WNDCLASSEXW windowClass{};
    windowClass.cbSize        = sizeof(windowClass);           // 構造体サイズ
    windowClass.style         = CS_HREDRAW | CS_VREDRAW;       // クラススタイル
    windowClass.lpfnWndProc   = &windowProc;                   // ウィンドウプロシージャ
    windowClass.hInstance     = application::instance();       // アプリケーションインスタンスハンドル
    windowClass.hCursor       = LoadCursor(NULL, IDC_CROSS);   // カーソルの種類
    windowClass.lpszClassName = L"DX12SampleClass";            // クラス名

    RegisterClassExW(&windowClass);

    //----------------------------------------------------------
    // ウィンドウを作成
    //----------------------------------------------------------
    u32 style   = WS_OVERLAPPEDWINDOW;   // ウィンドウスタイル
    u32 styleEx = 0;                     // 拡張ウィンドウスタイル
    s32 x       = CW_USEDEFAULT;
    s32 y       = CW_USEDEFAULT;
    s32 w       = CW_USEDEFAULT;
    s32 h       = CW_USEDEFAULT;

    // ウィンドウサイズをUIサイズを考慮して補正
    {
        RECT windowRect{ 0, 0, 1280, 720 };

        AdjustWindowRectEx(&windowRect, style, false, styleEx);
        w = windowRect.right - windowRect.left;
        h = windowRect.bottom - windowRect.top;
    }
    // ウィンドウを作成
    hwnd_ = CreateWindowExW(
        styleEx,                     // 拡張ウィンドウスタイル
        windowClass.lpszClassName,   // ウィンドウクラス名
        L"Direct3D12 サンプル",      // タイトル名
        style,                       // ウィンドウスタイル
        x,                           // X座標
        y,                           // Y座標
        w,                           // ウィンドウの幅
        h,                           // ウィンドウの高さ
        nullptr,                     // 親ウィンドウ(なし)
        nullptr,                     // メニューハンドル(なし)
        application::instance(),     // アプリケーションインスタンスハンドル
        nullptr);                    // WM_CREATEへの引数(任意)

    //----------------------------------------------------------
    // 初期化
    //----------------------------------------------------------
    if(!gpu::render()->initialize(1280, 720)) {
        return false;
    }

    if constexpr(FULLSCREEN_MODE) {
        auto swapChain = gpu::render()->swapChain();
        swapChain->toggleFullScreen();
    }

    // ウィンドウを表示
    {
        // GetWindowPlacementからWinMainの引数のnCmdShowを取得可能。
        WINDOWPLACEMENT placement;
        GetWindowPlacement(application::hwnd(), &placement);
        ShowWindow(application::hwnd(), placement.showCmd);
    }

    //=============================================================
    // メインメッセージループ
    //=============================================================
    MSG msg{};

    while(msg.message != WM_QUIT) {
        if(PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    //----------------------------------------------------------
    // 解放
    //----------------------------------------------------------
    gpu::render()->finalize();

    // WM_QUITメッセージの戻り値を終了コードとして返す
    return static_cast<int>(msg.wParam);
}

namespace application {

//---------------------------------------------------------------------------
//! ウィンドウハンドルを取得
//---------------------------------------------------------------------------
HWND hwnd()
{
    return hwnd_;
}

//---------------------------------------------------------------------------
//! アプリケーションインスタンスハンドルを取得
//---------------------------------------------------------------------------
HINSTANCE instance()
{
    return GetModuleHandle(nullptr);
}

//---------------------------------------------------------------------------
//! マウスフォーカスを持っているかどうか
//---------------------------------------------------------------------------
bool hasMouseFocus()
{
    return mouseDragging_ != 0;
}

}   // namespace application

//===========================================================================
//! アプリケーション開始点
//===========================================================================
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    return run();
}
