import IMP
import IMP.test
import IMP.pmi.topology
import IMP.pmi.macros
import IMP.pmi.restraints.basic
import IMP.pmi.dof
import shutil
import os
import ast
try:
    import jax
except ImportError:
    jax = None


class JAXDistanceRestraint(IMP.Restraint):
    """A simple distance restraint that has only a JAX implementation"""
    def __init__(self, m, p1, p2, d, k):
        super().__init__(m, "JAXDistanceRestraint%1%")
        self.m, self.p1, self.p2, self.d, self.k = m, p1, p2, d, k

    def do_add_score_and_derivatives(self, sa):
        # No IMP implementation
        raise NotImplementedError("JAX only restraint")

    def do_get_inputs(self):
        return [self.p1, self.p2]

    def _get_jax(self):
        import jax.numpy as jnp
        import functools

        def score(jm, indexes, d, k):
            xyzs = jm['xyz'][indexes]
            drs = jnp.linalg.norm(xyzs[0] - xyzs[1])
            return 0.5 * k * (d - drs)** 2
        f = functools.partial(
            score,
            indexes=jnp.asarray((self.p1.get_index(), self.p2.get_index())),
            d=self.d, k=self.k)
        return self._wrap_jax(f)


class JAXOnlyRestraint(IMP.pmi.restraints.RestraintBase):
    """PMI wrapper for JAX-only distance restraint"""
    def __init__(self, root_hier, tuple_selection1, tuple_selection2,
                 distance, label=None, resolution=1., weight=1.):
        model = root_hier.get_model()
        super().__init__(model, label=label, weight=weight)
        sel1 = IMP.atom.Selection(root_hier,
                                  resolution=resolution,
                                  molecule=tuple_selection1[2],
                                  residue_index=tuple_selection1[0])
        particles1 = sel1.get_selected_particles()
        sel2 = IMP.atom.Selection(root_hier,
                                  resolution=resolution,
                                  molecule=tuple_selection2[2],
                                  residue_index=tuple_selection2[0])
        particles2 = sel2.get_selected_particles()
        if len(particles1) > 1 or len(particles2) > 1:
            raise ValueError("more than one particle selected")
        self.rs.add_restraint(
            JAXDistanceRestraint(self.model, particles1[0], particles2[0],
                                 distance, 20.0))


class MockSampler:
    def get_number_of_replicas(self):
        return 3


class MockREX(IMP.pmi.macros.ReplicaExchange):
    # Override the slow init from the base class
    def __init__(self, model):
        self.model = model
        self.vars = {'molecular_dynamics_steps':12,
                     'monte_carlo_steps':23,
                     'num_sample_rounds':2,
                     'number_of_frames':100}
        self.replica_exchange_object = MockSampler()


class StatFileBase:
    def __init__(self, header_line):
        self._d = ast.literal_eval(header_line)
        self._keymap = {k: v for (k, v) in self._d.items()
                        if isinstance(k, int)}
        self._eqkeys = [k for (k, v) in self._keymap.items()
                        if v not in ('Stopwatch_None_delta_seconds',
                                     'rmf_file', 'rmf_frame_index')]

    def same_header(self, other):
        return self._keymap == other._keymap

    def assert_same_line(self, line1, line2, tf):
        # Reached EOF in one file
        tf.assertTrue(line1 != '', 'stat file 1 reached EOF')
        tf.assertTrue(line2 != '', 'stat file 2 reached EOF')

        line1 = ast.literal_eval(line1)
        line2 = ast.literal_eval(line2)
        # All keys should be exactly equal
        for k in self._eqkeys:
            tf.assertEqual(line1[k], line2[k],
                           "%r != %r for stat file keyword %r"
                           % (line1[k], line2[k], self._keymap[k]))
        return line1, line2


