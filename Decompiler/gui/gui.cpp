#include "gui.h"
#include <string>
#include <Windows.h>
#include <vector>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <streambuf>
#include <Shlwapi.h>

#pragma comment(lib, "shlwapi.lib")



#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_dx9.h"
#include "../imgui/imgui_impl_win32.h"

extern IMGUI_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static std::vector<float> fpsGraph;



long __stdcall WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept {

	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg) {
	case WM_DROPFILES:
	{
		HDROP hDrop = (HDROP)wParam;
		TCHAR szFile[MAX_PATH];
		DragQueryFile(hDrop, 0, szFile, MAX_PATH);
		DragFinish(hDrop);
		std::string path = szFile;
		if (std::filesystem::exists(path)) {
			if (std::filesystem::is_directory(path)) {
				gui::selectedPath = path;
				gui::selectedFile = "";
			}
			else {
				gui::selectedPath = std::filesystem::path(path).parent_path().string();
				gui::selectedFile = std::filesystem::path(path).filename().string();
			}
		}
		return 0;
	}
	case WM_SIZE:
		if (gui::device != nullptr && wParam != SIZE_MINIMIZED) {
			gui::parameters.BackBufferWidth = LOWORD(lParam);
			gui::parameters.BackBufferHeight = HIWORD(lParam);
			gui::resetDevice();
		}
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		gui::exit = true;
		return 0;
		break;
	case WM_LBUTTONDOWN:
		gui::position = MAKEPOINTS(lParam);
		return 0;
		break;
	case WM_MOUSEMOVE:
		if (wParam == MK_LBUTTON) {
			const auto points = MAKEPOINTS(lParam);
			auto rect = ::RECT{};
			GetWindowRect(hWnd, &rect);
			rect.left += points.x - gui::position.x;
			rect.top += points.y - gui::position.y;

			if (gui::position.x >= 0 && gui::position.x <= gui::WIDTH && gui::position.y >= 0 && gui::position.y <= 30)
				SetWindowPos(hWnd, HWND_TOPMOST, rect.left, rect.top, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER);
		}
		return 0;
		break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}


void gui::createHWindow(const char* windowName,const char* className) noexcept {
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_CLASSDC;
	windowClass.lpfnWndProc = WndProc;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = GetModuleHandle(nullptr);
	windowClass.hIcon = nullptr;
	windowClass.hCursor = nullptr;
	windowClass.hbrBackground = nullptr;
	windowClass.lpszMenuName = nullptr;
	windowClass.lpszClassName = className;
	windowClass.hIconSm = nullptr;

	RegisterClassEx(&windowClass);
	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	gui::windowWidth = screenWidth / 2 - 500;
	gui::windowHeight = screenHeight / 2 - 350;

	window = CreateWindowEx(
		WS_EX_LAYERED,
		className,
		windowName,
		WS_POPUP,
		windowWidth,
		windowHeight,
		WIDTH,
		HEIGHT,
		nullptr,
		nullptr,
		windowClass.hInstance,
		nullptr
	);
	SetLayeredWindowAttributes(window, RGB(0, 0, 0), 0, ULW_COLORKEY);


	if (window == nullptr) {
		exit = true;
	}
	else {
		ShowWindow(window, SW_SHOWDEFAULT);
		UpdateWindow(window);
	}
}

void gui::DestroyWindow() noexcept {
	if (window != nullptr) {
		DestroyWindow(window);
		UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
		window = nullptr;
	}

}
bool gui::createDevice() noexcept {
	d3d = Direct3DCreate9(D3D_SDK_VERSION);
	if (d3d == nullptr)
		return false;

	ZeroMemory(&parameters, sizeof(parameters));

	parameters.Windowed = TRUE;
	parameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	parameters.BackBufferFormat = D3DFMT_UNKNOWN;
	parameters.EnableAutoDepthStencil = TRUE;
	parameters.AutoDepthStencilFormat = D3DFMT_D16;
	parameters.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

	if (d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window, D3DCREATE_HARDWARE_VERTEXPROCESSING, &parameters, &device) < 0)
		return false;

	return true;

}
void gui::resetDevice() noexcept {
	ImGui_ImplDX9_InvalidateDeviceObjects();
	const auto result = gui::device->Reset(&gui::parameters);
	if (result == D3DERR_INVALIDCALL)
		IM_ASSERT(0);

	ImGui_ImplDX9_CreateDeviceObjects();

}
void gui::destroyDevice() noexcept {
	if (device != nullptr) {
		device->Release();
		device = nullptr;
	}
	if (d3d != nullptr) {
		d3d->Release();
		d3d = nullptr;
	}
}

