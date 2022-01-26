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

struct GuiFonts
{
    ImFont* regular = nullptr;
    ImFont* large = nullptr;
    ImFont* largeBold = nullptr;
};

extern GuiFonts guiFonts;

bool __dragFloat(const char* label, float* v, float v_speed = 1.0f,
                        float v_min = 0.0f, float v_max = 0.0f,
                        const char* format = "%.3f", ImGuiSliderFlags flags = 0);
#define TWEAK_FLOAT(label, v, ...) __dragFloat(label, &v, __VA_ARGS__);

bool __dragInt(const char* label, int* v, float v_speed = 1.0f, int v_min = 0,
               int v_max = 0, const char* format = "%d", ImGuiSliderFlags flags = 0);
#define TWEAK_INT(label, v, ...) __dragInt(label, &v, __VA_ARGS__);

bool __checkbox(const char* label, bool* v);
#define TWEAK_BOOL(label, v) __checkbox(label, &v);

bool __dragFloat3(const char* label, float v[3], float v_speed = 1.0f,
                  float v_min = 0.0f, float v_max = 0.0f,
                  const char* format = "%.3f", ImGuiSliderFlags flags = 0);
#define TWEAK_VEC3(label, v, ...) __dragFloat3(label, glm::value_ptr(v), __VA_ARGS__);

bool __colorPicker3(const char* label, float col[3], ImGuiColorEditFlags flags = 0);
#define TWEAK_COLOR3(label, v, ...) __colorPicker3(label, glm::value_ptr(v), __VA_ARGS__);

void imguiInit();
void imguiBegin();
void imguiEnd();
