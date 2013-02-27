#!/usr/bin/env python

import sys,os
import tempfile
import shutil

import IMP.test
import IMP.isd
try:
    import numpy
except ImportError:
    numpy = None

class MockGP:
    def __init__(self,a,b):
        self.a=a
        self.b=b

    def get_posterior_mean(self,q):
        return self.a*q[0]**2+self.b

    def get_posterior_covariance(self,q,r):
        return self.a*q[0] + self.b

class MockFunction:

    def __call__(self,q):
        return q

class SAXSProfileTest(IMP.test.ApplicationTestCase):

    def setUp(self):
        IMP.test.ApplicationTestCase.setUp(self)
        if numpy is None:
            self.skipTest("could not import numpy")
        try:
            import scipy
        except ImportError:
            self.skipTest("could not import scipy")
        merge = self.import_python_application('saxs_merge.py')
        self.SAXSProfile = merge.SAXSProfile
        self.tempdir = None

    def tearDown(self):
        if self.tempdir:
            shutil.rmtree(self.tempdir, ignore_errors=True)
        IMP.test.ApplicationTestCase.tearDown(self)

    def write_data(self, data):
        self.tempdir = tempfile.mkdtemp()
        self.datafile = os.path.join(self.tempdir, 'data')
        datalines = []
        for i in data:
            datalines.append(' '.join(['%f' % k for k in i]))
        datastr='\n'.join(datalines) + '\n'
        fl = open(self.datafile, 'w')
        fl.write(datastr)
        fl.close()

    def test_get_flag_names(self):
        """test get_flag_names """
        #get_flag_names returns ("q", "I", "err") when no flags were added
        p=self.SAXSProfile()
        test = p.get_flag_names()
        self.assertEqual(test, ("q", "I", "err"))

    def test_new_flag(self):
        """test new_flag """
        #get_flag_names returns ("q", "I", "err") when no flags were added
        p=self.SAXSProfile()
        self.assertEqual(p.new_flag("test",float), None)
        test = p.get_flag_names()
        self.assertEqual(test, ("q", "I", "err", "test"))

    def test_get_data(self):
        """test get_data """
        #get_data returns (id, q, I, err) when no flags were added
        #id is an integer identifier, starting at 0
        data = [[0,0.1,3],[0.01,10,3.5],[0.02,21,4.0]]
        self.write_data(data)
        p=self.SAXSProfile()
        p.add_data(self.datafile)
        test = p.get_data()
        for ident,(d,q) in enumerate(zip(data,test)):
            for i,v in enumerate(q):
                if i == 0:
                    self.assertEqual(v,ident)
                elif 1 <= i <=3:
                    self.assertAlmostEqual(v,data[ident][i-1])
                else:
                    self.fail("get_data returned more than 4 elements")

    def test_get_data_colwise(self):
        """test get_data colwise """
        data = [[0,0.1,3],[0.01,10,3.5],[0.02,21,4.0]]
        self.write_data(data)
        p=self.SAXSProfile()
        p.add_data(self.datafile)
        test = p.get_data(colwise=True)
        self.assertEqual(len(test),4)
        self.assertEqual(set(test.keys()),set(['id','q','I','err']))
        self.assertEqual(test['id'],range(3))
        self.assertEqual(test['q'],[i[0] for i in data])
        self.assertEqual(test['I'],[i[1] for i in data])
        self.assertEqual(test['err'],[i[2] for i in data])

    def test_get_raw_data_colwise(self):
        """test get_raw_data colwise """
        data = [[0,0.1,3],[0.01,10,3.5],[0.02,21,4.0]]
        self.write_data(data)
        p=self.SAXSProfile()
        p.add_data(self.datafile)
        test = p.get_raw_data(colwise=True)
        self.assertEqual(len(test),3)
        self.assertEqual(set(test.keys()),set(['q','I','err']))
        self.assertEqual(test['q'],[i[0] for i in data])
        self.assertEqual(test['I'],[i[1] for i in data])
        self.assertEqual(test['err'],[i[2] for i in data])

    def test_get_data_2(self):
        """test get_data with gamma != 1 """
        data = [[0,1,3],[0.01,10,3.5],[0.02,21,4.0]]
        gamma = 1.5
        self.write_data(data)
        p=self.SAXSProfile()
        p.add_data(self.datafile)
        p.set_gamma(gamma)
        test = p.get_data()
        for ident,(d,q) in enumerate(zip(data,test)):
            for i,v in enumerate(q):
                if i == 0:
                    self.assertEqual(v,ident)
                elif i == 1:
                    self.assertAlmostEqual(v,data[ident][i-1])
                elif 2 <= i <=3:
                    self.assertAlmostEqual(v,gamma*data[ident][i-1])
                else:
                    self.fail("get_data returned more than 4 elements")

    def test_get_data_3(self):
        """test get_data with gamma != 1 twice"""
        data = [[0,1,3],[0.01,10,3.5],[0.02,21,4.0]]
        gamma = 1.5
        self.write_data(data)
        p=self.SAXSProfile()
        p.add_data(self.datafile)
        p.set_gamma(gamma)
        p.get_data()
        test = p.get_data()
        for ident,(d,q) in enumerate(zip(data,test)):
            for i,v in enumerate(q):
                if i == 0:
                    self.assertEqual(v,ident)
                elif i == 1:
                    self.assertAlmostEqual(v,data[ident][i-1])
                elif 2 <= i <=3:
                    self.assertAlmostEqual(v,gamma*data[ident][i-1])
                else:
                    self.fail("get_data returned more than 4 elements")

    def test_get_data_4(self):
        """test get_data with gamma != 1 and offset != 1 """
        data = [[0,1,3],[0.01,10,3.5],[0.02,21,4.0]]
        gamma = 1.5
        offset=3
        self.write_data(data)
        p=self.SAXSProfile()
        p.add_data(self.datafile)
        p.set_gamma(gamma)
        p.set_offset(offset)
        test = p.get_data()
        for ident,(d,q) in enumerate(zip(data,test)):
            for i,v in enumerate(q):
                if i == 0:
                    self.assertEqual(v,ident)
                elif i == 1:
                    self.assertAlmostEqual(v,data[ident][i-1])
                elif i == 2:
                    self.assertAlmostEqual(v,gamma*(data[ident][i-1]+offset))
                elif i == 3:
                    self.assertAlmostEqual(v,gamma*data[ident][i-1])
                else:
                    self.fail("get_data returned more than 4 elements")

    def test_get_data_within_range(self):
        """test get_data with a specified range """
        #get_data returns (id, q, I, err) when no flags were added
        #id is an integer identifier, starting at 0
        data = [[0,0.1,3],[0.01,10,3.5],[0.02,21,4.0]]
        p=self.SAXSProfile()
        p.add_data(data)
        test = p.get_data(0.005,0.015)
        self.assertEqual(len(test),1)
        self.assertEqual(test[0][0],1)
        self.assertEqual(test[0][1],0.01)
        self.assertEqual(test[0][2],10)
        self.assertEqual(test[0][3],3.5)

    def test_get_data_failures(self):
        """get_data should fail with the wrong number of arguments"""
        p=self.SAXSProfile()
        #argument size mismatch
        self.assertRaises(TypeError, p.get_data, 1.0)
        self.assertRaises(TypeError, p.get_data, 1.0,"a","c")
        #argument type mismatch
        self.assertRaises(TypeError, p.get_data, "a","c")
        self.assertRaises(TypeError, p.get_data, 1.0,"c")
        self.assertRaises(TypeError, p.get_data, "c",1.0)
        #values should be ordered
        self.assertRaises(ValueError, p.get_data, 5.0,1.0)

    def test_get_data_sorting(self):
        """test get_data for correct sorting """
        data = [[0,0.1,3],[0.05,10,3.5],[0.02,21,4.0]]
        self.write_data(data)
        p=self.SAXSProfile()
        p.add_data(self.datafile)
        test = p.get_data()
        self.assertEqual(test[0][1:],tuple(data[0]))
        self.assertEqual(test[1][1:],tuple(data[2]))
        self.assertEqual(test[2][1:],tuple(data[1]))

    def test_get_data_sorting_2(self):
        """test get_data for correct sorting with two data sets """
        data = [[0,0.1,3],[0.05,10,3.5],[0.02,21,4.0]]
        data2 = [[0.01,0.1,3],[0.04,10,3.5],[0.012,21,4.0]]
        self.write_data(data)
        p=self.SAXSProfile()
        p.add_data(data)
        p.add_data(data2)
        data = data + data2
        data.sort(key=lambda a:a[0])
        test = p.get_data()
        for i,(d,q) in enumerate(zip(data,test)):
            self.assertEqual(tuple(d),q[1:])
            self.assertEqual(q[0],i)

    def test_get_raw_data_sorting(self):
        """test get_raw_data for correct sorting """
        data = [[0,0.1,3],[0.05,10,3.5],[0.02,21,4.0]]
        self.write_data(data)
        p=self.SAXSProfile()
        p.add_data(self.datafile)
        test = p.get_raw_data()
        self.assertEqual(test[0],data[0])
        self.assertEqual(test[1],data[2])
        self.assertEqual(test[2],data[1])

    def test_get_raw_data_sorting_2(self):
        """test get_raw_data for correct sorting with two data sets"""
        data = [[0,0.1,3],[0.05,10,3.5],[0.02,21,4.0]]
        data2 = [[0.01,0.1,3],[0.04,10,3.5],[0.012,21,4.0]]
        self.write_data(data)
        p=self.SAXSProfile()
        p.add_data(data)
        p.add_data(data2)
        data = data + data2
        data.sort(key=lambda a:a[0])
        test = p.get_raw_data()
        for d,q in zip(data,test):
            self.assertEqual(d,q)

    def test_get_data_copy(self):
        """get_data should copy the data, not reference it"""
        data = [[0,0.1,3],[0.01,10,3.5],[0.02,21,4.0]]
        p=self.SAXSProfile()
        p.add_data(data)
        data[1][2]=3.1
        test = p.get_data()
        self.assertAlmostEqual(test[1][3],3.5)

    def test_get_raw_data_copy(self):
        """get_raw_data should copy the data, not reference it"""
        data = [[0,0.1,3],[0.01,10,3.5],[0.02,21,4.0]]
        p=self.SAXSProfile()
        p.add_data(data)
        data[1][2]=3.1
        test = p.get_raw_data()
        self.assertAlmostEqual(test[1][2],3.5)

    def test_get_raw_data(self):
        """test get_raw_data with gamma != 1 """
        #get_raw_data returns the data as-is
        data = [[0,1,3],[0.01,10,3.5],[0.02,21,4.0]]
        gamma = 1.5
        self.write_data(data)
        p=self.SAXSProfile()
        p.add_data(self.datafile)
        p.set_gamma(gamma)
        test = p.get_raw_data()
        for d,q in zip(data,test):
            for i,v in enumerate(q):
                if i <=2:
                    self.assertAlmostEqual(v,d[i])
                else:
                    self.fail("get_raw_data returned more than 3 elements")

    def test_add_data(self):
        """test add_data with filename and list"""
        data = [[0,1,3],[0.01,10,3.5],[0.02,21,4.0]]
        data2 = [[0.05,5,3],[0.06,10,3.6],[0.2,21,4.0]]
        self.write_data(data)
        p=self.SAXSProfile()
        p.add_data(self.datafile) #as a filename
        p.add_data(data2) #as a list
        data += data2
        test = p.get_raw_data()
        for d,q in zip(data,test):
            for ind,(i,j) in enumerate(zip(d,q)):
                self.assertAlmostEqual(i,j)

    def test_add_data_offset(self):
        """test add_data offset=1"""
        data = [[0,0,1,3],[1,0.01,10,3.5],[2,0.02,21,4.0]]
        data2 = [[3,0.05,5,3],[4,0.06,10,3.6],[5,0.2,21,4.0]]
        self.write_data(data)
        p=self.SAXSProfile()
        p.add_data(self.datafile, offset=1) #as a filename
        p.add_data(data2, offset=1) #as a list
        data += data2
        test = p.get_data()
        for d,q in zip(data,test):
            for ind,(i,j) in enumerate(zip(d,q)):
                self.assertAlmostEqual(i,j)

    def test_add_data_2(self):
        """test add_data """
        data = [[0.3,1,3],[0.4,10,3.5],[0.5,21,4.0]]
        data2 = [[0,5,3],[0.01,10,3.6],[0.2,21,4.0]]
        self.write_data(data)
        p=self.SAXSProfile()
        p.add_data(data2)
        p.add_data(self.datafile)
        data = data2 + data
        test = p.get_raw_data()
        for d,q in zip(data,test):
            for ind,(i,j) in enumerate(zip(d,q)):
                self.assertAlmostEqual(i,j)

    def test_add_data_with_flags_failure(self):
        """test add_data with flags failure"""
        data = [[0,1,3],[0.01,10,3.5],[0.02,21,4.0]]
        p=self.SAXSProfile()
        p.new_flag("test",float)
        self.assertRaises(TypeError, p.add_data, data)

    def test_add_data_with_flags(self):
        """test add_data with flags"""
        data = [[0,1,3,1],[0.01,10,3.5,2],[0.02,21,4.0,4]]
        p=self.SAXSProfile()
        p.new_flag("test",float)
        p.add_data(data)
        test=p.get_data()
        for entry,(d,q) in enumerate(zip(data,test)):
            for ind,j in enumerate(q):
                if ind == 0:
                    self.assertEqual(j,entry)
                elif 1 <= ind <= 4:
                    self.assertAlmostEqual(d[ind-1],j)
                else:
                    self.fail("get_data has more than 5 entries")

    def test_set_gamma(self):
        p=self.SAXSProfile()
        p.set_gamma(1.5)
        self.assertAlmostEqual(p.get_gamma(),1.5)

    def test_default_gamma(self):
        p=self.SAXSProfile()
        self.assertAlmostEqual(p.get_gamma(),1.)

    def test_set_flag(self):
        """test set_flag """
        data = [[0,0.1,3],[0.01,10,3.5],[0.02,21,4.0]]
        p=self.SAXSProfile()
        p.add_data(data)
        p.new_flag("test",float)
        p.set_flag(0,"test",1)
        p.set_flag(1,"test",1.6)
        p.set_flag(2,"test",1.7)
        test = [i[4] for i in p.get_data()]
        self.assertEqual(test, [1,1.6,1.7])

    def test_get_flag(self):
        """test get_flag """
        data = [[0,0.1,3],[0.01,10,3.5],[0.02,21,4.0]]
        p=self.SAXSProfile()
        p.add_data(data)
        p.new_flag("test",float)
        p.set_flag(0,"test",1)
        p.set_flag(1,"test",1.6)
        p.set_flag(2,"test",1.7)
        self.assertEqual(p.get_flag(0,"test"), 1)
        self.assertEqual(p.get_flag(1,"test"), 1.6)
        self.assertEqual(p.get_flag(2,"test"), 1.7)

    def test_get_flag_2(self):
        """get_flag raises an error for invalid arguments """
        data = [[0,0.1,3],[0.01,10,3.5],[0.02,21,4.0]]
        p=self.SAXSProfile()
        p.add_data(data)
        p.new_flag("test",float)
        p.set_flag(0,"test",1)
        p.set_flag(1,"test",1.6)
        p.set_flag(2,"test",1.7)
        #invalid 1st arg
        self.assertRaises(ValueError, p.get_flag, None, "blag")
        #invalid 2nd arg
        self.assertRaises(KeyError, p.get_flag, 1, "blag")
        self.assertRaises(KeyError, p.get_flag, 1.0, "blag")

    def test_get_flag_3(self):
        """get_flag returns something for a value outside of known bounds"""
        data = [[0,0.1,3],[0.01,10,3.5],[0.02,21,4.0]]
        p=self.SAXSProfile()
        p.add_data(data)
        p.new_flag("blah",float)
        p.new_flag("test",float)
        p.set_flag(0,"test",1)
        p.set_flag(1,"test",1.6)
        p.set_flag(2,"test",1.7)
        self.assertEqual(p.get_flag(0,"blah",default='---'), '---')
        self.assertEqual(p.get_flag(0.12,"test",default='---'), '---')
        self.assertEqual(p.get_flag(0.12,"blah",default='---'), '---')

    def test_set_interpolant(self):
        gp=MockGP(1,2)
        p=self.SAXSProfile()
        m=IMP.Model()
        s=IMP.isd.Scale.setup_particle(IMP.Particle(m),3.0)
        self.assertRaises(TypeError, p.set_interpolant, gp)
        self.assertRaises(TypeError, p.set_interpolant, gp, [])
        self.assertRaises(TypeError, p.set_interpolant, gp, {'test':1.0})
        self.assertEqual(p.set_interpolant(gp,{'sigma':s},
            {'mean':None,'covariance':None},'test',IMP.Model(), None), None)

    def test_get_params(self):
        "get_params should return a dict"
        gp=MockGP(1,2)
        p=self.SAXSProfile()
        m=IMP.Model()
        s=IMP.isd.Scale.setup_particle(IMP.Particle(m),3.0)
        functions={}
        functions['mean']=MockFunction()
        functions['covariance']=MockFunction()
        p.set_interpolant(gp,{'sigma':s}, functions, 'test', IMP.Model(), None)
        self.assertEqual(p.get_params(),{'sigma':3.0})

    def test_get_mean_defaults(self):
        """test get_mean defaults"""
        data = [[0,1,3],[0.01,10,3.5],[0.02,21,4.0]]
        p=self.SAXSProfile()
        p.add_data(data)
        gp = MockGP(1,2)
        m=IMP.Model()
        s=IMP.isd.Scale.setup_particle(IMP.Particle(m),3.0)
        functions={}
        functions['mean']=MockFunction()
        functions['covariance']=MockFunction()
        p.set_interpolant(gp,{'sigma':s}, functions, 'test', IMP.Model(), None)
        test = p.get_mean()
        expected_q = numpy.linspace(0,0.02,num=200)
        expected_I = map(gp.get_posterior_mean,[[i] for i in expected_q])
        expected_err = map(lambda a:
                           numpy.sqrt(gp.get_posterior_covariance([a],[1.0])),
                expected_q)
        expected_mean = numpy.array(map(lambda a:
                         functions['mean'](a), expected_q))
        for q,I,err,mean,v in zip(expected_q,expected_I,expected_err,
                                    expected_mean,test):
            self.assertEqual(len(v),4)
            self.assertAlmostEqual([q],v[0])
            self.assertAlmostEqual(I,v[1])
            self.assertAlmostEqual(err,v[2])
            self.assertAlmostEqual(mean,v[3])

    def test_get_mean_colwise(self):
        """test get_mean colwise"""
        data = [[0,1,3],[0.01,10,3.5],[0.02,21,4.0]]
        p=self.SAXSProfile()
        p.add_data(data)
        gp = MockGP(1,2)
        m=IMP.Model()
        s=IMP.isd.Scale.setup_particle(IMP.Particle(m),3.0)
        functions={}
        functions['mean']=MockFunction()
        functions['covariance']=MockFunction()
        p.set_interpolant(gp,{'sigma':s}, functions, 'test', IMP.Model(), None)
        test = p.get_mean(colwise=True)
        expected_q = numpy.linspace(0,0.02,num=200)
        expected_I = numpy.array(map(gp.get_posterior_mean,
                                     [[i] for i in expected_q]))
        expected_err = numpy.array(map(lambda a:
                         numpy.sqrt(gp.get_posterior_covariance([a],[1.0])),
                         expected_q))
        expected_mean = numpy.array(map(lambda a:
                         functions['mean'](a), expected_q))
        self.assertEqual(len(test), 4)
        self.assertEqual(set(['q','I','err','mean']), set(test.keys()))
        self.assertTrue((expected_q == test['q']).all())
        self.assertTrue((expected_I == test['I']).all())
        self.assertTrue((expected_err == test['err']).all())
        self.assertTrue((expected_mean == test['mean']).all())

    def test_get_mean_gamma(self):
        """test get_mean gamma"""
        data = [[0,1,3],[0.01,10,3.5],[0.02,21,4.0]]
        p=self.SAXSProfile()
        p.add_data(data)
        gamma = 21.5
        p.set_gamma(gamma)
        gp = MockGP(1,2)
        m=IMP.Model()
        s=IMP.isd.Scale.setup_particle(IMP.Particle(m),3.0)
        functions={}
        functions['mean']=MockFunction()
        functions['covariance']=MockFunction()
        p.set_interpolant(gp,{'sigma':s}, functions, 'test', IMP.Model(), None)
        test = p.get_mean()
        expected_q = numpy.linspace(0,0.02,num=200)
        expected_I = map(lambda a:
                            gamma*gp.get_posterior_mean([a]),
                        expected_q)
        expected_err = map(lambda a:
                        gamma*numpy.sqrt(gp.get_posterior_covariance([a],[a])),
                        expected_q)
        expected_mean = numpy.array(map(lambda a:
                         gamma*functions['mean'](a), expected_q))
        for q,I,err,mean,v in zip(expected_q,expected_I,expected_err,
                expected_mean,test):
            self.assertEqual(len(v),4)
            self.assertAlmostEqual([q],v[0])
            self.assertAlmostEqual(I,v[1])
            self.assertAlmostEqual(err,v[2])
            self.assertAlmostEqual(mean,v[3])

    def test_get_mean_num(self):
        """test get_mean_num"""
        data = [[0,1,3],[0.01,10,3.5],[0.02,21,4.0]]
        p=self.SAXSProfile()
        p.add_data(data)
        gp = MockGP(1,2)
        m=IMP.Model()
        s=IMP.isd.Scale.setup_particle(IMP.Particle(m),3.0)
        functions={}
        functions['mean']=MockFunction()
        functions['covariance']=MockFunction()
        p.set_interpolant(gp,{'sigma':s}, functions, 'test', IMP.Model(), None)
        test = p.get_mean(num=100)
        expected_q = numpy.linspace(0,0.02,num=100)
        expected_I = map(gp.get_posterior_mean,[[i] for i in expected_q])
        expected_err = map(lambda a:
                           numpy.sqrt(gp.get_posterior_covariance([a],[1.0])),
                expected_q)
        for q,I,err,v in zip(expected_q,expected_I,expected_err,test):
            self.assertAlmostEqual([q],v[0])
            self.assertAlmostEqual(I,v[1])
            self.assertAlmostEqual(err,v[2])

    def test_get_mean_qm(self):
        """test qmin and qmax"""
        data = [[0,1,3],[0.01,10,3.5],[0.02,21,4.0]]
        p=self.SAXSProfile()
        p.add_data(data)
        gp = MockGP(1,2)
        m=IMP.Model()
        s=IMP.isd.Scale.setup_particle(IMP.Particle(m),3.0)
        functions={}
        functions['mean']=MockFunction()
        functions['covariance']=MockFunction()
        p.set_interpolant(gp,{'sigma':s}, functions, 'test', IMP.Model(), None)
        test = p.get_mean(num=100,qmin=1.5,qmax=1.67)
        expected_q = numpy.linspace(1.5,1.67,num=100)
        expected_I = map(gp.get_posterior_mean,[[i] for i in expected_q])
        expected_err = map(lambda a:
                           numpy.sqrt(gp.get_posterior_covariance([a],[1.0])),
                expected_q)
        for q,I,err,v in zip(expected_q,expected_I,expected_err,test):
            self.assertAlmostEqual(q,v[0])
            self.assertAlmostEqual(I,v[1])
            self.assertAlmostEqual(err,v[2])

    def test_get_mean_qvalues(self):
        """test qvalues"""
        data = [[0,1,3],[0.01,10,3.5],[0.02,21,4.0]]
        p=self.SAXSProfile()
        p.add_data(data)
        gp = MockGP(1,2)
        m=IMP.Model()
        s=IMP.isd.Scale.setup_particle(IMP.Particle(m),3.0)
        functions={}
        functions['mean']=MockFunction()
        functions['covariance']=MockFunction()
        p.set_interpolant(gp,{'sigma':s}, functions, 'test', IMP.Model(), None)
        expected_q=numpy.random.uniform(0,[1]*10)
        expected_q.sort()
        test = p.get_mean(qvalues=expected_q)
        expected_I = map(gp.get_posterior_mean,[[i] for i in expected_q])
        expected_err = map(lambda a:
                           numpy.sqrt(gp.get_posterior_covariance([a],[1.0])),
                expected_q)
        for q,I,err,v in zip(expected_q,expected_I,expected_err,test):
            self.assertAlmostEqual([q],v[0])
            self.assertAlmostEqual(I,v[1])
            self.assertAlmostEqual(err,v[2])

    def test_set_flag_interval(self):
        p=self.SAXSProfile()
        p.new_flag("test",bool)
        self.assertEqual(p.set_flag_interval("test",0,1,True), None)
        self.assertRaises(ValueError, p.set_flag_interval,"dldm",0,1,True)
        self.assertRaises(TypeError, p.set_flag_interval,"test")
        self.assertRaises(TypeError, p.set_flag_interval,"test",0)
        self.assertRaises(TypeError, p.set_flag_interval,"test",0,1)
        self.assertRaises(TypeError, p.set_flag_interval,"test",0,1,True,False)

    def test_get_flag_intervals(self):
        data = [[0,1,3],[0.01,10,3.5],[0.02,21,4.0]]
        p=self.SAXSProfile()
        p.add_data(data)
        gp = MockGP(1,2)
        m=IMP.Model()
        s=IMP.isd.Scale.setup_particle(IMP.Particle(m),3.0)
        functions={}
        functions['mean']=MockFunction()
        functions['covariance']=MockFunction()
        p.set_interpolant(gp,{'sigma':s}, functions, 'test', IMP.Model(), None)
        expected_q=numpy.random.uniform(0,[1]*10)
        expected_q.sort()
        p.new_flag("test",bool)
        p.set_flag_interval("test",0,0.01,True)
        p.set_flag_interval("test",0.01,0.02,False)
        test = p.get_flag_intervals("test")
        self.assertEqual(len(test),2)
        self.assertEqual(test[0],(0,0.01,True))
        self.assertEqual(test[1],(0.01,0.02,False))
        test = p.get_mean()
        nflags = len(p.get_flag_names())
        for i in test:
            self.assertEqual(len(i), nflags+1)
        for q,I,err,mean,flag in test:
            if q < 0.01:
                self.assertTrue(flag)
            else:
                self.assertFalse(flag)

    def test_set_flag_intervals_overlap_right(self):
        "test set_flag_intervals when they overlap right"
        data = [[0,1,3],[0.01,10,3.5],[0.02,21,4.0]]
        p=self.SAXSProfile()
        p.add_data(data)
        gp = MockGP(1,2)
        m=IMP.Model()
        s=IMP.isd.Scale.setup_particle(IMP.Particle(m),3.0)
        functions={}
        functions['mean']=MockFunction()
        functions['covariance']=MockFunction()
        p.set_interpolant(gp,{'sigma':s}, functions, 'test', IMP.Model(), None)
        expected_q=numpy.random.uniform(0,[1]*10)
        expected_q.sort()
        p.new_flag("test",bool)
        p.set_flag_interval("test",0,0.01,True)
        p.set_flag_interval("test",0.005,0.02,False)
        test = p.get_flag_intervals("test")
        self.assertEqual(len(test),2)
        self.assertEqual(test[0],(0,0.005,True))
        self.assertEqual(test[1],(0.005,0.02,False))
        test = p.get_mean()
        nflags = len(p.get_flag_names())
        for i in test:
            self.assertEqual(len(i), nflags+1)
        for q,I,err,mean,flag in test:
            if q < 0.005:
                self.assertTrue(flag)
            else:
                self.assertFalse(flag)

    def test_set_flag_intervals_overlap_left(self):
        "test set_flag_intervals when they overlap left"
        data = [[0,1,3],[0.01,10,3.5],[0.02,21,4.0]]
        p=self.SAXSProfile()
        p.add_data(data)
        gp = MockGP(1,2)
        m=IMP.Model()
        s=IMP.isd.Scale.setup_particle(IMP.Particle(m),3.0)
        functions={}
        functions['mean']=MockFunction()
        functions['covariance']=MockFunction()
        p.set_interpolant(gp,{'sigma':s}, functions, 'test', IMP.Model(), None)
        expected_q=numpy.random.uniform(0,[1]*10)
        expected_q.sort()
        p.new_flag("test",bool)
        p.set_flag_interval("test",0.01,0.03,True)
        p.set_flag_interval("test",0.0,0.02,False)
        test = p.get_flag_intervals("test")
        self.assertEqual(len(test),2)
        self.assertEqual(test[0],(0.,0.02,False))
        self.assertEqual(test[1],(0.02,0.03,True))
        test = p.get_mean()
        nflags = len(p.get_flag_names())
        for i in test:
            self.assertEqual(len(i), nflags+1)
        for q,I,err,mean,flag in test:
            if q > 0.02:
                self.assertTrue(flag)
            else:
                self.assertFalse(flag)

    def test_set_flag_intervals_overlap_included(self):
        "test set_flag_intervals when they overlap included"
        data = [[0,1,3],[0.01,10,3.5],[0.02,21,4.0]]
        p=self.SAXSProfile()
        p.add_data(data)
        gp = MockGP(1,2)
        m=IMP.Model()
        s=IMP.isd.Scale.setup_particle(IMP.Particle(m),3.0)
        functions={}
        functions['mean']=MockFunction()
        functions['covariance']=MockFunction()
        p.set_interpolant(gp,{'sigma':s}, functions, 'test', IMP.Model(), None)
        expected_q=numpy.random.uniform(0,[1]*10)
        expected_q.sort()
        p.new_flag("test",bool)
        p.set_flag_interval("test",0,0.03,True)
        p.set_flag_interval("test",0.005,0.02,False)
        test = p.get_flag_intervals("test")
        self.assertEqual(len(test),3)
        self.assertEqual(test[0],(0,0.005,True))
        self.assertEqual(test[1],(0.005,0.02,False))
        self.assertEqual(test[2],(0.02,0.03,True))
        test = p.get_mean()
        nflags = len(p.get_flag_names())
        for i in test:
            self.assertEqual(len(i), nflags+1)
        for q,I,err,mean,flag in test:
            if q < 0.005 or q > 0.02:
                self.assertTrue(flag)
            else:
                self.assertFalse(flag)

    def test_set_flag_intervals_overlap_excluded(self):
        "test set_flag_intervals when they overlap excluded"
        data = [[0.005,1,3],[0.01,10,3.5],[0.02,21,4.0]]
        p=self.SAXSProfile()
        p.add_data(data)
        gp = MockGP(1,2)
        m=IMP.Model()
        s=IMP.isd.Scale.setup_particle(IMP.Particle(m),3.0)
        functions={}
        functions['mean']=MockFunction()
        functions['covariance']=MockFunction()
        p.set_interpolant(gp,{'sigma':s}, functions, 'test', IMP.Model(), None)
        expected_q=numpy.random.uniform(0,[1]*10)
        expected_q.sort()
        p.new_flag("test",bool)
        p.set_flag_interval("test",0.01,0.02,True)
        p.set_flag_interval("test",0.005,0.03,False)
        test = p.get_flag_intervals("test")
        self.assertEqual(len(test),1)
        self.assertEqual(test[0],(0.005,0.03,False))
        test = p.get_mean()
        nflags = len(p.get_flag_names())
        for i in test:
            self.assertEqual(len(i), nflags+1)
        for q,I,err,mean,flag in test:
            self.assertFalse(flag)

    def test_get_data_filter(self):
        """test get_data with the filter keyword as a string"""
        data = [[0,1,3,True],[0.01,10,3.5,False],[0.02,21,4.0,True]]
        p=self.SAXSProfile()
        p.new_flag("test",bool)
        p.add_data(data)
        test=p.get_data(filter='test')
        self.assertEqual(len(test),2)
        self.assertEqual(test[0][0],0)
        self.assertAlmostEqual(test[0][1],data[0][0])
        self.assertAlmostEqual(test[0][2],data[0][1])
        self.assertAlmostEqual(test[0][3],data[0][2])
        self.assertEqual(test[0][4],data[0][3])
        self.assertEqual(test[1][0],2)
        self.assertAlmostEqual(test[1][1],data[2][0])
        self.assertAlmostEqual(test[1][2],data[2][1])
        self.assertAlmostEqual(test[1][3],data[2][2])
        self.assertEqual(test[1][4],data[2][3])

    def test_get_data_filter_2(self):
        """test get_data with the filter keyword as a list"""
        data = [[0,1,3,True],[0.01,10,3.5,False],[0.02,21,4.0,True]]
        p=self.SAXSProfile()
        p.new_flag("test",bool)
        p.add_data(data)
        test=p.get_data(filter=['test'])
        self.assertEqual(len(test),2)
        self.assertAlmostEqual(test[0][1],data[0][0])
        self.assertAlmostEqual(test[0][2],data[0][1])
        self.assertAlmostEqual(test[0][3],data[0][2])
        self.assertEqual(test[0][4],data[0][3])
        self.assertAlmostEqual(test[1][1],data[2][0])
        self.assertAlmostEqual(test[1][2],data[2][1])
        self.assertAlmostEqual(test[1][3],data[2][2])
        self.assertEqual(test[1][4],data[2][3])

    def test_get_data_filter_3(self):
        """test get_data with the filter keyword as None"""
        data = [[0,1,3,True],[0.01,10,3.5,False],[0.02,21,4.0,True]]
        p=self.SAXSProfile()
        p.new_flag("test",bool)
        p.add_data(data)
        test=p.get_data(filter=None)
        self.assertEqual(len(test),3)
        for (i,j) in zip(data,test):
            for q,r in zip(i[:3],j[1:4]):
                self.assertAlmostEqual(q,r)
            self.assertEqual(i[3],j[4])

    def test_get_data_filter_4(self):
        """test get_data with an invalid filter keyword"""
        data = [[0,1,3,True],[0.01,10,3.5,False],[0.02,21,4.0,True]]
        p=self.SAXSProfile()
        p.new_flag("test",bool)
        p.add_data(data)
        self.assertRaises(KeyError, p.get_data, filter='blah')

    def test_get_mean_filter(self):
        """test get_mean with the filter keyword as a string"""
        data = [[0,1,3],[0.01,10,3.5],[0.02,21,4.0]]
        p=self.SAXSProfile()
        p.add_data(data)
        gp = MockGP(1,2)
        m=IMP.Model()
        s=IMP.isd.Scale.setup_particle(IMP.Particle(m),3.0)
        functions={}
        functions['mean']=MockFunction()
        functions['covariance']=MockFunction()
        p.set_interpolant(gp,{'sigma':s}, functions, 'test', IMP.Model(), None)
        expected_q=numpy.random.uniform(0,[1]*10)
        expected_q.sort()
        p.new_flag("test",bool)
        p.set_flag_interval("test",0,0.01,True)
        p.set_flag_interval("test",0.01,0.02,False)
        test = p.get_mean(filter='test',num=20)
        self.assertEqual(len(test),10)
        nflags = len(p.get_flag_names())
        for i in test:
            self.assertEqual(len(i), nflags+1)
        for q,I,err,mean,flag in test:
            self.assertTrue(flag)
            self.assertTrue(0 <= q < 0.01)

    def test_get_mean_filter_2(self):
        """test get_mean with the filter keyword as a list"""
        data = [[0,1,3],[0.01,10,3.5],[0.02,21,4.0]]
        p=self.SAXSProfile()
        p.add_data(data)
        gp = MockGP(1,2)
        m=IMP.Model()
        s=IMP.isd.Scale.setup_particle(IMP.Particle(m),3.0)
        functions={}
        functions['mean']=MockFunction()
        functions['covariance']=MockFunction()
        p.set_interpolant(gp,{'sigma':s}, functions, 'test', IMP.Model(), None)
        expected_q=numpy.random.uniform(0,[1]*10)
        expected_q.sort()
        p.new_flag("test",bool)
        p.set_flag_interval("test",0,0.01,True)
        p.set_flag_interval("test",0.01,0.02,False)
        test = p.get_mean(filter=['test'],num=20)
        self.assertEqual(len(test),10)
        nflags = len(p.get_flag_names())
        for i in test:
            self.assertEqual(len(i), nflags+1)
        for q,I,err,mean,flag in test:
            self.assertTrue(flag)
            self.assertTrue(0 <= q < 0.01)

    def test_get_mean_filter_3(self):
        """test get_mean with the filter keyword as None"""
        data = [[0,1,3],[0.01,10,3.5],[0.02,21,4.0]]
        p=self.SAXSProfile()
        p.add_data(data)
        gp = MockGP(1,2)
        m=IMP.Model()
        s=IMP.isd.Scale.setup_particle(IMP.Particle(m),3.0)
        functions={}
        functions['mean']=MockFunction()
        functions['covariance']=MockFunction()
        p.set_interpolant(gp,{'sigma':s}, functions, 'test', IMP.Model(), None)
        expected_q=numpy.random.uniform(0,[1]*10)
        expected_q.sort()
        p.new_flag("test",bool)
        p.set_flag_interval("test",0,0.01,True)
        p.set_flag_interval("test",0.01,0.02,False)
        test = p.get_mean(filter=None,num=20)
        self.assertEqual(len(test),20)
        nflags = len(p.get_flag_names())
        for i in test:
            self.assertEqual(len(i), nflags+1)
        for q,I,err,mean,flag in test:
            if 0 <= q < 0.01:
                self.assertTrue(flag)
            else:
                self.assertFalse(flag)

    def test_get_mean_filter_4(self):
        """test get_mean with an invalid filter keyword"""
        data = [[0,1,3],[0.01,10,3.5],[0.02,21,4.0]]
        p=self.SAXSProfile()
        p.add_data(data)
        gp = MockGP(1,2)
        m=IMP.Model()
        s=IMP.isd.Scale.setup_particle(IMP.Particle(m),3.0)
        functions={}
        functions['mean']=MockFunction()
        functions['covariance']=MockFunction()
        p.set_interpolant(gp,{'sigma':s}, functions, 'test', IMP.Model(), None)
        expected_q=numpy.random.uniform(0,[1]*10)
        expected_q.sort()
        p.new_flag("test",bool)
        p.set_flag_interval("test",0,0.01,True)
        p.set_flag_interval("test",0.01,0.02,False)
        self.assertRaises(KeyError, p.get_mean, filter='blah', num=20)

if __name__ == "__main__":
    IMP.test.main()