void MoonLightStyle() noexcept
{
	ImGuiStyle& style = ImGui::GetStyle();

	style.Alpha = 1.0f;
	style.DisabledAlpha = 1.0f;
	style.WindowPadding = ImVec2(12.0f, 12.0f);
	style.WindowRounding = 11.5f;
	style.WindowBorderSize = 0.0f;
	style.WindowMinSize = ImVec2(20.0f, 20.0f);
	style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
	style.WindowMenuButtonPosition = ImGuiDir_Right;
	style.ChildRounding = 0.0f;
	style.ChildBorderSize = 1.0f;
	style.PopupRounding = 0.0f;
	style.PopupBorderSize = 1.0f;
	style.FramePadding = ImVec2(20.0f, 3.400000095367432f);
	style.FrameRounding = 11.89999961853027f;
	style.FrameBorderSize = 0.0f;
	style.ItemSpacing = ImVec2(4.300000190734863f, 5.5f);
	style.ItemInnerSpacing = ImVec2(7.099999904632568f, 1.799999952316284f);
	style.CellPadding = ImVec2(12.10000038146973f, 9.199999809265137f);
	style.IndentSpacing = 0.0f;
	style.ColumnsMinSpacing = 4.900000095367432f;
	style.ScrollbarSize = 11.60000038146973f;
	style.ScrollbarRounding = 15.89999961853027f;
	style.GrabMinSize = 3.700000047683716f;
	style.GrabRounding = 20.0f;
	style.TabRounding = 0.0f;
	style.TabBorderSize = 0.0f;
	style.TabMinWidthForCloseButton = 0.0f;
	style.ColorButtonPosition = ImGuiDir_Right;
	style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
	style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

	style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.2745098173618317f, 0.3176470696926117f, 0.4509803950786591f, 1.0f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(0.09411764889955521f, 0.1019607856869698f, 0.1176470592617989f, 1.0f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.1568627506494522f, 0.168627455830574f, 0.1921568661928177f, 1.0f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.1137254908680916f, 0.125490203499794f, 0.1529411822557449f, 1.0f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.1568627506494522f, 0.168627455830574f, 0.1921568661928177f, 1.0f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.1568627506494522f, 0.168627455830574f, 0.1921568661928177f, 1.0f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.09803921729326248f, 0.105882354080677f, 0.1215686276555061f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.1568627506494522f, 0.168627455830574f, 0.1921568661928177f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.9725490212440491f, 1.0f, 0.4980392158031464f, 1.0f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.9725490212440491f, 1.0f, 0.4980392158031464f, 1.0f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 0.7960784435272217f, 0.4980392158031464f, 1.0f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.1803921610116959f, 0.1882352977991104f, 0.196078434586525f, 1.0f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.1529411822557449f, 0.1529411822557449f, 0.1529411822557449f, 1.0f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.1411764770746231f, 0.1647058874368668f, 0.2078431397676468f, 1.0f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.105882354080677f, 0.105882354080677f, 0.105882354080677f, 1.0f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
	style.Colors[ImGuiCol_Separator] = ImVec4(0.1294117718935013f, 0.1490196138620377f, 0.1921568661928177f, 1.0f);
	style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.1568627506494522f, 0.1843137294054031f, 0.250980406999588f, 1.0f);
	style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.1568627506494522f, 0.1843137294054031f, 0.250980406999588f, 1.0f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1450980454683304f, 1.0f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.9725490212440491f, 1.0f, 0.4980392158031464f, 1.0f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_Tab] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
	style.Colors[ImGuiCol_TabHovered] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
	style.Colors[ImGuiCol_TabActive] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
	style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
	style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.125490203499794f, 0.2745098173618317f, 0.572549045085907f, 1.0f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.5215686559677124f, 0.6000000238418579f, 0.7019608020782471f, 1.0f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.03921568766236305f, 0.9803921580314636f, 0.9803921580314636f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.8823529481887817f, 0.7960784435272217f, 0.5607843399047852f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.95686274766922f, 0.95686274766922f, 0.95686274766922f, 1.0f);
	style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
	style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
	style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
	style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.09803921729326248f, 0.105882354080677f, 0.1215686276555061f, 1.0f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.9372549057006836f, 0.9372549057006836f, 0.9372549057006836f, 1.0f);
	style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.2666666805744171f, 0.2901960909366608f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 0.501960813999176f);
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 0.501960813999176f);

}

