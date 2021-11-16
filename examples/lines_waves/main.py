from ledcd import CubeDrawer as cd


drawer = cd.get_obj()
# drawer.translate(0, 7.5, 0)
time = 0

while True:
    drawer.clear()
    drawer.set_color(255)
    drawer.line(0, 0, 0, 0, 15, 0)

    drawer.set_color(255, 0, 0)
    drawer.point(0, 0, 0)
    drawer.point(0, 15, 0)
    # for z in range(15):
    #     for x in range(15):
    #         drawer.line((x, 5, z), (x, 10, z))

    drawer.show()
    time += drawer.delta_time
