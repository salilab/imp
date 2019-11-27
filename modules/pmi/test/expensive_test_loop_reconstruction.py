from __future__ import print_function
import os
import IMP
import IMP.test

import IMP.pmi.restraints.stereochemistry
import IMP.pmi.topology
import IMP.pmi.dof
import IMP.pmi.tools as tools
import IMP.pmi.samplers as samplers
import IMP.pmi.output as output

IMP.set_log_level(IMP.SILENT)


class Tests(IMP.test.TestCase):
    def test_loop_reconstruction(self):
        """Test loop reconstruction"""

        # input parameter
        pdbfile = self.get_input_file_name(
                   "loop_reconstruction/starting.structure.pdb")
        fastafile = self.get_input_file_name(
                   "loop_reconstruction/sequence.fasta")
        sequences = IMP.pmi.topology.Sequences(fastafile)

        # create the representation
        log_objects = []
        optimizable_objects = []

        sw = tools.Stopwatch()
        log_objects.append(sw)

        m = IMP.Model()
        s = IMP.pmi.topology.System(m)
        st = s.create_state()

        cA = st.create_molecule("chainA", sequence=sequences[0])
        atomic = cA.add_structure(pdbfile, chain_id='A')
        cA.add_representation(atomic, resolutions=[1, 10], color=0.)
        cA.add_representation(cA.get_non_atomic_residues(),
                              resolutions=[1], color=0.)

        cB = st.create_molecule("chainB", sequence=sequences[0])
        atomic = cB.add_structure(pdbfile, chain_id='B')
        cB.add_representation(atomic, resolutions=[1, 10], color=0.5)
        cB.add_representation(cB.get_non_atomic_residues(),
                              resolutions=[1], color=0.)
        root_hier = s.build()

        dof = IMP.pmi.dof.DegreesOfFreedom(m)
        dof.create_rigid_body(cA)
        dof.create_rigid_body(cB)

        cr = IMP.pmi.restraints.stereochemistry.ConnectivityRestraint(root_hier)
        cr.add_to_model()
        log_objects.append(cr)

        listofexcludedpairs = []

        lof = [cA[:12], cB[:12],
               cA[293:339], cB[293:339],
               cA[685:701], cB[685:701],
               cA[453:464], cB[453:464],
               cA[471:486], cB[471:486],
               cA[813:859], cB[813:859]]

        # add bonds and angles
        for l in lof:

            print(l)
            rbr = IMP.pmi.restraints.stereochemistry.ResidueBondRestraint(
                objects=l)
            rbr.add_to_model()
            listofexcludedpairs += rbr.get_excluded_pairs()
            log_objects.append(rbr)

            rar = IMP.pmi.restraints.stereochemistry.ResidueAngleRestraint(
                objects=l)
            rar.add_to_model()
            listofexcludedpairs += rar.get_excluded_pairs()
            log_objects.append(rar)

        # add excluded volume

        ev = IMP.pmi.restraints.stereochemistry.ExcludedVolumeSphere(
            included_objects=root_hier, resolution=10.0)
        ev.add_excluded_particle_pairs(listofexcludedpairs)
        ev.add_to_model()
        log_objects.append(ev)

        mc = samplers.MonteCarlo(m, dof.get_movers(), 1.0)
        log_objects.append(mc)

        o = output.Output()

        print("Starting test")
        o.write_test("test.current.dict", log_objects)
        o.test(self.get_input_file_name(
                     "loop_reconstruction/test.IMP-ee1763c6.PMI-4669cfca.dict"),
               log_objects)
        os.unlink('test.current.dict')

if __name__ == '__main__':
    IMP.test.main()
