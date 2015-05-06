import IMP
import IMP.test

import IMP.pmi.restraints.stereochemistry
import IMP.pmi.representation
import IMP.pmi.tools

class Tests(IMP.test.TestCase):
    def make_representation(self):
        pdbfile = self.get_input_file_name("nonbond.pdb")
        fastafile = self.get_input_file_name("nonbond.fasta")
        fastids = IMP.pmi.tools.get_ids_from_fasta_file(fastafile)

        m = IMP.Model()
        r = IMP.pmi.representation.Representation(m)

        r.create_component("A", color=0.)
        r.add_component_sequence("A", fastafile, id=fastids[0])
        r.autobuild_model("A", pdbfile, "A",
                          resolutions=[1, 10], missingbeadsize=1)
        return m, r

    def test_nonbond_check(self):
        """Test nonbond score.
           This test causes an internal check failure (see issue #853)"""
        m, r = self.make_representation()

        r.set_floppy_bodies()

        listofexcludedpairs = []

        rbr = IMP.pmi.restraints.stereochemistry.ResidueBondRestraint(r,
                                                          (1, 2, "A"))
        rbr.add_to_model()
        listofexcludedpairs += rbr.get_excluded_pairs()

        ev = IMP.pmi.restraints.stereochemistry.ExcludedVolumeSphere(r,
                                                    resolution=10.0)
        ev.add_excluded_particle_pairs(listofexcludedpairs)
        ev.add_to_model()
        sf = IMP.core.RestraintsScoringFunction(
                                   IMP.pmi.tools.get_restraint_set(m))
        sf.evaluate(False)

    def test_is_rigid(self):
        """Test violation of is_rigid flag.
           This test causes an internal check failure (see issue #853)"""
        m, r = self.make_representation()

        r.set_rigid_bodies(["A"])

        ev = IMP.pmi.restraints.stereochemistry.ExcludedVolumeSphere(
                                  r, resolution=10.0)
        ev.add_to_model()
        sf = IMP.core.RestraintsScoringFunction(
                                   IMP.pmi.tools.get_restraint_set(m))
        sf.evaluate(False)

if __name__ == '__main__':
    IMP.test.main()
