from __future__ import print_function
import RMF
import unittest
import shutil
import os
import sys

class Tests(unittest.TestCase):

    def get_filename_key(self, fh):
        cat = fh.get_category("provenance")
        return fh.get_key(cat, "structure filename", RMF.string_tag)

    def make_test_node(self, fh):
        fh.add_frame('zero', RMF.FRAME)
        rt = fh.get_root_node()

        f = RMF.StructureProvenanceFactory(fh)
        c0 = rt.add_child("c0", RMF.PROVENANCE)
        c = f.get(c0)
        return c0, c

    def test_paths_in_buffer(self):
        """Test paths when stored in an in-memory RMF"""
        b = RMF.BufferHandle()
        rmf = RMF.create_rmf_buffer(b)
        key = self.get_filename_key(rmf)
        node, prov = self.make_test_node(rmf)

        # In-memory RMF is considered to be in the current directory
        prov.set_filename('bar/foo.pdb')

        if sys.platform == 'win32':
            # On Windows, filenames are not altered
            self.assertEqual(node.get_value(key), 'bar/foo.pdb')
            self.assertEqual(prov.get_filename(), 'bar/foo.pdb')
        else:
            # On Mac/Linux, internally, a relative path should be stored
            self.assertEqual(node.get_value(key), 'bar/foo.pdb')
            # API should return an absolute path
            self.assertEqual(prov.get_filename(),
                             os.path.join(os.getcwd(), 'bar', 'foo.pdb'))

    def test_paths(self):
        """Test paths in a named RMF file"""
        tmpdir = RMF._get_temporary_file_path('test_paths')
        subdir = os.path.join(tmpdir, 'sub1', 'sub2')
        os.makedirs(subdir)
        rmf = RMF.create_rmf_file(os.path.join(subdir, 'test.rmf3'))
        key = self.get_filename_key(rmf)
        node, prov = self.make_test_node(rmf)

        ap = os.path.join(tmpdir, 'foo.pdb')
        prov.set_filename(ap)

        if sys.platform == 'win32':
            # On Windows, filenames are not altered
            self.assertEqual(node.get_value(key), ap)
            self.assertEqual(prov.get_filename(), ap)
        else:
            # On Mac/Linux, internally, a relative path should be stored
            self.assertEqual(node.get_value(key), '../../foo.pdb')
            # API should return an absolute path
            self.assertEqual(prov.get_filename(), ap)

        ap = os.path.join(subdir, 'foo.pdb')
        prov.set_filename(ap)

        if sys.platform == 'win32':
            self.assertEqual(node.get_value(key), ap)
            self.assertEqual(prov.get_filename(), ap)
        else:
            self.assertEqual(node.get_value(key), 'foo.pdb')
            self.assertEqual(prov.get_filename(), ap)

        del rmf
        if sys.platform != 'win32':
            shutil.rmtree(tmpdir)

    def test_parent_path(self):
        """Test parent paths in a named RMF file"""
        tmpdir = RMF._get_temporary_file_path('test_parent_path')
        subdir = os.path.join(tmpdir, 'sub1', 'sub2')
        os.makedirs(subdir)
        # Earlier versions of RMF incorrectly handled RMF files with ..
        # in their path, so check this here:
        rmf = RMF.create_rmf_file(os.path.join(subdir, '..', 'test.rmf3'))
        key = self.get_filename_key(rmf)
        node, prov = self.make_test_node(rmf)

        ap = os.path.join(subdir, 'foo.pdb')
        prov.set_filename(ap)

        if sys.platform == 'win32':
            # On Windows, filenames are not altered
            self.assertEqual(node.get_value(key), ap)
            self.assertEqual(prov.get_filename(), ap)
        else:
            # On Mac/Linux, internally, a relative path should be stored
            self.assertEqual(node.get_value(key), 'sub2/foo.pdb')
            # API should return an absolute path
            self.assertEqual(prov.get_filename(), ap)

    def test_clone_rewrite_path(self):
        """Test that clone rewrites relative paths"""
        tmpdir = RMF._get_temporary_file_path('test_clone_rewrite_path')
        subdir = os.path.join(tmpdir, 'sub1')
        subsubdir = os.path.join(subdir, 'sub2')
        os.makedirs(subsubdir)
        rmf = RMF.create_rmf_file(os.path.join(subdir, 'test.rmf3'))
        key = self.get_filename_key(rmf)
        node, prov = self.make_test_node(rmf)

        ap = os.path.join(subsubdir, 'foo.pdb')
        prov.set_filename(ap)

        if sys.platform != 'win32':
            # On Mac/Linux, internally, a relative path should be stored
            self.assertEqual(node.get_value(key), 'sub2/foo.pdb')
            # API should return an absolute path
            self.assertEqual(prov.get_filename(), ap)
        del rmf, node, prov

        inr = RMF.open_rmf_file_read_only(os.path.join(subdir, 'test.rmf3'))
        outr = RMF.create_rmf_file(os.path.join(subsubdir, 'test.rmf3'))
        RMF.clone_file_info(inr, outr)
        RMF.clone_hierarchy(inr, outr)
        RMF.clone_static_frame(inr, outr)
        rt = outr.get_root_node()
        f = RMF.StructureProvenanceFactory(outr)
        node, = rt.get_children()
        self.assertTrue(f.get_is(node))
        prov = f.get(node)
        if sys.platform != 'win32':
            # New file should have a different relative path
            key = self.get_filename_key(outr)
            self.assertEqual(node.get_value(key), 'foo.pdb')
            # API should return the same absolute path
            self.assertEqual(prov.get_filename(), ap)


if __name__ == '__main__':
    unittest.main()
