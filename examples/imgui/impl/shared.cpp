#include "shared.h"

#include <format>
#include <imgui.h>

void impl::showExampleWindow(std::string_view comment) {
	const auto title = std::format("Kiero Dear ImGui Example ({})", comment);

	ImGui::Begin(title.c_str());
	{
		ImGui::Text("Hello");
		ImGui::Button("World!");
	}
	ImGui::End();
}
