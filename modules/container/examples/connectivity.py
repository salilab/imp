## \example container/connectivity.py
## Shows how to use and visualize the IMP::misc::ConnectingPairContainer.

import IMP.container
import IMP.display

m= IMP.Model()
ds=IMP.core.create_xyzr_particles(m, 20, .1)
sc= IMP.container.ListSingletonContainer(ds)
cpc= IMP.container.ConnectingPairContainer(sc, .1)
m.evaluate(False)
pg= IMP.core.EdgePairsGeometry(cpc)
w= IMP.display.ChimeraWriter("pairs.py")
w.add_geometry(pg)
print pg.get_name()
del w
