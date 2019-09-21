from __future__ import print_function
import IMP
import IMP.test

import IMP.pmi.restraints.stereochemistry
import IMP.pmi.restraints.basic
import IMP.pmi.restraints.proteomics
import IMP.pmi.restraints.crosslinking
import IMP.pmi.representation
import IMP.pmi.tools
try:
    import IMP.isd_emxl
    no_isd_emxl = False
except ImportError:
    no_isd_emxl = True

class Tests(IMP.test.TestCase):
    def test_restraints(self):
        """Test PMI restraints"""
        # input parameter
        pdbfile = self.get_input_file_name("1WCM.pdb")
        fastafile = self.get_input_file_name("1WCM.fasta.txt")

        components = ["Rpb3", "Rpb3.copy", "Rpb4"]

        chains = "CCD"

        colors = [0., 0.5, 1.0]

        beadsize = 20

        fastids = IMP.pmi.tools.get_ids_from_fasta_file(fastafile)

        m = IMP.Model()
        with IMP.allow_deprecated():
            simo = IMP.pmi.representation.Representation(m)

        hierarchies = {}

        for n in range(len(components)):
            simo.create_component(components[n], color=colors[n])
            simo.add_component_sequence(components[n], fastafile, id=fastids[n + 2])
            hierarchies[components[n]] = simo.autobuild_model(
                components[n], pdbfile, chains[n],
                resolutions=[1, 10, 100], missingbeadsize=beadsize)
            simo.setup_component_sequence_connectivity(components[n], 1)

        ev1 = IMP.pmi.restraints.stereochemistry.ExcludedVolumeSphere(simo)
        ev1.add_to_model()
        print(ev1.get_output())

        ev2 = IMP.pmi.restraints.stereochemistry.ExcludedVolumeSphere(
            simo,
            [simo.hier_dict["Rpb3"]],
            [simo.hier_dict["Rpb4"]])
        ev2.add_to_model()
        print(ev2.get_output())

        rb = IMP.pmi.restraints.stereochemistry.ResidueBondRestraint(
            simo, (30, 40, "Rpb3"))
        rb.add_to_model()
        print(rb.get_output())

        ra = IMP.pmi.restraints.stereochemistry.ResidueAngleRestraint(
            simo, (30, 40, "Rpb3"))
        ra.add_to_model()
        print(ra.get_output())

        rd = IMP.pmi.restraints.stereochemistry.ResidueDihedralRestraint(
            simo, (30, 40, "Rpb3"))
        rd.add_to_model()
        print(ra.get_output())

        if not no_isd_emxl:
            ss = IMP.pmi.restraints.stereochemistry.SecondaryStructure(
                simo, (30, 40, "Rpb3"), "HHHHHHHHHHH")
            ss.add_to_model()
            print(ss.get_output())

        eb1 = IMP.pmi.restraints.basic.ExternalBarrier(simo, 50)
        eb1.add_to_model()
        print(eb1.get_output())

        cr1 = IMP.pmi.restraints.proteomics.ConnectivityRestraint(
            simo, [(1, 100, "Rpb3"), (1, 100, "Rpb4")], resolution=100)
        cr1.add_to_model()
        print(cr1.get_output())

        cr2 = IMP.pmi.restraints.proteomics.CompositeRestraint(
            simo, [(1, 100, "Rpb3"), (200, 300, "Rpb3")],
            [[(1, 100, "Rpb4")], [(200, 500, "Rpb4")]], resolution=100)
        cr2.add_to_model()
        print(cr2.get_output())

        restraints = '''#
        Rpb3 Rpb4 100 150
        Rpb4 Rpb4 50 150'''

        cr2 = IMP.pmi.restraints.proteomics.AmbiguousCompositeRestraint(
            simo,
            restraints,
            resolution=1)
        cr2.add_to_model()
        print(cr2.get_output())


        restraints = '''#
        Rpb3 Rpb4 100 150 0.5
        Rpb4 Rpb4 50 150 0.7'''

        pm = IMP.pmi.restraints.proteomics.SimplifiedPEMAP(
            simo,
            restraints,
            20,
            1,
            resolution=1)
        pm.add_to_model()
        print(pm.get_output())

        restraints = '''#
        Rpb3 Rpb4 100 150 1 1
        Rpb4 Rpb4 50 150 1 2'''

        with IMP.allow_deprecated():
            xl4 = IMP.pmi.restraints.crosslinking.ISDCrossLinkMS(
                simo, restraints, 25, resolution=1)
        xl4.add_to_model()
        print(xl4.get_output())

        restraints = '''#
        100 Rpb3 150 Rpb4 0.5 epsilon1
        50 Rpb4 150 Rpb4  0.7 epsilon2'''

        xl5 = IMP.pmi.restraints.crosslinking.CysteineCrossLinkRestraint(
            [simo],
            restraints)
        xl5.add_to_model()
        xl5.set_output_level("high")
        print(xl5.get_output())

if __name__ == '__main__':
    IMP.test.main()
