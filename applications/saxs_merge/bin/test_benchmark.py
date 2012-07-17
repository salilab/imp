#!/usr/bin/python

import IMP.test
import IMP.atom
import IMP.saxs
import os
import sys
import shutil
try:
    import numpy
    import scipy
except:
    numpy = None
    scipy = None

"""
TODO: Add More
- Nup2 (3N7C) : 25014
- Nup53 (3P3D) : 24888
- Hemoglobin S (2HBS) :
- Nup133 (the whole piece): 23904
"""

class SAXSApplicationTest(IMP.test.ApplicationTestCase):

    def setUp(self):
        IMP.test.ApplicationTestCase.setUp(self)
        if numpy is None or scipy is None:
            self.skipTest("could not import numpy or scipy")

    def run_app(self, name, args):
        destname = 'runapp_'+name
        if os.path.isdir(destname):
            return
        args.append('--destdir='+destname)
        args.append('--blimit_fitting=800')
        args.append('--elimit_fitting=800')
        args.append('--allfiles')
        args.append('--outlevel=full')
        p = self.run_python_application('saxs_merge.py',args)
        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)
        #do some preliminary tests
        for i in [destname+'/data_merged.dat', destname+'/mean_merged.dat',
                destname+'/summary.txt']:
            self.assertTrue(os.path.exists(i))
        text=open(destname+'/summary.txt').read()
        m = text.find('Classification')
        self.assertIsNotNone(m,
                msg="Classification output not found in summary.txt")

    def uniqify(self, seq):
        #taken from http://stackoverflow.com/a/480227/958110
        seen = set()
        seen_add = seen.add
        return [ x for x in seq if x not in seen and not seen_add(x)]

    def plot_data_overlaid(self, name, d1, d2, datarange, title=None):
        # data overlaid in log scale
        fl=open('Cpgnuplot','w')
        fl.write('set term png\n')
        fl.write('set output "%s_data_log_overlaid.png"\n' % name)
        fl.write('set log y\n')
        fl.write('set xrange [%f:%f]\n' % (datarange[0], datarange[1]))
        fl.write('set yrange [%f:%f]\n' % (datarange[2], datarange[3]))
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
        fl.write('set xrange [%f:%f]\n' % (datarange[0], datarange[1]))
        fl.write('set yrange [0:%f]\n' % datarange[3])
        if title:
            fl.write('set title "%s"\n' % title)
        fl.write('p "%s" u 1:2 w p t "automatic", "%s" u 1:2 w p t "manual"\n' %
            (d1,d2))
        fl.close()
        os.system('gnuplot Cpgnuplot')

    def plot_data_colored(self, name, d1, datarange, transform=(1,0)):
        # data in linear scale, colored
        fl=open('Cpgnuplot','w')
        fl.write('set term png\n')
        fl.write('set xrange [%f:%f]\n' % (datarange[0], datarange[1]))
        fl.write('set yrange [0:%f]\n' % datarange[3])
        fl.write('set output "%s_data_lin_colored.png"\n' % name)
        fl.write('p "%s" u 1:(%f*($2+%f)):5 w p lc var t "automatic"\n' %
                   (d1, transform[0], transform[1]))
        fl.close()
        os.system('gnuplot Cpgnuplot')
        # data shifted in log scale
        fl=open('Cpgnuplot','w')
        fl.write('set term png\n')
        fl.write('set output "%s_data_log_colored.png"\n' % name)
        fl.write('set log y\n')
        fl.write('set xrange [%f:%f]\n' % (datarange[0], datarange[1]))
        fl.write('set yrange [%f:%f]\n' % (datarange[2], datarange[3]))
        fl.write('p "%s" u 1:(%f*($2+%f)):5 w p lc var t "automatic"\n' %
                (d1, transform[0],transform[1]))
        fl.close()
        os.system('gnuplot Cpgnuplot')

    def plot_means(self, name, d1, d2, datarange):
        #linear scale
        fl=open('Cpgnuplot','w')
        fl.write('set term png\n')
        fl.write('set output "%s_means_lin.png"\n' % name)
        fl.write('set xrange [%f:%f]\n' % (datarange[0], datarange[1]))
        fl.write('set yrange [0:%f]\n' % datarange[3])
        fl.write('p "%s" u 1:2 w l lt 1 t "automatic", '
                 '"" u 1:($2+$3) w l lt 1 not, "" u 1:($2-$3) w l lt 1 not, '
                 '"%s" u 1:2 w l lt 2 t "manual", '
                 '"" u 1:($2+$3) w l lt 2 not, '
                 '"" u 1:($2-$3) w l lt 2 not\n' % (d1,d2))
        fl.close()
        os.system('gnuplot Cpgnuplot')
        #log scale
        fl=open('Cpgnuplot','w')
        fl.write('set term png\n')
        fl.write('set output "%s_means_log.png"\n' % name)
        fl.write('set log y\n')
        fl.write('set xrange [%f:%f]\n' % (datarange[0], datarange[1]))
        fl.write('set yrange [%f:%f]\n' % (datarange[2], datarange[3]))
        fl.write('p "%s" u 1:2 w l lt 1 t "automatic", '
                 '"" u 1:($2+$3) w l lt 1 not, "" u 1:($2-$3) w l lt 1 not, '
                 '"%s" u 1:2 w l lt 2 t "manual", '
                 '"" u 1:($2+$3) w l lt 2 not, '
                 '"" u 1:($2-$3) w l lt 2 not\n' % (d1,d2))
        fl.close()
        os.system('gnuplot Cpgnuplot')

    def plot_data_mean(self, name, data, mean, datarange):
        #linear scale
        fl=open('Cpgnuplot','w')
        fl.write('set term png\n')
        fl.write('set output "%s_data_lin_mean.png"\n' % name)
        fl.write('set xrange [%f:%f]\n' % (datarange[0], datarange[1]))
        fl.write('set yrange [0:%f]\n' % datarange[3])
        fl.write('p "%s" u 1:2:3 w yerr t "data", "%s" u 1:2 w l t "mean" lw 2,'
                 ' "" u 1:($2+$3) w l lw 2 not, "" u 1:($2-$3) w l lw 2 not\n'  % (data,mean)   )
        fl.close()
        os.system('gnuplot Cpgnuplot')
        #log scale
        fl=open('Cpgnuplot','w')
        fl.write('set term png\n')
        fl.write('set output "%s_data_log_mean.png"\n' % name)
        fl.write('set log y\n')
        fl.write('set xrange [%f:%f]\n' % (datarange[0], datarange[1]))
        fl.write('set yrange [%f:%f]\n' % (datarange[2], datarange[3]))
        fl.write('p "%s" u 1:2:3 w yerr t "data", "%s" u 1:2 w l t "mean" lw 2,'
                 ' "" u 1:($2+$3) w l lw 2 not, "" u 1:($2-$3) w l lw 2 not\n'  % (data,mean)   )
        fl.close()
        os.system('gnuplot Cpgnuplot')

    def plot_guinier(self, name, data, mean):
        fl=open('Cpgnuplot','w')
        fl.write('set term png\n')
        fl.write('set output "%s_guinier.png"\n' % name)
        fl.write('set xrange [0:0.0025]\n')
        fl.write('p "%s" u ($1*$1):(log($2)):($3/$2) w yerr t "data", '
                 '  "%s" u ($1*$1):(log($2)) w l t "mean", '
                 '"" u ($1*$1):(log($2)+$3/$2) w l not, '
                 '"" u ($1*$1):(log($2)-$3/$2) w l not\n' % (data,mean))
        fl.close()
        os.system('gnuplot Cpgnuplot')

    def plot_inputs(self, name, inputs):
        #linear scale
        fl=open('Cpgnuplot','w')
        fl.write('set term png\n')
        fl.write('set output "%s_inputs_lin.png"\n' % name)
        fl.write('p ')
        for i,inp in enumerate(inputs):
            fl.write('"%s" u 1:(%d+$2):3 w yerr t "%s"'
                    % (inp, i*10, os.path.basename(inp)))
            if i < len(inputs)-1:
                fl.write(', \\\n')
            else:
                fl.write('\n')
        fl.close()
        os.system('gnuplot Cpgnuplot')
        #log scale
        fl=open('Cpgnuplot','w')
        fl.write('set term png\n')
        fl.write('set output "%s_inputs_log.png"\n' % name)
        fl.write('set log y\n')
        fl.write('p ')
        for i,inp in enumerate(inputs):
            fl.write('"%s" u 1:(%d*$2):(%d*$3) w yerr t "%s"'
                    % (inp, i*10, i*10, os.path.basename(inp)))
            if i < len(inputs)-1:
                fl.write(', \\\n')
            else:
                fl.write('\n')
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

    def get_transform(self, fname):
        data=open(fname).readlines()
        gamma = float([i for i in data if 'gamma' in i][0].split()[2])
        offset = [i for i in data if 'offset :' in i]
        if offset:
            offset=float(offset[0].split()[2])
            return gamma,offset
        else:
            return gamma,0

    def get_pdb_data(self, pdb, automerge, manualmerge):
        """get chi and radius of gyration of pdb"""
        m = IMP.Model()
        mp =IMP.atom.read_pdb(pdb, m,
                      IMP.atom.NonWaterNonHydrogenPDBSelector())
        particles = IMP.atom.get_by_type(mp, IMP.atom.ATOM_TYPE)

        # compute surface accessibility
        s = IMP.saxs.SolventAccessibleSurface()
        surface_area = s.get_solvent_accessibility(IMP.core.XYZRs(particles))

        # compute the theoretical profile
        model_profile = IMP.saxs.Profile()
        model_profile.calculate_profile_partial(particles, surface_area)

        # fit the data
        exp_profile = IMP.saxs.Profile(automerge)
        saxs_score = IMP.saxs.Score(exp_profile)
        chi = (saxs_score.fit_profile(model_profile)).get_chi()
        Rg = model_profile.radius_of_gyration()

        # fit manual merge
        exp_profile = IMP.saxs.Profile(manualmerge)
        saxs_score = IMP.saxs.Score(exp_profile)
        mchi = (saxs_score.fit_profile(model_profile)).get_chi()
        mRg = model_profile.radius_of_gyration()

        return chi,mchi,Rg, mRg

    def get_guinier_Rg(self, profile):
        #guinier
        exp_profile = IMP.saxs.Profile(profile)
        gRg = exp_profile.radius_of_gyration()
        return gRg

    def get_GPI_Rg(self, summary):
        #GPI
        lines=open(summary).readlines()
        lines = [ float(i.split()[2]) for i in lines if " Rg " in i ]
        return lines

    def run_results(self, name, manual_merge, inputs, pdb=None,
            extra_args=None):
        #rescale and fit the two curves
        destdir='compapp_'+name
        if not os.path.isdir(destdir):
            p = self.run_python_application('saxs_merge.py',
                    ['--destdir='+destdir,
                 '--blimit_fitting=800', '--elimit_fitting=800',
                 '--stop=rescaling', '--postpone_cleanup',
                 #'--lambdamin=0.05',
                 '--npoints=-1', '--allfiles', '--outlevel=full',
                 'runapp_'+name+'/data_merged.dat', manual_merge])
            out, err = p.communicate()
            sys.stderr.write(err)
            self.assertApplicationExitedCleanly(p.returncode, err)
        #compute chi2 of data
        datachi = self.chisquare(destdir+'/data_data_merged.dat',
                destdir+'/data_'+os.path.basename(manual_merge))
        #compute chi2 of fits
        fitchi = self.chisquare(destdir+'/mean_data_merged.dat',
                destdir+'/mean_'+os.path.basename(manual_merge))
        #compute chi2 to pdb structure using foxs
        if pdb:
            pdbchi, mpdbchi, pdbRg = \
                    self.get_pdb_data(pdb, destdir+'/mean_data_merged.dat',
                        destdir+'/mean_'+os.path.basename(manual_merge))
        else:
            pdbchi = None
            mpdbchi = None
            pdbRg = None
        #radius of gyration
        guinierRg = self.get_guinier_Rg(destdir+'/mean_data_merged.dat')
        mguinierRg = self.get_guinier_Rg(destdir+'/mean_'
                                      +os.path.basename(manual_merge))
        Rg, mRg = self.get_GPI_Rg(destdir+'/summary.txt')
        #get proper bounds
        points=map(lambda a:map(float,a.split()[:2]),
                open(manual_merge).readlines())
        xmin = 0
        xmax = max([i[0] for i in points if len(i) >= 2])*1.2
        ymin = min([i[1] for i in points if len(i) >= 2])*0.8
        if ymin <=0: ymin = 0.01
        ymax = max([i[1] for i in points if len(i) >= 2])*1.2
        datarange = (xmin,xmax,ymin,ymax)
        #plot data
        self.plot_data_overlaid(name, destdir+'/data_data_merged.dat',
                destdir+'/data_'+os.path.basename(manual_merge), datarange)
        self.plot_data_colored(name,
                'runapp_'+name+'/data_merged.dat', datarange,
                transform = self.get_transform(destdir+'/summary.txt') )
        #plot mean
        self.plot_means(name, destdir+'/mean_data_merged.dat',
                destdir+'/mean_'+os.path.basename(manual_merge), datarange)
        self.plot_data_mean(name, destdir+'/data_data_merged.dat',
                destdir+'/mean_data_merged.dat', datarange)
        #guinier plot
        self.plot_guinier(name, destdir+'/data_data_merged.dat',
                destdir+'/mean_data_merged.dat')
        #plot all curves
        self.plot_inputs(name, inputs)
        print name,datachi,fitchi,Rg,guinierRg,mRg,mguinierRg,\
                pdbRg,pdbchi,mpdbchi

    def tearDown(self):
        return
        shutil.rmtree('runapp',True)
        shutil.rmtree('compapp',True)
        os.unlink('Cpgnuplot')

    def get_params1(self):
        """aalpha 1e-7"""
        return ['--cmodel=normal']

    def get_params2(self):
        """aalpha 1e-4"""
        return ['--aalpha=1e-4', '--cmodel=normal']

    def get_params3(self):
        """aalpha 1e-7 postpone cleanup"""
        return ['--postpone_cleanup', '--cmodel=normal']

    def get_params4(self):
        """aalpha 1e-7 rescale with offset"""
        return ['--cmodel=normal-offset']

    def get_params5(self):
        """aalpha 1e-7 rescale with offset and model comparison"""
        return ['--cmodel=normal', '--bcomp', '--blimit_hessian=100',
                '--boptimize=Full']

    # a. Simple test of SAXS merge benchmark / application for Nup116 (3NF5)
    def test_case1a(self):
        """Simple test of SAXS merge benchmark / application for Nup116"""
        inputs = [self.get_input_file_name('Nup116/25043_01C_S059_0_01.sub'),
                 self.get_input_file_name('Nup116/25043_01B_S057_0_01.sub'),
                 self.get_input_file_name('Nup116/25043_01D_S061_0_01.sub'),
                 self.get_input_file_name('Nup116/25043_01E_S063_0_01.sub'),
                 self.get_input_file_name('Nup116/25043_01F_S065_0_01.sub')]
        self.run_app( 'Nup116_1', self.get_params1() + inputs )
        self.run_results('Nup116_1',
                self.get_input_file_name('Nup116/25043_manual_merge.dat'),
                inputs,
                pdb = self.get_input_file_name('Nup116/3nf5_model.pdb'))

    def test_case2a(self):
        """Simple test of SAXS merge benchmark / application for Nup116"""
        inputs = [self.get_input_file_name('Nup116/25043_01C_S059_0_01.sub'),
                 self.get_input_file_name('Nup116/25043_01B_S057_0_01.sub'),
                 self.get_input_file_name('Nup116/25043_01D_S061_0_01.sub'),
                 self.get_input_file_name('Nup116/25043_01E_S063_0_01.sub'),
                 self.get_input_file_name('Nup116/25043_01F_S065_0_01.sub')]
        self.run_app( 'Nup116_2', self.get_params2() + inputs )
        self.run_results('Nup116_2',
                self.get_input_file_name('Nup116/25043_manual_merge.dat'),
                inputs,
                pdb = self.get_input_file_name('Nup116/3nf5_model.pdb'))

    def test_case3a(self):
        """Simple test of SAXS merge benchmark / application for Nup116"""
        inputs = [self.get_input_file_name('Nup116/25043_01C_S059_0_01.sub'),
                 self.get_input_file_name('Nup116/25043_01B_S057_0_01.sub'),
                 self.get_input_file_name('Nup116/25043_01D_S061_0_01.sub'),
                 self.get_input_file_name('Nup116/25043_01E_S063_0_01.sub'),
                 self.get_input_file_name('Nup116/25043_01F_S065_0_01.sub')]
        self.run_app( 'Nup116_3', self.get_params3() + inputs )
        self.run_results('Nup116_3',
                self.get_input_file_name('Nup116/25043_manual_merge.dat'),
                inputs,
                pdb = self.get_input_file_name('Nup116/3nf5_model.pdb'))

    def test_case4a(self):
        """Simple test of SAXS merge benchmark / application for Nup116"""
        inputs = [self.get_input_file_name('Nup116/25043_01C_S059_0_01.sub'),
                 self.get_input_file_name('Nup116/25043_01B_S057_0_01.sub'),
                 self.get_input_file_name('Nup116/25043_01D_S061_0_01.sub'),
                 self.get_input_file_name('Nup116/25043_01E_S063_0_01.sub'),
                 self.get_input_file_name('Nup116/25043_01F_S065_0_01.sub')]
        self.run_app( 'Nup116_4', self.get_params4() + inputs )
        self.run_results('Nup116_4',
                self.get_input_file_name('Nup116/25043_manual_merge.dat'),
                inputs,
                pdb = self.get_input_file_name('Nup116/3nf5_model.pdb'))

    def test_case5a(self):
        """Simple test of SAXS merge benchmark / application for Nup116"""
        inputs = [self.get_input_file_name('Nup116/25043_01C_S059_0_01.sub'),
                 self.get_input_file_name('Nup116/25043_01B_S057_0_01.sub'),
                 self.get_input_file_name('Nup116/25043_01D_S061_0_01.sub'),
                 self.get_input_file_name('Nup116/25043_01E_S063_0_01.sub'),
                 self.get_input_file_name('Nup116/25043_01F_S065_0_01.sub')]
        self.run_app( 'Nup116_5', self.get_params5() + inputs )
        self.run_results('Nup116_5',
                self.get_input_file_name('Nup116/25043_manual_merge.dat'),
                inputs,
                pdb = self.get_input_file_name('Nup116/3nf5_model.pdb'))

    # b. Simple test of SAXS merge benchmark / application for Nup192 (N/A yet)
    def test_case1b(self):
        """Simple test of SAXS merge benchmark / application for Nup192"""
        inputs = [self.get_input_file_name('Nup192/Nup192_01B_S014_0_01.sub'),
                 self.get_input_file_name('Nup192/Nup192_01C_S016_0_01.sub'),
                 self.get_input_file_name('Nup192/Nup192_01D_S018_0_01.sub'),
                 self.get_input_file_name('Nup192/Nup192_01E_S020_0_02.sub'),
                 self.get_input_file_name('Nup192/Nup192_01F_S022_0_02.sub')]
        self.run_app( 'Nup192_1', self.get_params1() + inputs )
        self.run_results('Nup192_1',
                self.get_input_file_name('Nup192/Nup192_merge.dat'), inputs)

    def test_case2b(self):
        """Simple test of SAXS merge benchmark / application for Nup192"""
        inputs = [self.get_input_file_name('Nup192/Nup192_01B_S014_0_01.sub'),
                 self.get_input_file_name('Nup192/Nup192_01C_S016_0_01.sub'),
                 self.get_input_file_name('Nup192/Nup192_01D_S018_0_01.sub'),
                 self.get_input_file_name('Nup192/Nup192_01E_S020_0_02.sub'),
                 self.get_input_file_name('Nup192/Nup192_01F_S022_0_02.sub')]
        self.run_app( 'Nup192_2', self.get_params2() + inputs )
        self.run_results('Nup192_2',
                self.get_input_file_name('Nup192/Nup192_merge.dat'), inputs)

    def test_case3b(self):
        """Simple test of SAXS merge benchmark / application for Nup192"""
        inputs = [self.get_input_file_name('Nup192/Nup192_01B_S014_0_01.sub'),
                 self.get_input_file_name('Nup192/Nup192_01C_S016_0_01.sub'),
                 self.get_input_file_name('Nup192/Nup192_01D_S018_0_01.sub'),
                 self.get_input_file_name('Nup192/Nup192_01E_S020_0_02.sub'),
                 self.get_input_file_name('Nup192/Nup192_01F_S022_0_02.sub')]
        self.run_app( 'Nup192_3', self.get_params3() + inputs )
        self.run_results('Nup192_3',
                self.get_input_file_name('Nup192/Nup192_merge.dat'), inputs)

    def test_case4b(self):
        """Simple test of SAXS merge benchmark / application for Nup192"""
        inputs = [self.get_input_file_name('Nup192/Nup192_01B_S014_0_01.sub'),
                 self.get_input_file_name('Nup192/Nup192_01C_S016_0_01.sub'),
                 self.get_input_file_name('Nup192/Nup192_01D_S018_0_01.sub'),
                 self.get_input_file_name('Nup192/Nup192_01E_S020_0_02.sub'),
                 self.get_input_file_name('Nup192/Nup192_01F_S022_0_02.sub')]
        self.run_app( 'Nup192_4', self.get_params4() + inputs )
        self.run_results('Nup192_4',
                self.get_input_file_name('Nup192/Nup192_merge.dat'), inputs)

    def test_case5b(self):
        """Simple test of SAXS merge benchmark / application for Nup192"""
        inputs = [self.get_input_file_name('Nup192/Nup192_01B_S014_0_01.sub'),
                 self.get_input_file_name('Nup192/Nup192_01C_S016_0_01.sub'),
                 self.get_input_file_name('Nup192/Nup192_01D_S018_0_01.sub'),
                 self.get_input_file_name('Nup192/Nup192_01E_S020_0_02.sub'),
                 self.get_input_file_name('Nup192/Nup192_01F_S022_0_02.sub')]
        self.run_app( 'Nup192_5', self.get_params5() + inputs )
        self.run_results('Nup192_5',
                self.get_input_file_name('Nup192/Nup192_merge.dat'), inputs)

    # c. Simple test of SAXS merge benchmark / application for Nup145 (3KEP)
    def test_case1c(self):
        """Simple test of SAXS merge benchmark / application for Nup145"""
        inputs = [self.get_input_file_name('Nup145/23923_A3_2.mccd.dat'),
                 self.get_input_file_name('Nup145/23923_A2_2.mccd.dat'),
                 self.get_input_file_name('Nup145/23923_A4_2.mccd.dat'),
                 self.get_input_file_name('Nup145/23923_A5_2.mccd.dat')]
        self.run_app( 'Nup145_1', self.get_params1() + inputs )
        self.run_results('Nup145_1',
                self.get_input_file_name('Nup145/23923_merge.dat'),
                inputs,
                pdb = self.get_input_file_name('Nup145/3kep_model.pdb'))

    def test_case2c(self):
        """Simple test of SAXS merge benchmark / application for Nup145"""
        inputs = [self.get_input_file_name('Nup145/23923_A3_2.mccd.dat'),
                 self.get_input_file_name('Nup145/23923_A2_2.mccd.dat'),
                 self.get_input_file_name('Nup145/23923_A4_2.mccd.dat'),
                 self.get_input_file_name('Nup145/23923_A5_2.mccd.dat')]
        self.run_app( 'Nup145_2', self.get_params2() + inputs )
        self.run_results('Nup145_2',
                self.get_input_file_name('Nup145/23923_merge.dat'),
                inputs,
                pdb = self.get_input_file_name('Nup145/3kep_model.pdb'))

    def test_case3c(self):
        """Simple test of SAXS merge benchmark / application for Nup145"""
        inputs = [self.get_input_file_name('Nup145/23923_A3_2.mccd.dat'),
                 self.get_input_file_name('Nup145/23923_A2_2.mccd.dat'),
                 self.get_input_file_name('Nup145/23923_A4_2.mccd.dat'),
                 self.get_input_file_name('Nup145/23923_A5_2.mccd.dat')]
        self.run_app( 'Nup145_3', self.get_params3() + inputs )
        self.run_results('Nup145_3',
                self.get_input_file_name('Nup145/23923_merge.dat'),
                inputs,
                pdb = self.get_input_file_name('Nup145/3kep_model.pdb'))

    def test_case4c(self):
        """Simple test of SAXS merge benchmark / application for Nup145"""
        inputs = [self.get_input_file_name('Nup145/23923_A3_2.mccd.dat'),
                 self.get_input_file_name('Nup145/23923_A2_2.mccd.dat'),
                 self.get_input_file_name('Nup145/23923_A4_2.mccd.dat'),
                 self.get_input_file_name('Nup145/23923_A5_2.mccd.dat')]
        self.run_app( 'Nup145_4', self.get_params4() + inputs )
        self.run_results('Nup145_4',
                self.get_input_file_name('Nup145/23923_merge.dat'),
                inputs,
                pdb = self.get_input_file_name('Nup145/3kep_model.pdb'))

    def test_case5c(self):
        """Simple test of SAXS merge benchmark / application for Nup145"""
        inputs = [self.get_input_file_name('Nup145/23923_A3_2.mccd.dat'),
                 self.get_input_file_name('Nup145/23923_A2_2.mccd.dat'),
                 self.get_input_file_name('Nup145/23923_A4_2.mccd.dat'),
                 self.get_input_file_name('Nup145/23923_A5_2.mccd.dat')]
        self.run_app( 'Nup145_5', self.get_params5() + inputs )
        self.run_results('Nup145_5',
                self.get_input_file_name('Nup145/23923_merge.dat'),
                inputs,
                pdb = self.get_input_file_name('Nup145/3kep_model.pdb'))

    # d. Simple test of SAXS merge benchmark / application for Nup133 (3KFO)
    def test_case1d(self):
        """Simple test of SAXS merge benchmark / application for Nup133"""
        inputs = [self.get_input_file_name('Nup133/23922_G4_2.mccd.dat'),
                 self.get_input_file_name('Nup133/23922_G2_2.mccd.dat'),
                 self.get_input_file_name('Nup133/23922_G3_2.mccd.dat'),
                 self.get_input_file_name('Nup133/23922_G5_2.mccd.dat')]
        self.run_app( 'Nup133_1', self.get_params1() + inputs )
        self.run_results('Nup133_1',
                self.get_input_file_name('Nup133/23922_merge.dat'),
                inputs,
                pdb = self.get_input_file_name('Nup133/3kfo_model.pdb'))

    def test_case2d(self):
        """Simple test of SAXS merge benchmark / application for Nup133"""
        inputs = [self.get_input_file_name('Nup133/23922_G4_2.mccd.dat'),
                 self.get_input_file_name('Nup133/23922_G2_2.mccd.dat'),
                 self.get_input_file_name('Nup133/23922_G3_2.mccd.dat'),
                 self.get_input_file_name('Nup133/23922_G5_2.mccd.dat')]
        self.run_app( 'Nup133_2', self.get_params2() + inputs )
        self.run_results('Nup133_2',
                self.get_input_file_name('Nup133/23922_merge.dat'),
                inputs,
                pdb = self.get_input_file_name('Nup133/3kfo_model.pdb'))

    def test_case3d(self):
        """Simple test of SAXS merge benchmark / application for Nup133"""
        inputs = [self.get_input_file_name('Nup133/23922_G4_2.mccd.dat'),
                 self.get_input_file_name('Nup133/23922_G2_2.mccd.dat'),
                 self.get_input_file_name('Nup133/23922_G3_2.mccd.dat'),
                 self.get_input_file_name('Nup133/23922_G5_2.mccd.dat')]
        self.run_app( 'Nup133_3', self.get_params3() + inputs )
        self.run_results('Nup133_3',
                self.get_input_file_name('Nup133/23922_merge.dat'),
                inputs,
                pdb = self.get_input_file_name('Nup133/3kfo_model.pdb'))

    def test_case4d(self):
        """Simple test of SAXS merge benchmark / application for Nup133"""
        inputs = [self.get_input_file_name('Nup133/23922_G4_2.mccd.dat'),
                 self.get_input_file_name('Nup133/23922_G2_2.mccd.dat'),
                 self.get_input_file_name('Nup133/23922_G3_2.mccd.dat'),
                 self.get_input_file_name('Nup133/23922_G5_2.mccd.dat')]
        self.run_app( 'Nup133_4', self.get_params4() + inputs )
        self.run_results('Nup133_4',
                self.get_input_file_name('Nup133/23922_merge.dat'),
                inputs,
                pdb = self.get_input_file_name('Nup133/3kfo_model.pdb'))

    def test_case5d(self):
        """Simple test of SAXS merge benchmark / application for Nup133"""
        inputs = [self.get_input_file_name('Nup133/23922_G4_2.mccd.dat'),
                 self.get_input_file_name('Nup133/23922_G2_2.mccd.dat'),
                 self.get_input_file_name('Nup133/23922_G3_2.mccd.dat'),
                 self.get_input_file_name('Nup133/23922_G5_2.mccd.dat')]
        self.run_app( 'Nup133_5', self.get_params5() + inputs )
        self.run_results('Nup133_5',
                self.get_input_file_name('Nup133/23922_merge.dat'),
                inputs,
                pdb = self.get_input_file_name('Nup133/3kfo_model.pdb'))

    # e. Simple test of SAXS merge benchmark / application for Mouse LAIR1 (4ESK)
    def test_case1e(self):
        """Simple test of SAXS merge benchmark / application for mo_lair1s"""
        inputs = [self.get_input_file_name('mo_lair1s/mo_lig_apo_02B_S012_0_02.sub'),
                 self.get_input_file_name('mo_lair1s/mo_lig_apo_02C_S014_0_02.sub'),
                 self.get_input_file_name('mo_lair1s/mo_lig_apo_02D_S016_0_02.sub'),
                 self.get_input_file_name('mo_lair1s/mo_lig_apo_02E_S018_0_02.sub'),
                 self.get_input_file_name('mo_lair1s/mo_lig_apo_02F_S020_0_02.sub')]
        self.run_app( 'mo_lair1s_1', self.get_params1() + inputs )
        self.run_results('mo_lair1s_1',
                self.get_input_file_name('mo_lair1s/mo_lair1s_merged.dat'),
                inputs)

    def test_case2e(self):
        """Simple test of SAXS merge benchmark / application for mo_lair1s"""
        inputs = [self.get_input_file_name('mo_lair1s/mo_lig_apo_02B_S012_0_02.sub'),
                 self.get_input_file_name('mo_lair1s/mo_lig_apo_02C_S014_0_02.sub'),
                 self.get_input_file_name('mo_lair1s/mo_lig_apo_02D_S016_0_02.sub'),
                 self.get_input_file_name('mo_lair1s/mo_lig_apo_02E_S018_0_02.sub'),
                 self.get_input_file_name('mo_lair1s/mo_lig_apo_02F_S020_0_02.sub')]
        self.run_app( 'mo_lair1s_2', self.get_params2() + inputs )
        self.run_results('mo_lair1s_2',
                self.get_input_file_name('mo_lair1s/mo_lair1s_merged.dat'),
                inputs)

    def test_case3e(self):
        """Simple test of SAXS merge benchmark / application for mo_lair1s"""
        inputs = [self.get_input_file_name('mo_lair1s/mo_lig_apo_02B_S012_0_02.sub'),
                 self.get_input_file_name('mo_lair1s/mo_lig_apo_02C_S014_0_02.sub'),
                 self.get_input_file_name('mo_lair1s/mo_lig_apo_02D_S016_0_02.sub'),
                 self.get_input_file_name('mo_lair1s/mo_lig_apo_02E_S018_0_02.sub'),
                 self.get_input_file_name('mo_lair1s/mo_lig_apo_02F_S020_0_02.sub')]
        self.run_app( 'mo_lair1s_3', self.get_params3() + inputs )
        self.run_results('mo_lair1s_3',
                self.get_input_file_name('mo_lair1s/mo_lair1s_merged.dat'),
                inputs)

    def test_case4e(self):
        """Simple test of SAXS merge benchmark / application for mo_lair1s"""
        inputs = [self.get_input_file_name('mo_lair1s/mo_lig_apo_02B_S012_0_02.sub'),
                 self.get_input_file_name('mo_lair1s/mo_lig_apo_02C_S014_0_02.sub'),
                 self.get_input_file_name('mo_lair1s/mo_lig_apo_02D_S016_0_02.sub'),
                 self.get_input_file_name('mo_lair1s/mo_lig_apo_02E_S018_0_02.sub'),
                 self.get_input_file_name('mo_lair1s/mo_lig_apo_02F_S020_0_02.sub')]
        self.run_app( 'mo_lair1s_4', self.get_params4() + inputs )
        self.run_results('mo_lair1s_4',
                self.get_input_file_name('mo_lair1s/mo_lair1s_merged.dat'),
                inputs)

    def test_case5e(self):
        """Simple test of SAXS merge benchmark / application for mo_lair1s"""
        inputs = [self.get_input_file_name('mo_lair1s/mo_lig_apo_02B_S012_0_02.sub'),
                 self.get_input_file_name('mo_lair1s/mo_lig_apo_02C_S014_0_02.sub'),
                 self.get_input_file_name('mo_lair1s/mo_lig_apo_02D_S016_0_02.sub'),
                 self.get_input_file_name('mo_lair1s/mo_lig_apo_02E_S018_0_02.sub'),
                 self.get_input_file_name('mo_lair1s/mo_lig_apo_02F_S020_0_02.sub')]
        self.run_app( 'mo_lair1s_5', self.get_params5() + inputs )
        self.run_results('mo_lair1s_5',
                self.get_input_file_name('mo_lair1s/mo_lair1s_merged.dat'),
                inputs)

    # f. Simple test of SAXS merge benchmark / application
        #       for Mouse LAIR1, Extra Cellular Domain (4ETY)
    def test_case1f(self):
        """Simple test of SAXS merge benchmark / application for mo_lair1"""
        inputs = [self.get_input_file_name('mo_lair1_ecd/mo_lecd_apo_01B_S036_0_02.sub'),
                 self.get_input_file_name('mo_lair1_ecd/mo_lecd_apo_01C_S038_0_02.sub'),
                 self.get_input_file_name('mo_lair1_ecd/mo_lecd_apo_01D_S040_0_02.sub'),
                 self.get_input_file_name('mo_lair1_ecd/mo_lecd_apo_01E_S042_0_02.sub'),
                 self.get_input_file_name('mo_lair1_ecd/mo_lecd_apo_01F_S044_0_02.sub')]
        self.run_app( 'mo_lair1_ecd_1', self.get_params1() + inputs )
        self.run_results('mo_lair1_ecd_1',
                self.get_input_file_name('mo_lair1_ecd/mo_lair1_ecd_merged.dat'),
                inputs)

    def test_case2f(self):
        """Simple test of SAXS merge benchmark / application for mo_lair1_ecd"""
        inputs = [self.get_input_file_name('mo_lair1_ecd/mo_lecd_apo_01B_S036_0_02.sub'),
                 self.get_input_file_name('mo_lair1_ecd/mo_lecd_apo_01C_S038_0_02.sub'),
                 self.get_input_file_name('mo_lair1_ecd/mo_lecd_apo_01D_S040_0_02.sub'),
                 self.get_input_file_name('mo_lair1_ecd/mo_lecd_apo_01E_S042_0_02.sub'),
                 self.get_input_file_name('mo_lair1_ecd/mo_lecd_apo_01F_S044_0_02.sub')]
        self.run_app( 'mo_lair1_ecd_2', self.get_params2() + inputs )
        self.run_results('mo_lair1_ecd_2',
                self.get_input_file_name('mo_lair1_ecd/mo_lair1_ecd_merged.dat'),
                inputs)

    def test_case3f(self):
        """Simple test of SAXS merge benchmark / application for mo_lair1"""
        inputs = [self.get_input_file_name('mo_lair1_ecd/mo_lecd_apo_01B_S036_0_02.sub'),
                 self.get_input_file_name('mo_lair1_ecd/mo_lecd_apo_01C_S038_0_02.sub'),
                 self.get_input_file_name('mo_lair1_ecd/mo_lecd_apo_01D_S040_0_02.sub'),
                 self.get_input_file_name('mo_lair1_ecd/mo_lecd_apo_01E_S042_0_02.sub'),
                 self.get_input_file_name('mo_lair1_ecd/mo_lecd_apo_01F_S044_0_02.sub')]
        self.run_app( 'mo_lair1_ecd_3', self.get_params3() + inputs )
        self.run_results('mo_lair1_ecd_3',
                self.get_input_file_name('mo_lair1_ecd/mo_lair1_ecd_merged.dat'),
                inputs)

    def test_case4f(self):
        """Simple test of SAXS merge benchmark / application for mo_lair1"""
        inputs = [self.get_input_file_name('mo_lair1_ecd/mo_lecd_apo_01B_S036_0_02.sub'),
                 self.get_input_file_name('mo_lair1_ecd/mo_lecd_apo_01C_S038_0_02.sub'),
                 self.get_input_file_name('mo_lair1_ecd/mo_lecd_apo_01D_S040_0_02.sub'),
                 self.get_input_file_name('mo_lair1_ecd/mo_lecd_apo_01E_S042_0_02.sub'),
                 self.get_input_file_name('mo_lair1_ecd/mo_lecd_apo_01F_S044_0_02.sub')]
        self.run_app( 'mo_lair1_ecd_4', self.get_params4() + inputs )
        self.run_results('mo_lair1_ecd_4',
                self.get_input_file_name('mo_lair1_ecd/mo_lair1_ecd_merged.dat'),
                inputs)

    def test_case5f(self):
        """Simple test of SAXS merge benchmark / application for mo_lair1"""
        inputs = [self.get_input_file_name('mo_lair1_ecd/mo_lecd_apo_01B_S036_0_02.sub'),
                 self.get_input_file_name('mo_lair1_ecd/mo_lecd_apo_01C_S038_0_02.sub'),
                 self.get_input_file_name('mo_lair1_ecd/mo_lecd_apo_01D_S040_0_02.sub'),
                 self.get_input_file_name('mo_lair1_ecd/mo_lecd_apo_01E_S042_0_02.sub'),
                 self.get_input_file_name('mo_lair1_ecd/mo_lecd_apo_01F_S044_0_02.sub')]
        self.run_app( 'mo_lair1_ecd_5', self.get_params5() + inputs )
        self.run_results('mo_lair1_ecd_5',
                self.get_input_file_name('mo_lair1_ecd/mo_lair1_ecd_merged.dat'),
                inputs)

    # g. Simple test of SAXS merge benchmark / application for Amelogenin, rH174 at pH 5.6
    def test_case1g(self):
        """Simple test of SAXS merge benchmark / application for amelogenin pH 5.6"""
        inputs = [self.get_input_file_name('amelogenin_pH56/3_pH56_lCaP_01C_S014_0_01.sub'),
                 self.get_input_file_name('amelogenin_pH56/3_pH56_lCaP_01B_S012_0_01.sub'),
                 self.get_input_file_name('amelogenin_pH56/3_pH56_lCaP_01D_S016_0_01.sub'),
                 self.get_input_file_name('amelogenin_pH56/3_pH56_lCaP_01E_S018_0_01.sub')]
        self.run_app( 'amelogenin_pH56_1', self.get_params1() + inputs )
        self.run_results('amelogenin_pH56_1',
                self.get_input_file_name('amelogenin_pH56/amelogenin_pH56_merged.dat'),
                inputs)

    def test_case2g(self):
        """Simple test of SAXS merge benchmark / application for amelogenin pH 5.6"""
        inputs = [self.get_input_file_name('amelogenin_pH56/3_pH56_lCaP_01C_S014_0_01.sub'),
                 self.get_input_file_name('amelogenin_pH56/3_pH56_lCaP_01B_S012_0_01.sub'),
                 self.get_input_file_name('amelogenin_pH56/3_pH56_lCaP_01D_S016_0_01.sub'),
                 self.get_input_file_name('amelogenin_pH56/3_pH56_lCaP_01E_S018_0_01.sub')]
        self.run_app( 'amelogenin_pH56_2', self.get_params2() + inputs )
        self.run_results('amelogenin_pH56_2',
                self.get_input_file_name('amelogenin_pH56/amelogenin_pH56_merged.dat'),
                inputs)

    def test_case3g(self):
        """Simple test of SAXS merge benchmark / application for amelogenin pH 5.6"""
        inputs = [self.get_input_file_name('amelogenin_pH56/3_pH56_lCaP_01C_S014_0_01.sub'),
                 self.get_input_file_name('amelogenin_pH56/3_pH56_lCaP_01B_S012_0_01.sub'),
                 self.get_input_file_name('amelogenin_pH56/3_pH56_lCaP_01D_S016_0_01.sub'),
                 self.get_input_file_name('amelogenin_pH56/3_pH56_lCaP_01E_S018_0_01.sub')]
        self.run_app( 'amelogenin_pH56_3', self.get_params3() + inputs )
        self.run_results('amelogenin_pH56_3',
                self.get_input_file_name('amelogenin_pH56/amelogenin_pH56_merged.dat'),
                inputs)

    def test_case4g(self):
        """Simple test of SAXS merge benchmark / application for amelogenin pH 5.6"""
        inputs = [self.get_input_file_name('amelogenin_pH56/3_pH56_lCaP_01C_S014_0_01.sub'),
                 self.get_input_file_name('amelogenin_pH56/3_pH56_lCaP_01B_S012_0_01.sub'),
                 self.get_input_file_name('amelogenin_pH56/3_pH56_lCaP_01D_S016_0_01.sub'),
                 self.get_input_file_name('amelogenin_pH56/3_pH56_lCaP_01E_S018_0_01.sub')]
        self.run_app( 'amelogenin_pH56_4', self.get_params4() + inputs )
        self.run_results('amelogenin_pH56_4',
                self.get_input_file_name('amelogenin_pH56/amelogenin_pH56_merged.dat'),
                inputs)

    def test_case5g(self):
        """Simple test of SAXS merge benchmark / application for amelogenin pH 5.6"""
        inputs = [self.get_input_file_name('amelogenin_pH56/3_pH56_lCaP_01C_S014_0_01.sub'),
                 self.get_input_file_name('amelogenin_pH56/3_pH56_lCaP_01B_S012_0_01.sub'),
                 self.get_input_file_name('amelogenin_pH56/3_pH56_lCaP_01D_S016_0_01.sub'),
                 self.get_input_file_name('amelogenin_pH56/3_pH56_lCaP_01E_S018_0_01.sub')]
        self.run_app( 'amelogenin_pH56_5', self.get_params5() + inputs )
        self.run_results('amelogenin_pH56_5',
                self.get_input_file_name('amelogenin_pH56/amelogenin_pH56_merged.dat'),
                inputs)

    # h. Simple test of SAXS merge benchmark / application for Amelogenin, rH174 at pH 7.5
    def test_case1h(self):
        """Simple test of SAXS merge benchmark / application for amelogenin pH 7.5"""
        inputs = [self.get_input_file_name('amelogenin_pH75/5_pH75_lCaP_01f_01C_S014_0_01.sub'),
                 self.get_input_file_name('amelogenin_pH75/5_pH75_lCaP_01f_01B_S012_0_01.sub'),
                 self.get_input_file_name('amelogenin_pH75/5_pH75_lCaP_01f_01D_S016_0_01.sub'),
                 self.get_input_file_name('amelogenin_pH75/5_pH75_lCaP_01f_01E_S018_0_01.sub')]
        self.run_app( 'amelogenin_pH75_1', self.get_params1() + inputs )
        self.run_results('amelogenin_pH75_1',
                self.get_input_file_name('amelogenin_pH75/amelogenin_pH75_merged.dat'),
                inputs)

    def test_case2h(self):
        """Simple test of SAXS merge benchmark / application for amelogenin pH 7.5"""
        inputs = [self.get_input_file_name('amelogenin_pH75/5_pH75_lCaP_01f_01C_S014_0_01.sub'),
                 self.get_input_file_name('amelogenin_pH75/5_pH75_lCaP_01f_01B_S012_0_01.sub'),
                 self.get_input_file_name('amelogenin_pH75/5_pH75_lCaP_01f_01D_S016_0_01.sub'),
                 self.get_input_file_name('amelogenin_pH75/5_pH75_lCaP_01f_01E_S018_0_01.sub')]
        self.run_app( 'amelogenin_pH75_2', self.get_params2() + inputs )
        self.run_results('amelogenin_pH75_2',
                self.get_input_file_name('amelogenin_pH75/amelogenin_pH75_merged.dat'),
                inputs)

    def test_case3h(self):
        """Simple test of SAXS merge benchmark / application for amelogenin pH 7.5"""
        inputs = [self.get_input_file_name('amelogenin_pH75/5_pH75_lCaP_01f_01C_S014_0_01.sub'),
                 self.get_input_file_name('amelogenin_pH75/5_pH75_lCaP_01f_01B_S012_0_01.sub'),
                 self.get_input_file_name('amelogenin_pH75/5_pH75_lCaP_01f_01D_S016_0_01.sub'),
                 self.get_input_file_name('amelogenin_pH75/5_pH75_lCaP_01f_01E_S018_0_01.sub')]
        self.run_app( 'amelogenin_pH75_3', self.get_params3() + inputs )
        self.run_results('amelogenin_pH75_3',
                self.get_input_file_name('amelogenin_pH75/amelogenin_pH75_merged.dat'),
                inputs)

    def test_case4h(self):
        """Simple test of SAXS merge benchmark / application for amelogenin pH 7.5"""
        inputs = [self.get_input_file_name('amelogenin_pH75/5_pH75_lCaP_01f_01C_S014_0_01.sub'),
                 self.get_input_file_name('amelogenin_pH75/5_pH75_lCaP_01f_01B_S012_0_01.sub'),
                 self.get_input_file_name('amelogenin_pH75/5_pH75_lCaP_01f_01D_S016_0_01.sub'),
                 self.get_input_file_name('amelogenin_pH75/5_pH75_lCaP_01f_01E_S018_0_01.sub')]
        self.run_app( 'amelogenin_pH75_4', self.get_params4() + inputs )
        self.run_results('amelogenin_pH75_4',
                self.get_input_file_name('amelogenin_pH75/amelogenin_pH75_merged.dat'),
                inputs)

    def test_case5h(self):
        """Simple test of SAXS merge benchmark / application for amelogenin pH 7.5"""
        inputs = [self.get_input_file_name('amelogenin_pH75/5_pH75_lCaP_01f_01C_S014_0_01.sub'),
                 self.get_input_file_name('amelogenin_pH75/5_pH75_lCaP_01f_01B_S012_0_01.sub'),
                 self.get_input_file_name('amelogenin_pH75/5_pH75_lCaP_01f_01D_S016_0_01.sub'),
                 self.get_input_file_name('amelogenin_pH75/5_pH75_lCaP_01f_01E_S018_0_01.sub')]
        self.run_app( 'amelogenin_pH75_5', self.get_params5() + inputs )
        self.run_results('amelogenin_pH75_5',
                self.get_input_file_name('amelogenin_pH75/amelogenin_pH75_merged.dat'),
                inputs)

    def test_case1i(self):
        """Simple test of SAXS merge benchmark / application for Y1"""
        inputs = [self.get_input_file_name('Y1/in1.dat'),
                 self.get_input_file_name('Y1/in2.dat'),
                 self.get_input_file_name('Y1/in3.dat')]
        self.run_app( 'Y1_1', self.get_params1() + inputs )
        self.run_results('Y1_1',
                self.get_input_file_name('Y1/Y1_merge.dat'),
                inputs)

    def test_case2i(self):
        """Simple test of SAXS merge benchmark / application for Y1"""
        inputs = [self.get_input_file_name('Y1/in1.dat'),
                 self.get_input_file_name('Y1/in2.dat'),
                 self.get_input_file_name('Y1/in3.dat')]
        self.run_app( 'Y1_2', self.get_params2() + inputs )
        self.run_results('Y1_2',
                self.get_input_file_name('Y1/Y1_merge.dat'),
                inputs)

    def test_case3i(self):
        """Simple test of SAXS merge benchmark / application for Y1"""
        inputs = [self.get_input_file_name('Y1/in1.dat'),
                 self.get_input_file_name('Y1/in2.dat'),
                 self.get_input_file_name('Y1/in3.dat')]
        self.run_app( 'Y1_3', self.get_params3() + inputs )
        self.run_results('Y1_3',
                self.get_input_file_name('Y1/Y1_merge.dat'),
                inputs)

    def test_case4i(self):
        """Simple test of SAXS merge benchmark / application for Y1"""
        inputs = [self.get_input_file_name('Y1/in1.dat'),
                 self.get_input_file_name('Y1/in2.dat'),
                 self.get_input_file_name('Y1/in3.dat')]
        self.run_app( 'Y1_4', self.get_params4() + inputs )
        self.run_results('Y1_4',
                self.get_input_file_name('Y1/Y1_merge.dat'),
                inputs)

    def test_case5i(self):
        """Simple test of SAXS merge benchmark / application for Y1"""
        inputs = [self.get_input_file_name('Y1/in1.dat'),
                 self.get_input_file_name('Y1/in2.dat'),
                 self.get_input_file_name('Y1/in3.dat')]
        self.run_app( 'Y1_5', self.get_params5() + inputs )
        self.run_results('Y1_5',
                self.get_input_file_name('Y1/Y1_merge.dat'),
                inputs)

    def test_case1j(self):
        """Simple test of SAXS merge benchmark / application for Y2"""
        inputs = [self.get_input_file_name('Y2/in1.dat'),
                 self.get_input_file_name('Y2/in2.dat'),
                 self.get_input_file_name('Y2/in3.dat')]
        self.run_app( 'Y2_1', self.get_params1() + inputs )
        self.run_results('Y2_1',
                self.get_input_file_name('Y2/Y2_merge.dat'),
                inputs)

    def test_case2j(self):
        """Simple test of SAXS merge benchmark / application for Y2"""
        inputs = [self.get_input_file_name('Y2/in1.dat'),
                 self.get_input_file_name('Y2/in2.dat'),
                 self.get_input_file_name('Y2/in3.dat')]
        self.run_app( 'Y2_2', self.get_params2() + inputs )
        self.run_results('Y2_2',
                self.get_input_file_name('Y2/Y2_merge.dat'),
                inputs)

    def test_case3j(self):
        """Simple test of SAXS merge benchmark / application for Y2"""
        inputs = [self.get_input_file_name('Y2/in1.dat'),
                 self.get_input_file_name('Y2/in2.dat'),
                 self.get_input_file_name('Y2/in3.dat')]
        self.run_app( 'Y2_3', self.get_params3() + inputs )
        self.run_results('Y2_3',
                self.get_input_file_name('Y2/Y2_merge.dat'),
                inputs)

    def test_case4j(self):
        """Simple test of SAXS merge benchmark / application for Y2"""
        inputs = [self.get_input_file_name('Y2/in1.dat'),
                 self.get_input_file_name('Y2/in2.dat'),
                 self.get_input_file_name('Y2/in3.dat')]
        self.run_app( 'Y2_4', self.get_params4() + inputs )
        self.run_results('Y2_4',
                self.get_input_file_name('Y2/Y2_merge.dat'),
                inputs)

    def test_case5j(self):
        """Simple test of SAXS merge benchmark / application for Y2"""
        inputs = [self.get_input_file_name('Y2/in1.dat'),
                 self.get_input_file_name('Y2/in2.dat'),
                 self.get_input_file_name('Y2/in3.dat')]
        self.run_app( 'Y2_5', self.get_params5() + inputs )
        self.run_results('Y2_5',
                self.get_input_file_name('Y2/Y2_merge.dat'),
                inputs)

if __name__ == '__main__':
    IMP.test.main()
