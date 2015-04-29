## \example container/connectivity.py
# Shows how to use and visualize the IMP::misc::ConnectingPairContainer.

from __future__ import print_function
import IMP.container
import IMP.display
import sys

IMP.setup_from_argv(sys.argv, "connectivity")

m = IMP.Model()
ds = IMP.core.create_xyzr_particles(m, 20, .1)
sc = IMP.container.ListSingletonContainer(m, IMP.get_indexes(ds))
cpc = IMP.container.ConnectingPairContainer(sc, .1)
m.update()
pg = IMP.core.EdgePairsGeometry(cpc)
w = IMP.display.ChimeraWriter("pairs.py")
w.add_geometry(pg)
print(pg.get_name())
del w