void ComfyStyle()
{
	ImGuiStyle& style = ImGui::GetStyle();
	style.Alpha = 1.0f;
	style.DisabledAlpha = 0.1000000014901161f;
	style.WindowPadding = ImVec2(8.0f, 8.0f);
	style.WindowRounding = 10.0f;
	style.WindowBorderSize = 0.0f;
	style.WindowMinSize = ImVec2(30.0f, 30.0f);
	style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
	style.WindowMenuButtonPosition = ImGuiDir_Right;
	style.ChildRounding = 5.0f;
	style.ChildBorderSize = 1.0f;
	style.PopupRounding = 10.0f;
	style.PopupBorderSize = 0.0f;
	style.FramePadding = ImVec2(5.0f, 3.5f);
	style.FrameRounding = 5.0f;
	style.FrameBorderSize = 0.0f;
	style.ItemSpacing = ImVec2(5.0f, 4.0f);
	style.ItemInnerSpacing = ImVec2(5.0f, 5.0f);
	style.CellPadding = ImVec2(4.0f, 2.0f);
	style.IndentSpacing = 5.0f;
	style.ColumnsMinSpacing = 5.0f;
	style.ScrollbarSize = 15.0f;
	style.ScrollbarRounding = 9.0f;
	style.GrabMinSize = 15.0f;
	style.GrabRounding = 5.0f;
	style.TabRounding = 5.0f;
	style.TabBorderSize = 0.0f;
	style.TabMinWidthForCloseButton = 0.0f;
	style.ColorButtonPosition = ImGuiDir_Right;
	style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
	style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

	style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(1.0f, 1.0f, 1.0f, 0.3605149984359741f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.09803921729326248f, 0.09803921729326248f, 0.09803921729326248f, 1.0f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.09803921729326248f, 0.09803921729326248f, 0.09803921729326248f, 1.0f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.4235294163227081f, 0.3803921639919281f, 0.572549045085907f, 0.54935622215271f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.1568627506494522f, 0.1568627506494522f, 0.1568627506494522f, 1.0f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.3803921639919281f, 0.4235294163227081f, 0.572549045085907f, 0.5490196347236633f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.6196078658103943f, 0.5764706134796143f, 0.7686274647712708f, 0.5490196347236633f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.09803921729326248f, 0.09803921729326248f, 0.09803921729326248f, 1.0f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.09803921729326248f, 0.09803921729326248f, 0.09803921729326248f, 1.0f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.2588235437870026f, 0.2588235437870026f, 0.2588235437870026f, 0.0f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.1568627506494522f, 0.1568627506494522f, 0.1568627506494522f, 0.0f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.1568627506494522f, 0.1568627506494522f, 0.1568627506494522f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.2352941185235977f, 0.2352941185235977f, 0.2352941185235977f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.294117659330368f, 0.294117659330368f, 0.294117659330368f, 1.0f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.294117659330368f, 0.294117659330368f, 0.294117659330368f, 1.0f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.6196078658103943f, 0.5764706134796143f, 0.7686274647712708f, 0.5490196347236633f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.8156862854957581f, 0.772549033164978f, 0.9647058844566345f, 0.5490196347236633f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.6196078658103943f, 0.5764706134796143f, 0.7686274647712708f, 0.5490196347236633f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.7372549176216125f, 0.6941176652908325f, 0.886274516582489f, 0.5490196347236633f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.8156862854957581f, 0.772549033164978f, 0.9647058844566345f, 0.5490196347236633f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.6196078658103943f, 0.5764706134796143f, 0.7686274647712708f, 0.5490196347236633f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.7372549176216125f, 0.6941176652908325f, 0.886274516582489f, 0.5490196347236633f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.8156862854957581f, 0.772549033164978f, 0.9647058844566345f, 0.5490196347236633f);
	style.Colors[ImGuiCol_Separator] = ImVec4(0.6196078658103943f, 0.5764706134796143f, 0.7686274647712708f, 0.5490196347236633f);
	style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.7372549176216125f, 0.6941176652908325f, 0.886274516582489f, 0.5490196347236633f);
	style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.8156862854957581f, 0.772549033164978f, 0.9647058844566345f, 0.5490196347236633f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.6196078658103943f, 0.5764706134796143f, 0.7686274647712708f, 0.5490196347236633f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.7372549176216125f, 0.6941176652908325f, 0.886274516582489f, 0.5490196347236633f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.8156862854957581f, 0.772549033164978f, 0.9647058844566345f, 0.5490196347236633f);
	style.Colors[ImGuiCol_Tab] = ImVec4(0.6196078658103943f, 0.5764706134796143f, 0.7686274647712708f, 0.5490196347236633f);
	style.Colors[ImGuiCol_TabHovered] = ImVec4(0.7372549176216125f, 0.6941176652908325f, 0.886274516582489f, 0.5490196347236633f);
	style.Colors[ImGuiCol_TabActive] = ImVec4(0.8156862854957581f, 0.772549033164978f, 0.9647058844566345f, 0.5490196347236633f);
	style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.0f, 0.4509803950786591f, 1.0f, 0.0f);
	style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.1333333402872086f, 0.2588235437870026f, 0.4235294163227081f, 0.0f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.294117659330368f, 0.294117659330368f, 0.294117659330368f, 1.0f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.7372549176216125f, 0.6941176652908325f, 0.886274516582489f, 0.5490196347236633f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.6196078658103943f, 0.5764706134796143f, 0.7686274647712708f, 0.5490196347236633f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.7372549176216125f, 0.6941176652908325f, 0.886274516582489f, 0.5490196347236633f);
	style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.1882352977991104f, 0.1882352977991104f, 0.2000000029802322f, 1.0f);
	style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.4235294163227081f, 0.3803921639919281f, 0.572549045085907f, 0.5490196347236633f);
	style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.4235294163227081f, 0.3803921639919281f, 0.572549045085907f, 0.2918455004692078f);
	style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.03433477878570557f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.7372549176216125f, 0.6941176652908325f, 0.886274516582489f, 0.5490196347236633f);
	style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 1.0f, 0.0f, 0.8999999761581421f);
	style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.699999988079071f);
	style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.2000000029802322f);
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.3499999940395355f);
}
void gui::SetupImGuiStyle() noexcept
{
	MoonLightStyle();
}
void gui::createImgui() noexcept {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO io = ::ImGui::GetIO();
	ImFont *DefaultFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 20.0f, NULL, io.Fonts->GetGlyphRangesCyrillic());
	io.FontDefault = DefaultFont;
	io.IniFilename = 0;

	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX9_Init(device);
	

}
void gui::destroyImgui() noexcept {

	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

}
void gui::BeginRender() noexcept {

	MSG message;
	while (PeekMessage(&message, window, 0, 0, PM_REMOVE)) {
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

}
void gui::EndRender() noexcept {

	ImGui::EndFrame();

	device->SetRenderState(D3DRS_ZENABLE, FALSE);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

	device->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0, 0, 0, 0), 1.0f, 0);

	if (device->BeginScene() >= 0) {
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		device->EndScene();
	}

	const auto result = device->Present(nullptr, nullptr, nullptr, nullptr);

	if (result == D3DERR_DEVICELOST && device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
		resetDevice();

}


