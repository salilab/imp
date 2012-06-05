#!/usr/bin/python

import IMP.test
import os
import sys
import shutil
try:
    import numpy
    import scipy
except:
    numpy = None
    scipy = None

class SAXSApplicationTest(IMP.test.ApplicationTestCase):

    def setUp(self):
        IMP.test.ApplicationTestCase.setUp(self)
        if numpy is None or scipy is None:
            self.skipTest("could not import numpy or scipy")

    def run_app(self, args):
        args.append('--destdir=runapp')
        p = self.run_python_application('saxs_merge.py',args)
        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)
        #do some preliminary tests
        for i in ['runapp/data_merged.dat', 'runapp/mean_merged.dat', 'runapp/summary.txt']:
            self.assertTrue(os.path.exists(i))
        text=open('runapp/summary.txt').read()
        m = text.find('Classification')
        self.assertIsNotNone(m, msg="Classification output not found in summary.txt")

    def uniqify(self, seq):
        #taken from http://stackoverflow.com/a/480227/958110
        seen = set()
        seen_add = seen.add
        return [ x for x in seq if x not in seen and not seen_add(x)]

    def plot_data_overlaid(self, name, d1, d2, title=None):
        # data overlaid in log scale
        fl=open('Cpgnuplot','w')
        fl.write('set term png\n')
        fl.write('set output "%s_data_log_overlaid.png"\n' % name)
        fl.write('set log y\n')
        if title:
            fl.write('set title "%s"\n' % title)
        fl.write('p "%s" u 1:2 w p t "automatic", "%s" u 1:2 w p t "manual"\n' %
            (d1,d2))
        fl.close()
        os.system('gnuplot Cpgnuplot')
        # data overlaid in linear scale
        fl=open('Cpgnuplot','w')
        fl.write('set term png\n')
        fl.write('set output "%s_data_lin_overlaid.png"\n' % name)
        fl.write('set log y\n')
        fl.write('p "%s" u 1:2 w p t "automatic", "%s" u 1:2 w p t "manual"\n' %
            (d1,d2))
        fl.close()
        os.system('gnuplot Cpgnuplot')

    def plot_data_colored(self, name, d1):
        # data in linear scale, colored
        fl=open('Cpgnuplot','w')
        fl.write('set term png\n')
        fl.write('set output "%s_data_lin_colored.png"\n' % name)
        fl.write('p "%s" u 1:($2+100):4 w p lc var t "automatic"\n' % (d1))
        fl.close()
        os.system('gnuplot Cpgnuplot')
        # data shifted in log scale
        fl=open('Cpgnuplot','w')
        fl.write('set term png\n')
        fl.write('set output "%s_data_log_colored.png"\n' % name)
        fl.write('set log y\n')
        fl.write('p "%s" u 1:($2*20):4 w p lc var t "automatic"\n' % (d1))
        fl.close()
        os.system('gnuplot Cpgnuplot')

    def plot_data_mean(self, name, data, mean):
        #linear scale
        fl=open('Cpgnuplot','w')
        fl.write('set term png\n')
        fl.write('set output "%s_data_lin_mean.png"\n' % name)
        fl.write('p "%s" u 1:2:3 w yerr t "data", "%s" u 1:2 w l t "mean" lw 2,'
                 ' "" u 1:($2+$3) w l lw 2 not, "" u 1:($2-$3) w l lw 2 not\n'  % (data,mean)   )
        fl.close()
        os.system('gnuplot Cpgnuplot')
        #log scale
        fl=open('Cpgnuplot','w')
        fl.write('set term png\n')
        fl.write('set output "%s_data_log_mean.png"\n' % name)
        fl.write('set log y\n')
        fl.write('p "%s" u 1:2:3 w yerr t "data", "%s" u 1:2 w l t "mean" lw 2,'
                 ' "" u 1:($2+$3) w l lw 2 not, "" u 1:($2-$3) w l lw 2 not\n'  % (data,mean)   )
        fl.close()
        os.system('gnuplot Cpgnuplot')

    def plot_means(self, name, d1, d2):
        #linear scale
        fl=open('Cpgnuplot','w')
        fl.write('set term png\n')
        fl.write('set output "%s_means_lin.png"\n' % name)
        fl.write('p "%s" u 1:2 w l t "automatic", '
                 '"" u 1:($2+$3) w l not, "" u 1:($2-$3) w l not, '
                 '"%s" u 1:2 w l t "manual", '
                 '"" u 1:($2+$3) w l not, "" u 1:($2-$3) w l not\n' % (d1,d2))
        fl.close()
        os.system('gnuplot Cpgnuplot')
        #log scale
        fl=open('Cpgnuplot','w')
        fl.write('set term png\n')
        fl.write('set output "%s_means_log.png"\n' % name)
        fl.write('set log y\n')
        fl.write('p "%s" u 1:2 w l t "automatic", '
                 '"" u 1:($2+$3) w l not, "" u 1:($2-$3) w l not, '
                 '"%s" u 1:2 w l t "manual", '
                 '"" u 1:($2+$3) w l not, "" u 1:($2-$3) w l not\n' % (d1,d2))
        fl.close()
        os.system('gnuplot Cpgnuplot')

    def chisquare(self, fla, flb):
        #read first 3 columns
        da=[map(float, i.split()[:3]) for i in open(fla).readlines()]
        db=[map(float, i.split()[:3]) for i in open(flb).readlines()]
        #check that data has common q values, removing the first ones if different
        qa = self.uniqify(map(lambda a:a[0], da))
        qb = self.uniqify(map(lambda a:a[0], db))
        #print qa[0],qb[0]
        if qa[0] < qb[0]:
            i=0
            while i+1 < len(qa) and abs(qa[i] - qb[0]) > 1e-5:
                #print "   qa[%d]=%f" % (i,qa[i])
                i += 1
            qa = qa[i:]
            #print "qa[0] -> %f (%d)" % (qa[0],len(qa))
        else:
            i=0
            while i+1 < len(qb) and abs(qa[0] - qb[i]) > 1e-5:
                #print "   qb[%d]=%f" % (i,qb[i])
                i += 1
            qb = qb[i:]
            #print "qb[0] -> %f (%d)" % (qb[0],len(qb))
        if len(qa) == 0 or len(qb) == 0:
            self.fail("datasets do not have common q values!")
        #project back to data
        qa.reverse()
        qb.reverse()
        da.reverse()
        db.reverse()
        nda = []
        ndb = []
        while qa and qb:
            #find next q value that is common in both datasets
            qva = qa.pop()
            qvb = qb.pop()
            while qa and qb:
                #print qva,qvb,
                if abs(qva-qvb) < 1e-5:
                    #print "equal"
                    break
                elif qva < qvb:
                    qva = qa.pop()
                else:
                    qvb = qb.pop()
                #print "different"
            if abs(qva-qvb)>1e-5:
                break
            #find it in the datasets
            qval = qva
            #print "new qval",qval
            while da:
                if da[-1][0] < qval - 1e-5:
                    da.pop()
                    #print "  skipped da",da.pop()
                else:
                    break
            while da:
                if da[-1][0] > qval + 1e-5:
                    break
                else:
                    nda.append(da.pop())
                    #print "  appended",nda[-1]
            #same for db
            while db:
                if db[-1][0] < qval - 1e-5:
                    db.pop()
                    #print "  skipped db",db.pop()
                else:
                    break
            while db:
                if db[-1][0] > qval + 1e-5:
                    break
                else:
                    ndb.append(db.pop())
                    #print "  appended",ndb[-1]
            #same for db
        #compute normalized chisquare.
        #need to find better statistic
        #Multiple datapoints at same q value are downweighted.
        #each chunk is a set of values at the same q
        chi = []
        fl=open('nda','w')
        for i in nda:
            fl.write(' '.join(["%s" % j for j in i]))
            fl.write('\n')
        fl=open('ndb','w')
        for i in ndb:
            fl.write(' '.join(["%s" % j for j in i]))
            fl.write('\n')
        while nda and ndb:
            #get chunk
            qval = nda[-1][0]
            chunka = []
            while nda and abs(nda[-1][0] - qval) < 1e-5:
                chunka.append(nda.pop()[1:])
            chunkb = []
            while ndb and abs(ndb[-1][0] - qval) < 1e-5:
                chunkb.append(ndb.pop()[1:])
            #compute contributions
            contribs = []
            for ia,sa in chunka:
                for ib,sb in chunkb:
                    contribs.append((ia-ib)**2/(sa**2+sb**2))
            #downweight and add them
            chi.append(sum(contribs)/float(len(contribs)))
        if len(chi) < 50:
            print "warning: chisquare computed on only %d points!" % len(chi)
        return sum(chi)/len(chi)

    def run_results(self, name, manual_merge):
        #rescale and fit the two curves
        p = self.run_python_application('saxs_merge.py', ['--destdir=compapp',
                 '--stop=rescaling', '--postpone_cleanup', '--blimit_fitting=200',
                 '--npoints=-1', '--allfiles', 'runapp/data_merged.dat', manual_merge])
        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)
        #compute chi2 of data
        datachi = self.chisquare('compapp/data_data_merged.dat',
                'compapp/data_'+os.path.basename(manual_merge))
        #compute chi2 of fits
        fitchi = self.chisquare('compapp/mean_data_merged.dat',
                'compapp/mean_'+os.path.basename(manual_merge))
        #plot data
        self.plot_data_overlaid(name, 'compapp/data_data_merged.dat',
                'compapp/data_'+os.path.basename(manual_merge))
        self.plot_data_colored(name, 'runapp/data_merged.dat')
        #plot mean
        self.plot_means(name, 'compapp/mean_data_merged.dat',
                'compapp/mean_'+os.path.basename(manual_merge))
        self.plot_data_mean(name, 'compapp/data_data_merged.dat',
                'compapp/mean_data_merged.dat')
        print name,datachi,fitchi

    def tearDown(self):
        return
        shutil.rmtree('runapp',True)
        shutil.rmtree('compapp',True)
        os.unlink('Cpgnuplot')

    def get_params1(self):
        """aalpha 1e-7"""
        return ['--blimit_fitting=100', '--elimit_fitting=100']

    def get_params2(self):
        """aalpha 1e-4"""
        return ['--blimit_fitting=100', '--elimit_fitting=100', '--aalpha=1e-4']

    def get_params3(self):
        """aalpha 1e-7 postpone cleanup"""
        return ['--blimit_fitting=100', '--elimit_fitting=100',
                '--postpone_cleanup']

    def test_case1a(self):
        """Simple test of SAXS merge benchmark / application for Nup116"""
        self.run_app(
             self.get_params1() +
             [self.get_input_file_name('Nup116/25043_01C_S059_0_01.sub'),
             self.get_input_file_name('Nup116/25043_01B_S057_0_01.sub'),
             self.get_input_file_name('Nup116/25043_01D_S061_0_01.sub'),
             self.get_input_file_name('Nup116/25043_01E_S063_0_01.sub'),
             self.get_input_file_name('Nup116/25043_01F_S065_0_01.sub')]
             )
        self.run_results('Nup116_1',
                self.get_input_file_name('Nup116/25043_manual_merge.dat'))

    def test_case1b(self):
        """Simple test of SAXS merge benchmark / application for Nup192"""
        self.run_app(
             self.get_params1() +
             [self.get_input_file_name('Nup192/Nup192_01B_S014_0_01.sub'),
             self.get_input_file_name('Nup192/Nup192_01C_S016_0_01.sub'),
             self.get_input_file_name('Nup192/Nup192_01D_S018_0_01.sub'),
             self.get_input_file_name('Nup192/Nup192_01E_S020_0_02.sub'),
             self.get_input_file_name('Nup192/Nup192_01F_S022_0_02.sub')]
             )
        self.run_results('Nup192_1',
                self.get_input_file_name('Nup192/Nup192_merge.dat'))

    def test_case2a(self):
        """Simple test of SAXS merge benchmark / application for Nup116"""
        self.run_app(
             self.get_params2() +
             [self.get_input_file_name('Nup116/25043_01C_S059_0_01.sub'),
             self.get_input_file_name('Nup116/25043_01B_S057_0_01.sub'),
             self.get_input_file_name('Nup116/25043_01D_S061_0_01.sub'),
             self.get_input_file_name('Nup116/25043_01E_S063_0_01.sub'),
             self.get_input_file_name('Nup116/25043_01F_S065_0_01.sub')]
             )
        self.run_results('Nup116_2',
                self.get_input_file_name('Nup116/25043_manual_merge.dat'))

    def test_case2b(self):
        """Simple test of SAXS merge benchmark / application for Nup192"""
        self.run_app(
             self.get_params2() +
             [self.get_input_file_name('Nup192/Nup192_01B_S014_0_01.sub'),
             self.get_input_file_name('Nup192/Nup192_01C_S016_0_01.sub'),
             self.get_input_file_name('Nup192/Nup192_01D_S018_0_01.sub'),
             self.get_input_file_name('Nup192/Nup192_01E_S020_0_02.sub'),
             self.get_input_file_name('Nup192/Nup192_01F_S022_0_02.sub')]
             )
        self.run_results('Nup192_2',
                self.get_input_file_name('Nup192/Nup192_merge.dat'))

    def test_case3a(self):
        """Simple test of SAXS merge benchmark / application for Nup116"""
        self.run_app(
             self.get_params3() +
             [self.get_input_file_name('Nup116/25043_01C_S059_0_01.sub'),
             self.get_input_file_name('Nup116/25043_01B_S057_0_01.sub'),
             self.get_input_file_name('Nup116/25043_01D_S061_0_01.sub'),
             self.get_input_file_name('Nup116/25043_01E_S063_0_01.sub'),
             self.get_input_file_name('Nup116/25043_01F_S065_0_01.sub')]
             )
        self.run_results('Nup116_3',
                self.get_input_file_name('Nup116/25043_manual_merge.dat'))

    def test_case3b(self):
        """Simple test of SAXS merge benchmark / application for Nup192"""
        self.run_app(
             self.get_params3() +
             [self.get_input_file_name('Nup192/Nup192_01B_S014_0_01.sub'),
             self.get_input_file_name('Nup192/Nup192_01C_S016_0_01.sub'),
             self.get_input_file_name('Nup192/Nup192_01D_S018_0_01.sub'),
             self.get_input_file_name('Nup192/Nup192_01E_S020_0_02.sub'),
             self.get_input_file_name('Nup192/Nup192_01F_S022_0_02.sub')]
             )
        self.run_results('Nup192_3',
                self.get_input_file_name('Nup192/Nup192_merge.dat'))

if __name__ == '__main__':
    IMP.test.main()
