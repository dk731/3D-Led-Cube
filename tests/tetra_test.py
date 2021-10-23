from ledcd import CubeDrawer as cd

a = cd.get_obj()
a.draw_immediate = True

a.tetr(15, 0, 0, 0, 0, 0, 0, 15, 0, 0, 0, 15)
input()