void drawSettings() noexcept {
	ImGui::Text("Settings");

	// FPS
	float fps = ImGui::GetIO().Framerate;
	fpsGraph.push_back(fps);
	if (fpsGraph.size() > 100)
		fpsGraph.erase(fpsGraph.begin());

	std::string fpsText = "FPS: " + std::to_string(static_cast<int>(fps));
	ImGui::Text(fpsText.c_str());
	ImGui::PlotLines("FPS"
		, fpsGraph.data(), fpsGraph.size(), 0, nullptr, 0, 100, ImVec2(0, 80));

	ImGui::NewLine();

	// Theming options
	static int theme = gui::themeSelected;
	static const char* themes[] = { "MoonLightStyle","ComfyStyle","Dark","Light" };
	ImGui::Text("Theme");
	if (ImGui::Combo("##Theme", &theme, themes, IM_ARRAYSIZE(themes))) {
		if (theme == 0) {
			gui::themeSelected = 0;
			MoonLightStyle();
		}
		else if (theme == 1) {
			gui::themeSelected = 1;
			ComfyStyle();
		}
		else if (theme == 2) {
			gui::themeSelected = 2;
			ImGui::StyleColorsDark();
		}
		else if (theme == 3) {
			gui::themeSelected = 3;
			ImGui::StyleColorsLight();
		}
	}
	ImGui::NewLine();
	
	// Custom theming options
	ImGui::Text("Custom Theme");
	 
	// Background color
	static ImVec4 bgColor = ImVec4(0.09803921729326248f, 0.09803921729326248f, 0.09803921729326248f, 1.0f);
	ImGui::Text("Background Color");
	ImGui::ColorEdit4("##Background Color", (float*)&bgColor);

	// Text color
	static ImVec4 textColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	ImGui::Text("Text Color");
	ImGui::ColorEdit4("##Text Color", (float*)&textColor);

	// Button color
	static ImVec4 buttonColor = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
	ImGui::Text("Button Color");

	ImGui::ColorEdit4("##Button Color", (float*)&buttonColor);

	// Button hover color
	static ImVec4 buttonHoverColor = ImVec4(0.1803921610116959f, 0.1882352977991104f, 0.196078434586525f, 1.0f);

	ImGui::Text("Button Hover Color");
	ImGui::ColorEdit4("##Button Hover Color", (float*)&buttonHoverColor);

	// Button active color
	static ImVec4 buttonActiveColor = ImVec4(0.1529411822557449f, 0.1529411822557449f, 0.1529411822557449f, 1.0f);
	ImGui::Text("Button Active Color");
	ImGui::ColorEdit4("##Button Active Color", (float*)&buttonActiveColor);

	// Frame color
	static ImVec4 frameColor = ImVec4(0.1137254908680916f, 0.125490203499794f, 0.1529411822557449f, 1.0f);
	ImGui::Text("Frame Color");
	ImGui::ColorEdit4("##Frame Color", (float*)&frameColor);

	// Frame hover color
	static ImVec4 frameHoverColor = ImVec4(0.1568627506494522f, 0.168627455830574f, 0.1921568661928177f, 1.0f);
	ImGui::Text("Frame Hover Color");
	ImGui::ColorEdit4("##Frame Hover Color", (float*)&frameHoverColor);

	// Frame active color
	static ImVec4 frameActiveColor = ImVec4(0.1568627506494522f, 0.168627455830574f, 0.1921568661928177f, 1.0f);
	ImGui::Text("Frame Active Color");
	ImGui::ColorEdit4("##Frame Active Color", (float*)&frameActiveColor);

	// Title background color
	static ImVec4 titleBgColor = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
	ImGui::Text("Title Background Color");
	ImGui::ColorEdit4("##Title Background Color", (float*)&titleBgColor);

	// Title background active color
	static ImVec4 titleBgActiveColor = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
	ImGui::Text("Title Background Active Color");
	ImGui::ColorEdit4("##Title Background Active Color", (float*)&titleBgActiveColor);

	// Title background collapsed color
	static ImVec4 titleBgCollapsedColor = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
	ImGui::Text("Title Background Collapsed Color");
	ImGui::ColorEdit4("##Title Background Collapsed Color", (float*)&titleBgCollapsedColor);

	// Menu bar background color
	static ImVec4 menuBarBgColor = ImVec4(0.09803921729326248f, 0.105882354080677f, 0.1215686276555061f, 1.0f);
	ImGui::Text("Menu Bar Background Color");
	ImGui::ColorEdit4("##Menu Bar Background Color", (float*)&menuBarBgColor);

	// Scrollbar background color
	static ImVec4 scrollbarBgColor = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
	ImGui::Text("Scrollbar Background Color");
	ImGui::ColorEdit4("##Scrollbar Background Color", (float*)&scrollbarBgColor);

	// Scrollbar grab color
	static ImVec4 scrollbarGrabColor = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
	ImGui::Text("Scrollbar Grab Color");
	ImGui::ColorEdit4("##Scrollbar Grab Color", (float*)&scrollbarGrabColor);

	// Scrollbar grab hovered color
	static ImVec4 scrollbarGrabHoveredColor = ImVec4(0.1568627506494522f, 0.168627455830574f, 0.1921568661928177f, 1.0f);
	ImGui::Text("Scrollbar Grab Hovered Color");
	ImGui::ColorEdit4("##Scrollbar Grab Hovered Color", (float*)&scrollbarGrabHoveredColor);

	// Scrollbar grab active color
	static ImVec4 scrollbarGrabActiveColor = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
	ImGui::Text("Scrollbar Grab Active Color");
	ImGui::ColorEdit4("##Scrollbar Grab Active Color", (float*)&scrollbarGrabActiveColor);

	// Checkmark color
	static ImVec4 checkMarkColor = ImVec4(0.9725490212440491f, 1.0f, 0.4980392158031464f, 1.0f);
	ImGui::Text("Checkmark Color");
	ImGui::ColorEdit4("##Checkmark Color", (float*)&checkMarkColor);

	// Slider grab color
	static ImVec4 sliderGrabColor = ImVec4(0.9725490212440491f, 1.0f, 0.4980392158031464f, 1.0f);
	ImGui::Text("Slider Grab Color");

	ImGui::ColorEdit4("##Slider Grab Color", (float*)&sliderGrabColor);

	// Slider grab active color
	static ImVec4 sliderGrabActiveColor = ImVec4(1.0f, 0.7960784435272217f, 0.4980392158031464f, 1.0f);
	ImGui::Text("Slider Grab Active Color");
	ImGui::ColorEdit4("##Slider Grab Active Color", (float*)&sliderGrabActiveColor);

	

	// appy button
	if (ImGui::Button("Apply", ImVec2(100, 30))) {
		ImGui::StyleColorsDark();
		ImGuiStyle& style = ImGui::GetStyle();
		style.Colors[ImGuiCol_Text] = textColor;
		style.Colors[ImGuiCol_WindowBg] = bgColor;
		style.Colors[ImGuiCol_Button] = buttonColor;
		style.Colors[ImGuiCol_ButtonHovered] = buttonHoverColor;
		style.Colors[ImGuiCol_ButtonActive] = buttonActiveColor;
		style.Colors[ImGuiCol_FrameBg] = frameColor;
		style.Colors[ImGuiCol_FrameBgHovered] = frameHoverColor;
		style.Colors[ImGuiCol_FrameBgActive] = frameActiveColor;
		style.Colors[ImGuiCol_TitleBg] = titleBgColor;
		style.Colors[ImGuiCol_TitleBgActive] = titleBgActiveColor;
		style.Colors[ImGuiCol_TitleBgCollapsed] = titleBgCollapsedColor;
		style.Colors[ImGuiCol_MenuBarBg] = menuBarBgColor;
		style.Colors[ImGuiCol_ScrollbarBg] = scrollbarBgColor;
		style.Colors[ImGuiCol_ScrollbarGrab] = scrollbarGrabColor;
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = scrollbarGrabHoveredColor;
		style.Colors[ImGuiCol_ScrollbarGrabActive] = scrollbarGrabActiveColor;
		style.Colors[ImGuiCol_CheckMark] = checkMarkColor;
		style.Colors[ImGuiCol_SliderGrab] = sliderGrabColor;
		style.Colors[ImGuiCol_SliderGrabActive] = sliderGrabActiveColor;
		style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.7372549176216125f, 0.6941176652908325f, 0.886274516582489f, 0.5490196347236633f);
	}

}

