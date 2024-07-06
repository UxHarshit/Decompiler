#include "gui.h"
#include <thread>

int __stdcall wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
	gui::createHWindow("Decompiler", "decompiler class");
	gui::createDevice();
	gui::createImgui();
	gui::SetupImGuiStyle();


	while (gui::exit) {
		gui::BeginRender();
		gui::Render();
		gui::EndRender();

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	gui::destroyImgui();
	gui::destroyDevice();
	gui::DestroyWindow();

	return EXIT_SUCCESS;
}