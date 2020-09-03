#include "Texture.hpp"

// NOTE: NOT in header files!
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Texture::~Texture()
{
    glDeleteTextures(1, &texture);
}

int Texture::load_texture(std::string path)
{
    glGenTextures(1, &texture);
    use();

    int width = 0, height = 0;
    stbi_uc *earth_image = stbi_load(path.c_str(), &width, &height, 0, STBI_rgb);
    if (earth_image == NULL) {
        std::cerr << "Could not load " << path << std::endl;
        return LOAD_TEXTURE_FAILURE;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, earth_image);
    stbi_image_free(earth_image);

    // Wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Filtering
    // Use a mipmap (load texture in advance)
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);

    return LOAD_TEXTURE_SUCCESS;
}

void Texture::use() const
{
    glBindTexture(GL_TEXTURE_2D, texture);
}