bool isFirstFile = true;
std::string statusMessage = "";

std::string DecompileFile(std::string path) noexcept {

	
	// directory of the current exe
	char currentDirectory[MAX_PATH];
	GetModuleFileNameA(NULL, currentDirectory, MAX_PATH);

	// remove the exe name from the path
	PathRemoveFileSpec(currentDirectory);

	std::string currentDirectoryStr = currentDirectory;


	std::string command = "java -jar .\\lib\\baksmali.jar d "+ path +" -o " + currentDirectoryStr + "\\output";

	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if (!CreateProcess(NULL, (LPSTR)command.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
		return "Failed to decompile " + GetLastError();
	}

	WaitForSingleObject(pi.hProcess, INFINITE);

	DWORD exitCode;
	GetExitCodeProcess(pi.hProcess, &exitCode);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);


	return "Decompiled successfully " + currentDirectoryStr + "\\output";
}



void drawDecompiler() noexcept {
	ImGui::Text("Decompiler");
	ImGui::NewLine();

	ImGui::Text("Select file to decompile");

	static char file[256] = "";
	ImGui::InputText("##File", file, IM_ARRAYSIZE(file), ImGuiInputTextFlags_ReadOnly);

	// Browse button
	if (ImGui::Button("Browse", ImVec2(100, 30))) {
		OPENFILENAME ofn;
		char szFile[260];
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = NULL;
		ofn.lpstrFile = szFile;
		ofn.lpstrFile[0] = '\0';
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = "Dex Files\0*.dex\0\0";
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
		if (GetOpenFileName(&ofn) == TRUE) {
			strcpy_s(file, ofn.lpstrFile);
		}
		isFirstFile = false;
	}
	if (isFirstFile)
		return;


	HANDLE fileHandle = CreateFile(file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fileHandle == INVALID_HANDLE_VALUE) {
		ImGui::Text("File not found");
		return;
	}
	

	ImGui::Text("File Details");
	// set window size to file path length 
	float windowSize = strlen(file) * 10;

	ImGui::BeginChild("File Details", ImVec2(windowSize, 100), true);
	ImGui::Text("File Name: %s", file);
	ImGui::Text("File Size: %d bytes", GetFileSize(fileHandle, NULL));
	ImGui::EndChild();


	// Decompile button
	if (ImGui::Button("Decompile", ImVec2(100, 30))) 
	{
		try {
			statusMessage = DecompileFile(file);
		}
		catch (const std::exception& e) {
			statusMessage =  e.what();
		}
	}

	ImGui::Text(statusMessage.c_str());





	ImGui::NewLine();

}

