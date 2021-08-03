#version 330 core

//// call type defines section
#define CALL_POINT_TYPE 0
#define CALL_POLYGON_TYPE 1
#define CALL_POLYPRISM_TYPE 2
#define CALL_POLYPYR_TYPE 3
#define CALL_LINE_TYPE 4
#define CALL_CIRCLE_TYPE 5
#define CALL_FCIRCLE_TYPE 6
#define CALL_SPHERE_TYPE 7
#define CALL_FSPHERE_TYPE 8
////

#define EPSILON 0.0001

layout (location = 0) in vec3 in_pos;

layout (location = 1) in int call_type;
layout (location = 2) in vec3 cur_color;

layout (location = 3) in mat4 trans_mat;

layout (location = 7) in mat4 call_data;

uniform int prim_calls_sum;

out float id;
out vec3 color;

bool check_bind();

bool point_check();
bool poly_check();
bool polyprism_check();
bool polypyr_check();
bool line_check();
bool cicle_check();
bool fcircle_check();
bool sphere_check();
bool fsphere_check();

void main()
{
  id = 1.0 - gl_InstanceID / float(prim_calls_sum);

  if (check_bind())
    gl_Position = vec4(in_pos.x / 8.0 - 1.0, (in_pos.y + in_pos.z * 16) / -128.0 + 0.9921875, 0.0, 1.0);
  else
    gl_Position = vec4(-1.0); // spawn point outside window

  color = cur_color / 255.0;
}

bool check_bind()
{
  // TODO: Sort for calling rate for better performance
  switch (call_type) {
  case CALL_POINT_TYPE:
    return point_check();
  case CALL_POLYGON_TYPE:
    return poly_check();
  case CALL_POLYPRISM_TYPE:
    return polyprism_check();
  case CALL_POLYPYR_TYPE:
    return polypyr_check();
  case CALL_LINE_TYPE:
    return line_check();
  case CALL_CIRCLE_TYPE:
    return cicle_check();
  case CALL_FCIRCLE_TYPE:
    return fcircle_check();
  case CALL_SPHERE_TYPE:
    return sphere_check();
  case CALL_FSPHERE_TYPE:
    return fsphere_check();
  }
  return false;
}

bool point_check()
{
  vec4 pos = trans_mat * vec4(call_data[0].xyz, 1.0);
  float line_width = call_data[0].w;
  return length(in_pos - pos.xyz) <= line_width;
}

bool poly_check() {return false;}
bool polyprism_check() {return false;}
bool polypyr_check() {return false;}
bool line_check() {return false;}
bool cicle_check() {return false;}
bool fcircle_check() {return false;}
bool sphere_check() {return false;}
bool fsphere_check() {return false;}