#include "util.h"

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include <iostream>

static unsigned int compile_shader(unsigned int type, const std::string &source)
{
  unsigned int id = glCreateShader(type);
  const char *src = source.c_str();
  glShaderSource(id, 1, &src, nullptr);
  glCompileShader(id);

  int result;
  glGetShaderiv(id, GL_COMPILE_STATUS, &result);
  if (result == GL_FALSE)
  {
    int length;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
    char *message = (char *)alloca((unsigned long)length * sizeof(char));
    glGetShaderInfoLog(id, length, &length, message);

    std::cout << "Failed to compile!! "
              << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader" << std::endl;
    std::cout << message << std::endl;

    glDeleteShader(id);
    return 0;
  }

  return id;
}

static unsigned int create_shaders(const std::string &vertex_shader, const std::string &fragment_shader)
{
  unsigned int program = glCreateProgram();
  unsigned int vs = compile_shader(GL_VERTEX_SHADER, vertex_shader);
  assert(vs);
  unsigned int fs = compile_shader(GL_FRAGMENT_SHADER, fragment_shader);
  assert(fs);

  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glLinkProgram(program);
  glValidateProgram(program);

  int success;
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success)
  {
    int length;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
    char *message = (char *)alloca((unsigned long)length * sizeof(char));
    glGetProgramInfoLog(program, length, &length, message);

    std::cout << message << std::endl;

    return 0;
  }
  glDeleteShader(vs);
  glDeleteShader(fs);

  return program;
}

void framebuffer_size_callback(GLFWwindow *, int width, int height)
{
  glViewport(0, 0, width, height);
}

void process_input(GLFWwindow *window)
{
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
}

void setup_debug(bool enable)
{
  glDebugMessageCallbackKHR(enable ? gl_debug_logger : nullptr, stderr);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_KHR);
  if (GL_NO_ERROR != glGetError())
    std::cerr << "Unable to set synchronous debug output\n";
}

int main() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

#ifndef NDEBUG
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

  constexpr auto INIT_WIDTH = 800u;
  constexpr auto INIT_HEIGHT = 600u;
  GLFWwindow* window = glfwCreateWindow(INIT_WIDTH, INIT_HEIGHT,
                                        "Learn OpenGL", nullptr, nullptr);
  if (!window)
  {
    std::cout << "Failed to create GLFW window\n";
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    std::cout << "Failed to initialize GLAD\n";
    return -1;
  }
#ifndef NDEBUG
  if (GLAD_GL_KHR_debug)
  {
    setup_debug(true);
  }
#endif
  glViewport(0, 0, INIT_WIDTH, INIT_HEIGHT);
  glClearColor(0.188f, 0.349f, 0.506f, 1.0f);

  float positions[6] = {
      -0.5f, -0.5f,
      0.0f, 0.5f,
      0.5f, -0.5f};

  unsigned int buffer;
  glGenBuffers(1, &buffer);

  unsigned int vertex_array;
  glGenVertexArrays(1, &vertex_array);

  glBindBuffer(GL_ARRAY_BUFFER, buffer);
  glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), positions, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0 /*index*/, 2 /*size*/, GL_FLOAT, GL_FALSE /*normalised*/, sizeof(float) * 2 /*stride*/, (void *)0 /*attribute pointer*/);

  std::string vertex_shader =
      "#version 330 core\n"
      "layout (location = 0) in vec2 aPos;\n"
      "void main()\n"
      "{\n"
      "   gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);\n"
      "}\n";

  std::string fragment_shader =
      "#version 330 core\n"
      "\n"
      "out vec4 color;\n"
      "void main()\n"
      "{\n"
      " color = vec4(1.0, 0.0, 0.0, 1.0);\n"
      "}\n";

  unsigned int shader_program = create_shaders(vertex_shader, fragment_shader);
  glUseProgram(shader_program);
  glBindVertexArray(vertex_array);

  while (!glfwWindowShouldClose(window))
  {
    glClear(GL_COLOR_BUFFER_BIT);
    process_input(window);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
