#include <chrono>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <glad/glad.h>
// NOTE: Contains static initializers that, when GLFW is
// not used in code, causes a segmentation fault on exit
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Constants.hpp"
#include "Shader.hpp"
#include "Sphere.hpp"
#include "Texture.hpp"

using std::chrono::high_resolution_clock;

#define CLEAR_COLOR 0.0f, 0.0f, 0.0f, 0.0f

enum axis {
    X,
    Y,
    Z
};

static const float FOV = 90.0f;
static const float ROTATION_SPEED = 5.0f;
// The LOWER the FASTER
static const float SCROLL_SPEED = 5.0f;
static const float SPHERE_RADIUS = 1.0f;
static const std::string EARTH_TEXTURE_SRC = "img/earth_960.jpg";

GLFWwindow *window = nullptr;
const int window_width = 800, window_height = 800;
axis rotation_mode = X;
bool mouse_pressed = false;
double xpos_prev = 0, ypos_prev = 0;
double dx = 0, dy = 0;

float pos_x = 2.0f;
float scroll = 0.0f;

GLuint vbo[2] = { 0, 0 };
GLuint ebo = 0;
GLuint program = 0;
GLuint vao = 0;

/* CALLBACKS */

static void cursor_pos_callback(GLFWwindow *window, double xpos, double ypos)
{
    (void) window;

    dx = xpos - xpos_prev;
    dy = ypos - ypos_prev;
    xpos_prev = xpos;
    ypos_prev = ypos;
}

static void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    (void) window;

    glViewport(0, 0, width, height);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void) window;
    (void) scancode;
    (void) mods;

    // The US keyboard layout is used for the symbolic constants
    // https://www.glfw.org/docs/latest/group__keys.html
    // Therefore use the keys 1, 2, 3 for a little portability
    // since Y and Z are switched on German keyboards for instance
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_1:
                rotation_mode = X;
                break;
            case GLFW_KEY_2:
                rotation_mode = Y;
                break;
            case GLFW_KEY_3:
                rotation_mode = Z;
        }
    }
}

static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    (void) window;
    (void) mods;

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        mouse_pressed = (action == GLFW_PRESS);
    }
}

static void scroll_callback(GLFWwindow *window, double xOffset, double yOffset) {
    (void) window;
    (void) xOffset;

    scroll += yOffset / SCROLL_SPEED;
}

/* MAIN */

