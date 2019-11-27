from __future__ import print_function
import IMP
import IMP.test
import os
import IMP.pmi.topology
import IMP.pmi.dof
import IMP.pmi.restraints.basic
import IMP.core
import IMP.pmi.restraints.crosslinking
import IMP.pmi.samplers
import IMP.pmi.output

class Tests(IMP.test.TestCase):
    def test_multistate(self):
        psiv=0.01
        sigmav=12.0
        slope=0.0
        length=21
        inputx=70

        m = IMP.Model()
        s = IMP.pmi.topology.System(m)

        # define the particles in state 1
        st1 = s.create_state()
        mol1 = []
        for mn in range(1, 4):
            mol = st1.create_molecule("particle%d" % mn, "G" * 10, "ABC"[mn-1])
            mol.add_representation(resolutions=[10])
            mol1.append(mol)

        # define the particles in state 2
        st2 = s.create_state()
        mol2 = []
        for mn in range(1, 4):
            mol = st2.create_molecule("particle%d" % mn, "G" * 10, "ABC"[mn-1])
            mol.add_representation(resolutions=[10])
            mol2.append(mol)

        hier = s.build()

        dof = IMP.pmi.dof.DegreesOfFreedom(m)

        # only allow particle3 in each state to move
        dof.create_flexible_beads(mol1[2])
        dof.create_flexible_beads(mol2[2])

        pp1, pp2, pp3 = IMP.atom.get_leaves(st1.hier)

        xyz11 = IMP.core.XYZ(pp1)
        xyz21 = IMP.core.XYZ(pp2)
        xyz31 = IMP.core.XYZ(pp3)
        xyz11.set_coordinates((0, 0, 0))
        print(xyz11.get_coordinates())
        xyz21.set_coordinates((inputx, 0, 0))
        xyz31.set_coordinates((0, 0, 0))

        pp1, pp2, pp3 = IMP.atom.get_leaves(st2.hier)
        xyz12 = IMP.core.XYZ(pp1)
        xyz22 = IMP.core.XYZ(pp2)
        xyz32 = IMP.core.XYZ(pp3)
        xyz12.set_coordinates((0, 0, 0))
        xyz22.set_coordinates((inputx, 0, 0))
        xyz32.set_coordinates((inputx, 0, 0))

        eb = IMP.pmi.restraints.basic.ExternalBarrier(hierarchies=st1.hier,
                                                      radius=1000)
        eb.add_to_model()

        eb = IMP.pmi.restraints.basic.ExternalBarrier(hierarchies=st2.hier,
                                                      radius=1000)
        eb.add_to_model()

        xldbkwc = IMP.pmi.io.crosslink.CrossLinkDataBaseKeywordsConverter()
        xldbkwc.set_protein1_key("prot1")
        xldbkwc.set_protein2_key("prot2")
        xldbkwc.set_residue1_key("res1")
        xldbkwc.set_residue2_key("res2")
        xldbkwc.set_unique_id_key("id")

        cldb = IMP.pmi.io.crosslink.CrossLinkDataBase(xldbkwc)
        cldb.create_set_from_file(self.get_input_file_name('multistate.csv'))

        xl = IMP.pmi.restraints.crosslinking.CrossLinkingMassSpectrometryRestraint(
            root_hier=hier, CrossLinkDataBase=cldb, length=length, slope=slope,
            resolution=1.0)

        psi = xl.psi_dictionary['PSI'][0]
        psi.set_scale(psiv)

        sigma = xl.sigma_dictionary['SIGMA'][0]
        sigma.set_scale(sigmav)

        xl.set_psi_is_sampled(False)
        xl.set_sigma_is_sampled(False)

        # psi.set_scale(0.1)
        # xl.get_sigma(1.0)

        out_dict = xl.get_output()
        sorted_keys = sorted(out_dict.keys())
        for entry in sorted_keys:
            print(entry, out_dict[entry])

        print(xyz11.get_coordinates())

        xl.add_to_model()
        rset = IMP.pmi.tools.get_restraint_set(m)
        self.assertAlmostEqual(rset.evaluate(False), -3.03166, delta=1e-2)

        o = IMP.pmi.output.Output()
        o.init_rmf(
            "trajectory.rmf3",
            [st1.hier, st2.hier])

        print(o.dictionary_rmfs)

        mc = IMP.pmi.samplers.MonteCarlo(m, dof.get_movers(), 1.0)
        mc.set_simulated_annealing(min_temp=1.0,
                                   max_temp=2.0,
                                   min_temp_time=200,
                                   max_temp_time=50)

        o.init_stat2("modeling.stat", [mc, xl])


        for i in range(1,20):
            xyz31.set_coordinates((float(i), 0, 0))
            for j in range(1,20):
                xyz32.set_coordinates((float(j), 0, 0))
                print(i,j,rset.evaluate(False))
                o.write_stats2()


        po = IMP.pmi.output.ProcessOutput("modeling.stat")
        print(po.get_keys())

        self.assertEqual(len(po.get_keys()), 14)

        fs = po.get_fields(
            ['CrossLinkingMassSpectrometryRestraint_Distance_||2.1|particle1|5|particle3|5|1|PSI|',
             'CrossLinkingMassSpectrometryRestraint_Distance_||1.1|particle2|5|particle3|5|1|PSI|',
             'CrossLinkingMassSpectrometryRestraint_Data_Score',
             'CrossLinkingMassSpectrometryRestraint_Linear_Score',
             'CrossLinkingMassSpectrometryRestraint_Psi_PSI'])
        print(fs.keys())
        o.close_rmf("trajectory.rmf3")

        for output in ['excluded.None.xl.db', 'included.None.xl.db',
                       'missing.None.xl.db', 'modeling.stat',
                       'trajectory.rmf3']:
            os.unlink(output)
if __name__ == '__main__':
    IMP.test.main()
