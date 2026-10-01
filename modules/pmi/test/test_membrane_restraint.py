import IMP
import IMP.atom
import IMP.pmi
import IMP.test
import IMP.isd
import IMP.pmi.restraints.proteomics
import IMP.pmi.io
import IMP.pmi.restraints
import IMP.pmi.restraints.basic
import IMP.rmf
import IMP.pmi.dof
import IMP.pmi.tools
import math
import sys


class MembraneRestraintPrototype(IMP.Restraint):

    def __init__(self,
                 m,
                 z_nuisance,
                 thickness=30.0,
                 softness=3.0,
                 plateau=0.0000000001,
                 linear=0.02):
        """
        input a list of particles, the slope and theta of the sigmoid potential
        """
        super().__init__(m, "MembraneRestraintPrototype_ %1%")
        self.set_was_used(True)
        self.thickness = thickness
        self.z_nuisance = z_nuisance
        self.softness = softness
        self.plateau = plateau
        self.particle_list_below = []
        self.particle_list_above = []
        self.particle_list_inside = []
        self.max_float = sys.float_info.max
        self.log_max_float = math.log(self.max_float)
        self.linear = linear

    def add_particles_below(self, particles):
        self.particle_list_below += particles

    def add_particles_above(self, particles):
        self.particle_list_above += particles

    def add_particles_inside(self, particles):
        self.particle_list_inside += particles

    def score_above(self, z):
        argvalue = (z - self.z_slope_center_upper) / self.softness
        prob = (1.0 - self.plateau) / (1.0 + math.exp(-argvalue))
        return -math.log(prob * self.max_float) + self.log_max_float

    def score_below(self, z):
        argvalue = (z - self.z_slope_center_lower) / self.softness
        prob = (1.0 - self.plateau) / (1.0 + math.exp(argvalue))
        return -math.log(prob * self.max_float) + self.log_max_float

    def score_inside(self, z):

        argvalue = (z - self.z_slope_center_upper) / self.softness
        prob1 = 1.0 - (1.0 - self.plateau) / (1.0 + math.exp(-argvalue))

        argvalue = (z - self.z_slope_center_lower) / self.softness
        prob2 = 1.0 - (1.0 - self.plateau) / (1.0 + math.exp(argvalue))
        return (-math.log(prob1 * self.max_float)
                - math.log(prob2 * self.max_float)
                + 2 * self.log_max_float)

    def unprotected_evaluate(self, da):

        z_center = IMP.isd.Nuisance(self.z_nuisance).get_nuisance()
        self.z_slope_center_lower = z_center - self.thickness / 2.0
        self.z_slope_center_upper = z_center + self.thickness / 2.0

        score_above = sum([self.score_above(IMP.core.XYZ(p).get_z())
                          for p in self.particle_list_above])
        score_below = sum([self.score_below(IMP.core.XYZ(p).get_z())
                          for p in self.particle_list_below])
        score_inside = sum([self.score_inside(IMP.core.XYZ(p).get_z())
                           for p in self.particle_list_inside])

        return score_above + score_below + score_inside

    def do_get_inputs(self):
        particle_list = self.particle_list_above + \
            self.particle_list_inside + self.particle_list_below

        return particle_list


