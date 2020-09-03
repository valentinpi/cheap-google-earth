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
    size_t vertex_count = 2 + (stack_count - 1) * sector_count;
    // Subtract one sector of rects (each two triangles)
    size_t index_count = (stack_count - 1) * (2 * 3 * sector_count);
    vertices.reserve(vertex_count * 3);
    indices.reserve(index_count);
    texcoords.reserve(vertex_count * 2);
    std::cout << "vertex_count: " << vertex_count << std::endl;
    std::cout << "index_count:  " << index_count << std::endl;
    
    // Push top vertex and generate first row and connect the triangles. Generate texcoords in the process
    {
        glm::vec3 top = center + glm::vec3(0.0f, 0.0f, radius);
        vertices.insert(vertices.end(), { top.x, top.y, top.z });
        texcoords.insert(texcoords.end(), { 0.0f, 0.0f });

        const GLuint top_index = 0;
        // sector_count corresponds to the last added vertex
        GLuint prev_index = sector_count;
        float phi = PI / 2.0f - PI * (1.0f / (float) stack_count);
        
        for (uint64_t sector_step = 0; sector_step < sector_count; sector_step++) {;
            float theta = 2.0f * PI * ((float) sector_step / (float) sector_count);
            float x = radius * std::cos(phi) * std::cos(theta);
            float y = radius * std::cos(phi) * std::sin(theta);
            float z = radius * std::sin(phi);
            
            glm::vec3 vertex = center + glm::vec3(x, y, z);
            vertices.insert(vertices.end(), { vertex.x, vertex.y, vertex.z });

            // TODO: Possible problem when rendering texture: No end
            // vertex with the texcoords 1.0, x
            // Reworking this might require to increase the reserved
            // memory in the vectors
            texcoords.insert(texcoords.end(), {
                (float) sector_step / (float) sector_count,
                1.0f / (float) stack_count
            });
            
            // The vector we just added
            GLuint index = vertices.size() / 3 - 1;
            indices.insert(indices.end(), { top_index, prev_index, index });
            prev_index = index;
        }
    }
    
#if DEBUG
    std::cout << "===== After initializing top vertex =====" << std::endl;
    log_coords();
#endif

    // Start generating second row and generate rectangles by indexing two triangles each
    // Does not necessarily run, for instance, when the stack_count is 2, we never generate
    // any rectangles.
    for (uint64_t stack_step = 2; stack_step < stack_count; stack_step++) {
        /*
            We always consider the two triangles to create per vertex to have a
            rectangle. The top coordinates, not including the vertex created,
            we will call their indices top indices and the bottom indices
            per sector will be called bottom indices, which include the vertex
            added. We cycle through these indices to create the triangles and
            have to keep track of the last index.
        */
        float phi = PI / 2.0f - PI * ( (float) stack_step / (float) stack_count);
        GLuint prev_bottom_index = vertices.size() / 3 - 1 + sector_count;
        GLuint initial_bottom_index = vertices.size() / 3;
        GLuint next_top_index = vertices.size() / 3 - sector_count + 1;
        for (uint64_t sector_step = 0; sector_step < sector_count; sector_step++) {
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

            // Per vertex add two triangles
            // One left "above"
            // One right "above"
            // of the vertex
            GLuint bottom_index = vertices.size() / 3 - 1;
            indices.insert(indices.end(), {
                prev_bottom_index,
                bottom_index - (GLuint) sector_count,
                bottom_index,
                bottom_index,
                bottom_index - (GLuint) sector_count,
                next_top_index,
            });
            prev_bottom_index = bottom_index;
            next_top_index++;

            if (next_top_index == initial_bottom_index) {
                next_top_index = initial_bottom_index - (GLuint) sector_count;
            }
        }
    }

#if DEBUG
    std::cout << "===== After initializing vertex body =====" << std::endl;
    log_coords();
#endif

    // Push bottom vertex and connect the last triangles
    {
        glm::vec3 bottom = center + glm::vec3(0.0f, 0.0f, -radius);
        vertices.insert(vertices.end(), { bottom.x, bottom.y, bottom.z });
        texcoords.insert(texcoords.end(), { 0.0f, 1.0f });

        GLuint bottom_index = vertices.size() / 3 - 1;
        GLuint last_sectors_start = bottom_index - sector_count;
        GLuint prev_index = last_sectors_start + sector_count - 1;
        for (uint64_t sector_step = 0; sector_step < sector_count; sector_step++) {
            GLuint next_index = last_sectors_start + (GLuint) sector_step;
            indices.insert(indices.end(), {
                prev_index,
                next_index,
                bottom_index,
            });
            prev_index = next_index;
        }
    }

#if DEBUG
    std::cout << "===== After initializing bottom vertex =====" << std::endl;
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
