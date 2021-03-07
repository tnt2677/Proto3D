#include "util.h"

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <sstream>

struct ShaderProgramSource
{
  std::string vertex_source;
  std::string fragment_source;
};

static ShaderProgramSource parse_shader(const std::string &file_path)
{
  std::ifstream stream(file_path);

  enum class ShaderType
  {
    kNone = -1,
    kVertex = 0,
    kFragment = 1
  };

  std::string line;
  std::stringstream ss[2];
  ShaderType type = ShaderType::kNone;

  while (getline(stream, line))
  {
    if (line.find("#shader") != std::string::npos)
    {
      if (line.find("vertex") != std::string::npos)
        type = ShaderType::kVertex;
      else if (line.find("fragment") != std::string::npos)
        type = ShaderType::kFragment;
    }
    else
    {
      ss[(int)type] << line << '\n';
    }
  }

  return {ss[0].str(), ss[1].str()};
}

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
  glBindBuffer(GL_ARRAY_BUFFER, buffer);
  glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), positions, GL_STATIC_DRAW);

  unsigned int vertex_array;
  glGenVertexArrays(1, &vertex_array);
  glBindVertexArray(vertex_array);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0 /*index*/, 2 /*size*/, GL_FLOAT, GL_FALSE /*normalised*/, sizeof(float) * 2 /*stride*/, (void *)0 /*attribute pointer*/);

  ShaderProgramSource source = parse_shader("src/res/shaders/basic.shader");
  unsigned int shader_program = create_shaders(source.vertex_source, source.fragment_source);
  
  glUseProgram(shader_program);

  while (!glfwWindowShouldClose(window))
  {
    glClear(GL_COLOR_BUFFER_BIT);
    process_input(window);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glDeleteProgram(shader_program);

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
