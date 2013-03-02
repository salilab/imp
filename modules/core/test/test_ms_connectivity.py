import os
import IMP
import IMP.test
import IMP.core
import IMP.algebra

class Tests(IMP.test.TestCase):
    """Class to test MS connectivity restraints"""

    def setUp(self):
        """Build test model."""
        IMP.test.TestCase.setUp(self)

        # Setup for example 1
        self.m = IMP.Model()
        self.ps= self.create_particles_in_box(self.m,6)
        self.ds= map(lambda p: IMP.core.XYZ.decorate_particle(p), self.ps)
        ub = IMP.core.HarmonicUpperBound(1.0, 0.1)
        self.ss= IMP.core.DistancePairScore(ub)
        self.r= IMP.core.MSConnectivityRestraint(self.ss)
        pa = self.r.add_type([self.ds[0], self.ds[1]])
        pb = self.r.add_type([self.ds[2], self.ds[3]])
        pc = self.r.add_type([self.ds[4]])
        pd = self.r.add_type([self.ds[5]])
        i1 = self.r.add_composite([pa, pa, pb, pb, pc])
        i2 = self.r.add_composite([pa, pb, pb, pc], i1)
        i3 = self.r.add_composite([pa, pa, pb, pb], i1)
        i4 = self.r.add_composite([pa, pb], i1)
        i5 = self.r.add_composite([pa, pb, pb], i2)
        i6 = self.r.add_composite([pb, pc], i2)
        i7 = self.r.add_composite([pa, pa, pb], i3)
        i8 = self.r.add_composite([pa, pb], i5)
        idk = IMP.StringKey("id")
        self.ds[0].add_attribute(idk, "A")
        self.ds[1].add_attribute(idk, "A")
        self.ds[2].add_attribute(idk, "B")
        self.ds[3].add_attribute(idk, "B")
        self.ds[4].add_attribute(idk, "C")
        self.ds[5].add_attribute(idk, "D")
        self.m.add_restraint(self.r)

        # Setup for example 2
        self.m2 = IMP.Model()
        self.ps2= self.create_particles_in_box(self.m2,8)
        self.ds2= map(lambda p: IMP.core.XYZ.decorate_particle(p), self.ps2)
        self.r2= IMP.core.MSConnectivityRestraint(self.ss)
        ppa = self.r2.add_type([self.ds2[0], self.ds2[1], self.ds2[2]])
        ppb = self.r2.add_type([self.ds2[3], self.ds2[4]])
        ppc = self.r2.add_type([self.ds2[5], self.ds2[6]])
        ppd = self.r2.add_type([self.ds2[7]])
        ii1 = self.r2.add_composite([ppa, ppa, ppa, ppb, ppb, ppc, ppc, ppd])
        ii2 = self.r2.add_composite([ppa, ppa, ppb, ppc, ppd], ii1)
        ii3 = self.r2.add_composite([ppa, ppa, ppa, ppb, ppb, ppc, ppd], ii1)
        ii4 = self.r2.add_composite([ppa, ppa, ppb, ppb, ppc, ppc, ppd], ii1)
        ii5 = self.r2.add_composite([ppa, ppa, ppa, ppb, ppc, ppc, ppd], ii1)
        ii6 = self.r2.add_composite([ppa, ppa, ppb], ii2)
        ii7 = self.r2.add_composite([ppc, ppd], ii2)
        ii8 = self.r2.add_composite([ppa, ppb, ppc, ppd], ii3)
        ii9 = self.r2.add_composite([ppa, ppa, ppa, ppb, ppc, ppc], ii5)
        ii7pp = self.r2.add_composite([ppc, ppd], ii8)
        ii10 = self.r2.add_composite([ppa, ppb], ii6)
        ii11 = self.r2.add_composite([ppa, ppa, ppc, ppc], ii9)
        self.ds2[0].add_attribute(idk, "A")
        self.ds2[1].add_attribute(idk, "A")
        self.ds2[2].add_attribute(idk, "A")
        self.ds2[3].add_attribute(idk, "B")
        self.ds2[4].add_attribute(idk, "B")
        self.ds2[5].add_attribute(idk, "C")
        self.ds2[6].add_attribute(idk, "C")
        self.ds2[7].add_attribute(idk, "D")
        self.m2.add_restraint(self.r2)

    def test_ms_connectivity_graph_1(self):
        """Test for the correctness of the graph 1."""
        IMP.base.set_log_level(IMP.base.SILENT)

        self.ds[0].set_coordinates(IMP.algebra.Vector3D(0.0, 0.0, 0.0))    #A
        self.ds[1].set_coordinates(IMP.algebra.Vector3D(1.0, 1.0, 0.0))    #A
        self.ds[2].set_coordinates(IMP.algebra.Vector3D(2.0, 0.0, 0.0))    #B
        self.ds[3].set_coordinates(IMP.algebra.Vector3D(3.0, 0.0, 0.0))    #B
        self.ds[4].set_coordinates(IMP.algebra.Vector3D(4.0, -1.0, 0.0))   #C
        self.ds[5].set_coordinates(IMP.algebra.Vector3D(1000, 1000, 1000)) #D

        idk = IMP.StringKey("id")
        connections = list()
        pps= self.r.get_connected_pairs()
        for p in pps:
            conn = [p[0].get_value(idk), p[1].get_value(idk)]
            conn.sort()
            connections.append('%s <-> %s' % (conn[0], conn[1]))
        connections.sort()
        connections = ', '.join(connections)
        self.assertEqual(connections, 'A <-> A, A <-> B, A <-> B, B <-> B, B <-> C',
            'Bad connectivity graph')

    def test_ms_connectivity_graph_2(self):
        """Test for the correctness of the graph 2."""
        IMP.base.set_log_level(IMP.base.SILENT)

        self.ds2[0].set_coordinates(IMP.algebra.Vector3D(1.0, 6.0, 0.0))  #A
        self.ds2[1].set_coordinates(IMP.algebra.Vector3D(3.0, 5.0, 0.0))  #A
        self.ds2[2].set_coordinates(IMP.algebra.Vector3D(0.0, 4.0, 0.0))  #A
        self.ds2[3].set_coordinates(IMP.algebra.Vector3D(1.0, 5.0, 0.0))  #B
        self.ds2[4].set_coordinates(IMP.algebra.Vector3D(1.0, 3.0, 0.0))  #B
        self.ds2[5].set_coordinates(IMP.algebra.Vector3D(0.0, 2.0, 0.0))  #C
        self.ds2[6].set_coordinates(IMP.algebra.Vector3D(0.0, 0.0, 0.0))  #C
        self.ds2[7].set_coordinates(IMP.algebra.Vector3D(1.0, 1.0, 0.0))  #D

        idk = IMP.StringKey("id")
        connections = list()
        pps= self.r2.get_connected_pairs()
        for p in pps:
            conn = [p[0].get_value(idk), p[1].get_value(idk)]
            conn.sort()
            connections.append('%s <-> %s' % (conn[0], conn[1]))
        connections.sort()
        connections = ', '.join(connections)
        self.assertEqual(connections, 'A <-> A, A <-> B, A <-> B, A <-> B, A <-> B, A <-> C, B <-> B, B <-> C, C <-> C, C <-> D, C <-> D')

    def test_ms_connectivity(self):
        """Test for the connectivity restraint."""
        IMP.base.set_log_level(IMP.base.SILENT)

        self.randomize_particles(self.m.get_particles(), 50.0)
        self.ds[5].set_coordinates(IMP.algebra.Vector3D(1000, 1000, 1000))

