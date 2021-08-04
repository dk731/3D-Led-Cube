#version 330 core

//// Call type defines section
#define CALL_POINT_TYPE 0
#define CALL_POLYGON_TYPE 1
#define CALL_POLYPYR_TYPE 2
#define CALL_LINE_TYPE 3
#define CALL_CIRCLE_TYPE 4
#define CALL_FCIRCLE_TYPE 5
#define CALL_SPHERE_TYPE 6
////

#define EPSILON 0.0001

#define COMP_FLOAT(val1, val2) (abs(val1 - val2) < EPSILON)

//// Inputs
layout (location = 0) in vec3 in_pos;

layout (location = 1) in int call_type;
layout (location = 2) in vec3 cur_color;

layout (location = 3) in mat4 trans_mat;
layout (location = 7) in mat4 call_data;

uniform int prim_calls_sum;
////

out float id;
out vec3 color;

bool check_bind();

bool point_check();
bool poly_check();
bool polypyr_check();
bool line_check();
bool cicle_check();
bool fcircle_check();
bool sphere_check();

void main()
{
  id = gl_InstanceID / float(prim_calls_sum);  
  color = cur_color / 255.0;
  
  if (check_bind())
    gl_Position = vec4(in_pos.x / 8.0 - 1.0, (in_pos.y + in_pos.z * 16) / -128.0 + 0.9921875, 0.0, 1.0);
  else
    gl_Position = vec4(-1.0); // spawn point outside window
}

bool check_bind()
{
  // TODO: Sort for calling rate for better performance
  switch (call_type) {
  case CALL_POINT_TYPE:
    return point_check();
  case CALL_POLYGON_TYPE:
    return poly_check();
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
  }
  return false;
}

bool point_check()
{
  return length(in_pos - (trans_mat * call_data[0]).xyz) <= call_data[1].x;
}

bool poly_check() 
{
  vec3 p1 = (trans_mat * call_data[0]).xyz;
  vec3 p2 = (trans_mat * call_data[1]).xyz;
  vec3 p3 = (trans_mat * call_data[2]).xyz;
  float line_width = call_data[3].x;

  if (abs(dot(p1 - in_pos, normalize(cross(p2 - p1, p3 - p1)))) > line_width)
    return false;

  vec3 parray[9] = vec3[9](p1, p2, p3, p2, p3, p1, p3, p1, p2);
                          
  for (int i = 0; i < 9; i+= 3)
  {
    vec3 tmpxx = parray[i + 2] - parray[i + 1];
    vec3 m = dot(in_pos - parray[i + 1], tmpxx) / pow(length(tmpxx), 2) * tmpxx + parray[i + 1];

    if (dot(in_pos - m, parray[i] - m) < -EPSILON)
      return false;
  }
  return true;
}

bool polypyr_check() 
{
  // vec3 in_pos
  vec3 p1 = (trans_mat * call_data[0]).xyz;
  vec3 p2 = (trans_mat * call_data[1]).xyz;
  vec3 p3 = (trans_mat * call_data[2]).xyz;
  vec3 p4 = (trans_mat * call_data[3]).xyz;

  // 1, 2, 3   2 - 1, 3 - 1, in - 1
  // 2, 3, 4   3 - 2, 4 - 2, in - 2
  // 3, 4, 1   4 - 3, 1 - 3, in - 3
  // 4, 1, 2   1 - 4, 2 - 4, in - 4

  if (determinant(mat3(p2 - p1, p3 - p1, in_pos - p1)) < -EPSILON)
      return false;
  if (determinant(mat3(p3 - p2, p4 - p2, in_pos - p2)) < -EPSILON)
      return false;
  if (determinant(mat3(p4 - p3, p1 - p3, in_pos - p3)) < -EPSILON)
      return false;
  if (determinant(mat3(p1 - p4, p2 - p4, in_pos - p4)) < -EPSILON)
      return false;

  return true;
}
bool line_check() 
{ 
  vec3 p1 = (trans_mat * call_data[0]).xyz;
  vec3 p2 = (trans_mat * call_data[1]).xyz;
  float line_width = call_data[2].x;

  float l12 = length(p2 - p1);
  if (length(cross(in_pos - p1, in_pos - p2)) / l12 > line_width)
    return false;

  float val = dot(p2 - p1, in_pos - p1);
  return val >= 0.0 && val - EPSILON <= l12 * l12;
}

bool cicle_check()
{
  vec3 p1 = (trans_mat * call_data[0]).xyz;
  float r = call_data[1].x;
  float line_width = call_data[1].y / 2.0;
  float z_height = call_data[1].z;

  vec3 dir = normalize((trans_mat * vec4(0.0, 0.0, 1.0, 1.0)).xyz); // default circle normal is (0, 0, 1)
  

  if ( abs(dot(in_pos - p1, dir)) > z_height)
    return false;

  float dif = length((in_pos - dir) - p1) - r;
  return dif >= -line_width && dif <= line_width;
}

bool fcircle_check()
{
  vec3 p1 = (trans_mat * call_data[0]).xyz;
  float r = call_data[1].x;
  float z_height = call_data[1].y;

  vec3 dir = normalize((trans_mat * vec4(0.0, 0.0, 1.0, 1.0)).xyz); // default circle normal is (0, 0, 1)
  

  if ( abs(dot(in_pos - p1, dir)) > z_height)
    return false;

  return length((in_pos - dir) - p1) <= r;
}

bool sphere_check()
{
  float dif = length(in_pos - (trans_mat * call_data[0]).xyz) - call_data[1].x;
  float line_width = call_data[1].y / 2.0f + EPSILON;
  return dif >= -line_width && dif <= line_width;
}