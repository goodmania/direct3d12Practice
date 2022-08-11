#include "App.h"

namespace {
	const auto ClassName = TEXT("SmapleWindowClass");
	template<typename T> 
	void SafeRelease(T*& ptr) { 
		if (ptr != nullptr) {
			ptr— > Release();
			ptr = nullptr; }
	}
}

App::App(uint32_t width, uint32_t height) 
	: m_hInst(nullptr)
	, m_hWnd(nullptr)
	, m_Width(width)
	, m_Height(height) 
{} 

App::~App()
{
}

void App::Run()
{
	if (InitApp()) {
		MainLoop();
	}
	TermApp();
}

bool App::InitApp()
{
	// ウィンドウの初期化
	if (!InitWnd()) {
		return false;
	}

	// 正常終了
	return true;
}

void App::TermApp()
{
	TermWnd();
}

bool App::InitWnd()
{
	// インスタンスハンドルを取得
	auto hInst = GetModuleHandle(nullptr);
	if (hInst == nullptr) {
		return false;
	}

	// ウィンドウの設定
	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hIcon = LoadIcon(hInst, IDI_APPLICATION);
	wc.hCursor = LoadCursor(hInst, IDC_ARROW);
	wc.hbrBackground = GetSysColorBrush(COLOR_BACKGROUND);
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = ClassName;
	wc.hIconSm = LoadIcon(hInst, IDI_APPLICATION);

	// ウィンドウの登録
	if (!RegisterClassEx(&wc)) {
		return false;
	}

	// インスタンスハンドルの設定
	m_hInst = hInst;

	// ウィンドウサイズを設定
	RECT rc = {};
	rc.right = static_cast<LONG>(m_Width);
	rc.bottom = static_cast<LONG>(m_Height);

	// ウィンドウサイズを調整
	auto style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
	AdjustWindowRect(&rc, style, FALSE);

	m_hWnd = CreateWindowEx(
		0,
		ClassName,
		TEXT("sample"),
		style,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		nullptr,
		nullptr,
		m_hInst,
		nullptr
	);

	if (m_hWnd == nullptr) {
		return false;
	}

	ShowWindow(m_hWnd, SW_SHOWNORMAL);

	UpdateWindow(m_hWnd);

	return true;
}

void App::TermWnd()
{
	if (m_hInst != nullptr) {
		UnregisterClass(ClassName, m_hInst);
	}
	m_hInst = nullptr;
	m_hWnd = nullptr;
}

void App::MainLoop()
{
	MSG msg = {};

	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) == TRUE) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