#        print 'BEFORE optimization'
#        for i in xrange(6):
#            for j in xrange(6):
#                if j > i:
#                    print "distance from %d to %d is %s" % (i, j, IMP.core.get_distance(self.ds[i], self.ds[j]))
        score= self.m.evaluate(False)
#        print 'Score = ', score

        o = IMP.core.ConjugateGradients()
        o.set_threshold(1e-4)
        o.set_model(self.m)
        o.optimize(100)

#        print 'AFTER optimization'
        dist = [[0]*6 for x in xrange(6)]
        ok   = [[0]*6 for x in xrange(6)]
        sum  = 0
        for i in xrange(6):
            i_connected = False
            for j in xrange(6):
                if j > i:
                    dist[j][i] = dist[i][j] = IMP.core.get_distance(self.ds[i], self.ds[j])
                    ok[j][i] = ok[i][j] = (dist[i][j] < 1.2)
                    sum += ok[i][j]
#                    print "distance from %d to %d is %s" % (i, j, dist[i][j])
                i_connected = i_connected or ok[i][j]
#            self.assertTrue(i_connected or i==5, "Point %d is not connected" %(i))

        score= self.m.evaluate(False)
        self.assertLess(score, 10, "Score too high")
#        print 'Score = ', score
        pps= self.r.get_connected_pairs()
        lscore = 0
        for p in pps:
            lscore = lscore+self.ss.evaluate((p[0], p[1]), None)
        self.assertAlmostEqual(score, lscore, delta=.1)
#        self.assertGreaterEqual(sum, 4, "Wrong number of close pairs")

if __name__ == '__main__':
    IMP.test.main()
