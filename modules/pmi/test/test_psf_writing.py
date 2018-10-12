import os
import IMP
import IMP.test

import IMP.pmi.topology
import IMP.pmi.output

class Tests(IMP.test.TestCase):
    def test_psf_writing(self):
        """Test writing of PSF files"""

        # input parameter
        pdbfile = self.get_input_file_name("mini.pdb")
        fastafile = self.get_input_file_name("mini.fasta")
        seqs = IMP.pmi.topology.Sequences(fastafile)

        components = ["Rpb1", "Rpb2" ]

        chains = "AB"

        beadsize = 1

        m = IMP.Model()
        s = IMP.pmi.topology.System(m)
        st = s.create_state()

        rpb1 = st.create_molecule("Rpb1", seqs[0], chain_id='A')
        sres = rpb1.add_structure(pdbfile, 'A', soft_check=True)
        rpb1.add_representation(sres, [1])
        rpb1.add_representation(rpb1[:] - sres, [beadsize])

        rpb2 = st.create_molecule("Rpb2", seqs[1], chain_id='B')
        sres = rpb2.add_structure(pdbfile, 'B', soft_check=True)
        rpb2.add_representation(sres, [1])
        rpb2.add_representation(rpb2[:] - sres, [beadsize])

        root_hier = s.build()
        output = IMP.pmi.output.Output(atomistic=True)
        output.init_pdb("test_psf_writing.pdb", root_hier)
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
