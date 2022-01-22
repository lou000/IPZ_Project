#pragma once

#include "glad/glad.h"
#include "math.h"
#include "application.h"
#include "../AssetManagement/asset_manager.h"
#define IM_VEC2_CLASS_EXTRA                                                 \
ImVec2(const glm::vec2& f) { x = f.x; y = f.y; }                       \
operator glm::vec2() const { return glm::vec2(x,y); }

#define IM_VEC4_CLASS_EXTRA                                                 \
ImVec4(const glm::vec4& f) { x = f.x; y = f.y; z = f.z; w = f.w; }     \
operator glm::vec4() const { return glm::vec4(x,y,z,w); }
#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imgui.h"
#include <thread>
#include <string>
#include <unordered_map>


inline bool __dragFloat(const char* label, float* v, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0)
{
    ImGui::Columns(2, label);
    ImGui::SetColumnWidth(0, 100);
    ImGui::TextUnformatted(label);
    ImGui::NextColumn();
    ImGui::PushItemWidth(-1);
    std::string str = "##"+std::string(label);
    bool changed = false;
    changed = ImGui::DragFloat(str.c_str(), v, v_speed, v_min, v_max, format, flags);
    ImGui::PopItemWidth();
    ImGui::Columns();
    return changed;
}
#define TWEAK_FLOAT(label, v, ...) __dragFloat(label, &v, __VA_ARGS__);

inline bool __dragInt(const char* label, int* v, float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", ImGuiSliderFlags flags = 0)
{
    ImGui::Columns(2, label);
    ImGui::SetColumnWidth(0, 100);
    ImGui::TextUnformatted(label);
    ImGui::NextColumn();
    ImGui::PushItemWidth(-1);
    std::string str = "##"+std::string(label);
    bool changed = false;
    changed = ImGui::DragInt(str.c_str(), v, v_speed, v_min, v_max, format, flags);
    ImGui::PopItemWidth();
    ImGui::Columns();
    return changed;
}
#define TWEAK_INT(label, v, ...) __dragInt(label, &v, __VA_ARGS__);

inline bool __checkbox(const char* label, bool* v)
{
    ImGui::Columns(2, label);
    ImGui::SetColumnWidth(0, 100);
    ImGui::TextUnformatted(label);
    ImGui::NextColumn();
    ImGui::PushItemWidth(-1);
    std::string str = "##"+std::string(label);
    bool changed = false;
    changed = ImGui::Checkbox(str.c_str(), v);
    ImGui::PopItemWidth();
    ImGui::Columns();
    return changed;
}
#define TWEAK_BOOL(label, v) __checkbox(label, &v);

inline bool __dragFloat3(const char* label, float v[3], float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0)
{
    ImGui::Columns(2, label);
    ImGui::SetColumnWidth(0, 100);
    ImGui::TextUnformatted(label);
    ImGui::NextColumn();
    ImGui::PushItemWidth(-1);
    std::string str = "##"+std::string(label);
    bool changed = false;
    changed = ImGui::DragFloat3(str.c_str(), v, v_speed, v_min, v_max, format, flags);
    ImGui::PopItemWidth();
    ImGui::Columns();
    return changed;
}
#define TWEAK_VEC3(label, v, ...) __dragFloat3(label, glm::value_ptr(v), __VA_ARGS__);

inline bool __colorPicker3(const char* label, float col[3], ImGuiColorEditFlags flags = 0)
{
    ImGui::Columns(2, label);
    ImGui::SetColumnWidth(0, 100);
    ImGui::TextUnformatted(label);
    ImGui::NextColumn();
    ImGui::PushItemWidth(-1);
    std::string str = "##"+std::string(label);
    bool changed = false;
    changed = ImGui::ColorPicker3(str.c_str(), col, flags);
    ImGui::PopItemWidth();
    ImGui::Columns();
    return changed;
}
#define TWEAK_COLOR3(label, v, ...) __colorPicker3(label, glm::value_ptr(v), __VA_ARGS__);

inline void imguiInit()
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
//    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
//    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
//    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

    ImGui::StyleColorsDark();
    auto& style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    style.ItemSpacing = vec2(5.f, 10.f);
    style.WindowPadding = vec2(10.f, 10.f);

    auto window = App::getWindowHandle();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410");
}

inline void imguiBegin()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

inline void imguiEnd()
{
    ImGuiIO& io = ImGui::GetIO();
    auto winSize = (vec2)App::getWindowSize();
    io.DisplaySize = winSize;

    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

//    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
//    {
//        GLFWwindow* backup_current_context = glfwGetCurrentContext();
//        ImGui::UpdatePlatformWindows();
//        ImGui::RenderPlatformWindowsDefault();
//        glfwMakeContextCurrent(backup_current_context);
//    }
}
