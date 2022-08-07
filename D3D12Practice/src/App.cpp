#include "App.h"

namespace {
	const auto ClassName = TEXT("SmapleWindowClass");
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
	// �E�B���h�E�̏�����
	if (!InitWnd()) {
		return false;
	}

	// ����I��
	return true;
}

void App::TermApp()
{
	TermWnd();
}

bool App::InitWnd()
{
	// �C���X�^���X�n���h�����擾
	auto hInst = GetModuleHandle(nullptr);
	if (hInst == nullptr) {
		return false;
	}

	// �E�B���h�E�̐ݒ�
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

	// �E�B���h�E�̓o�^
	if (!RegisterClassEx(&wc)) {
		return false;
	}

	// �C���X�^���X�n���h���̐ݒ�
	m_hInst = hInst;

	// �E�B���h�E�T�C�Y��ݒ�
	RECT rc = {};
	rc.right = static_cast<LONG>(m_Width);
	rc.bottom = static_cast<LONG>(m_Height);

	// �E�B���h�E�T�C�Y�𒲐�
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
