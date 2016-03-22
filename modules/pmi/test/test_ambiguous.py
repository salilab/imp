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
        r = IMP.pmi.representation.Representation(m)

        r.create_component("particle1", color=0.1)
        p11 = r.add_component_beads("particle1", [(1, 10)])
        r.create_component("particle2", color=0.5)
        p21 = r.add_component_beads("particle2", [(1, 10)])
        r.create_component("particle3", color=0.9)
        p31 = r.add_component_beads("particle3", [(1, 10)])

        representations.append(r)

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

        restraints = '''#
        particle2 particle3 1 5 1 1
        particle1 particle3 1 2 1 1 '''

        with IMP.allow_deprecated():
            xl = IMP.pmi.restraints.crosslinking.ISDCrossLinkMS(representations,
                                                                restraints,
                                                                length=21.0,
                                                                slope=0.0,
                                                                inner_slope=0.1,
                                                                resolution=1.0)

        psi = xl.get_psi(0.05)

        psi[0].set_scale(0.1)

        sigma = xl.get_sigma(1.0)

        sigma[0].set_scale(5.0)

        out_dict = xl.get_output()
        sorted_keys = sorted(out_dict.keys())
        for entry in sorted_keys:
            print(entry, out_dict[entry])

        print(xyz11.get_coordinates())

        xl.add_to_model()
        rset = IMP.pmi.tools.get_restraint_set(m)
        self.assertAlmostEqual(rset.evaluate(False), -2.197418, delta=1e-2)

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

        fs = po.get_fields(
            ['ISDCrossLinkMS_Distance_interrb-State:0-1:particle1_2:particle3-1-1-0.05_None',
             'ISDCrossLinkMS_Distance_interrb-State:0-1:particle2_5:particle3-1-1-0.05_None',
             'ISDCrossLinkMS_Data_Score_None',
             'ISDCrossLinkMS_Linear_Score_None',
             'ISDCrossLinkMS_Psi_0.05_None'])

        for output in ["excluded.None.xl.db", "included.None.xl.db",
                       "missing.None.xl.db", "modeling.stat"]:
            os.unlink(output)

if __name__ == '__main__':
    IMP.test.main()
