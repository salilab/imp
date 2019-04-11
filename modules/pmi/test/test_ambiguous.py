from __future__ import print_function
import IMP
import IMP.test
import IMP.core
import os
import IMP.pmi.representation
import IMP.pmi.restraints.crosslinking
import IMP.pmi.samplers
import IMP.pmi.output

class Tests(IMP.test.TestCase):
    def test_ambiguous(self):
        representations = []

        m = IMP.Model()
        with IMP.allow_deprecated():
            r = IMP.pmi.representation.Representation(m)

        r.create_component("particle1", color=0.1)
        p11 = r.add_component_beads("particle1", [(1, 10)])
        r.create_component("particle2", color=0.5)
        p21 = r.add_component_beads("particle2", [(1, 10)])
        r.create_component("particle3", color=0.9)
        p31 = r.add_component_beads("particle3", [(1, 10)])

        representations.append(r)

        with IMP.allow_deprecated():
            r = IMP.pmi.representation.Representation(m)

        representations[0].floppy_bodies.pop(0)
        representations[0].floppy_bodies.pop(0)

        print(representations[0].floppy_bodies)

        pp1 = IMP.atom.get_leaves(p11[0])[0]
        pp2 = IMP.atom.get_leaves(p21[0])[0]
        pp3 = IMP.atom.get_leaves(p31[0])[0]
        xyz11 = IMP.core.XYZ(pp1.get_particle())
        xyz21 = IMP.core.XYZ(pp2.get_particle())
        xyz31 = IMP.core.XYZ(pp3.get_particle())
        xyz11.set_coordinates((0, 0, 0))
        print(xyz11.get_coordinates())
        xyz21.set_coordinates((40, 0, 0))
        xyz31.set_coordinates((0, 0, 0))

        tname = self.get_tmp_file_name("test.txt")
        with open(tname, "w") as fh:
            fh.write("prot1,prot2,res1,res2,uniq,Sigma1,Sigma2,Psi\n")
            fh.write("particle2,particle3,1,5,1,1.0,1.0,0.05\n")
            fh.write("particle1,particle3,1,2,1,1.0,1.0,0.05\n")
        cldbkc=IMP.pmi.io.crosslink.CrossLinkDataBaseKeywordsConverter()
        cldbkc.set_protein1_key("prot1")
        cldbkc.set_protein2_key("prot2")
        cldbkc.set_residue1_key("res1")
        cldbkc.set_residue2_key("res2")
        cldbkc.set_unique_id_key("uniq")
        cldb = IMP.pmi.io.crosslink.CrossLinkDataBase(cldbkc)
        cldb.create_set_from_file(tname)
        self.assertEqual(cldb.get_number_of_xlid(), 1)

        xl = \
          IMP.pmi.restraints.crosslinking.CrossLinkingMassSpectrometryRestraint(
                representations, CrossLinkDataBase=cldb,
                length=21.0, slope=0.0, resolution=1.0)
        xl.sigma_dictionary['1.0'][0].set_scale(5.0)
        xl.psi_dictionary['0.05'][0].set_scale(0.1)

        out_dict = xl.get_output()
        sorted_keys = sorted(out_dict.keys())
        for entry in sorted_keys:
            print(entry, out_dict[entry])

        print(xyz11.get_coordinates())

        xl.add_to_model()
        rset = IMP.pmi.tools.get_restraint_set(m)
        self.assertAlmostEqual(rset.evaluate(False), -2.20827, delta=1e-2)

        print(xyz11.get_coordinates())

        mc = IMP.pmi.samplers.MonteCarlo(m, representations, 1.0)
        mc.set_simulated_annealing(min_temp=1.0,
                                   max_temp=2.0,
                                   min_temp_time=200,
                                   max_temp_time=50)


        o = IMP.pmi.output.Output()
        o.init_stat2("modeling.stat", [mc, xl] + representations)

        for i in range(0, 100):
            mc.optimize(10)
            o.write_stats2()
            if i % 100 == 0:
                print(i)

        po = IMP.pmi.output.ProcessOutput("modeling.stat")

        print(po.get_keys())
        self.assertEqual(len(po.get_keys()), 15)

        fs = po.get_fields(
            ['CrossLinkingMassSpectrometryRestraint_Distance_||1.2|particle1|1|particle3|2|0|0.05|',
             'CrossLinkingMassSpectrometryRestraint_Distance_||1.1|particle2|1|particle3|5|0|0.05|',
             'CrossLinkingMassSpectrometryRestraint_Data_Score',
             'CrossLinkingMassSpectrometryRestraint_Linear_Score',
             'CrossLinkingMassSpectrometryRestraint_Psi_0.05'])

        for output in ["excluded.None.xl.db", "included.None.xl.db",
                       "missing.None.xl.db", "modeling.stat"]:
            os.unlink(output)

if __name__ == '__main__':
    IMP.test.main()
