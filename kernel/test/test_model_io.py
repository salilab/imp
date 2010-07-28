import unittest
import IMP
import IMP.test
import StringIO

class YamlTests(IMP.test.TestCase):
    def _create_hetero_model(self):
        IMP.set_log_level(IMP.VERBOSE)
        m = IMP.Model()
        ps=IMP.ParticlesTemp()
        p= IMP.Particle(m)
        p.add_attribute(IMP.FloatKey("x"), 0, False)
        p.add_attribute(IMP.FloatKey("y"), 1, True)
        p.add_attribute(IMP.FloatKey("z"), 2, False)
        ps.append(p)
        p= IMP.Particle(m)
        p.add_attribute(IMP.FloatKey("y"), 4, True)
        p.add_attribute(IMP.FloatKey("z"), 7, False)
        p.add_attribute(IMP.FloatKey("q"), 9, False)
        p.add_attribute(IMP.StringKey("hi"), "there")
        ps.append(p)
        p= IMP.Particle(m)
        p.add_attribute(IMP.FloatKey("q"), 9, False)
        p.add_attribute(IMP.IntKey("int"), 1)
        p.add_attribute(IMP.ParticleKey("k"), p)
        ps.append(p)
        IMP.set_log_level(IMP.VERBOSE)
        return (m, ps)

    def _create_homo_model(self):
        IMP.set_log_level(IMP.VERBOSE)
        m = IMP.Model()
        ps=IMP.ParticlesTemp()
        p= IMP.Particle(m)
        p.add_attribute(IMP.FloatKey("x"), 0, False)
        p.add_attribute(IMP.FloatKey("y"), 1, True)
        p.add_attribute(IMP.FloatKey("z"), 2, False)
        ps.append(p)
        p= IMP.Particle(m)
        p.add_attribute(IMP.FloatKey("x"), 4, True)
        p.add_attribute(IMP.FloatKey("y"), 7, False)
        p.add_attribute(IMP.FloatKey("z"), 9, False)
        ps.append(p)
        p= IMP.Particle(m)
        p.add_attribute(IMP.FloatKey("x"), 10, True)
        p.add_attribute(IMP.FloatKey("y"), 11, False)
        p.add_attribute(IMP.FloatKey("z"), 12, False)
        ps.append(p)
        IMP.set_log_level(IMP.VERBOSE)
        return (m, ps)
    def test_yaml(self):
        """Check writing to yaml """
        (m, ps)= self._create_hetero_model()
        s= StringIO.StringIO()
        IMP.write_model(ps, s)
        first=s.getvalue()
        print first
        sto= StringIO.StringIO(first)
        ps[0].set_value(IMP.FloatKey("x"), 11)
        ps[0].add_attribute(IMP.ParticleKey("y"), ps[1])
        IMP.read_model(sto, ps)
        s= StringIO.StringIO()
        IMP.write_model(m, ps, s)
        print s.getvalue()
        self.assertEqual(first, s.getvalue())

    def test_yaml_attr(self):
        """Check writing to yaml with attributes"""
        (m, ps)= self._create_homo_model()
        s= StringIO.StringIO()
        fks= [IMP.FloatKey("x"), IMP.FloatKey("y")]
        IMP.write_model(ps, fks, s)
        first=s.getvalue()
        print first
        sto= StringIO.StringIO(first)
        ps[0].set_value(IMP.FloatKey("x"), 11)
        IMP.read_model(sto, ps, fks)
        s= StringIO.StringIO()
        IMP.write_model(ps, fks, s)
        print s.getvalue()
        self.assertEqual(first, s.getvalue())
    def test_netcdf(self):
        """Check writing to netcdf with attributes"""
        (m, ps)= self._create_homo_model()
        fks= [IMP.FloatKey("x"), IMP.FloatKey("y")]
        IMP.write_binary_model(ps, fks, self.get_tmp_file_name("test0.imp"))
        first= ps[0].get_value(IMP.FloatKey("x"))
        print first
        ps[0].set_value(IMP.FloatKey("x"), 11)
        IMP.read_binary_model(self.get_tmp_file_name("test0.imp"), ps, fks)
        self.assertEqual(first, ps[0].get_value(IMP.FloatKey("x")))




if __name__ == '__main__':
    unittest.main()