void gui::InitTextEditor() noexcept
{
	// Setup Editor
	Editor.SetLanguageDefinition(lang);

}


float lastTime = 0.0f;


static char file[256] = "";
void drawIde() noexcept {
	ImGui::Text("IDE");

	// Drag and drop file
	ImGui::Text("Select your file");
	ImGui::InputText("##File", file, IM_ARRAYSIZE(file), ImGuiInputTextFlags_ReadOnly);
	ImGui::SameLine();
	if (ImGui::Button("Browse", ImVec2(100, 30))) {

		OPENFILENAME ofn;
		char szFile[260];
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = NULL;
		ofn.lpstrFile = szFile;
		ofn.lpstrFile[0] = '\0';
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = "Smali Files\0*.smali\0\0";
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
		if (GetOpenFileName(&ofn) == TRUE) {
			strcpy_s(file, ofn.lpstrFile);
			std::ifstream t(file);
			std::string str((std::istreambuf_iterator<char>(t)),
				std::istreambuf_iterator<char>());
			gui::Editor.SetText(str);
			t.close();
		}
	}




	static char code[4096] = "";
	int width = ImGui::GetWindowSize().x - 50;

	gui::Editor.Render("##Code", ImVec2(width, 500), code);
	//ImGui::InputTextMultiline("##Code", code, IM_ARRAYSIZE(code), ImVec2(width, 500), ImGuiInputTextFlags_AllowTabInput);

	if (ImGui::Button("Save", ImVec2(90, 30))) {
		std::string code =  gui::Editor.GetText();
		std::ofstream out(file);
		out << code;
		out.close();
		lastTime = ImGui::GetTime();
	}
	if (ImGui::IsItemHovered())
	{
		ImGui::SetTooltip("Save the file Ctrl + S");
	}


	// Ctrl + S to save
	ImGuiIO& io = ImGui::GetIO();
	io.WantCaptureKeyboard = true;

	if (io.KeyCtrl && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_S))) {
		std::string code = gui::Editor.GetText();
		std::ofstream out(file);
		out << code;
		out.close();
		lastTime = ImGui::GetTime();
	}
	
	if (ImGui::IsKeyDown(ImGuiKey_RightCtrl) && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_S))) {
		std::string code = gui::Editor.GetText();
		std::ofstream out(file);
		out << code;
		out.close();
		lastTime = ImGui::GetTime();
	}

	ImGui::SameLine();
	if (ImGui::GetTime() - lastTime < 2.0f)
		ImGui::Text("Saved successfully");

	io.WantCaptureKeyboard = false;

}

