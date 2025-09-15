#include "ImGuiContext.h"
#include "imgui.h"
#include "Macros.h"

namespace CgEngine {
    bool ImGuiContext::wantCaptureMouse() {
        #ifdef CG_ENABLE_DEBUG_FEATURES
            ImGuiIO& io = ImGui::GetIO();
            return io.WantCaptureMouse;
        #else
            return false;
        #endif
    }

    bool ImGuiContext::wantCaptureKeyboard() {
        #ifdef CG_ENABLE_DEBUG_FEATURES
            ImGuiIO& io = ImGui::GetIO();
            return io.WantCaptureKeyboard;
        #else
            return false;
        #endif
    }
}
