#include "ImGuiContext.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "Macros.h"

namespace CgEngine {

    void ImGuiContext::init(GLFWwindow* glfwWindow, float scale) {
        #ifdef CG_ENABLE_DEBUG_FEATURES
            IMGUI_CHECKVERSION();

            ImGui::CreateContext();

            ImGuiIO& io = ImGui::GetIO(); (void)io;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

            ImGui::StyleColorsDark();
            ImGui::GetStyle().ScaleAllSizes(scale);

            ImGui_ImplGlfw_InitForOpenGL(glfwWindow, true);
            ImGui_ImplOpenGL3_Init("#version 450 core");
        #endif
    }

    void ImGuiContext::shutdown() {
        #ifdef CG_ENABLE_DEBUG_FEATURES
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
        #endif
    }

    void ImGuiContext::newFrame() {
        #ifdef CG_ENABLE_DEBUG_FEATURES
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
        #endif
    }

    void ImGuiContext::render() {
        #ifdef CG_ENABLE_DEBUG_FEATURES
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        #endif
    }

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
