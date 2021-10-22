#version 310 es

//// Call type defines section
#define CALL_POINT_TYPE 0 // 1D Point; 3D Solid Sphere
#define CALL_POLYGON_TYPE 1 // 2D Triangle; 3D Triangle Prism
#define CALL_POLYPYR_TYPE 2 // 3D Triangle Pyramid
#define CALL_LINE_TYPE 3 // 2D Line; 3D Cylider; 2D Circle
#define CALL_CIRCLE_TYPE 4 // 2D Circle line; 2D Ellipse Line; 3D Elliptic Cylinder without foundations
#define CALL_FCIRCLE_TYPE 5 // 2D Ellipse; 3D Elliptic Cylinder
#define CALL_SPHERE_TYPE 6 // 3D Ellipsoid Hollow
#define CALL_FSPHERE_TYPE 7 // 3D Ellipsoid
#define CALL_CLEAR_TYPE 8
////

#define EPSILON 0.0001

#define COMP_FLOAT(val1, val2) (abs(val1 - val2) < EPSILON)

//// Inputs
layout (location = 0) in vec3 in_pos;

layout (location = 1) in int call_type;
layout (location = 2) in vec3 cur_color;

layout (location = 3) in mat4 call_data;

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
bool fsphere_check();

void main()
{
  id = 0.999 - float(gl_InstanceID) / float(prim_calls_sum);  
  color = cur_color / 255.0;
  // color = vec3(1.0, 0.0, 0.0);
  
  if (check_bind())
  // if (true)
    gl_Position = vec4(-0.875 + in_pos.x * 0.125, -0.9921875 + (in_pos.y + in_pos.z * 16.0) * 0.0078125, 0.0, 1.0);
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
  case CALL_FSPHERE_TYPE:
    return fsphere_check();
  case CALL_CLEAR_TYPE:
    return true;
  }
  return false;
}

// POINT_CHECK(VEC3 POS, FLOAT LINE_WIDTH)
// Data Matrix layout:
// x y z .  <- POS
// w . . .  <- LINE_WIDTH
// . . . .
// . . . .
///////////////////
bool point_check()
{
  return length(in_pos - call_data[0].xyz) <= call_data[0].w;
}

// POLY_CHECK(VEC3 POS1, VEC3 POS2, VEC3 POS3, FLOAT Z_HEIGHT)
// Data Matrix layout:
// x y z .  <- POS1
// x y z .  <- POS2
// x y z .  <- POS3
// w . . .  <- Z_HEIGHT
///////////////////
bool poly_check() 
{
  vec3 p1 = call_data[0].xyz;
  vec3 p2 = call_data[1].xyz;
  vec3 p3 = call_data[2].xyz;
  float z_height = call_data[3].x;

  if (abs(dot(p1 - in_pos, normalize(cross(p2 - p1, p3 - p1)))) > z_height)
    return false;

  vec3 parray[9] = vec3[9](p1, p2, p3, p2, p3, p1, p3, p1, p2);
                          
  for (int i = 0; i < 9; i+= 3)
  {
    vec3 tmpxx = parray[i + 2] - parray[i + 1];
    float ltmpxx = length(tmpxx);
    vec3 m = dot(in_pos - parray[i + 1], tmpxx) / (ltmpxx * ltmpxx) * tmpxx + parray[i + 1];

    if (dot(in_pos - m, parray[i] - m) < -EPSILON)
      return false;
  }
  return true;
}

