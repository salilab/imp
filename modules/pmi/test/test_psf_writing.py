import os
import IMP
import IMP.test

import IMP.pmi.restraints.stereochemistry
import IMP.pmi.representation
import IMP.pmi.tools
import IMP.pmi.output

class Tests(IMP.test.TestCase):
    def test_pdb_writing(self):
        """Test writing of PSF files"""

        # input parameter
        pdbfile = self.get_input_file_name("mini.pdb")
        fastafile = self.get_input_file_name("mini.fasta")

        components = ["Rpb1", "Rpb2" ]

        chains = "AB"

        colors = [0., 1.0]

        beadsize = 1

        fastids = IMP.pmi.tools.get_ids_from_fasta_file(fastafile)


        m = IMP.Model()
        simo = IMP.pmi.representation.Representation(m)


        simo.create_component("Rpb1", color=colors[0])
        simo.add_component_sequence("Rpb1", fastafile, id=fastids[0])
        simo.autobuild_model("Rpb1", pdbfile, "A",
                             resolutions=[1], missingbeadsize=beadsize)
        simo.setup_component_sequence_connectivity("Rpb1", 1)

        simo.add_component_name("Rpb2", color=colors[1])
        simo.add_component_sequence("Rpb2", fastafile, id=fastids[1])
        simo.autobuild_model("Rpb2", pdbfile, "B",
                             resolutions=[1], missingbeadsize=beadsize)
        simo.setup_component_sequence_connectivity("Rpb2", 1)

        output = IMP.pmi.output.Output()
        output.init_pdb("test_psf_writing.pdb", simo.prot)
        output.write_pdb("test_psf_writing.pdb")
        output.write_psf("test_psf_writing.psf","test_psf_writing.pdb")
        psf_content='''PSF CMAP CHEQ
17 !NATOM
       1 A    1    "MET" C    C         1.000000      0.000000       0      0.000000      0.000000
       2 A    2    "VAL" C    C         1.000000      0.000000       0      0.000000      0.000000
       3 A    3    "GLY" C    C         1.000000      0.000000       0      0.000000      0.000000
       4 A    4    "GLN" C    C         1.000000      0.000000       0      0.000000      0.000000
       5 A    5    "GLN" C    C         1.000000      0.000000       0      0.000000      0.000000
       6 A    6    "TYR" C    C         1.000000      0.000000       0      0.000000      0.000000
       7 A    7    "SER" C    C         1.000000      0.000000       0      0.000000      0.000000
       8 A    8    "SER" C    C         1.000000      0.000000       0      0.000000      0.000000
       9 B    1    "ALA" C    C         1.000000      0.000000       0      0.000000      0.000000
      10 B    2    "ALA" C    C         1.000000      0.000000       0      0.000000      0.000000
      11 B    3    "ASP" C    C         1.000000      0.000000       0      0.000000      0.000000
      12 B    4    "GLU" C    C         1.000000      0.000000       0      0.000000      0.000000
      13 B    5    "SER" C    C         1.000000      0.000000       0      0.000000      0.000000
      14 B    6    "ALA" C    C         1.000000      0.000000       0      0.000000      0.000000
      15 B    7    "PRO" C    C         1.000000      0.000000       0      0.000000      0.000000
      16 B    8    "ILE" C    C         1.000000      0.000000       0      0.000000      0.000000
      17 B    9    "THR" C    C         1.000000      0.000000       0      0.000000      0.000000
15 !NBOND: bonds
       1       2       2       3       3       4       4       5
       5       6       6       7       7       8       9      10
      10      11      11      12      12      13      13      14
      14      15      15      16      16      17'''.split("\n")

        with open("test_psf_writing.psf") as f:
            for nl, l in enumerate(f):
                self.assertEqual(psf_content[nl],l.replace('\n',''))
        os.unlink('test_psf_writing.psf')
        os.unlink('test_psf_writing.pdb')

if __name__ == '__main__':
    IMP.test.main()
