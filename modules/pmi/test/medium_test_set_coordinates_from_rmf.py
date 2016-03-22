import os
import IMP
import IMP.test

import IMP.pmi.restraints.stereochemistry
import IMP.pmi.representation
import IMP.pmi.tools
import IMP.pmi.output
import IMP.pmi.macros
import IMP.pmi.output

class Tests(IMP.test.TestCase):
    def test_set_from_rmf(self):
        """Test setting coordinates from RMF"""

        # input parameter
        pdbfile = self.get_input_file_name("mini.pdb")
        fastafile = self.get_input_file_name("mini.fasta")

        components = ["Rpb1", "Rpb2"]

        chains = "AB"

        colors = [0., 1.0]

        beadsize = 20

        fastids = IMP.pmi.tools.get_ids_from_fasta_file(fastafile)

        m = IMP.Model()
        simo = IMP.pmi.representation.Representation(m)


        simo.create_component("Rpb1", color=colors[0])
        simo.add_component_sequence("Rpb1", fastafile, id=fastids[0])
        simo.autobuild_model("Rpb1", pdbfile, "A",
                             resolutions=[1], missingbeadsize=beadsize)
        simo.setup_component_sequence_connectivity("Rpb1", 1)

        simo.create_component("Rpb2", color=colors[1])
        simo.add_component_sequence("Rpb2", fastafile, id=fastids[1])
        simo.autobuild_model("Rpb2", pdbfile, "B",
                             resolutions=[10], missingbeadsize=beadsize)
        simo.setup_component_sequence_connectivity("Rpb2", 1)

        output = IMP.pmi.output.Output()
        output.init_rmf("test_set_coordinates_from_rmf.rmf3", [simo.prot])
        output.write_rmf("test_set_coordinates_from_rmf.rmf3")
        output.close_rmf("test_set_coordinates_from_rmf.rmf3")

        simo.set_coordinates_from_rmf("Rpb1",
                                      "test_set_coordinates_from_rmf.rmf3", 0)

        os.unlink("test_set_coordinates_from_rmf.rmf3")

    def test_set_from_rmf_roundtrip(self):
        pdbfile = self.get_input_file_name("1WCM.pdb")
        fastafile = self.get_input_file_name("1WCM.fasta.txt")
        m = IMP.Model()
        simo = IMP.pmi.representation.Representation(m,upperharmonic=True,disorderedlength=False)

        domains=[("Rpb8",   "Rpb8",        0.0,   fastafile, "1WCM:I",  "BEADS", None, (1,-1),      "BEADSONLY", 20,       0,     None, 0, None, None, None),
                 ("Rpb9",   "Rpb9",        0.0,   fastafile, "1WCM:I",  "BEADS", None, (1,-1),      None, 20,              1,     None, 0, None, None, None),
                 ("Rpb10",  "Rpb10",       0.0,   fastafile, "1WCM:J",  "BEADS", None, (1,-1),      None, 20,              None,     None, 0, None, None, None),
                 ("Rpb11",  "Rpb11",       0.0,   fastafile, "1WCM:K",  pdbfile, "K",  (1,-1),      None, 20,              2,     None, 0, None, None, None),
                 ("Rpb12",  "Rpb12",       0.0,   fastafile, "1WCM:L",  pdbfile, "L",  (1,-1),      True, 20,              3,     None, 2, self.get_input_file_name("gmm_Rpb12.txt"), "gmm_Rpb12.mrc", None)]
        with IMP.allow_deprecated():
            bm=IMP.pmi.macros.BuildModel1(simo)
        bm.build_model(domains)
        simo.shuffle_configuration(100)
        simo.optimize_floppy_bodies(10, temperature=100.0)
        o=IMP.pmi.output.Output()
        o.init_rmf("test_set_from_rmf_roundtrip_1.rmf3", [simo.prot])
        o.write_rmf("test_set_from_rmf_roundtrip_1.rmf3")

        simo.dump_particle_descriptors()
        simo.shuffle_configuration(100)
        simo.optimize_floppy_bodies(10, temperature=100.0)
        o.write_rmf("test_set_from_rmf_roundtrip_1.rmf3")
        simo.load_particle_descriptors()
        o.write_rmf("test_set_from_rmf_roundtrip_1.rmf3")
        o.close_rmf("test_set_from_rmf_roundtrip_1.rmf3")

        os.unlink("test_set_from_rmf_roundtrip_1.rmf3")

if __name__ == '__main__':
    IMP.test.main()
