#!/usr/bin/env python

import sys,os
import tempfile
import copy

import IMP.test
import IMP.isd

class MockArgs:
    def __init__(self,**kwargs):
        for k,v in kwargs.iteritems():
            setattr(self, k, v)

class MockGP:
    def __init__(self,a,b):
        self.a=a
        self.b=b

    def get_posterior_mean(self,q):
        return self.a*((q[0]-self.b)**2+1)

    def get_posterior_covariance(self,q,r):
        return self.a*(q[0]+1)

class MockGP2:
    def __init__(self,a,b):
        self.a=a
        self.b=b

    def get_posterior_mean(self,q):
        if q[0] > self.a:
            return self.b
        else:
            return 1

    def get_posterior_covariance(self,q,r):
        return self.a*(q[0]+1)

class MockGP3:
    def __init__(self,a,b):
        self.a=a
        self.b=b

    def get_posterior_mean(self,q):
        return self.a*(q[0]+self.b)

    def get_posterior_covariance(self,q,r):
        return self.a**2

class MockFunction:

    def __call__(self,q):
        return q

class SAXSProfileTestTwo(IMP.test.ApplicationTestCase):

    def setUp(self):
        IMP.test.ApplicationTestCase.setUp(self)
        try:
            import scipy
            import numpy
        except ImportError:
            self.skipTest("could not import scipy and numpy")
        merge = self.import_python_application('saxs_merge.py')
        self.SAXSProfile = merge.SAXSProfile
        self.merge = merge

    def set_interpolant(self, profile, a, b, interpolant=MockGP):
        m=IMP.Model()
        s=IMP.isd.Scale.setup_particle(IMP.Particle(m),3.0)
        gp=interpolant(a,b)
        functions={}
        functions['mean']=MockFunction()
        functions['covariance']=MockFunction()
        profile.set_interpolant(gp, {'sigma':s}, functions, 'test', m, None)
        return gp

    def test_cleanup_simple(self):
        """Test cleanup good/bad"""
        data=[[0,0,1],[1,10,1]]
        p=self.SAXSProfile()
        p.add_data(data)
        args=MockArgs(verbose=0, aalpha=0.05, acutoff=2)
        self.merge.cleanup([p],args)
        self.assertTrue(
                set(p.get_flag_names()).issuperset(set(['agood','apvalue'])))
        test = p.get_data(colwise=True)
        self.assertEqual(len(test),6)
        self.assertFalse(test['agood'][0])
        self.assertTrue(test['agood'][1])

    def test_cleanup_cutoff(self):
        """Test cleanup q_cutoff"""
        data=[[0,0,1],[1,10,1],[2,0,1],[3,10,1]]
        p=self.SAXSProfile()
        p.add_data(data)
        args=MockArgs(verbose=0, aalpha=0.05, acutoff=1.5)
        self.merge.cleanup([p],args)
        test = p.get_data(colwise=True)
        self.assertEqual(test['agood'],[False,True,False,False])

    def test_rescaling_normal(self):
        """Test rescaling of two perfectly agreeing normal functions"""
        #data just used to set the q-range
        data=[[0,1,1,True],[1,10,1,True],[2,1,1,True],[3,10,1,True]]
        p1=self.SAXSProfile()
        p1.new_flag('agood',bool)
        p1.add_data(data)
        gp1=self.set_interpolant(p1,1,0)
        p2=self.SAXSProfile()
        p2.new_flag('agood',bool)
        p2.add_data(data)
        gp2=self.set_interpolant(p2,10,0)
        args=MockArgs(verbose=0, cmodel='normal', cnpoints=100, creference='last',
                baverage=False)
        self.assertEqual(p1.get_gamma(),1)
        self.assertEqual(p2.get_gamma(),1)
        self.assertEqual(p1.get_offset(),0)
        self.assertEqual(p2.get_offset(),0)
        self.merge.rescaling([p1,p2],args)
        self.assertTrue('cgood' in p2.get_flag_names())
        self.assertTrue('cgood' in p1.get_flag_names())
        self.assertEqual(p1.get_data(colwise=True)['cgood'],[True]*len(data))
        self.assertEqual(p2.get_data(colwise=True)['cgood'],[True]*len(data))
        self.assertAlmostEqual(p1.get_gamma(),10)
        self.assertAlmostEqual(p2.get_gamma(),1)
        self.assertAlmostEqual(p1.get_offset(),0)
        self.assertAlmostEqual(p2.get_offset(),0)

    def test_rescaling_normal_offset(self):
        """Test rescaling with offset of two perfectly agreeing functions"""
        #data just used to set the q-range
        data=[[0,1,1,True],[1,10,1,True],[2,1,1,True],[3,10,1,True]]
        p1=self.SAXSProfile()
        p1.new_flag('agood',bool)
        p1.add_data(data)
        gp1=self.set_interpolant(p1,1,0,interpolant=MockGP3)
        p2=self.SAXSProfile()
        p2.new_flag('agood',bool)
        p2.add_data(data)
        gp2=self.set_interpolant(p2,10,2,interpolant=MockGP3)
        args=MockArgs(verbose=0, cmodel='normal-offset', cnpoints=100,
                creference='last', baverage=False)
        self.assertEqual(p1.get_gamma(),1)
        self.assertEqual(p2.get_gamma(),1)
        self.assertEqual(p1.get_offset(),0)
        self.assertEqual(p2.get_offset(),0)
        d1=p1.get_mean()
        d2=p2.get_mean()
        self.merge.rescaling([p1,p2],args)
        self.assertTrue('cgood' in p2.get_flag_names())
        self.assertTrue('cgood' in p1.get_flag_names())
        self.assertEqual(p1.get_data(colwise=True)['cgood'],[True]*len(data))
        self.assertEqual(p2.get_data(colwise=True)['cgood'],[True]*len(data))
        for i1,i2,oi1,oi2 in zip(p1.get_mean(),p2.get_mean(),d1,d2):
            self.assertAlmostEqual(i1[1],i2[1]) #I
            self.assertAlmostEqual(i1[2],i2[2]) #err

    def test_rescaling_lognormal(self):
        """Test rescaling of two perfectly agreeing lognormal functions"""
        #data just used to set the q-range
        data=[[0,1,1,True],[1,10,1,True],[2,1,1,True],[3,10,1,True]]
        p1=self.SAXSProfile()
        p1.new_flag('agood',bool)
        p1.add_data(data)
        gp1=self.set_interpolant(p1,1,0)
        p2=self.SAXSProfile()
        p2.new_flag('agood',bool)
        p2.add_data(data)
        gp2=self.set_interpolant(p2,10,0)
        args=MockArgs(verbose=0, cmodel='lognormal',
                cnpoints=100,creference='last', baverage=False)
        self.assertEqual(p1.get_gamma(),1)
        self.assertEqual(p2.get_gamma(),1)
        self.merge.rescaling([p1,p2],args)
        self.assertTrue('cgood' in p2.get_flag_names())
        self.assertTrue('cgood' in p1.get_flag_names())
        self.assertEqual(p1.get_data(colwise=True)['cgood'],[True]*len(data))
        self.assertEqual(p2.get_data(colwise=True)['cgood'],[True]*len(data))
        self.assertAlmostEqual(p1.get_gamma(),10)
        self.assertAlmostEqual(p2.get_gamma(),1)

    def test_classification(self):
        """Simple classification test"""
        data=[[0,1,1,True],[1,10,1,True],[2,1,1,True],[3,10,1,True]]
        #prepare p1
        p1=self.SAXSProfile()
        p1.new_flag('agood',bool)
        p1.add_data(data[:3])
        p1.set_filename('test1')
        gp1=self.set_interpolant(p1,1.5,10,MockGP2)
        self.merge.create_intervals_from_data(p1,'agood')
        #prepare p2
        p2=self.SAXSProfile()
        p2.new_flag('agood',bool)
        p2.add_data(data)
        p2.set_filename('test2')
        gp2=self.set_interpolant(p2,2.5,10,MockGP2)
        self.merge.create_intervals_from_data(p2,'agood')
        #run classification
        args=MockArgs(verbose=0, dalpha=0.05, baverage=False)
        self.merge.classification([p1,p2],args)
        #p1
        self.assertTrue(
                set(p1.get_flag_names()).issuperset(
                    set(['drefnum','drefname','dgood','dselfref','dpvalue'])))
        test1=p1.get_data(colwise=True)
        self.assertEqual(test1['drefnum'],[0,0,0])
        self.assertEqual(test1['drefname'],['test1']*3)
        self.assertEqual(test1['dgood'], [True]*3)
        self.assertEqual(test1['dselfref'],[True]*3)
        #p2
        self.assertTrue(
                set(p2.get_flag_names()).issuperset(
                    set(['drefnum','drefname','dgood','dselfref','dpvalue'])))
        test2=p2.get_data(colwise=True)
        self.assertEqual(test2['drefnum'],[0,0,0,1])
        self.assertEqual(test2['drefname'],['test1']*3+['test2'])
        self.assertEqual(test2['dgood'], [True,True,False,True])
        self.assertEqual(test2['dselfref'],[False,False,False,True])

    def test_merging(self):
        """Simple merge test without the fitting part"""
        data=[[0,1,1,True],[1,10,1,True],[2,1,1,True],[3,10,1,True]]
        #prepare p1
        p1=self.SAXSProfile()
        p1.new_flag('agood',bool)
        p1.add_data(data[:3])
        p1.set_filename('test1')
        gp1=self.set_interpolant(p1,1.5,10,MockGP2)
        self.merge.create_intervals_from_data(p1,'agood')
        #prepare p2
        p2=self.SAXSProfile()
        p2.new_flag('agood',bool)
        p2.add_data(data)
        p2.set_filename('test2')
        gp2=self.set_interpolant(p2,2.5,10,MockGP2)
        self.merge.create_intervals_from_data(p2,'agood')
        #run classification and merging
        args=MockArgs(verbose=0, mergename="merge",
                dalpha=0.05, eextrapolate=0, enoextrapolate=False,
                baverage=False, enocomp=True, emean='Flat',
                elimit_fitting=-1, elimit_hessian=-1,
                lambdamin=0.005)
        self.merge.classification([p1,p2],args)
        def find_fit(a,b,c,model_comp=None, mean_function=None,
                        model_comp_maxpoints=None, lambdamin=0.005):
            return 'test',b,None
        self.merge.find_fit = find_fit
        def setup_process(b,c,e):
            m=IMP.Model()
            s=IMP.isd.Scale.setup_particle(IMP.Particle(m),3.0)
            gp=MockGP(1,10)
            functions={'mean':MockFunction(),'covariance':MockFunction()}
            return m,{'sigma':s},functions,gp
        self.merge.setup_process = setup_process
        merge, profiles, args = self.merge.merging([p1,p2],args)
        #p1
        test=merge.get_data(colwise=True)
        self.assertTrue(
                set(merge.get_flag_names()).issuperset(
                    set(['eorigin','eoriname'])))
        self.assertEqual(len(test['q']),6)
        self.assertEqual(set(zip(test['q'],test['eorigin'])),
                set([(0,0),(0,1),(1,0),(1,1),(2,0),(3,1)]))

if __name__ == "__main__":
    IMP.test.main()
