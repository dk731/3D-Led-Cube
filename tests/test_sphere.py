import glm

#  [    0.333,   -0.000,    0.000,   -7.500]
#  [   -0.000,    0.333,   -0.000,   -7.500]
#  [    0.000,   -0.000,    0.333,   -7.500]
#  [   -0.000,    0.000,   -0.000,    1.000]]

a = glm.mat4(
    [
        0.333,
        -0.000,
        0.000,
        -0.000,
    ],
    [
        -0.000,
        0.333,
        -0.000,
        0.000,
    ],
    [
        0.000,
        -0.000,
        0.333,
        -0.000,
    ],
    [
        -7.500,
        -7.500,
        -7.500,
        1.000,
    ],
)
# print()
# print(a * glm.inverse(a))
# print()

b = glm.vec4(7.5, 7.5, 7.5, 1.0)

# print(a)
# print(a * b)
# print(glm.length(a * b))

# c = glm.mat4()
# c = glm.translate(c, glm.vec3(3, 2, 6))
# print(c)
# print()
# cc = glm.inverse(c)

# print(cc)

# print()

# print(c * glm.vec4(1, 2, 3, 1.0))
# print()


# print(cc * glm.vec4(1, 2, 3, 1.0))
# print()

a = glm.scale(glm.translate(glm.mat4(), glm.vec3(7.5, 7.5, 7.5)), glm.vec3(3, 3, 3))


print(glm.length((glm.inverse(a) * glm.vec4(11.5, 7.5, 7.5, 1.0)).xyz))
