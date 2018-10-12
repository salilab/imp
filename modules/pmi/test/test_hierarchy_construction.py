import IMP
import os
import IMP.test

import IMP.pmi.restraints.stereochemistry
import IMP.pmi.representation
import IMP.pmi.tools
import IMP.pmi.output

class Tests(IMP.test.TestCase):
    def test_hierarchy_construction(self):
        """Test construction of a hierarchy"""

        # input parameter
        pdbfile = self.get_input_file_name("mini.pdb")
        fastafile = self.get_input_file_name("mini.fasta")

        components = ["Rpb1", "Rpb2" ]
        chains = "AB"
        colors = [0.,  1.]
        beadsize = 20
        fastids = IMP.pmi.tools.get_ids_from_fasta_file(fastafile)

        m = IMP.Model()
        with IMP.allow_deprecated():
            simo = IMP.pmi.representation.Representation(m)

        for n in range(len(components)):
            simo.create_component(components[n], color=colors[n])
            simo.add_component_sequence(components[n], fastafile, id=fastids[n])
            simo.autobuild_model(
                               components[n], pdbfile, chains[n],
                               resolutions=[1, 10], missingbeadsize=beadsize)
            simo.setup_component_sequence_connectivity(components[n], 1)

        ev = IMP.pmi.restraints.stereochemistry.ExcludedVolumeSphere(
                                                    simo, resolution=10)
        ev.add_to_model()

        o = IMP.pmi.output.Output()
        o.init_rmf("conformations.rmf", [simo.prot])
        o.write_rmf("conformations.rmf")

        simo.optimize_floppy_bodies(1000)

        o.write_rmf("conformations.rmf")
        o.close_rmf("conformations.rmf")
        os.unlink('conformations.rmf')


if __name__ == '__main__':
    IMP.test.main()
