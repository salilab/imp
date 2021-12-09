import IMP
import IMP.test
import IMP.pmi.restraints.stereochemistry
import IMP.pmi.tools
import IMP.pmi.output
import IMP.pmi.topology
import IMP.pmi.dof
import IMP.pmi.macros
import ihm.reader
import os,shutil

try:
    import IMP.mpi
    rem = IMP.mpi.ReplicaExchange()
except ImportError:
    rem = None

class Tests(IMP.test.TestCase):

    def _make_test_model(self):
        mdl = IMP.Model()
        pdb_file = self.get_input_file_name("mini.pdb")
        fasta_file = self.get_input_file_name("mini.fasta")

        seqs = IMP.pmi.topology.Sequences(fasta_file)
        s = IMP.pmi.topology.System(mdl)
        st = s.create_state()

        molA = st.create_molecule("P1",seqs[0],chain_id='A')
        aresA = molA.add_structure(pdb_file,chain_id='A',soft_check=True)
        molA.add_representation(aresA,[1])
        molA.add_representation(molA[:]-aresA,20)

        molB = st.create_molecule("P2",seqs[1],chain_id='B')
        aresB = molB.add_structure(pdb_file,chain_id='B',soft_check=True)
        molB.add_representation(aresB,[10])
        molB.add_representation(molB[:]-aresB,20)

        # Multi-character chain ID
        molC = molB.create_copy(chain_id='CA')
        aresC = molC.add_structure(pdb_file,chain_id='B',soft_check=True)
        molC.add_representation(aresC,[1,10])
        molC.add_representation(molC[:]-aresC,2)

        # Duplicate chain ID
        molA2 = molB.create_copy(chain_id='A')
        aresA2 = molA2.add_structure(pdb_file,chain_id='B',soft_check=True)
        molA2.add_representation(aresA2,[1,10])
        molA2.add_representation(molA2[:]-aresA2,2)

        # No chain ID
        molA3 = molB.create_copy(chain_id='')
        aresA3 = molA3.add_structure(pdb_file,chain_id='B',soft_check=True)
        molA3.add_representation(aresA3,[1,10])
        molA3.add_representation(molA3[:]-aresA3,2)

        # Blank chain ID
        molA4 = molA.create_copy(chain_id=' ')
        aresA4 = molA4.add_structure(pdb_file,chain_id='A',soft_check=True)
        molA4.add_representation(aresA4,[1])
        molA4.add_representation(molA4[:]-aresA4,20)

        root_hier = s.build()
        return mdl, root_hier

    def test_pdb_mmcif(self):
        """Test PDB writing with mmcif=True"""
        mdl, root_hier = self._make_test_model()

        # write PDB and check it's ok
        output = IMP.pmi.output.Output(atomistic=True)
        output.init_pdb("test_pdb_writing.cif", root_hier, mmcif=True)
        output.write_pdbs()
        print('init best scoring')
        output.init_pdb_best_scoring("test_pdb_writing", root_hier, 10, mmcif=True)
        print('scoring')
        for i in range(20):
            score = -float(i)
            output.write_pdb_best_scoring(score)

        with open('test_pdb_writing.cif') as fh:
            s, = ihm.reader.read(fh)
        self.assertEqual([x.id for x in s.asym_units],
                         [' ', ' 1', 'A', 'B', 'A1', 'CA'])

        for i in range(10):
            os.unlink('test_pdb_writing.'+str(i)+'.cif')
        os.unlink('test_pdb_writing.cif')

    def test_pdb_pmi2(self):
        """Test PDB writing in PMI2"""
        mdl, root_hier = self._make_test_model()

        # write PDB and check it's ok
        output = IMP.pmi.output.Output(atomistic=True)
        # This will fail due to the 3rd chain being called "CA"
        self.assertRaises(ValueError, output.init_pdb,
                          "test_pdb_writing.pdb", root_hier)
        # Fix chain ID to be a single character
        chains = IMP.atom.get_by_type(root_hier, IMP.atom.CHAIN_TYPE)
        self.assertTrue(IMP.atom.Chain.get_is_setup(chains[2]))
        IMP.atom.Chain(chains[2]).set_id('C')
        output.init_pdb("test_pdb_writing.pdb", root_hier)
        output.write_pdbs()
        print('init best scoring')
        output.init_pdb_best_scoring("test_pdb_writing", root_hier, 10)
        print('scoring')
        for i in range(20):
            score = -float(i)
            output.write_pdb_best_scoring(score)

        pdb_content='''ATOM      1  CA  BEA     2      70.427  58.819  51.717  1.00  3.50           C
ATOM      2  CA  ASP     3      70.427  58.819  51.717  1.00  2.87           C
ATOM      3  CA  GLU     4      68.584  58.274  48.425  1.00  3.00           C
ATOM      4  CA  BEA     6      68.584  58.274  48.425  1.00  3.50           C
ATOM      5  CA  BEA     8      68.584  58.274  48.425  1.00  3.50           C
ATOM      6  CA  THR     9      68.584  58.274  48.425  1.00  2.80           C
ATOM      7  CA  MET A   1     114.370  27.980 -26.088  1.00  3.07           C
ATOM      8  CA  VAL A   2     114.370  27.980 -26.088  1.00  2.89           C
ATOM      9  CA  GLY A   3     111.506  26.368 -28.075  1.00  2.27           C
ATOM     10  CA  GLN A   4     113.468  23.113 -28.639  1.00  3.01           C
ATOM     11  CA  GLN A   5     113.808  21.534 -32.168  1.00  3.01           C
ATOM     12  CA  TYR A   6     116.743  22.770 -34.259  1.00  3.32           C
ATOM     13  CA  SER A   7     116.626  25.161 -37.229  1.00  2.61           C
ATOM     14  CA  SER A   8     119.165  25.590 -40.036  1.00  2.61           C
ATOM     15  CA  BEA B   2      70.427  58.819  51.717  1.00  3.50           C
ATOM     16  CA  BEA B   4      68.799  58.791  50.131  1.00  3.69           C
ATOM     17  CA  BEA B   7      68.584  58.274  48.425  1.00  4.76           C
ATOM     18  CA  BEA C   2      70.427  58.819  51.717  1.00  3.50           C
ATOM     19  CA  ASP C   3      70.427  58.819  51.717  1.00  2.87           C
ATOM     20  CA  GLU C   4      68.584  58.274  48.425  1.00  3.00           C
ATOM     21  CA  BEA C   6      68.584  58.274  48.425  1.00  3.50           C
ATOM     22  CA  BEA C   8      68.584  58.274  48.425  1.00  3.50           C
ATOM     23  CA  THR C   9      68.584  58.274  48.425  1.00  2.80           C
ATOM     24  CA  MET     1     114.370  27.980 -26.088  1.00  3.07           C
ATOM     25  CA  VAL     2     114.370  27.980 -26.088  1.00  2.89           C
ATOM     26  CA  GLY     3     111.506  26.368 -28.075  1.00  2.27           C
ATOM     27  CA  GLN     4     113.468  23.113 -28.639  1.00  3.01           C
ATOM     28  CA  GLN     5     113.808  21.534 -32.168  1.00  3.01           C
ATOM     29  CA  TYR     6     116.743  22.770 -34.259  1.00  3.32           C
ATOM     30  CA  SER     7     116.626  25.161 -37.229  1.00  2.61           C
ATOM     31  CA  SER     8     119.165  25.590 -40.036  1.00  2.61           C
ATOM     32  CA  BEA A   2      70.427  58.819  51.717  1.00  3.50           C
ATOM     33  CA  ASP A   3      70.427  58.819  51.717  1.00  2.87           C
ATOM     34  CA  GLU A   4      68.584  58.274  48.425  1.00  3.00           C
ATOM     35  CA  BEA A   6      68.584  58.274  48.425  1.00  3.50           C
ATOM     36  CA  BEA A   8      68.584  58.274  48.425  1.00  3.50           C
ATOM     37  CA  THR A   9      68.584  58.274  48.425  1.00  2.80           C
ENDMDL'''.split("\n")

        with open("test_pdb_writing.pdb") as f:
            for nl, l in enumerate(f):
                self.assertEqual(pdb_content[nl],l.rstrip('\r\n '))

        with open("test_pdb_writing.0.pdb") as f:
            for nl, l in enumerate(f):
                self.assertEqual(pdb_content[nl],l.rstrip('\r\n '))

        for i in range(10):
            os.unlink('test_pdb_writing.'+str(i)+'.pdb')
        os.unlink('test_pdb_writing.pdb')

    def test_pdb_from_rex(self):
        """Test PDB writing in PMI2 from replica exchange"""
        mdl = IMP.Model()
        pdb_file = self.get_input_file_name("mini.pdb")
        fasta_file = self.get_input_file_name("mini.fasta")

        seqs = IMP.pmi.topology.Sequences(fasta_file)
        s = IMP.pmi.topology.System(mdl)
        st = s.create_state()

        molA = st.create_molecule("P1",seqs[0],chain_id='A')
        aresA = molA.add_structure(pdb_file,chain_id='A',soft_check=True)
        molA.add_representation(aresA,[1])
        molA.add_representation(molA[:]-aresA,20)

        molB = st.create_molecule("P2",seqs[1],chain_id='B')
        aresB = molB.add_structure(pdb_file,chain_id='B',soft_check=True)
        molB.add_representation(aresB,[10])
        molB.add_representation(molB[:]-aresB,20)

        molC = molB.create_copy(chain_id='C')
        aresC = molC.add_structure(pdb_file,chain_id='B',soft_check=True)
        molC.add_representation(aresC,[1,10])
        molC.add_representation(molC[:]-aresC,2)
        root_hier = s.build()

        dof = IMP.pmi.dof.DegreesOfFreedom(mdl)
        rb_movers = dof.create_rigid_body([molA,molB,molC],
                                          name="test RB")
        rex = IMP.pmi.macros.ReplicaExchange0(mdl,
                                              root_hier=root_hier,
                                              monte_carlo_sample_objects = dof.get_movers(),
                                              number_of_frames=3,
                                              monte_carlo_steps=10,
                                              number_of_best_scoring_models=3,
                                              global_output_directory='pdb_test/',
                                              replica_exchange_object = rem)
        rex.execute_macro()
        for i in range(3):
            self.assertTrue(os.path.isfile('pdb_test/pdbs/model.%i.pdb'%i))
        mhtest = IMP.atom.read_pdb('pdb_test/pdbs/model.0.pdb',mdl)
        self.assertEqual(len(IMP.core.get_leaves(mhtest)),17)
        shutil.rmtree("pdb_test/")

    def test_pdb_multistate(self):
        """Test PDB writing in PMI2 from replica exchange"""
        mdl = IMP.Model()
        pdb_file = self.get_input_file_name("mini.pdb")
        fasta_file = self.get_input_file_name("mini.fasta")

        seqs = IMP.pmi.topology.Sequences(fasta_file)
        s = IMP.pmi.topology.System(mdl)
        st = s.create_state()

        molA = st.create_molecule("P1",seqs[0],chain_id='A')
        aresA = molA.add_structure(pdb_file,chain_id='A',soft_check=True)
        molA.add_representation(aresA,[0])

        st2 = s.create_state()
        molB = st2.create_molecule("P1",seqs[0],chain_id='A')
        aresB = molB.add_structure(pdb_file,chain_id='A',soft_check=True)
        molB.add_representation(molB,[1])

        root_hier = s.build()

        dof = IMP.pmi.dof.DegreesOfFreedom(mdl)
        dof.create_rigid_body(molA,name="rbA")
        dof.create_rigid_body(molB,name="rbB")
        rex = IMP.pmi.macros.ReplicaExchange0(mdl,
                                              root_hier=root_hier,
                                              monte_carlo_sample_objects = dof.get_movers(),
                                              number_of_frames=3,
                                              monte_carlo_steps=10,
                                              number_of_best_scoring_models=3,
                                              global_output_directory='pdb_test/',
                                              replica_exchange_object = rem)
        rex.execute_macro()
        for i in range(3):
            self.assertTrue(os.path.isfile('pdb_test/pdbs/0/model.%i.pdb'%i))
            self.assertTrue(os.path.isfile('pdb_test/pdbs/1/model.%i.pdb'%i))
        testA = IMP.atom.read_pdb('pdb_test/pdbs/0/model.0.pdb',mdl)
        self.assertEqual(len(IMP.core.get_leaves(testA)),53)
        testB = IMP.atom.read_pdb('pdb_test/pdbs/1/model.0.pdb',mdl)
        self.assertEqual(len(IMP.core.get_leaves(testB)),8)
        shutil.rmtree("pdb_test/")

if __name__ == '__main__':
    IMP.test.main()
