from __future__ import print_function
import IMP
import IMP.test

import IMP.pmi.restraints.stereochemistry
import IMP.pmi.restraints.basic
import IMP.pmi.restraints.proteomics
import IMP.pmi.restraints.crosslinking
import IMP.pmi.topology
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
        seqs = IMP.pmi.topology.Sequences(self.get_input_file_name(
                                                '1WCM.fasta.txt'))

        components = ["Rpb3", "Rpb3.copy", "Rpb4"]
        chains = "CCD"
        colors = [0., 0.5, 1.0]
        beadsize = 20

        m = IMP.Model()
        simo = IMP.pmi.topology.System(m)
        st1 = simo.create_state()
        hier_dict = {}

        for n in range(len(components)):
            mol = st1.create_molecule(components[n],
                                      sequence=seqs["1WCM:" + chains[n]])
            hier_dict[components[n]] = mol
            atomic = mol.add_structure(pdbfile, chain_id=chains[n])
            mol.add_representation(mol.get_atomic_residues(),
                                   resolutions=[1, 10, 100], color=colors[n])
            mol.add_representation(mol.get_non_atomic_residues(),
                                   resolutions=[10], color=colors[n])
        root_hier = simo.build()

        ev1 = IMP.pmi.restraints.stereochemistry.ExcludedVolumeSphere(
            included_objects=root_hier)
        ev1.add_to_model()
        print(ev1.get_output())

        ev2 = IMP.pmi.restraints.stereochemistry.ExcludedVolumeSphere(
            included_objects=[hier_dict["Rpb3"]],
            other_objects=[hier_dict["Rpb4"]])
        ev2.add_to_model()
        print(ev2.get_output())

        if not no_isd_emxl:
            ss = IMP.pmi.restraints.stereochemistry.SecondaryStructure(
                simo, (30, 40, "Rpb3"), "HHHHHHHHHHH")
            ss.add_to_model()
            print(ss.get_output())

        eb1 = IMP.pmi.restraints.basic.ExternalBarrier(
            hierarchies=root_hier, radius=50)
        eb1.add_to_model()
        print(eb1.get_output())

        sel1 = IMP.atom.Selection(root_hier, molecule="Rpb3",
                                  residue_indexes=range(1,100))
        sel2 = IMP.atom.Selection(root_hier, molecule="Rpb4",
                                  residue_indexes=range(1,100))
        cr1 = IMP.pmi.restraints.proteomics.ConnectivityRestraint(
            (sel1, sel2), resolution=100)
        cr1.add_to_model()
        print(cr1.get_output())

        cr2 = IMP.pmi.restraints.proteomics.CompositeRestraint(
                [hier_dict["Rpb3"][:102], hier_dict["Rpb3"][298:308]],
                [[hier_dict["Rpb4"][:106]], [hier_dict["Rpb4"][117:500]]],
                resolution=100)
        cr2.add_to_model()
        print(cr2.get_output())

        restraints = '''#
        Rpb3 Rpb4 100 150
        Rpb4 Rpb4 50 150'''

        cr2 = IMP.pmi.restraints.proteomics.AmbiguousCompositeRestraint(
            root_hier, restraints, resolution=1)
        cr2.add_to_model()
        print(cr2.get_output())


        restraints = '''#
        Rpb3 Rpb4 100 150 0.5
        Rpb4 Rpb4 50 150 0.7'''

        pm = IMP.pmi.restraints.proteomics.SimplifiedPEMAP(
            root_hier, restraints, 20, 1, resolution=1)
        pm.add_to_model()
        print(pm.get_output())

        restraints = '''#
        100 Rpb3 150 Rpb4 0.5 epsilon1
        50 Rpb4 150 Rpb4  0.7 epsilon2'''

        xl5 = IMP.pmi.restraints.crosslinking.CysteineCrossLinkRestraint(
            root_hier, restraints)
        xl5.add_to_model()
        xl5.set_output_level("high")
        print(xl5.get_output())

if __name__ == '__main__':
    IMP.test.main()
