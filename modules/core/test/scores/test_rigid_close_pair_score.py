import unittest
import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.display

rk = IMP.FloatKey("radius")

class Test(IMP.test.TestCase):
    """Tests for bond refiner"""


    def _test_rops(self):
        """Checking rigid body pair score"""
        IMP.set_log_level(IMP.VERBOSE)
        m= IMP.Model()
        threshold=.3
        psa= IMP.core.create_xyzr_particles(m, 50, 3)
        psb= IMP.core.create_xyzr_particles(m, 50, 3)
        rba= IMP.core.RigidBodyDecorator.create(IMP.Particle(m), psa)
        rbb= IMP.core.RigidBodyDecorator.create(IMP.Particle(m), psb)
        cp= IMP.Particle(m)
        cpd= IMP.core.XYZRDecorator.create(cp, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(0,0,0), 1))
        tps= IMP.test.LogPairScore(1)
        rps= IMP.core.RigidClosePairScore(tps, threshold)
        vc= rps.evaluate(rba.get_particle(), rbb.get_particle(), None)
        for a in psa:
            for b in psb:
                da= IMP.core.XYZRDecorator(a)
                db= IMP.core.XYZRDecorator(b)
                dist=IMP.core.distance(da, db)
                if dist < threshold -.1:
                    da.show()
                    db.show()
                    print ": "+str(dist)
                    tps.get_log().index((a,b))
                    print "Found " + a.get_name() + " " + b.get_name()
        tps.clear_log()
        vc=rps.evaluate(cp, rba.get_particle(), None)
        for a in psa:
            da= IMP.core.XYZRDecorator(a)
            dist=IMP.core.distance(da, cpd)
            if dist < threshold -.1:
                tps.get_log().index((cpd.get_particle(), a))
                print "Found " + a.get_name() + " " + b.get_name()

    def test_rops(self):
        """Checking rigid body pair score on a protein"""
        IMP.set_log_level(IMP.VERBOSE)
        m= IMP.Model()
        threshold=1
        pa= IMP.atom.read_pdb(self.get_input_file_name("input.pdb"),m)
        pb= IMP.atom.read_pdb(self.get_input_file_name("input.pdb"),m)
        rba= IMP.core.RigidBodyDecorator.create(pa.get_particle(),
                                                IMP.atom.get_by_type(pa,
                                                                     IMP.atom.MolecularHierarchyDecorator.ATOM))
        rbb= IMP.core.RigidBodyDecorator.create(pb.get_particle(),
                                                IMP.atom.get_by_type(pb,
                                                                     IMP.atom.MolecularHierarchyDecorator.ATOM))
        rbb.set_transformation(IMP.algebra.compose(rbb.get_transformation(),
                                                   IMP.algebra.Transformation3D(IMP.algebra.Vector3D(0,5,0))))
        tps= IMP.core.DistancePairScore(IMP.core.Harmonic(0,1))
        rps= IMP.core.RigidClosePairScore(tps, threshold)
        vc= rps.evaluate(rba.get_particle(), rbb.get_particle(), None)
        treea= rps.get_tree(rba.get_particle())
        treeb= rps.get_tree(rbb.get_particle())
        rps.show_tree(rba.get_particle())
        rps.show_tree(rbb.get_particle())
        wa= IMP.display.ChimeraWriter(self.get_tmp_file_name("treea.py"))
        for i in range(0, len(treea)):
            g=IMP.display.SphereGeometry(treea[i])
            g.set_name(str("Node "+ str(i)))
            wa.add_geometry(g)
        wb= IMP.display.ChimeraWriter(self.get_tmp_file_name("treeb.py"))
        for i in range(0, len(treeb)):
            g=IMP.display.SphereGeometry(treeb[i])
            g.set_name(str("Node "+ str(i)))
            wb.add_geometry(g)
        lsp=rps.get_last_sphere_pairs()
        wa= IMP.display.ChimeraWriter(self.get_tmp_file_name("exploreda.py"))
        wb= IMP.display.ChimeraWriter(self.get_tmp_file_name("exploredb.py"))
        for i in range(0, len(lsp)):
            g=IMP.display.SphereGeometry(lsp[i][0])
            g.set_name(str("Node "+ str(i)))
            wa.add_geometry(g)
            g=IMP.display.SphereGeometry(lsp[i][1])
            g.set_name(str("Node "+ str(i)))
            wb.add_geometry(g)
if __name__ == '__main__':
    unittest.main()
