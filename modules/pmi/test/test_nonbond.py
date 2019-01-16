import IMP
import IMP.test

import IMP.pmi.restraints.stereochemistry
import IMP.pmi.topology
import IMP.pmi.dof
import IMP.pmi.tools

class Tests(IMP.test.TestCase):
    def make_representation(self):
        pdbfile = self.get_input_file_name("nonbond.pdb")

        m = IMP.Model()
        s = IMP.pmi.topology.System(m)
        state = s.create_state()

        c = state.create_molecule("A", sequence='KF')
        struc = c.add_structure(pdbfile, chain_id="A", offset=-10)
        c.add_representation(struc, resolutions=[1, 10])
        root_hier = s.build()
        return m, root_hier

    def test_nonbond_check(self):
        """Test nonbond score.
           This test causes an internal check failure (see issue #853)"""
        m, r = self.make_representation()

        listofexcludedpairs = []
        chainA = IMP.atom.get_by_type(r, IMP.atom.CHAIN_TYPE)[0]
        rbr = IMP.pmi.restraints.stereochemistry.ResidueBondRestraint(
                                                            objects=chainA)
        rbr.add_to_model()
        listofexcludedpairs += rbr.get_excluded_pairs()

        ev = IMP.pmi.restraints.stereochemistry.ExcludedVolumeSphere(
                                         included_objects=r, resolution=10.0)
        ev.add_excluded_particle_pairs(listofexcludedpairs)
        ev.add_to_model()
        sf = IMP.core.RestraintsScoringFunction(
                                   IMP.pmi.tools.get_restraint_set(m))
        sf.evaluate(False)

    def test_is_rigid(self):
        """Test violation of is_rigid flag.
           This test causes an internal check failure (see issue #853)"""
        m, r = self.make_representation()

        chainA = IMP.atom.get_by_type(r, IMP.atom.CHAIN_TYPE)[0]
        dof = IMP.pmi.dof.DegreesOfFreedom(m)
        rb1 = dof.create_rigid_body(chainA)

        ev = IMP.pmi.restraints.stereochemistry.ExcludedVolumeSphere(
                                  included_objects=r, resolution=10.0)
        ev.add_to_model()
        sf = IMP.core.RestraintsScoringFunction(
                                   IMP.pmi.tools.get_restraint_set(m))
        sf.evaluate(False)

if __name__ == '__main__':
    IMP.test.main()
