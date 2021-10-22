import glm


a = glm.scale(glm.translate(glm.mat4(), glm.vec3(3, 2, 1)), glm.vec3(2, 2, 1))

print(a)
