#include "ShaderDrawer.h"

ShaderDrawer::ShaderDrawer()
{
  prepare_shaders();
  gen_buf();

  glGenVertexArrays(1, &main_vao);
  glBindVertexArray(main_vao);

  GLfloat vertices_position[12288];
  for (int z = 0; z < 16; z++)
    for (int y = 0; y < 16; y++)
      for (int x = 0; x < 16; x++)
      {
        int start_ind = (x + (y + z * 16) * 16) * 3;
        vertices_position[start_ind] = x;
        vertices_position[start_ind + 1] = y;
        vertices_position[start_ind + 2] = z;
      }
  GLuint vbo;
  glGenBuffers(1, &vbo);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_position), vertices_position, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
}

bool ShaderDrawer::check_compile(GLuint obj)
{
  GLint status;
  glGetShaderiv(obj, GL_COMPILE_STATUS, &status);
  if (status == GL_FALSE)
  {
    GLint length;
    glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &length);
    std::vector<char> log(length);
    glGetShaderInfoLog(obj, length, NULL, &log[0]);
    std::cerr << &log[0];
    return false;
  }
  return true;
}

GLuint ShaderDrawer::compile_shader(GLuint type, const char **prog)
{
  vert_shade = glCreateShader(type);
  glShaderSource(vert_shade, 1, prog, NULL);
  glCompileShader(vert_shade);

  if (!check_compile(vert_shade))
    throw std::runtime_error("Error occured during shader compilation");
  return vert_shade;
}

void ShaderDrawer::gen_buf()
{
  glGenFramebuffers(1, &main_frameb);
  glBindFramebuffer(GL_FRAMEBUFFER, main_frameb);

  glGenTextures(1, &out_text);
  glBindTexture(GL_TEXTURE_2D, out_text);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 16, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, out_text, 0);

  GLenum draw_buffers[1] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, draw_buffers);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    throw std::runtime_error("Unable to create frame buffer");
}

void ShaderDrawer::prepare_shaders()
{
  std::ifstream in("./shaders/main.vert");
  std::string tmp_vert = std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
  const char *vert_str = tmp_vert.c_str();
  in.close();

  in = std::ifstream("./shaders/main.frag");
  std::string tmp_frag = std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
  const char *frag_str = tmp_frag.c_str();
  in.close();

  vert_shade = compile_shader(GL_VERTEX_SHADER, &vert_str);
  frag_shade = compile_shader(GL_FRAGMENT_SHADER, &frag_str);

  main_prog = glCreateProgram();

  glAttachShader(main_prog, vert_shade);
  glAttachShader(main_prog, frag_shade);

  glLinkProgram(main_prog);
  if (!check_compile(vert_shade))
    throw std::runtime_error("Error occured during vertice shader compilation");

  glUseProgram(main_prog);
}

void ShaderDrawer::calculate_points()
{
  glBindFramebuffer(GL_FRAMEBUFFER, main_frameb);
  glViewport(0, 0, 16, 256);
  glDrawArrays(GL_POINT, 0, 4096);
}

void ShaderDrawer::get_buf(unsigned char *data)
{
  glBindFramebuffer(GL_FRAMEBUFFER, main_frameb);
  glReadPixels(0, 0, 16, 256, GL_RGB, GL_UNSIGNED_BYTE, data);
}