bool isFullscreen = false;

void gui::Render() noexcept {
	
		
	ImGui::SetNextWindowPos(ImVec2(0, 0), 0);
	ImGui::SetNextWindowSize(ImVec2(WIDTH, HEIGHT), 0);
	ImGui::Begin("Decompiler", &exit, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
	DragAcceptFiles(window, FALSE);


	// add minimize and close buttons
	ImGui::SetCursorPos(ImVec2(WIDTH - 50, 10));
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
	if (ImGui::Button("##X", ImVec2(20, 20))) {
		gui::exit = false;
	}
	if (ImGui::IsItemHovered())
	{
		ImGui::SetTooltip("Exit");
	}
	ImGui::PopStyleColor();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
	ImGui::SetCursorPos(ImVec2(WIDTH - 80, 10));
	if (ImGui::Button("##_", ImVec2(20, 20))) {
	
		isFullscreen = !isFullscreen;
		
	}
	if (ImGui::IsItemHovered())
	{
		ImGui::SetTooltip(("Fullscreen (W.I.P) state: " + std::to_string(isFullscreen)).c_str());
	}
	ImGui::PopStyleColor();

	// Create tabs i.e Decompiler, IDE, Settings in center of the window
	ImGui::SetCursorPosX((ImGui::GetWindowSize().x - 350) / 2);
	if (ImGui::Button("Decompiler", ImVec2(120, 30))) {
		gui::tabs = 0;
	}
	ImGui::SameLine();
	if (ImGui::Button("IDE", ImVec2(120, 30))) {
		gui::tabs = 1;
	}
	ImGui::SameLine();
	if (ImGui::Button("Settings", ImVec2(120, 30))) {
		gui::tabs = 2;
	}

	// Render tabs
	switch (gui::tabs) {

	case 0:
		drawDecompiler();
		break;
	case 1:
		drawIde();
		break;
	case 2:
		drawSettings();
		break;

	}


	ImGui::End();
}
