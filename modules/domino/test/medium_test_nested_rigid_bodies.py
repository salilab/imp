import IMP
import IMP.test
import IMP.domino
import IMP.core
import IMP.container


class Tests(IMP.test.TestCase):

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(IMP.SILENT)  # TERSE)#VERBOSE)

    def test_docking_solutions(self):
        """Test nested rigid bodies"""
        # load components
        mdl = IMP.Model()
        mhs = []
        rbs = []
        aps = []
        for i in range(3):
            fn = "small_protein.pdb"
            ps = IMP._create_particles_from_pdb(
                self.get_input_file_name(fn),
                mdl)
            aps.extend(ps)
            p = IMP.Particle(mdl)
            rb = IMP.core.RigidBody.setup_particle(p, ps)
            mhs.append(rb)
            mhs[-1].set_name("molecule" + str(i))
            rbs.append(rb)
        ts = []
        bb = IMP.algebra.BoundingBox3D(
            IMP.algebra.Vector3D(-10., -10., -10.),
            IMP.algebra.Vector3D(10., 10., 10.))
        for i in range(5):
            ts.append(IMP.algebra.Transformation3D(
                IMP.algebra.get_random_rotation_3d(),
                IMP.algebra.get_random_vector_in(bb)))
        # set nesting
        for i in range(len(rbs) - 1):
            rbs[0].add_member(rbs[i + 1])
        # set ev
        IMP.set_log_level(IMP.SILENT)
        ls = IMP.container.ListSingletonContainer(mdl, aps)
        sev = IMP.core.ExcludedVolumeRestraint(ls)
        sev.evaluate(False)
        # set states
        pst = IMP.domino.ParticleStatesTable()
        for i in range(1):
            states = IMP.domino.NestedRigidBodyStates(ts)
            pst.set_particle_states(rbs[i + 1], states)
        id_trans = []
        id_trans.append(rbs[0].get_reference_frame())
        pst.set_particle_states(rbs[0], IMP.domino.RigidBodyStates(id_trans))
        ds = IMP.domino.DominoSampler(mdl, pst)
        ds.set_restraints([sev])
        cg = ds.create_sample()
        print(cg.get_number_of_configurations())

if __name__ == '__main__':
    IMP.test.main()
