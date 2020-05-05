from __future__ import print_function
import IMP
import IMP.test
import IMP.pmi.restraints.em2d
import os

class Tests(IMP.test.TestCase):
    def test_restraint(self):
        """Basic test of EM2D restraint"""
        m = IMP.Model()
        s = IMP.pmi.topology.System(m)
        seqs = IMP.pmi.topology.Sequences(
                self.get_input_file_name('seqs.fasta'))
        st1 = s.create_state()

        m1 = st1.create_molecule("Prot1", sequence=seqs["Protein_1"])
        a1 = m1.add_structure(self.get_input_file_name('prot.pdb'),
                              chain_id='A', res_range=(55,63), offset=-54)
        m1.add_representation(a1, resolutions=1)
        m1.add_representation(m1.get_residues()-a1, resolutions=1)
        hier = s.build()

        images = [self.get_input_file_name('image_1_binary.pgm')]
        r = IMP.pmi.restraints.em2d.ElectronMicroscopy2D(
                hier, images=images, pixel_size=2.2, image_resolution=20,
                resolution=1, projection_number=20)
        r.add_to_model()
        r.set_weight(1.0)
        os.unlink("images.pgm")

if __name__ == '__main__':
    IMP.test.main()
