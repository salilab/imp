import unittest
import IMP, IMP.test
import random

class OnePair(IMP.PairScore):
    def __init__(self):
        IMP.PairScore.__init__(self)
    def evaluate(self, pa, pb, da):
        return 1
    def get_version_info(self):
        return IMP.VersionInfo("Me", "0.5")
    def show(self, t):
        print "One Pair"

class OneScore(IMP.UnaryFunction):
    def __init__(self):
        IMP.UnaryFunction.__init__(self)
    def __call__(self, *args):
        return 100
    def show(self, *args):
        print "One score"

class TestNBL(IMP.test.TestCase):
    def setUp(self):
        IMP.set_log_level(IMP.VERBOSE)

    def test_it(self):
        """Test the nonbonded list and restraint which uses it"""
        m= IMP.Model()
        for i in range(0,100):
            p= IMP.Particle()
            m.add_particle(p)
            d=IMP.XYZDecorator.create(p)
            d.randomize_in_box(IMP.Vector3D(0,0,0),
                               IMP.Vector3D(10,10,10));
        s= IMP.AllNonbondedListScoreState(m.get_particles())
        m.add_score_state(s)
        o= OnePair()
        r= IMP.NonbondedRestraint(o, s, 15)
        m.add_restraint(r)
        score= m.evaluate(False)
        self.assertEqual(score, 4950, "Wrong score")

    def test_bl(self):
        """Test the bonded list"""
        m= IMP.Model()
        bds=[]
        for i in range(0,100):
            p= IMP.Particle()
            m.add_particle(p)
            d=IMP.XYZDecorator.create(p)
            d.set_x(0)
            d.set_y(10)
            d.set_z(10)
            bds.append(IMP.BondedDecorator.create(p))
        pts=IMP.Particles()
        for p in m.get_particles():
            pts.append(p)
        for i in range(1,100):
            IMP.custom_bond(bds[i-1], bds[i], 1, .1)
        s= IMP.AllNonbondedListScoreState(pts, 1)
        b= IMP.BondDecoratorListScoreState(pts)
        s.add_bonded_list(b)
        m.add_score_state(s)
        o= OnePair()
        r= IMP.NonbondedRestraint(o, s, 10)
        os=OneScore()
        print os(6)
        br= IMP.BondDecoratorRestraint(os, b)
        m.add_restraint(r)
        m.add_restraint(br)
        score= m.evaluate( False )
        print "Score with bonds is " + str(score)
        self.assertEqual(score, 9900+4950-99, "Wrong score")

    def test_distfilt(self):
        """Test filtering based on distance in nonbonded list"""
        m= IMP.Model()
        ps=IMP.Particles()
        for i in range(0,50):
            p= IMP.Particle()
            m.add_particle(p)
            d=IMP.XYZDecorator.create(p)
            ps.append(p)
            if (i < 25):
                d.randomize_in_box(IMP.Vector3D(0,0,0),
                                   IMP.Vector3D(10,10,10));
            else:
                d.randomize_in_box(IMP.Vector3D(60,60,60),
                                   IMP.Vector3D(70,70,70));
        s= IMP.AllNonbondedListScoreState(ps)
        m.add_score_state(s)
        o= OnePair()
        r= IMP.NonbondedRestraint(o, s, 15)
        m.add_restraint(r)
        score= m.evaluate(False)
        self.assertEqual(score, 1225, "Wrong score")
    def test_bi(self):
        """Test the bipartite nonbonded list and restraint which uses it"""
        m= IMP.Model()
        ps0=IMP.Particles()
        ps1=IMP.Particles()
        for i in range(0,10):
            p= IMP.Particle()
            m.add_particle(p)
            d=IMP.XYZDecorator.create(p)
            d.randomize_in_box(IMP.Vector3D(0,0,0),
                               IMP.Vector3D(10,10,10));
            if (i < 5):
                ps0.append(p)
            else:
                ps1.append(p)
        s= IMP.BipartiteNonbondedListScoreState(ps0, ps1, 1000)
        m.add_score_state(s)
        o= OnePair()
        r= IMP.NonbondedRestraint(o, s, 15)
        m.add_restraint(r)
        score= m.evaluate(False)
        self.assertEqual(score, 25, "Wrong score")
    def test_spheres2(self):
        """Test the nonbonded list of spheres (num pairs)"""
        # This uses the internal checks of the nonbonded list to
        # verify correctness
        m= IMP.Model()
        rk= IMP.FloatKey("radius")
        for i in range(0,100):
            p= IMP.Particle()
            m.add_particle(p)
            d=IMP.XYZDecorator.create(p)
            d.randomize_in_box(IMP.Vector3D(0,0,0),
                               IMP.Vector3D(10,10,10));
            p.add_attribute(rk, i, False)
            d.set_coordinates_are_optimized(True)
        s= IMP.AllSphereNonbondedListScoreState(m.get_particles(), rk)
        m.add_score_state(s)
        o= OnePair()
        r= IMP.NonbondedRestraint(o, s, 15)
        m.add_restraint(r)
        score= m.evaluate(False)
        self.assertEqual(score, 4950, "Wrong score")
    def test_frido_spheres(self):
        """Test the nonbonded list with frido's spheres"""
        m= IMP.Model()
        rk= IMP.FloatKey("radius")
        print "Frido begin"
        p=IMP.Particle()
        m.add_particle(p)
        d=IMP.XYZDecorator.create(p)
        d.set_x(87.0621490479)
        d.set_y(140.299957275)
        d.set_z(76.7119979858)
        d.set_coordinates_are_optimized(True)
        p.add_attribute(rk, 20.1244087219, False)
        p=IMP.Particle()
        m.add_particle(p)
        d=IMP.XYZDecorator.create(p)
        d.set_x(80.805770874)
        d.set_y(99.9667434692)
        d.set_z(66.9167098999)
        d.set_coordinates_are_optimized(True)
        p.add_attribute(rk, 19.8160457611, False)
        p=IMP.Particle()
        m.add_particle(p)
        d=IMP.XYZDecorator.create(p)
        d.set_x(112.992515564)
        d.set_y(119.602111816)
        d.set_z(53.6557235718)
        d.set_coordinates_are_optimized(True)
        p.add_attribute(rk, 20.3428726196, False)
        p=IMP.Particle()
        m.add_particle(p)
        d=IMP.XYZDecorator.create(p)
        d.set_x(118.784294128)
        d.set_y(86.842376709)
        d.set_z(65.2264938354)
        d.set_coordinates_are_optimized(True)
        p.add_attribute(rk, 20.1794681549, False)
        p=IMP.Particle()
        m.add_particle(p)
        d=IMP.XYZDecorator.create(p)
        d.set_x(132.930664062)
        d.set_y(85.9250793457)
        d.set_z(62.0034713745)
        d.set_coordinates_are_optimized(True)
        p.add_attribute(rk, 19.730260849, False)
        p=IMP.Particle()
        m.add_particle(p)
        d=IMP.XYZDecorator.create(p)
        d.set_x(97.6803894043)
        d.set_y(77.0734939575)
        d.set_z(45.9188423157)
        d.set_coordinates_are_optimized(True)
        p.add_attribute(rk, 20.0133743286, False)
        p=IMP.Particle()
        m.add_particle(p)
        d=IMP.XYZDecorator.create(p)
        d.set_x(74.9787445068)
        d.set_y(103.48789978)
        d.set_z(65.8464813232)
        d.set_coordinates_are_optimized(True)
        p.add_attribute(rk, 20.1519756317, False)
        p=IMP.Particle()
        m.add_particle(p)
        d=IMP.XYZDecorator.create(p)
        d.set_x(74.5077972412)
        d.set_y(114.997116089)
        d.set_z(103.185188293)
        d.set_coordinates_are_optimized(True)
        p.add_attribute(rk, 18.6368904114, False)
        p=IMP.Particle()
        m.add_particle(p)
        d=IMP.XYZDecorator.create(p)
        d.set_x(110.806472778)
        d.set_y(100.937240601)
        d.set_z(91.8201828003)
        d.set_coordinates_are_optimized(True)
        p.add_attribute(rk, 19.2294254303, False)
        p=IMP.Particle()
        m.add_particle(p)
        d=IMP.XYZDecorator.create(p)
        d.set_x(125.188072205)
        d.set_y(135.360610962)
        d.set_z(84.0617752075)
        d.set_coordinates_are_optimized(True)
        p.add_attribute(rk, 18.9221935272, False)
        p=IMP.Particle()
        m.add_particle(p)
        d=IMP.XYZDecorator.create(p)
        d.set_x(122.894897461)
        d.set_y(114.53062439)
        d.set_z(92.5285186768)
        d.set_coordinates_are_optimized(True)
        p.add_attribute(rk, 18.9533672333, False)
        p=IMP.Particle()
        m.add_particle(p)
        d=IMP.XYZDecorator.create(p)
        d.set_x(124.656105042)
        d.set_y(88.4534988403)
        d.set_z(99.5341949463)
        d.set_coordinates_are_optimized(True)
        p.add_attribute(rk, 18.8280506134, False)
        p=IMP.Particle()
        m.add_particle(p)
        d=IMP.XYZDecorator.create(p)
        d.set_x(109.428604126)
        d.set_y(60.1015129089)
        d.set_z(79.1919250488)
        d.set_coordinates_are_optimized(True)
        p.add_attribute(rk, 19.1991424561, False)
        p=IMP.Particle()
        m.add_particle(p)
        d=IMP.XYZDecorator.create(p)
        d.set_x(79.7605895996)
        d.set_y(78.1874771118)
        d.set_z(95.7365951538)
        d.set_coordinates_are_optimized(True)
        p.add_attribute(rk, 19.3197059631, False)
        for p in m.get_particles():
            d= IMP.XYZDecorator.cast(p)
            print ".sphere "+str(d.get_x())+ " " + str(d.get_y())\
                + " " + str(d.get_z()) + " " + str(p.get_value(rk))

        s= IMP.AllSphereNonbondedListScoreState(m.get_particles(), rk)
        m.add_score_state(s)
        sd= IMP.SphereDistancePairScore(IMP.HarmonicLowerBound(0,1),
                                        rk)
        r= IMP.NonbondedRestraint(sd, s, 1)
        m.add_restraint(r)
        score= m.evaluate(False)
        print "Frido score is"
        print score
    def test_spheres(self):
        """Test the nonbonded list of spheres (collision detection)"""
        m= IMP.Model()
        rk= IMP.FloatKey("radius")
        for i in range(0,100):
            p= IMP.Particle()
            m.add_particle(p)
            d=IMP.XYZDecorator.create(p)
            d.randomize_in_box(IMP.Vector3D(0,0,0),
                               IMP.Vector3D(20,20,20));
            p.add_attribute(rk, random.uniform(0,100), False)
            d.set_coordinates_are_optimized(True)
        s= IMP.AllSphereNonbondedListScoreState(m.get_particles(), rk)
        m.add_score_state(s)
        sd= IMP.SphereDistancePairScore(IMP.HarmonicLowerBound(0,1),
                                        rk)
        r= IMP.NonbondedRestraint(sd, s, 1)
        m.add_restraint(r)
        score= m.evaluate(False)
        opt= IMP.ConjugateGradients()
        opt.set_model(m)
        IMP.set_log_level(IMP.TERSE)
        score =opt.optimize(10000)
        print score
        for p in m.get_particles():
            dp= IMP.XYZDecorator.cast(p)
            print ".sphere "+str(dp.get_x()) + " " + str(dp.get_y())\
                + " " + str(dp.get_z()) + " " +str( p.get_value(rk))
        for p in m.get_particles():
            p.show()
            dp= IMP.XYZDecorator.cast(p)
            for q in m.get_particles():
                dq= IMP.XYZDecorator.cast(q)
                if (p.get_index() != q.get_index()):
                    d = IMP.distance(dp,dq)
                    rd= p.get_value(rk) + q.get_value(rk)
                    if (rd > d):
                        p.show()
                        q.show()
                        print d
                        print rd
                    # Allow a little extra, for imperfect optimization:
                    self.assert_(rd <= d + 1e-3,
                                 "Some spheres are not repelled")


if __name__ == '__main__':
    unittest.main()
