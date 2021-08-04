import glm

# a, b, c = glm.vec3(0.0, 0.0, 0.0), glm.vec3(15.0, 15.0, 0.0), glm.vec3(2.5, 5.0, 0.0)
# p = glm.vec3(0.0, 0.0, 0.0)

# x10 = a - p
# x21 = b - a

# print(
#     glm.length(glm.cross(p - a, p - b)) / glm.length(b - a),
#     glm.dot(b - a, p - a),
#     glm.length(a - b) ** 2,
# )

# print(
#     glm.length(glm.cross(p - a, p - b)) / glm.length(b - a),
#     glm.dot(b - a, p - a) >= 0 and glm.dot(b - a, p - a) <= glm.length(a - b) ** 2,
# )

dir = glm.normalize(glm.vec3(1.0, 1.0, 1.0))

print(glm.dot(dir, glm.vec3(-2.0, -2.0, -2.0)))
print(glm.length(glm.vec3(-2.0, -2.0, -2.0)), dir)
