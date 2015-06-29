import IMP
import IMP.test
import IMP.atom
from io import BytesIO

pdb = b"""
ATOM      1  HT1 ARG     1       3.836  65.763  35.255  1.00  0.00
ATOM      2  HT2 ARG     1       2.546  66.040  34.188  1.00  0.00
ATOM      3  N   ARG     1       3.477  66.382  34.500  1.00 33.79
ATOM      4  HT3 ARG     1       3.321  67.184  35.142  1.00  0.00
ATOM      5  CA  ARG     1       4.292  66.713  33.342  1.00 31.17
ATOM      6  CB  ARG     1       5.584  67.316  33.787  1.00 34.92
ATOM      7  CG  ARG     1       6.365  67.976  32.693  1.00 38.41
ATOM      8  CD  ARG     1       7.455  68.842  33.276  1.00 41.67
ATOM      9  NE  ARG     1       8.358  69.342  32.236  1.00 43.96
ATOM     10  HE  ARG     1       8.037  69.155  31.330  1.00  0.00
ATOM     11  CZ  ARG     1       9.490  69.981  32.485  1.00 45.32
ATOM     12  NH1 ARG     1       9.965  70.101  33.716  1.00 45.71
ATOM     13 HH11 ARG     1       9.492  69.673  34.486  1.00  0.00
ATOM     14 HH12 ARG     1      10.792  70.643  33.868  1.00  0.00
ATOM     15  NH2 ARG     1      10.255  70.400  31.511  1.00 45.55
ATOM     16 HH21 ARG     1       9.937  70.361  30.563  1.00  0.00
ATOM     17 HH22 ARG     1      11.177  70.729  31.715  1.00  0.00
ATOM     18  C   ARG     1       4.452  65.435  32.495  1.00 27.15
ATOM     19  O   ARG     1       3.493  65.038  31.801  1.00 25.37
ATOM     20  N   THR     2       5.607  64.782  32.604  1.00 22.75
ATOM     21  H   THR     2       6.326  65.034  33.220  1.00  0.00
ATOM     22  CA  THR     2       5.892  63.526  31.966  1.00 21.24
ATOM     23  CB  THR     2       7.391  63.353  31.697  1.00 22.20
"""

class Tests(IMP.test.TestCase):
    def test_charmm_patches(self):
        """Test CHARMM patches"""
        m = IMP.Model()
        sio = BytesIO(pdb)
        prot = IMP.atom.read_pdb(sio, m);
        # Read in the CHARMM heavy atom topology and parameter files
        ff = IMP.atom.get_all_atom_CHARMM_parameters()
        topology = ff.create_topology(prot)

        topology.apply_default_patches()

        topology.setup_hierarchy(prot)
        atoms = IMP.atom.get_by_type(prot, IMP.atom.ATOM_TYPE)

        # Molecular Dynamics
        md = IMP.atom.MolecularDynamics(m)
        pis = md.get_simulation_particle_indexes()
        # Any new coordinates added (e.g. by patching)
        # should be marked as optimizable (see issue #533)
        self.assertEqual(sorted(pis),
                         sorted(x.get_particle_index() for x in atoms))

if __name__ == '__main__':
    IMP.test.main()