int main()
{
    if (!glfwInit()) {
        const char *error = nullptr;
        glfwGetError(&error);
        std::cerr << "GLFW initialization failed!\n" << error << std::endl;
        
        return EXIT_FAILURE;
    }

    window = glfwCreateWindow(
        window_width,
        window_height,
        "Cheap Google Earth",
        nullptr,
        nullptr);
    if (window == NULL) {
        const char *error = nullptr;
        glfwGetError(&error);
        std::cerr << "GLFW window creation failed!\n" << error << std::endl;

        return EXIT_FAILURE;
    }
    glfwSetWindowSizeLimits(window, 200, 200, 2000, 2000);

    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    
    glfwMakeContextCurrent(window);

    if (!gladLoadGL()) {
        std::cerr << "glad initialization failed!" << std::endl;
        return EXIT_FAILURE;
    }

    glEnable(GL_DEPTH_TEST);

    // Create sphere for vertex coordinates, indices and texcoords
    Sphere sphere(glm::vec3(0.0f), SPHERE_RADIUS, 20, 20);

    const float *vertices = sphere.get_vertices().data();
    GLsizeiptr vertices_size = sphere.get_vertices().size() * sizeof(float);

    const GLuint *indices = sphere.get_indices().data();
    GLsizeiptr indices_size = sphere.get_indices().size() * sizeof(GLuint);
    
    const float *texcoords = sphere.get_texcoords().data();
    GLsizeiptr texcoords_size = sphere.get_texcoords().size() * sizeof(float);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(2, vbo);

    // Vertex data
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, vertices_size, vertices, GL_STATIC_DRAW);

    // Texcoords
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, texcoords_size, texcoords, GL_STATIC_DRAW);

    // Index data
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_size, indices, GL_STATIC_DRAW);

    // Load and compile shaders and shader program
    Shader vertex_shader(GL_VERTEX_SHADER), fragment_shader(GL_FRAGMENT_SHADER);
    if (vertex_shader.load_shader("shaders/vertex_shader.vert") != LOAD_SHADER_SUCCESS) {
        return EXIT_FAILURE;
    }
    if (fragment_shader.load_shader("shaders/fragment_shader.frag") != LOAD_SHADER_SUCCESS) {
        return EXIT_FAILURE;
    }

    program = glCreateProgram();
    glAttachShader(program, vertex_shader.get_id());
    glAttachShader(program, fragment_shader.get_id());
    glLinkProgram(program);

    {
        GLint success = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            GLint len = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
        
            char *log_ptr = (char*) calloc(len, 1);
            std::unique_ptr<char> log(log_ptr);
            glGetProgramInfoLog(program, len, nullptr, log.get());

            std::cerr << "Program linking failed!\n" << log.get() << std::endl;

            return EXIT_FAILURE;
        }
    }

    glUseProgram(program);

    // Now that we can get the attribute locations, set the data link to the arrays given
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    GLint pos_attr = glGetAttribLocation(program, "pos_attr");
    glVertexAttribPointer(pos_attr, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(pos_attr);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    GLint tex_attr = glGetAttribLocation(program, "tex_attr");
    // NOTE: Fails for some reason (probably optimization by the GPU driver),
    // if texcoords are not used in shaders
    glVertexAttribPointer(tex_attr, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glEnableVertexAttribArray(tex_attr);

    // Projections
    GLint model = glGetUniformLocation(program, "model");
    GLint view = glGetUniformLocation(program, "view");
    GLint proj = glGetUniformLocation(program, "proj");
    
    // Because of the texture, rotate the sphere around one time
    glm::mat4 model_transform = glm::rotate(
        glm::mat4(1.0f),
        glm::radians(180.0f),
        glm::vec3(0.0f, 0.0f, 1.0f)
    );
    glm::mat4 view_transform = glm::lookAt(
        glm::vec3(pos_x, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f)
    );
    glm::mat4 proj_transform = glm::perspective(
        glm::radians(FOV),
        (float) window_width / (float) window_height,
        0.1f,
        1000.0f
    );

    glUniformMatrix4fv(model, 1, GL_FALSE, glm::value_ptr(model_transform));
    glUniformMatrix4fv(view, 1, GL_FALSE, glm::value_ptr(view_transform));
    glUniformMatrix4fv(proj, 1, GL_FALSE, glm::value_ptr(proj_transform));

    // Load texture
    Texture earth_texture;
    if (earth_texture.load_texture(EARTH_TEXTURE_SRC) != LOAD_TEXTURE_SUCCESS) {
        return EXIT_FAILURE;
    }

    glfwGetCursorPos(window, &xpos_prev, &ypos_prev);
    auto begin = high_resolution_clock::now();
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if (mouse_pressed && (dx != 0 || dy != 0)) {
            float dir = 0.0f;
            if (std::abs(dx) > std::abs(dy)) {
                dir = dx;
            }
            else {
                dir = dy;
            }

            glm::vec3 rot(0.0f);
            switch (rotation_mode) {
                case X:
                    rot.x = dir;
                    break;
                case Y:
                    rot.y = dir;
                    break;
                case Z:
                    rot.z = dir;
            }
            model_transform = glm::rotate(model_transform, glm::radians(ROTATION_SPEED), glm::normalize(rot));
            dx = 0;
            dy = 0;

            glUniformMatrix4fv(model, 1, GL_FALSE, glm::value_ptr(model_transform));
        }

        if (scroll != 0.0f && pos_x + scroll > SPHERE_RADIUS) {
            pos_x += scroll;

            view_transform = glm::lookAt(
                glm::vec3(pos_x, 0.0f, 0.0f),
                glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(0.0f, 0.0f, 1.0f)
            );

            glUniformMatrix4fv(view, 1, GL_FALSE, glm::value_ptr(view_transform));
        }
        scroll = 0.0f;

        glClearColor(CLEAR_COLOR);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_DEPTH_BUFFER_BIT);

        glDrawElements(GL_TRIANGLES, indices_size, GL_UNSIGNED_INT, nullptr);
        glfwSwapBuffers(window);

        auto end = high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
        std::this_thread::sleep_for(std::chrono::milliseconds(16 - elapsed));
        begin = high_resolution_clock::now();
    }

    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(2, vbo);
    glDeleteVertexArrays(1, &vao);

    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
