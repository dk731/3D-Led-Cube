from ledcd import CubeDrawer

drawer = CubeDrawer.get_obj()

drawer.draw_immediate = True


drawer.clear()

drawer.circle(7.5, 7.5, 7, 3)
input()
