#include <iostream>
#include <fstream>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class ShaderDrawer
{
private:
  void prepare_shaders();

  GLuint compile_shader(GLuint type, const char **prog);
  bool check_compile(GLuint obj);
  bool check_link(GLuint obj);

  void gen_buf();

  GLuint vert_shade;
  GLuint frag_shade;

  GLuint main_prog;

  GLuint main_frameb;
  GLuint out_text;

  GLuint main_vao;

public:
  void calculate_points();
  void get_buf(unsigned char *data);
  ShaderDrawer();
};