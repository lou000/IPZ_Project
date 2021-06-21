#include "testcameras.h"
#include "imgui.h"

TestCameras::TestCameras()
{

}

void TestCameras::onStart()
{
    if(!vcap.open("rtsp://172.28.1.151/axis-media/media.amp?resolution=1280x720&videocodec=h264&framerate=20"))
        WARN("Couldnt open video stream!");

    texture = std::make_shared<Texture>(1280, 720, GL_RGB8, 1, true);
}

void TestCameras::onUpdate(float dt)
{
    if(pool.get_tasks_total() == 0)
    {
        if(image.rows>0)
            texture->setTextureData(image.data, image.cols*image.rows*sizeof (GLubyte)*3);
        pool.push_task([&](){
        if(vcap.read(image))
        {
            cvtColor(image, image, cv::COLOR_BGR2RGB);
        }
        }
        );
    }
    auto size = App::getWindowSize();

    static bool dockspaceOpen = true;
    static bool opt_fullscreen_persistant = true;
    bool opt_fullscreen = opt_fullscreen_persistant;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen)
    {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
    ImGui::PopStyleVar();
    ImGui::Image((void*)(intptr_t)texture->id(), ImVec2(image.cols, image.rows));
}
