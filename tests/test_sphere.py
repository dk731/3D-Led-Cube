import glm
from math import pi

print(glm.rotate.__doc__)
rotate_vec = glm.vec3(0, pi / 2, pi / 2)
a = glm.mat4()

a = glm.rotate(a, rotate_vec.x, glm.vec3(1.0, 0.0, 0.0))
a = glm.rotate(a, rotate_vec.y, glm.vec3(0.0, 1.0, 0.0))
a = glm.rotate(a, rotate_vec.z, glm.vec3(0.0, 0.0, 1.0))

t = glm.vec4(0, 0, 3, 1)

print(a)
print()

print((a * t).xyz)
