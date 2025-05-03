#include <vitasdk.h>
#include <vitaGL.h>
#include <imgui_vita.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "emu.h"
#include "gui.h"

void gui_init() {
	vglInitExtended(0, HOST_SCREEN_W, HOST_SCREEN_H, 8 * 1024 * 1024, SCE_GXM_MULTISAMPLE_NONE);
	ImGui::CreateContext();
	ImGui_ImplVitaGL_Init_Extended();
	ImGui::GetIO().MouseDrawCursor = false;
	ImGui_ImplVitaGL_TouchUsage(false);
	ImGui_ImplVitaGL_GamepadUsage(true);
	ImGui_ImplVitaGL_MouseStickUsage(false);
	sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_STOP);
}

rom_t *gui_rom_selector() {
	rom_t *ret = NULL;
	char titlebar[256];
	sprintf(titlebar, "SolarGB v.%s - Rom selector", EMU_VERSION);
	ImGui_ImplVitaGL_NewFrame();
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiSetCond_Always);
	ImGui::SetNextWindowSize(ImVec2(HOST_SCREEN_W, HOST_SCREEN_H), ImGuiSetCond_Always);
	ImGui::Begin(titlebar, NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
	rom_t *r = roms;
	while (r) {
		if (ImGui::Button(r->name, ImVec2(-1.0f, 0.0f))) {
			ret = r;
		}
		r = r->next;
	}
	ImGui::End();
	glViewport(0, 0, static_cast<int>(ImGui::GetIO().DisplaySize.x), static_cast<int>(ImGui::GetIO().DisplaySize.y));
	ImGui::Render();
	ImGui_ImplVitaGL_RenderDrawData(ImGui::GetDrawData());
	return ret;
}