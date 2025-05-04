#include <vitasdk.h>
#include <vitaGL.h>
#include <imgui_vita.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cart.h"
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

void gui_emu_options() {
	char titlebar[256];
	sprintf(titlebar, "SolarGB v.%s - Emulator options", EMU_VERSION);
	ImGui_ImplVitaGL_NewFrame();
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiSetCond_Always);
	ImGui::SetNextWindowSize(ImVec2(HOST_SCREEN_W, HOST_SCREEN_H), ImGuiSetCond_Always);
	ImGui::Begin(titlebar, NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
	ImGui::Checkbox("Verbose CPU interpreter logging", (bool *)&emu.opts.debug_log);
	ImGui::Checkbox("Show PPU Vram content on screen", (bool *)&emu.opts.debug_ppu);
	ImGui::Checkbox("Frametime logging", (bool *)&emu.opts.frametime_log);
	ImGui::Checkbox("Enable I/O Serial Port logging", (bool *)&emu.opts.serial_port_enabled);
	ImGui::End();
	glViewport(0, 0, static_cast<int>(ImGui::GetIO().DisplaySize.x), static_cast<int>(ImGui::GetIO().DisplaySize.y));
	ImGui::Render();
	ImGui_ImplVitaGL_RenderDrawData(ImGui::GetDrawData());
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

void gui_pause_menu() {
	char titlebar[256];
	sprintf(titlebar, "SolarGB v.%s - %s", EMU_VERSION, rom.name);
	ImGui_ImplVitaGL_NewFrame();
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiSetCond_Always);
	ImGui::SetNextWindowSize(ImVec2(HOST_SCREEN_W, HOST_SCREEN_H), ImGuiSetCond_Always);
	ImGui::Begin(titlebar, NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
	if (ImGui::Button("Resume emulation", ImVec2(-1.0f, 0.0f))) {
		emu.state = EMU_RUNNING;
	}
	if (ImGui::Button("Close game", ImVec2(-1.0f, 0.0f))) {
		emu.state = EMU_NOT_RUNNING;
	}
	ImGui::End();
	glViewport(0, 0, static_cast<int>(ImGui::GetIO().DisplaySize.x), static_cast<int>(ImGui::GetIO().DisplaySize.y));
	ImGui::Render();
	ImGui_ImplVitaGL_RenderDrawData(ImGui::GetDrawData());
}
