#pragma once

#include <d3d9.h>
#include <string>
#include "..\TextEditor\TextEditor.h"


namespace gui
{
	constexpr int WIDTH = 1000;
	constexpr int HEIGHT = 700;

	inline int windowWidth = 0;
	inline int windowHeight = 0;

	inline int screenWidth = 0;
	inline int screenHeight = 0;

	inline std::string selectedPath = "";

	inline auto selectedFile = std::string{ "" };

	inline int tabs = 0;

	inline int themeSelected = 0;

	inline bool exit = true;

	inline HWND window = nullptr;
	inline WNDCLASSEX windowClass = { 0 };

	inline POINTS position = { };

	inline PDIRECT3D9 d3d = nullptr;
	inline PDIRECT3DDEVICE9 device = nullptr;
	inline D3DPRESENT_PARAMETERS parameters = { };

	void createHWindow(
		const char* windowName,
		const char* className
	) noexcept;

	inline TextEditor Editor = { };
	inline auto lang = TextEditor::LanguageDefinition::Smali();
	
	void SetupImGuiStyle() noexcept;
	void InitTextEditor() noexcept;

	void DestroyWindow() noexcept;

	bool createDevice() noexcept;
	void resetDevice() noexcept;
	void destroyDevice() noexcept;

	void createImgui() noexcept;
	void destroyImgui() noexcept;

	void BeginRender() noexcept;
	void EndRender() noexcept;
	void Render() noexcept;
}