bool App::InitD3D()
{
	auto hr = D3D12CreateDevice(
		nullptr,
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&m_pDevice));

	if (FAILED(hr)) {
		return false;
	}

	// コマンドキューの生成
	{
		D3D12_COMMAND_QUEUE_DESC desc = {};
		desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.NodeMask = 0;

		hr = m_pDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_pQueue));
		if (FAILED(hr)) {
			return false;
		}
	}

	// スワップチェインの生成
	{
		IDXGIFactory4* pFactory = nullptr;
		hr = CreateDXGIFactory1(IID_PPV_ARGS(&pFactory));
		if (FAILED(hr)){
			return false; 
		}

		DXGI_SWAP_CHAIN_DESC desc = {};
		desc.BufferDesc.Width = m_Width;
		desc.BufferDesc.Height = m_Height;
		desc.BufferDesc.RefreshRate.Numerator = 60;
		desc.BufferDesc.RefreshRate.Denominator = 1;
		desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.BufferCount = FrameCount;
		desc.OutputWindow = m_hWnd;
		desc.Windowed = TRUE;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		IDXGISwapChain* pSwapChain = nullptr;

		hr = pFactory->CreateSwapChain(m_pQueue, &desc, &pSwapChain);
		if (FAILED(hr)) {
			SafeRelease(pFactory);
			return false; 
		}

		hr = pSwapChain->QueryInterface(IID_PPV_ARGS(&m_pSwapChain));
		if (FAILED(hr)) 
		{ SafeRelease(pFactory);
		  SafeRelease(pSwapChain);
		  return false; 
		}

		m_FrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();
		SafeRelease(pFactory);
		SafeRelease(pSwapChain);
	}

	// コマンドアロケーターの生成
	{
		// ダブルバファリングなので二フレーム分作成
		for (auto i = 0u; i < FrameCount; ++i) {
			hr = m_pDevice->CreateCommandAllocator(
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				IID_PPV_ARGS(&m_pCmdAllocator[i])
			);

			if (FAILED(hr)) {
				return false; 
			}
		}
	}

	// コマンドリストの生成
	{
		hr = m_pDevice->CreateCommandList(
			0, // GPUが一つの場合0
			D3D12_COMMAND_LIST_TYPE_DIRECT, // コマンドキューに直接登録可能なコマンドリスト
			m_pCmdAllocator[m_FrameIndex], // 描画コマンドを積むのはバックバッファ番号
			nullptr,
			IID_PPV_ARGS(&m_pCmdList)
		);  
		if (FAILED(hr)) { 
			return false; 
		}
	}

	// レンダーターゲットビューの生成 // 絵を書くためのキャンバス DirectXの場合はバックバッファやテクスチャ等のリソース	
	{
		// ディスクリプタのアドレスを取得する。
		// ディスクリプタヒープを作成
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = FrameCount;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		desc.NodeMask = 0;

		hr = m_pDevice->CreateDescriptorHeap(
			&desc,
			IID_PPV_ARGS(&m_pHeapRTV)
		); 
		if (FAILED(hr)) { 
			return false; 
		}

		auto handle = m_pHeapRTV->GetCPUDescriptorHandleForHeapStart(); // ディスクリプタの先頭アドレス
		auto incrementSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV); // アドレスをずらすためのオフセット

		for (auto i = 0u; i < FrameCount; ++i) {
			hr = m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_pColorBuffer[i]));
			if (FAILED(hr)) {
				return false;
			}

			// 次元情報、ピクセルフォーマット
			D3D12_RENDER_TARGET_VIEW_DESC viewDesc = {};
			viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
			viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			viewDesc.Texture2D.MipSlice = 0;
			viewDesc.Texture2D.PlaneSlice = 0;

			m_pDevice->CreateRenderTargetView(
				m_pColorBuffer[i],
				&viewDesc,
				handle
			); 
			// 割り当てるアドレスを指定。
			m_HandleRTV[i] = handle;
			handle.ptr += incrementSize;
		}
	}

	// フェンスの生成 CPU GPUの同期処理
	{
		for (auto i = 0u; i < FrameCount; ++i) {
			m_FenceCounter[i] = 0; 
		}

		hr = m_pDevice->CreateFence(
			m_FenceCounter[m_FrameIndex],
			D3D12_FENCE_FLAG_NONE,
			IID_PPV_ARGS(&m_pFence)
		);

		if (FAILED(hr)) {
			return false;
		}

		m_FenceCounter[m_FrameIndex]++; // 描画処理の完了は、フェンスの値がインクリメントされたかどうかで判断。

		// 終わるまで待つという処理
		m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (m_FenceEvent == nullptr) {
			return false; 
		}
	}
	// コマンドリストを閉じる
	m_pCmdList->Close();
	return true;
}

void App::TermD3D()
{
}

void App::Render()
{
	m_pCmdAllocator[m_FrameIndex]->Reset(); // コマンドバッファの内容を先頭に戻す。
	// 描画コマンドの作成を開始。
	m_pCmdList->Reset(m_pCmdAllocator[m_FrameIndex], nullptr); 

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = m_pColorBuffer[m_FrameIndex];
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	m_pCmdList->ResourceBarrier(1, &barrier);

	m_pCmdList->OMSetRenderTargets(1, &m_HandleRTV[m_FrameIndex], FALSE, nullptr);

	float clearColor[] = { 0.25f, 0.25f, 0.25f, 1.0f };

	m_pCmdList->ClearRenderTargetView(m_HandleRTV[m_FrameIndex], clearColor, 0, nullptr);

	// 描画処理
	{
		// todo : ポリゴン描画用の処理追加
	}

	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = m_pColorBuffer[m_FrameIndex];
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	m_pCmdList->ResourceBarrier(1, &barrier);

	m_pCmdList->Close();

	ID3D12CommandList* ppCmdLists[] = { m_pCmdList };
	m_pQueue->ExecuteCommandLists(1, ppCmdLists);
}

void App::WaitGpu()
{
}

void App::Present(uint32_t interval)
{
}

LRESULT App::WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg) {
	case WM_DESTROY:
	{PostQuitMessage(0); }
	break;

	default:
	{}
	break;
	}
	return DefWindowProc(hWnd, msg, wp, lp);
}
