﻿#pragma once

#include "glad/glad.h"
#include "math.h"
#include "application.h"
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

#define START_TWEAK(name, show) \
if (ImGui::Begin(name, &show))  \
{                               \
ImGui::Columns(2, #name"cols"); \
ImGui::SetColumnWidth(0, 100);

#define TWEAK_FLOAT(label, v, ...) \
ImGui::TextUnformatted(label); ImGui::NextColumn(); ImGui::DragFloat("##"#v, &v, __VA_ARGS__); ImGui::NextColumn();

#define TWEAK_INT(label, v, ...) \
ImGui::TextUnformatted(label); ImGui::NextColumn(); ImGui::DragInt("##"#v, &v, __VA_ARGS__); ImGui::NextColumn();

#define TWEAK_BOOL(label, v) \
ImGui::TextUnformatted(label); ImGui::NextColumn(); ImGui::Checkbox("##"#v, &v); ImGui::NextColumn();

#define TWEAK_VEC3(label, v, ...) \
ImGui::TextUnformatted(label); ImGui::NextColumn(); ImGui::DragFloat3("##"#v, glm::value_ptr(v), __VA_ARGS__); ImGui::NextColumn();

#define TWEAK_COLOR3(label, v, ...) \
ImGui::TextUnformatted(label); ImGui::NextColumn(); ImGui::ColorPicker3("##"#v, glm::value_ptr(v), __VA_ARGS__); ImGui::NextColumn();

#define STOP_TWEAK()\
ImGui::Columns();\
}                   \
ImGui::End();

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
