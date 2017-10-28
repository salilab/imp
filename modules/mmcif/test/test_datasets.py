from __future__ import print_function
import IMP.test
import IMP.mmcif.dataset

class Tests(IMP.test.TestCase):
    def test_duplicate_datasets_details(self):
        """Datasets with differing details should be considered duplicates"""
        fname = self.get_input_file_name('test.nup84.pdb')
        l1 = IMP.mmcif.dataset.FileLocation(fname, details='test details')
        d1 = IMP.mmcif.dataset.PDBDataset(l1)

        l2 = IMP.mmcif.dataset.FileLocation(fname, details='other details')
        d2 = IMP.mmcif.dataset.PDBDataset(l2)
        self.assertEqual(l1, l2)

    def test_duplicate_locations(self):
        """Datasets with same location should be considered duplicates"""
        fname1 = self.get_input_file_name('test.nup84.pdb')
        fname2 = self.get_input_file_name('test.nup85.pdb')
        loc1 = IMP.mmcif.dataset.FileLocation(fname1)
        loc2 = IMP.mmcif.dataset.FileLocation(fname2)

        # Identical datasets in the same location aren't duplicated
        pdb1 = IMP.mmcif.dataset.PDBDataset(loc1)
        pdb2 = IMP.mmcif.dataset.PDBDataset(loc1)
        self.assertEqual(pdb1, pdb2)

        # Datasets in different locations are OK
        pdb1 = IMP.mmcif.dataset.PDBDataset(loc1)
        pdb2 = IMP.mmcif.dataset.PDBDataset(loc2)
        self.assertNotEqual(pdb1, pdb2)


if __name__ == '__main__':
    IMP.test.main()