// POLYPYR_CHECK(VEC3 POS1, VEC3 POS2, VEC3 POS3, VEC3 POS4)
// Data Matrix layout:
// x y z .  <- POS1
// x y z .  <- POS2
// x y z .  <- POS3
// x y z .  <- POS4
///////////////////
bool polypyr_check() 
{
  vec3 p1 = call_data[0].xyz;
  vec3 p2 = call_data[1].xyz;
  vec3 p3 = call_data[2].xyz;
  vec3 p4 = call_data[3].xyz;

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

// LINE_CHECK(VEC3 POS1, VEC3 POS2, FLOAT LINE_WIDTH)
// Data Matrix layout:
// x y z .  <- POS1
// x y z .  <- POS2
// w . . .  <- LINE_WIDTH
// . . . .
///////////////////
bool line_check() 
{ 
  vec3 p1 = call_data[0].xyz;
  vec3 p2 = call_data[1].xyz;
  float line_width = call_data[2].x;

  float l12 = length(p2 - p1);
  if (length(cross(in_pos - p1, in_pos - p2)) / l12 > line_width)
    return false;

  float val = dot(p2 - p1, in_pos - p1);
  return val >= 0.0 && val - EPSILON <= l12 * l12;
}


// CIRCLE_CHECK(MAT4X3 MODEL_MAT; VEC2 R, FLOAT LINE_WIDTH, FLOAT Z_HEIGHT)
// Data Matrix layout:
// x y z w  <- MODEL_MAT(0)
// x y z w  <- MODEL_MAT(1)
// x y z w  <- MODEL_MAT(2)
// x y w h  <- R; LINE_WIDTH; Z_HEIGHT
///////////////////
bool cicle_check()
{
  mat4 inv_mat = mat4(
    vec4(call_data[0].xyz, 0.0), 
    vec4(call_data[1].xyz, 0.0), 
    vec4(call_data[2].xyz, 0.0), 
    vec4(call_data[3].xyz, 1.0)
  );

  vec3 res_vec = (inv_mat * vec4(in_pos, 1.0)).xyz;
  float res_len = length(res_vec.xy);
  float width = call_data[2].w / length(vec2(call_data[0].w, call_data[1].w) * normalize(res_vec.xy));
  
  return abs(res_vec.z) <= call_data[3].w && 1.0 - width <= res_len && res_len <= 1.0 + width;
}

// FCIRCLE_CHECK(MAT4X3 MODEL_MAT; VEC2 R, FLOAT Z_HEIGHT)
// Data Matrix layout:
// x y z w  <- MODEL_MAT(0)
// x y z w  <- MODEL_MAT(1)
// x y z w  <- MODEL_MAT(2)
// x y . h  <- R; Z_HEIGHT
///////////////////
bool fcircle_check()
{
  mat4 inv_mat = mat4(
    vec4(call_data[0].xyz, 0.0), 
    vec4(call_data[1].xyz, 0.0), 
    vec4(call_data[2].xyz, 0.0), 
    vec4(call_data[3].xyz, 1.0)
  );

  vec3 res_vec = (inv_mat * vec4(in_pos, 1.0)).xyz;
  
  return abs(res_vec.z) <= call_data[3].w && length(res_vec.xy) <= 1.0;
}

// FCIRCLE_CHECK(MAT4X3 MODEL_MAT; VEC3 R, FLOAT LINE_WIDTH)
// Data Matrix layout:
// x y z w  <- MODEL_MAT(0)
// x y z w  <- MODEL_MAT(1)
// x y z w  <- MODEL_MAT(2)
// x y z w  <- R; LINE_WIDTH
///////////////////
bool sphere_check()
{
  mat4 inv_mat = mat4(
    vec4(call_data[0].xyz, 0.0), 
    vec4(call_data[1].xyz, 0.0),
    vec4(call_data[2].xyz, 0.0), 
    vec4(call_data[3].xyz, 1.0)
  );
  vec3 res_vec = (inv_mat * vec4(in_pos, 1.0)).xyz;
  float res_len = length(res_vec);
  float width = call_data[3].w / length(vec3(call_data[0].w, call_data[1].w, call_data[2].w) * normalize(res_vec));

  return 1.0 - width <= res_len && res_len <= 1.0 + width;
}

// FCIRCLE_CHECK(MAT4X3 MODEL_MAT; VEC3 R)
// Data Matrix layout:
// x y z w  <- MODEL_MAT(0)
// x y z w  <- MODEL_MAT(1)
// x y z w  <- MODEL_MAT(2)
// x y z .  <- R
///////////////////
bool fsphere_check()
{
  mat4 inv_mat = mat4(
    vec4(call_data[0].xyz, 0.0), 
    vec4(call_data[1].xyz, 0.0),
    vec4(call_data[2].xyz, 0.0), 
    vec4(call_data[3].xyz, 1.0)
  );

  return length((inv_mat * vec4(in_pos, 1.0)).xyz) <= 1.0;
}