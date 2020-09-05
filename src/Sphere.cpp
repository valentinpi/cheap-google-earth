#include "Sphere.hpp"

Sphere::Sphere(glm::vec3 center, float radius, uint64_t stack_count, uint64_t sector_count)
    : center(center), radius(radius), stack_count(stack_count), sector_count(sector_count)
{
#if DEBUG
    std::cout << this->radius << std::endl;
#endif
    generate();
    generate_gl();
}

Sphere::~Sphere()
{
    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(2, vbo);
}

void Sphere::generate()
{
    if (stack_count < SPHERE_MINIMUM_STACK_COUNT || sector_count < SPHERE_MINIMUM_SECTOR_COUNT) {
        initialized = false;
        return;
    }

    vertices.clear();
    indices.clear();
    texcoords.clear();

    // Top and bottom plus the sector_count vertices of each sector. Excluding top, bottom
    size_t vertex_count = (stack_count + 1) * (sector_count + 1);
    // Subtract one sector of rects (each two triangles)
    size_t index_count = stack_count * (2 * 3 * (sector_count + 1));
    vertices.reserve(vertex_count * 3);
    indices.reserve(index_count);
    texcoords.reserve(vertex_count * 2);
    std::cout << "vertex_count: " << vertex_count << std::endl;
    std::cout << "index_count:  " << index_count << std::endl;
    
    // Push top vertices with same position but different texcoords
    // Correcting the texcoord glitches from the previous build
    {
        // stack_step = 0
        glm::vec3 top = center + glm::vec3(0.0f, 0.0f, radius);

        for (uint64_t sector_step = 0; sector_step <= sector_count; sector_step++) {;
            vertices.insert(vertices.end(), { top.x, top.y, top.z });
            texcoords.insert(texcoords.end(), {
                (float) sector_step / (float) sector_count,
                0.0f
            });
        }
    }
    
#if DEBUG
    std::cout << "===== After initializing top vertex =====" << std::endl;
    log_coords();
#endif

    for (uint64_t stack_step = 1; stack_step <= stack_count; stack_step++) {
        /*
            We always consider the two triangles to create per vertex to have a
            rectangle. The top coordinates, not including the vertex created,
            we will call their indices top indices and the bottom indices
            per sector will be called bottom indices, which include the vertex
            added. We cycle through these indices to create the triangles and
            have to keep track of the last index.
        */
        float phi = PI / 2.0f - PI * ((float) stack_step / (float) stack_count);
        // To connect the last vertex with the initial one
        GLuint initial_bottom_index = vertices.size() / 3;
        GLuint prev_bottom_index = initial_bottom_index + (GLuint) sector_count - 2;
        GLuint next_top_index = initial_bottom_index - (GLuint) sector_count;

        for (uint64_t sector_step = 0; sector_step <= sector_count; sector_step++) {
            float theta = 2.0f * PI * ((float) sector_step / (float) sector_count);
            float x = radius * std::cos(phi) * std::cos(theta);
            float y = radius * std::cos(phi) * std::sin(theta);
            float z = radius * std::sin(phi);

            glm::vec3 vertex = center + glm::vec3(x, y, z);
            vertices.insert(vertices.end(), { vertex.x, vertex.y, vertex.z });
            
            texcoords.insert(texcoords.end(), {
                (float) sector_step / (float) sector_count,
                (float) stack_step / (float) stack_count
            });

            // This is the last iteration for the sectors, so ignore the triangle next to the vertex
            // since it has already been added in the initial, first iteration. Fixes the final Sphere
            // glitch.
            GLuint bottom_index = vertices.size() / 3 - 1;
            GLuint top_index = bottom_index - (GLuint) sector_count - 1;
            if (bottom_index == initial_bottom_index) {
                indices.insert(indices.end(), {
                    bottom_index,
                    top_index,
                    next_top_index,
                });
                next_top_index++;
            }
            else if (next_top_index == initial_bottom_index) {
                indices.insert(indices.end(), {
                    prev_bottom_index,
                    top_index,
                    bottom_index,
                });
                next_top_index = initial_bottom_index - (GLuint) sector_count;
            }
            else {
                // Per vertex add two triangles
                // One left "above"
                // One right "above"
                // of the vertex
                indices.insert(indices.end(), {
                    prev_bottom_index,
                    top_index,
                    bottom_index,
                    bottom_index,
                    top_index,
                    next_top_index,
                });
                next_top_index++;
            }
            prev_bottom_index = bottom_index;
        }
    }

#if DEBUG
    std::cout << "===== After initializing remaining vertices =====" << std::endl;
    log_coords();
#endif

    initialized = true;
}

void Sphere::generate_gl() {
    if (!initialized) {
        return;
    }
    
    // Data pointers
    const float *vertices_ptr = vertices.data();
    GLsizeiptr vertices_size = vertices.size() * sizeof(float);

    const GLuint *indices_ptr = indices.data();
    GLsizeiptr indices_size = indices.size() * sizeof(GLuint);
    
    const float *texcoords_ptr = texcoords.data();
    GLsizeiptr texcoords_size = texcoords.size() * sizeof(float);

    glGenBuffers(2, vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, vertices_size, vertices_ptr, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, texcoords_size, texcoords_ptr, GL_STATIC_DRAW);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_size, indices_ptr, GL_STATIC_DRAW);
}

void Sphere::draw() const
{
    GLsizeiptr indices_size = indices.size() * sizeof(GLuint);
    
    glDrawElements(GL_TRIANGLES, indices_size, GL_UNSIGNED_INT, nullptr);
}

void Sphere::log_coords() const
{
    std::cout << "vertices" << std::endl;
    for (size_t i = 0; i < vertices.size(); i += 3) {
        std::cout << "("
            << vertices[i  ] << ", "
            << vertices[i+1] << ", "
            << vertices[i+2] << ")" << std::endl;
    }
    std::cout << "indices" << std::endl;
    for (size_t i = 0; i < indices.size(); i += 3) {
        std::cout << "("
            << indices[i  ] << ", "
            << indices[i+1] << ", "
            << indices[i+2] << ")" << std::endl;
    }
    std::cout << "texcoords" << std::endl;
    for (size_t i = 0; i < texcoords.size(); i += 2) {
        std::cout << "("
            << texcoords[i  ] << ", "
            << texcoords[i+1] << ")" << std::endl;
    }
}
