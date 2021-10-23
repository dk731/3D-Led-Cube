from ledcd import CubeDrawer as cd
a = cd.get_obj()

a.set_color(0, 255, 0)
a.filled_sphere(7.5, 7.5, 7.5, 8)
a.set_color(255)
a.filled_sphere(7.5, 7.5, 7.5, 4)
a.set_color(255, 0, 0)
a.line(0, 0, 0, 15, 15, 15)
a.show()

input()