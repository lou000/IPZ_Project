#include "testcameras.h"

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
            cv::flip(image, image, 0);
        }
        }
        );
    }
    auto size = App::getWindowSize();
    BatchRenderer::begin();
    BatchRenderer::drawQuad({0,0}, (vec2)size, texture);
    BatchRenderer::end();

}
