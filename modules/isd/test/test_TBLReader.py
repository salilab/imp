#!/usr/bin/env python

#general imports
import os,tempfile

#imp general
import IMP

#our project
from IMP.isd.TBLReader import TBLReader

#unit testing framework
import IMP.test

class TestTBLReader(IMP.test.TestCase):

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        self.tmpfile = None
        self.sequence = ["MET", "GLN", "ILE",
                         "PHE", "VAL", "LYS",
                         "THR", "LEU", "THR",
                         "GLY", "LYS", "THR",
                         "ILE", "THR", "LEU"]

    def writetotbl(self, data):
        handle, name = tempfile.mkstemp(suffix='.tbl', text=True)
        for line in data:
            os.write(handle, line+'\n')
        os.close(handle)
        return name

    def test_nonambiguous(self):
        "test whether the reader can read nonambiguous TBL files"
        tbl = TBLReader(self.sequence)
        data = [
        "assign (resid 1 and name HA2 ) (resid 10 and name HA3 ) 1.75 1.0 2.0",
        "assign (resid 15 and name HA2 ) (resid 11 and name HA ) 4.65 0.0 0.0"]
        name = self.writetotbl(data)
        res = tbl.read_distances(name, 'test')['test']
        #distance
        self.assertAlmostEqual(res[0][1], 1.75, delta=1e-7)
        self.assertAlmostEqual(res[1][1], 4.65, delta=1e-7)
        #lower
        self.assertAlmostEqual(res[0][2], 0.75, delta=1e-7)
        #upper
        self.assertAlmostEqual(res[0][3], 3.75, delta=1e-7)
        #volume
        self.assertAlmostEqual(res[0][4], 1.75**(-6), delta=1e-7)
        self.assertAlmostEqual(res[1][4], 4.65**(-6), delta=1e-7)
        #contributions : format
        self.assertEqual(len(res[0][0]),1)
        self.assertEqual(len(res[1][0]),1)
        self.assertEqual(len(res[0][0][0]),2)
        self.assertEqual(len(res[1][0][0]),2)
        #contributions : value
        self.assertEqual(res[0][0][0][0],(0, 'HA2'))
        self.assertEqual(res[0][0][0][1],(9, 'HA3'))
        self.assertEqual(res[1][0][0][0],(14, 'HA2'))
        self.assertEqual(res[1][0][0][1],(10, 'HA'))


    def tearDown(self):
        IMP.test.TestCase.tearDown(self)
        if self.tmpfile:
            os.remove(self.tmpfile)

if __name__ == '__main__':
    IMP.test.main()
