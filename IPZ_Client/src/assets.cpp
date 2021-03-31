#include "assets.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Sprite::Sprite(const std::filesystem::path& path)
{
    loadFromFile(path);
    this->path = path;
}

Sprite::~Sprite()
{
    if(data)
        delete[] data;
}

void Sprite::doReload()
{
    if(loadFromFile(path))
    {
        std::cout<<"Sprite "<<path<<" reloaded.\n";
        rld = false;
    }
}

bool Sprite::loadFromFile(const std::filesystem::path& path){

    //TODO: add error logging
    int w = 0, h = 0, ch = 0;
    stbi_set_flip_vertically_on_load(1);

    //Check if the file is still there
    if(!std::filesystem::exists(path))
        return false;

    //Try to load file
    // we should log this? it might happen often
    auto temp = stbi_load(path.string().c_str(), &w, &h, &ch, 4);
    if(!temp)
        return false;

    //Delete old data
    if(data != nullptr){
        delete[] data;
        data = nullptr;
    }

    width = w;
    height = h;

    data = new u8vec4[w * h];
    std::memcpy(data, temp, w * h * 4);
    stbi_image_free(temp);
    return true;
}
