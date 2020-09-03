#pragma once

#include <iostream>
#include <vector>

// For GLuint
#include <GL/gl.h>
#include <glm/glm.hpp>

#include "Constants.hpp"

static const double PI = 3.14159265358979323846264338327950288;
static const uint64_t SPHERE_MINIMUM_STACK_COUNT = 2;
static const uint64_t SPHERE_MINIMUM_SECTOR_COUNT = 3;

/*
    Creates a sphere of any radius.
    Notations according to https://www.songho.ca/opengl/gl_sphere.html
*/
class Sphere
{
private:
    glm::vec3 center;
    float radius;
    uint64_t stack_count;
    uint64_t sector_count;
    std::vector<float> vertices;
    // NOTE: You can get the number of triangles
    // to draw by dividing this size() by 3
    std::vector<GLuint> indices;
    // Not really part of this class in concept
    // But maybe it is since it is independent
    // of the actual texture used (assuming 2D)
    std::vector<float> texcoords;

    void generate();
public:
    Sphere(glm::vec3 center, float radius, uint64_t stack_count, uint64_t sector_count);
    ~Sphere() = default;

    const std::vector<float> &get_vertices() const {
        return vertices;
    }
    
    const std::vector<GLuint> &get_indices() const {
        return indices;
    }

    const std::vector<float> &get_texcoords() const {
        return texcoords;
    }

    void log_coords() const;
};
