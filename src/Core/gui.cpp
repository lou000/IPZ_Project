#include "gui.h"

GuiFonts guiFonts;

bool __dragFloat(const char* label, float* v, float v_speed, float v_min,
                 float v_max, const char* format, ImGuiSliderFlags flags)
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

bool __dragInt(const char *label, int *v, float v_speed, int v_min, int v_max, const char *format, ImGuiSliderFlags flags)
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

bool __checkbox(const char *label, bool *v)
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

bool __dragFloat3(const char *label, float v[], float v_speed, float v_min, float v_max, const char *format, ImGuiSliderFlags flags)
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

bool __colorPicker3(const char *label, float col[], ImGuiColorEditFlags flags)
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

void imguiInit()
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    guiFonts.regular   = io.Fonts->AddFontFromFileTTF("../Assets/fonts/Hack-Regular.ttf", 13);
    guiFonts.large     = io.Fonts->AddFontFromFileTTF("../Assets/fonts/Hack-Regular.ttf", 18);
    guiFonts.largeBold = io.Fonts->AddFontFromFileTTF("../Assets/fonts/Hack-Bold.ttf", 18);
    //    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
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

void imguiBegin()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void imguiEnd()
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
