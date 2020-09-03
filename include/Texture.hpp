#pragma once

#include <iostream>
#include <string>

#include <glad/glad.h>

static const int LOAD_TEXTURE_SUCCESS = 0;
static const int LOAD_TEXTURE_FAILURE = 1;

class Texture {
private:
    GLuint texture = 0;
public:
    Texture() = default;
    ~Texture();

    // Calls use()
    int load_texture(std::string path);
    void use() const;
};