class MembraneRestraint(IMP.test.TestCase):
    def test_inside(self):
        m = IMP.Model()

        atom = IMP.Particle(m)

        d = IMP.core.XYZ.setup_particle(atom)

        p = IMP.Particle(m)

        z_center = IMP.isd.Nuisance.setup_particle(p)
        z_center.set_nuisance(0.0)

        r = MembraneRestraintPrototype(m, z_center)
        r.add_particles_inside([atom])

        r2 = IMP.pmi.MembraneRestraint(
            m, z_center.get_particle_index(), 30.0, 3.0, 0.0000000001, 0.02)
        r2.set_was_used(True)
        r2.add_particles_inside([atom.get_index()])
        for z_c in range(-500, 500, 100):
            z_center.set_nuisance(z_c)
            for z in range(-500, 500, 10):
                IMP.core.XYZ(atom).set_z(z)
                self.assertAlmostEqual(
                    r.unprotected_evaluate(None), r2.unprotected_evaluate(None),
                    delta=1e-4)
        self.assertEqual(r2.get_inputs(), [atom, z_center.get_particle()])

    def test_above(self):
        m = IMP.Model()
        atom = IMP.Particle(m)
        d = IMP.core.XYZ.setup_particle(atom)
        p = IMP.Particle(m)

        z_center = IMP.isd.Nuisance.setup_particle(p)
        z_center.set_nuisance(0.0)

        r = MembraneRestraintPrototype(m, z_center)
        r.add_particles_above([atom])

        r2 = IMP.pmi.MembraneRestraint(
            m, z_center.get_particle_index(), 30.0, 3.0, 0.0000000001, 0.02)
        r2.set_was_used(True)
        r2.add_particles_above([atom.get_index()])
        for z_c in range(-500, 500, 100):
            z_center.set_nuisance(z_c)
            for z in range(-500, 500, 10):
                IMP.core.XYZ(atom).set_z(z)
                self.assertAlmostEqual(
                    r.unprotected_evaluate(None), r2.unprotected_evaluate(None),
                    delta=1e-4)

    def test_below(self):
        m = IMP.Model()
        atom = IMP.Particle(m)
        d = IMP.core.XYZ.setup_particle(atom)
        p = IMP.Particle(m)

        z_center = IMP.isd.Nuisance.setup_particle(p)
        z_center.set_nuisance(0.0)

        r = MembraneRestraintPrototype(m, z_center)
        r.add_particles_below([atom])

        r2 = IMP.pmi.MembraneRestraint(
            m, z_center.get_particle_index(), 30.0, 3.0, 0.0000000001, 0.02)
        r2.set_was_used(True)
        r2.add_particles_below([atom.get_index()])

        for z_c in range(-500, 500, 100):
            z_center.set_nuisance(z_c)
            for z in range(-500, 500, 10):
                IMP.core.XYZ(atom).set_z(z)
                self.assertAlmostEqual(
                    r.unprotected_evaluate(None), r2.unprotected_evaluate(None),
                    delta=1e-4)

    def test_membrane_side_info(self):
        """Restrained particles record their side and the membrane center"""
        m = IMP.Model()
        s = IMP.pmi.topology.System(m)
        st = s.create_state()
        mol = st.create_molecule("helix", sequence='A' * 40, chain_id='A')
        mol.add_representation(mol, resolutions=[1], ideal_helix=True)
        hier = s.build()
        # residue 10 is both above and inside.
        # Inside overwrites
        mr = IMP.pmi.restraints.basic.MembraneRestraint(
            hier, objects_above=[(1, 10, 'helix')],
            objects_inside=[(10, 30, 'helix')],
            objects_below=[(31, 40, 'helix')], center=5.0)
        side_key = IMP.pmi.tools._membrane_side_key
        center_key = IMP.pmi.tools._membrane_center_key
        self.assertIsInstance(side_key, IMP.SparseIntKey)
        self.assertIsInstance(center_key, IMP.SparseFloatKey)

        def sides(first, last):
            sel = IMP.atom.Selection(hier, molecule='helix',
                                     residue_indexes=range(first, last + 1))
            return set(p.get_value(side_key)
                       for p in sel.get_selected_particles())
        self.assertEqual(sides(1, 9), {1})
        self.assertEqual(sides(10, 30), {0})
        self.assertEqual(sides(31, 40), {-1})
        for p in mr.get_particles_inside():
            self.assertAlmostEqual(p.get_value(center_key), 5.0, delta=1e-6)

    def _make_membrane_helix(self, center=0.0):
        """Helix 1-30 in one rigid body (1-10 above, 11-20 inside, 21-30
        below), 31-40 flexible beads"""
        m = IMP.Model()
        s = IMP.pmi.topology.System(m)
        st = s.create_state()
        mol = st.create_molecule("helix", sequence='A' * 40, chain_id='A')
        mol.add_representation(mol[0:30], resolutions=[1], ideal_helix=True)
        mol.add_representation(mol[30:40], resolutions=[1])
        hier = s.build()

        dof = IMP.pmi.dof.DegreesOfFreedom(m)
        rb = dof.create_rigid_body(mol[0:30])[1]
        dof.create_flexible_beads(mol[30:40])
        IMP.pmi.restraints.basic.MembraneRestraint(
            hier, objects_above=[(1, 10, 'helix')],
            objects_inside=[(11, 20, 'helix')],
            objects_below=[(21, 30, 'helix')], center=center)
        beads = [IMP.core.XYZ(p) for p in IMP.atom.Selection(
            hier, molecule='helix',
            residue_indexes=range(31, 41)).get_selected_particles()]
        IMP.random_number_generator.seed(7)
        t = IMP.algebra.Transformation3D(
            IMP.algebra.get_random_rotation_3d(),
            IMP.algebra.Vector3D(40, -70, 120))
        IMP.core.transform(rb, t)
        for b in beads:
            IMP.core.transform(b, t)
        return m, hier, rb, beads

    def _get_centroid(self, hier, first, last):
        sel = IMP.atom.Selection(hier, molecule='helix',
                                 residue_indexes=range(first, last + 1))
        return IMP.algebra.get_centroid(
            [IMP.core.XYZ(p).get_coordinates()
             for p in sel.get_selected_particles()])

    def test_place_in_membrane(self):
        """Rigid body is turned right way up and centered in the membrane"""
        for center in (0.0, 12.0):
            m, hier, rb, beads = self._make_membrane_helix(center)
            member = IMP.core.XYZ(rb.get_rigid_members()[0])
            bead_distances = [IMP.core.get_distance(member, b) for b in beads]
            IMP.pmi.tools.place_in_membrane(hier)
            inside = self._get_centroid(hier, 11, 20)
            normal = (self._get_centroid(hier, 1, 10)
                      - self._get_centroid(hier, 21, 30)).get_unit_vector()
            self.assertAlmostEqual(inside[2], center, delta=1e-4)
            self.assertAlmostEqual(normal[2], 1.0, delta=1e-4)
            # flexible beads move with the rigid body
            for b, d in zip(beads, bead_distances):
                self.assertAlmostEqual(IMP.core.get_distance(member, b), d,
                                       delta=1e-4)

    def test_place_in_membrane_several_restraints(self):
        """A rigid body over two molecules, each with its own restraint
        """
        m = IMP.Model()
        s = IMP.pmi.topology.System(m)
        st = s.create_state()
        for name in ('top', 'bottom'):
            mol = st.create_molecule(name, sequence='A' * 20)
            mol.add_representation(mol, resolutions=[1], ideal_helix=True)
        hier = s.build()
        dof = IMP.pmi.dof.DegreesOfFreedom(m)
        rb = dof.create_rigid_body(
            IMP.atom.Selection(hier).get_selected_particles())[1]
        # 'top' has no particles below, 'bottom' none above
        IMP.pmi.restraints.basic.MembraneRestraint(
            hier, objects_above=[(1, 10, 'top')],
            objects_inside=[(11, 20, 'top')])
        IMP.pmi.restraints.basic.MembraneRestraint(
            hier, objects_below=[(11, 20, 'bottom')],
            objects_inside=[(1, 10, 'bottom')])
        IMP.random_number_generator.seed(3)
        IMP.core.transform(rb, IMP.algebra.Transformation3D(
            IMP.algebra.get_random_rotation_3d(),
            IMP.algebra.Vector3D(0, 0, 80)))
        IMP.pmi.tools.place_in_membrane([hier])

        def centroid(name, first, last):
            sel = IMP.atom.Selection(hier, molecule=name,
                                     residue_indexes=range(first, last + 1))
            return IMP.algebra.get_centroid(
                [IMP.core.XYZ(p).get_coordinates()
                 for p in sel.get_selected_particles()])
        normal = (centroid('top', 1, 10)
                  - centroid('bottom', 11, 20)).get_unit_vector()
        self.assertAlmostEqual(normal[2], 1.0, delta=1e-4)

    def test_shuffle_in_membrane(self):
        """Shuffling moves only x, y: z and orientation are kept"""
        m, hier, rb, beads = self._make_membrane_helix()
        IMP.pmi.tools.place_in_membrane(hier)
        coords = [IMP.core.XYZ(p).get_coordinates()
                  for p in rb.get_rigid_members()]
        rotation = rb.get_reference_frame().get_transformation_to() \
            .get_rotation()
        IMP.pmi.tools.shuffle_in_membrane(
            hier, bounding_box=((100, 100), (200, 200)))
        inside = self._get_centroid(hier, 11, 20)
        self.assertTrue(100 <= inside[0] <= 200 and 100 <= inside[1] <= 200)
        for p, c in zip(rb.get_rigid_members(), coords):
            self.assertAlmostEqual(IMP.core.XYZ(p).get_z(), c[2], delta=1e-4)
        new_rotation = rb.get_reference_frame().get_transformation_to() \
            .get_rotation()
        self.assertLess(IMP.algebra.get_distance(rotation, new_rotation), 1e-4)

    def test_shuffle_in_membrane_no_overlap(self):
        """Bodies shuffled in the membrane do not overlap"""
        m = IMP.Model()
        s = IMP.pmi.topology.System(m)
        st = s.create_state()
        dof = IMP.pmi.dof.DegreesOfFreedom(m)
        mols = []
        for name in ('h1', 'h2', 'h3'):
            mol = st.create_molecule(name, sequence='A' * 30)
            mol.add_representation(mol, resolutions=[1], ideal_helix=True)
            mols.append(mol)

        hier = s.build()
        rbs = [dof.create_rigid_body(mol)[1] for mol in mols]
        for name in ('h1', 'h2', 'h3'):
            IMP.pmi.restraints.basic.MembraneRestraint(
                hier, objects_above=[(1, 10, name)],
                objects_inside=[(11, 20, name)],
                objects_below=[(21, 30, name)])
        IMP.pmi.tools.place_in_membrane(hier)
        IMP.random_number_generator.seed(5)
        IMP.pmi.tools.shuffle_in_membrane(
            hier, bounding_box=((-300, -300), (300, 300)))

        def disc(rb):
            xy = [IMP.algebra.Vector2D(IMP.core.XYZ(p).get_x(),
                                       IMP.core.XYZ(p).get_y())
                  for p in rb.get_rigid_members()]
            center = IMP.algebra.get_centroid(
                [IMP.algebra.Vector3D(v[0], v[1], 0) for v in xy])
            center = IMP.algebra.Vector2D(center[0], center[1])
            return center, max(IMP.algebra.get_distance(center, v)
                               for v in xy)
        discs = [disc(rb) for rb in rbs]
        for i in range(len(discs)):
            for j in range(i + 1, len(discs)):
                (ci, ri), (cj, rj) = discs[i], discs[j]
                self.assertGreater(IMP.algebra.get_distance(ci, cj), ri + rj)

if __name__ == '__main__':
    IMP.test.main()
