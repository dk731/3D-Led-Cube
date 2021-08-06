import glm

# // CIRCLE_CHECK(VEC3 POS, VEC3 DIR, VEC3 R, FLOAT LINE_WIDTH, FLOAT Z_HEIGHT)
# // Data Matrix layout:
# // x y z .  <- POS
# // x y z .  <- DIR
# // x y z .  <- R
# // w h . .  <- LINE_WIDTH; FLOAT Z_HEIGHT
# ///////////////////

p = glm.vec3(0.0, 0.0, 0.0)
d = glm.vec3(0.0, 1.0, 0.0)

rad = glm.vec3(1, 1, 1)

tp = glm.vec3(0.0, 0.0, 0.0)

sx, sy = 60, 15

for y in range(sy):
    for x in range(sx):
        qp = glm.vec3((x - sx / 2) / (sx / 4), ((sy - y) - sy / 2) / (sy / 4), 0.0) - p
        tmpz = glm.dot(qp, d) ** 2 / glm.dot(d, d)
        tmpxy = glm.dot(qp, qp) - tmpz * 2

        print(1 if tmpxy / rad.x ** 2 + tmpz / rad.z ** 2 < 1 else 0, end="")
    print()