class StatFile(StatFileBase):
    def __init__(self, header_line):
        super().__init__(header_line)
        revkeymap = {v: k for (k, v) in self._d.items()
                     if isinstance(k, int)}
        self._rmf_key = revkeymap['rmf_file']
        self._rmf_frame_key = revkeymap['rmf_frame_index']

    def assert_same_line(self, line1, line2, tf):
        line1, line2 = super().assert_same_line(line1, line2, tf)
        return (line1[self._rmf_key], line2[self._rmf_key],
                line1[self._rmf_frame_key], line2[self._rmf_frame_key])


class ReplicaStatFile(StatFileBase):
    pass


class Tests(IMP.test.TestCase):

    def test_add_provenance_none(self):
        """Test adding replica exchange provenance, no MD or MC"""
        m = IMP.Model()
        h = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        r = MockREX(m)
        r.root_hier = h
        r._add_provenance(sampler_md=None, sampler_mc=None)
        # No provenance should have been added, since no sampling is done
        self.assertFalse(IMP.core.Provenanced.get_is_setup(h))

    def test_add_provenance_md(self):
        """Test adding replica exchange provenance, MD"""
        m = IMP.Model()
        h = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        r = MockREX(m)
        r.root_hier = h
        r._add_provenance(sampler_md=True, sampler_mc=None)
        self._check_provenance(r, h, "Molecular Dynamics", iterations=24)

    def test_add_provenance_mc(self):
        """Test adding replica exchange provenance, MC"""
        m = IMP.Model()
        h = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        r = MockREX(m)
        r.root_hier = h
        r._add_provenance(sampler_md=None, sampler_mc=True)
        self._check_provenance(r, h, "Monte Carlo", iterations=46)

    def test_add_provenance_md_mc(self):
        """Test adding replica exchange provenance, MD/MC"""
        m = IMP.Model()
        h = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        r = MockREX(m)
        r.root_hier = h
        r._add_provenance(sampler_md=True, sampler_mc=True)
        self._check_provenance(r, h, "Hybrid MD/MC", iterations=70)

    def _check_provenance(self, rex, hier, method, iterations):
        self.assertTrue(IMP.core.Provenanced.get_is_setup(hier))
        prov = IMP.core.Provenanced(hier).get_provenance()
        self.assertTrue(IMP.core.SampleProvenance.get_is_setup(prov))
        prov = IMP.core.SampleProvenance(prov)
        self.assertEqual(prov.get_method(), method)
        self.assertEqual(prov.get_number_of_frames(), 100)
        self.assertEqual(prov.get_number_of_iterations(), iterations)
        self.assertEqual(prov.get_number_of_replicas(), 3)

    def test_test_save_coordinates_mode(self):
        """Test ReplicaExchange test_mode with save_coordinates_mode"""
        m = IMP.Model()
        s = IMP.pmi.topology.System(m)
        st1 = s.create_state()
        nup84 = st1.create_molecule("Nup84", "MELS", "X")
        nup84.add_representation(resolutions=[1])
        hier = s.build()

        dof = IMP.pmi.dof.DegreesOfFreedom(m)
        dof.create_flexible_beads(nup84)
        rex = IMP.pmi.macros.ReplicaExchange(
            m, root_hier=hier,
            monte_carlo_sample_objects=dof.get_movers(), number_of_frames=2,
            test_mode=True, save_coordinates_mode="25th_score")
        rex.execute_macro()

    def test_adaptive(self):
        """Test ReplicaExchange with self-adaptive sampling"""
        m = IMP.Model()
        s = IMP.pmi.topology.System(m)
        st1 = s.create_state()
        nup84 = st1.create_molecule("Nup84", "MELS", "X")
        nup84.add_structure(self.get_input_file_name("test.nup84.pdb"), "A")
        nup84.add_representation(resolutions=[1])

        hier = s.build()

        dof = IMP.pmi.dof.DegreesOfFreedom(nup84)
        dof.create_flexible_beads(nup84, max_trans=1.0, resolution=1)

        dr1 = IMP.pmi.restraints.basic.DistanceRestraint(
            root_hier=hier, tuple_selection1=(2,2,"Nup84"),
            tuple_selection2=(3,3,"Nup84"), distancemin=10, distancemax=10)
        dr1.add_to_model()

        rex = IMP.pmi.macros.ReplicaExchange(
            m, root_hier=hier, monte_carlo_steps=100, number_of_frames=1,
            output_objects=[dr1],
            monte_carlo_sample_objects=dof.get_movers(),
            number_of_best_scoring_models=0,
            monte_carlo_temperature=0.0,
            self_adaptive=True,
            global_output_directory='test_adaptive/')
        rex.execute_macro()
        shutil.rmtree('test_adaptive')

    def test_restart(self):
        """Test restart of ReplicaExchange"""
        def setup_system():
            m = IMP.Model()
            s = IMP.pmi.topology.System(m)
            st1 = s.create_state()
            nup84 = st1.create_molecule("Nup84", "MELS", "X")
            nup84.add_structure(
                self.get_input_file_name("test.nup84.pdb"), "A")
            nup84.add_representation(resolutions=[1])
            hier = s.build()
            dof = IMP.pmi.dof.DegreesOfFreedom(nup84)
            dof.create_flexible_beads(nup84, max_trans=1.0, resolution=1)

            dr1 = IMP.pmi.restraints.basic.DistanceRestraint(
                root_hier=hier, tuple_selection1=(2,2,"Nup84"),
                tuple_selection2=(3,3,"Nup84"), distancemin=10, distancemax=10)
            dr1.add_to_model()
            return m, hier, dr1, dof

        IMP.random_number_generator.seed(42)
        m, hier, dr1, dof = setup_system()
        rex = IMP.pmi.macros.ReplicaExchange(
            m, root_hier=hier, monte_carlo_steps=10, number_of_frames=10,
            output_objects=[dr1],
            monte_carlo_sample_objects=dof.get_movers(),
            number_of_best_scoring_models=0,
            monte_carlo_temperature=1.0,
            global_output_directory='test_full')
        rex.execute_macro()

        IMP.random_number_generator.seed(42)
        m, hier, dr1, dof = setup_system()
        rex = IMP.pmi.macros.ReplicaExchange(
            m, root_hier=hier, monte_carlo_steps=10, number_of_frames=10,
            output_objects=[dr1],
            monte_carlo_sample_objects=dof.get_movers(),
            number_of_best_scoring_models=0,
            monte_carlo_temperature=1.0,
            global_output_directory='test_restart')
        rex.set_restart(7)
        rex.execute_macro()
        IMP.pmi.macros.restart_replica_exchange('test_restart/restart')

        # Restarted simulation should yield same outputs as the original
        # (except for the RMF files, which will have some duplicated frames)
        self._compare_stat('test_full/stat.0.out',
                           'test_restart/stat.0.out')
        self._compare_replica_stat('test_full/stat_replica.0.out',
                                   'test_restart/stat_replica.0.out')

        self.assertTrue(os.path.exists('test_full/rmfs/0.rmf3'))
        self.assertTrue(os.path.exists('test_restart/rmfs/0.rmf3'))
        self.assertTrue(os.path.exists('test_restart/rmfs/0.rs1.rmf3'))
        shutil.rmtree('test_restart')
        shutil.rmtree('test_full')

    def _compare_stat(self, full, restart):
        with open(full) as fh_full, open(restart) as fh_rst:
            sf_full = StatFile(fh_full.readline())
            sf_rst = StatFile(fh_rst.readline())
            self.assertTrue(sf_full.same_header(sf_rst))
            for i in range(10):
                line_full = fh_full.readline()
                line_rst = fh_rst.readline()
                (rmf_full, rmf_rst, frame_full, frame_rst) \
                    = sf_full.assert_same_line(line_full, line_rst, self)
                # Restarted simulation should change to a new RMF at frame 7
                self.assertTrue(rmf_full.endswith('0.rmf3'))
                self.assertEqual(frame_full, i)
                if i < 7:
                    self.assertTrue(rmf_rst.endswith('0.rmf3'))
                    self.assertEqual(frame_rst, i)
                else:
                    self.assertTrue(rmf_rst.endswith('0.rs1.rmf3'))
                    self.assertEqual(frame_rst, i - 7)
            # Should have reached the end of each file
            self.assertEqual(fh_full.readline(), "")
            self.assertEqual(fh_rst.readline(), "")

    def _compare_replica_stat(self, full, restart):
        with open(full) as fh_full, open(restart) as fh_rst:
            sf_full = ReplicaStatFile(fh_full.readline())
            sf_rst = ReplicaStatFile(fh_rst.readline())
            self.assertTrue(sf_full.same_header(sf_rst))
            for i in range(10):
                line_full = fh_full.readline()
                line_rst = fh_rst.readline()
                sf_full.assert_same_line(line_full, line_rst, self)
            # Should have reached the end of each file
            self.assertEqual(fh_full.readline(), "")
            self.assertEqual(fh_rst.readline(), "")

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax(self):
        """Test ReplicaExchange using JAX"""
        m = IMP.Model()
        s = IMP.pmi.topology.System(m)
        st1 = s.create_state()
        nup84 = st1.create_molecule("Nup84", "MELS", "X")
        nup84.add_structure(self.get_input_file_name("test.nup84.pdb"), "A")
        nup84.add_representation(resolutions=[1])

        hier = s.build()

        dof = IMP.pmi.dof.DegreesOfFreedom(nup84)
        dof.create_flexible_beads(nup84, max_trans=1.0, resolution=1)

        # Distance restraint that only has a JAX implementation
        dr1 = JAXOnlyRestraint(
            root_hier=hier, tuple_selection1=(1,1,"Nup84"),
            tuple_selection2=(2,2,"Nup84"), distance=10)
        dr1.add_to_model()

        # Regular IMP restraint that should have a JAX implementation too
        dr2 = IMP.pmi.restraints.basic.DistanceRestraint(
            root_hier=hier, tuple_selection1=(2,2,"Nup84"),
            tuple_selection2=(3,3,"Nup84"), distancemin=10, distancemax=10,
            kappa=40.0)
        dr2.add_to_model()

        dr3 = IMP.pmi.restraints.basic.DistanceRestraint(
            root_hier=hier, tuple_selection1=(3,3,"Nup84"),
            tuple_selection2=(4,4,"Nup84"), distancemin=10, distancemax=10,
            kappa=40.0)
        dr3.add_to_model()

        rex = IMP.pmi.macros.ReplicaExchange(
            m, root_hier=hier, monte_carlo_steps=5000, number_of_frames=2,
            output_objects=[dr1, dr2],
            monte_carlo_sample_objects=dof.get_movers(),
            number_of_best_scoring_models=0,
            monte_carlo_temperature=0.0,
            global_output_directory='test_jax/', use_jax=True)
        rex.execute_macro()

        # Both the JAX-only restraint and the IMP restraint
        # (with a JAX implementation) should be satisfied
        leaves = IMP.atom.get_leaves(hier)
        r1, r2, r3 = leaves[:3]
        dist = IMP.algebra.get_distance(IMP.core.XYZ(r1).get_coordinates(),
                                        IMP.core.XYZ(r2).get_coordinates())
        self.assertAlmostEqual(dist, 10.0, delta=2.0)

        dist = IMP.algebra.get_distance(IMP.core.XYZ(r2).get_coordinates(),
                                        IMP.core.XYZ(r3).get_coordinates())
        self.assertAlmostEqual(dist, 10.0, delta=2.0)
        shutil.rmtree('test_jax')


if __name__ == '__main__':
    IMP.test.main()
