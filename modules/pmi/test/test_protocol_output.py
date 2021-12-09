from __future__ import print_function
import os
import IMP.test
import IMP.pmi.output
import IMP.pmi.mmcif
import IMP.pmi.macros
import ihm.dumper


class TestPO(IMP.pmi.output.ProtocolOutput):
    _file_datasets = []
    _each_metadata = []
    def _add_state(self, obj):
        return self


class Tests(IMP.test.TestCase):

    def test_prot_add(self):
        """Test Representation.add_protocol_output()"""
        m = IMP.Model()
        s = IMP.pmi.topology.System(m)
        po = TestPO()
        s.add_protocol_output(po)

    def test_full_topology_reader(self):
        """Test full run with TopologyReader"""
        try:
            import sklearn
        except ImportError:
            self.skipTest("no sklearn package")
        mdl = IMP.Model()
        po = IMP.pmi.mmcif.ProtocolOutput()
        tfile = self.get_input_file_name('topology_new.txt')
        input_dir = os.path.dirname(tfile)
        t = IMP.pmi.topology.TopologyReader(tfile, pdb_dir=input_dir,
                                            fasta_dir=input_dir,
                                            gmm_dir=input_dir)
        bs = IMP.pmi.macros.BuildSystem(mdl)
        bs.system.add_protocol_output(po)
        bs.add_state(t)
        root_hier, dof = bs.execute_macro()
        po.finalize()
        with open('test_full_topology_reader.cif', 'w') as fh:
            ihm.dumper.write(fh, [po.system])
        os.unlink('test_full_topology_reader.cif')


if __name__ == '__main__':
    IMP.test.main()
