w = IMP.display.ChimeraWriter("out.py")
g0= IMP.XYZRsGeometry(xyzrs0)
g0.set_name("my particles")
g0.set_color(IMP.display.Color(1,0,0))
g0.add_geometry(g0)
g1= IMP.XYZRsGeometry(xyzrs1)
g1.set_name("my other particles")
g1.set_color(IMP.display.Color(0,1,0))
g1.add_geometry(g1)
# make sure that the file is written
del w
