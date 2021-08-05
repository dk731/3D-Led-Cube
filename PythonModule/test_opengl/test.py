import glm

r = glm.vec3(3.0, 2.0, 1.0)
p = glm.vec3(3.0, 1.0, 0.0)
tmp = (p / r) ** 2

print(tmp.x + tmp.y + tmp.z <= 1)
