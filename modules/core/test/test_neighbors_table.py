import IMP.kernel
import IMP.test
import IMP.core
import IMP.container

m = IMP.kernel.Model()
pis = [m.add_particle("P%1%") for i in range(0, 10)]
bb = IMP.algebra.get_unit_bounding_box_3d()
r = .1
for p in pis:
    IMP.core.XYZR.setup_particle(
        m, p, IMP.algebra.Sphere3D(IMP.algebra.get_random_vector_in(bb), r))

cpc = IMP.container.ClosePairContainer(
    IMP.container.ListSingletonContainer(m, pis), .3, .1)
nt = IMP.core.NeighborsTable(cpc)

for i in range(0, 10):
    for p in pis:
        IMP.core.XYZ(m, p).set_coordinates(
            IMP.algebra.get_random_vector_in(bb))
    m.update()
    for pip in cpc.get_indexes():
        assert(pip[0] in nt.get_neighbors(pip[1]))
        assert(pip[1] in nt.get_neighbors(pip[0]))
