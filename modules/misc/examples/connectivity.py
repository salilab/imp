import IMP.misc
import IMP.display

m= IMP.Model()
ds=IMP.core.create_xyzr_particles(m, 20, .1)
sc= IMP.core.ListSingletonContainer(ds)
cpc= IMP.misc.ConnectingPairContainer(sc, .1, True)
m.evaluate(False)
pg= IMP.display.EdgePairsGeometry(cpc)
w= IMP.display.ChimeraWriter("pairs.py")
w.add_geometry(pg)
print pg.get_name()
del w
