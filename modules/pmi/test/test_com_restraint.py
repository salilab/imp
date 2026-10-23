import math

import IMP
import IMP.algebra
import IMP.atom
import IMP.core
import IMP.pmi
import IMP.pmi.restraints.basic
import IMP.pmi.topology
import IMP.test


def com_of(ps):
    total = sum(IMP.atom.Mass(p).get_mass() for p in ps)
    c = IMP.algebra.Vector3D(0, 0, 0)
    for p in ps:
        c += IMP.core.XYZ(p).get_coordinates() * (IMP.atom.Mass(p).get_mass() / total)
    return c


class Tests(IMP.test.TestCase):
    """The PMI wrappers of IMP.core.COMRestraint"""

    def _make_pmi_system(self, n_copies=3):
        m = IMP.Model()
        s = IMP.pmi.topology.System(m)
        st = s.create_state()
        mol = st.create_molecule('P', sequence='A' * 20, chain_id='A')
        mol.add_representation(mol, resolutions=[1])
        mols = [mol] + [mol.create_copy(chain_id=c)
                        for c in 'BCDEFG'[:n_copies - 1]]
        for c in mols[1:]:
            c.add_representation(c, resolutions=[1])
        hier = s.build()
        for i, c in enumerate(mols):
            for j, p in enumerate(IMP.atom.Selection(
                    c.get_hierarchy(), resolution=1).get_selected_particles()):
                IMP.core.XYZ(p).set_coordinates(
                    IMP.algebra.Vector3D(30 * i, 0.5 * j, 10.0 * i - 5.0))
        return m, hier, mols

    def test_slab_wrapper(self):
        """COMSlabRestraint: soft walls on the COM z of each copy"""
        m, hier, mols = self._make_pmi_system()
        r = IMP.pmi.restraints.basic.COMSlabRestraint(
            hier, z_low=-2.0, z_top=12.0, softness=4.0)
        expected = 0.0
        for c in mols:
            z = com_of(IMP.atom.Selection(
                c.get_hierarchy(), resolution=1).get_selected_particles())[2]
            expected += (math.log1p(math.exp(-(z + 2.0) / 4.0))
                         + math.log1p(math.exp(-(12.0 - z) / 4.0)))
        self.assertEqual(r.com_restraint.get_number_of_groups(), 3)
        self.assertAlmostEqual(r.rs.evaluate(False), expected, delta=1e-6)

    def test_cylinder_wrapper(self):
        """COMCylinderBarrier: harmonic walls on the COM of each copy"""
        m, hier, mols = self._make_pmi_system()
        r = IMP.pmi.restraints.basic.COMCylinderBarrier(
            hier, radius=40.0, z_low=-1.0, z_top=10.0, k=10.0, resolution=1)
        expected = 0.0
        for c in mols:
            v = com_of(IMP.atom.Selection(
                c.get_hierarchy(), resolution=1).get_selected_particles())
            radial = math.hypot(v[0], v[1])
            for d in (radial - 40.0, -1.0 - v[2], v[2] - 10.0):
                if d > 0:
                    expected += 0.5 * 10.0 * d * d
        self.assertGreater(expected, 0)
        self.assertAlmostEqual(r.rs.evaluate(False), expected, delta=1e-6)

    def test_distance_wrapper(self):
        """COMDistanceRestraint: bounds on the distance between two COMs"""
        m, hier, mols = self._make_pmi_system(n_copies=2)
        r = IMP.pmi.restraints.basic.COMDistanceRestraint(
            mols[0], mols[1], distance_min=5.0, distance_max=20.0, k=2.0)
        d = IMP.algebra.get_distance(*[com_of(IMP.atom.Selection(
            c.get_hierarchy(), resolution=1).get_selected_particles())
            for c in mols])
        expected = 0.5 * 2.0 * max(0.0, d - 20.0) ** 2 \
            + 0.5 * 2.0 * max(0.0, 5.0 - d) ** 2
        self.assertGreater(expected, 0)
        self.assertAlmostEqual(r.rs.evaluate(False), expected, delta=1e-6)


if __name__ == '__main__':
    IMP.test.main()
