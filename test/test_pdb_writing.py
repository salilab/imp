import os
import IMP
import IMP.test

import IMP.pmi.restraints.stereochemistry
import IMP.pmi.representation
import IMP.pmi.tools
import IMP.pmi.output

class Tests(IMP.test.TestCase):
    def test_pdb_writing(self):
        """Test writing of PDB files"""




        # input parameter
        pdbfile = self.get_input_file_name("mini.pdb")
        fastafile = self.get_input_file_name("mini.fasta")

        comps = ["P1", "P2", "P3"]
        chains = "ABB"
        colors = [0., 0.5, 1.0]
        beadsize = 20
        fastids = IMP.pmi.tools.get_ids_from_fasta_file(fastafile)


        m = IMP.Model()
        simo = IMP.pmi.representation.Representation(m)

        simo.create_component(comps[0], color=colors[0])
        simo.add_component_sequence(comps[0], fastafile, id=fastids[0])
        simo.autobuild_model(comps[0], pdbfile, chains[0],
                             resolutions=[1], missingbeadsize=beadsize)
        simo.setup_component_sequence_connectivity(comps[0], 1)

        simo.create_component(comps[1], color=colors[1])
        simo.add_component_sequence(comps[1], fastafile, id=fastids[1])
        simo.autobuild_model(comps[1], pdbfile, chains[1],
                             resolutions=[10], missingbeadsize=beadsize)
        simo.setup_component_sequence_connectivity(comps[1], 1)

        simo.create_component(comps[2], color=colors[2])
        simo.add_component_sequence(comps[2], fastafile, id=fastids[1])
        simo.autobuild_model(comps[2], pdbfile, chains[2],
                             resolutions=[1,10], missingbeadsize=2)
        simo.setup_component_sequence_connectivity(comps[2], 1)

        output = IMP.pmi.output.Output()
        output.init_pdb("test_pdb_writing.pdb", simo.prot)
        output.write_pdbs()
        output.init_pdb_best_scoring("test_pdb_writing", simo.prot, 10)
        for i in range(20):
            score = -float(i)
            output.write_pdb_best_scoring(score)

        for i in range(10):
            os.unlink('test_pdb_writing.'+str(i)+'.pdb')

        pdb_content='''ATOM      1  CA  MET A   1     114.370  27.980 -26.088  1.00  3.07           C
ATOM      2  CA  VAL A   2     114.370  27.980 -26.088  1.00  2.89           C
ATOM      3  CA  GLY A   3     111.506  26.368 -28.075  1.00  2.27           C
ATOM      4  CA  GLN A   4     113.468  23.113 -28.639  1.00  3.01           C
ATOM      5  CA  GLN A   5     113.808  21.534 -32.168  1.00  3.01           C
ATOM      6  CA  TYR A   6     116.743  22.770 -34.259  1.00  3.32           C
ATOM      7  CA  SER A   7     116.626  25.161 -37.229  1.00  2.61           C
ATOM      8  CA  SER A   8     119.165  25.590 -40.036  1.00  2.61           C
ATOM      9  CA  BEA B   2      70.427  58.819  51.717  1.00  3.50           C
ATOM     10  CA  BEA B   4      68.799  58.791  50.131  1.00  3.69           C
ATOM     11  CA  BEA B   7      68.799  58.791  50.131  1.00  4.76           C
ATOM     12  CA  BEA C   2      70.427  58.819  51.717  1.00  3.50           C
ATOM     13  CA  ASP C   3      70.427  58.819  51.717  1.00  2.87           C
ATOM     14  CA  GLU C   4      68.584  58.274  48.425  1.00  3.00           C
ATOM     15  CA  BEA C   6      68.584  58.274  48.425  1.00  3.50           C
ATOM     16  CA  BEA C   8      68.584  58.274  48.425  1.00  3.50           C
ATOM     17  CA  THR C   9      68.584  58.274  48.425  1.00  2.80           C
ENDMDL'''.split("\n")

        with open("test_pdb_writing.pdb") as f:
            for nl, l in enumerate(f):
                self.assertEqual(pdb_content[nl],l.rstrip('\r\n '))
        os.unlink('test_pdb_writing.pdb')

if __name__ == '__main__':
    IMP.test.main()
