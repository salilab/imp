from __future__ import print_function
import IMP
import os
import IMP.test
import IMP.core
import IMP.container
import IMP.pmi
import IMP.pmi.representation
import IMP.pmi.restraints
import IMP.pmi.restraints.crosslinking
from math import *

def sphere_cap(r1, r2, d):
    sc = 0.0
    if d <= max(r1, r2) - min(r1, r2):
        sc = min(4.0 / 3 * pi * r1 * r1 * r1,
                      4.0 / 3 * pi * r2 * r2 * r2)
    elif d >= r1 + r2 :
        sc = 0
    else:
        sc = (pi / 12 / d * (r1 + r2 - d) * (r1 + r2 - d)) * \
             (d * d + 2 * d * r1 - 3 * r1 * r1 + 2 * d * r2 + 6 * r1 * r2 -
              3 * r2 * r2)
    return sc

def get_probability(xyz1s,xyz2s,sigma1s,sigma2s,psis,length,slope):
    onemprob = 1.0

    for n in range(len(xyz1s)):
        xyz1=xyz1s[n]
        xyz2=xyz2s[n]
        sigma1=sigma1s[n]
        sigma2=sigma2s[n]
        psi = psis[n]
        psi = psi.get_scale()
        dist=IMP.core.get_distance(xyz1, xyz2)

        sigmai = sigma1.get_scale()
        sigmaj = sigma2.get_scale()
        voli = 4.0 / 3.0 * pi * sigmai * sigmai * sigmai
        volj = 4.0 / 3.0 * pi * sigmaj * sigmaj * sigmaj
        fi = 0
        fj = 0
        if dist < sigmai + sigmaj :
            xlvol = 4.0 / 3.0 * pi * (length / 2) * (length / 2) * \
                           (length / 2)
            fi = min(voli, xlvol)
            fj = min(volj, xlvol)
        else:
            di = dist - sigmaj - length / 2
            dj = dist - sigmai - length / 2
            fi = sphere_cap(sigmai, length / 2, abs(di))
            fj = sphere_cap(sigmaj, length / 2, abs(dj))
        pofr = fi * fj / voli / volj

        factor = (1.0 - (psi * (1.0 - pofr) + pofr * (1 - psi))*exp(-slope*dist))
        onemprob = onemprob * factor
    prob = 1.0 - onemprob
    return prob

def log_evaluate(restraints):
    prob = 1.0
    score = 0.0

    for r in restraints:
        prob *= r.unprotected_evaluate(None)
        if prob<=0.0000000001:
            score=score-log(prob)
            prob=1.0

    score=score-log(prob)
    return score

def init_representation_beads(m):
    r = IMP.pmi.representation.Representation(m)
    r.create_component("ProtA",color=1.0)
    r.add_component_beads("ProtA", [(1,10)],incoord=(0,0,0))
    r.add_component_beads("ProtA", [(11,20)],incoord=(10,0,0))
    r.add_component_beads("ProtA", [(21,30)],incoord=(20,0,0))
    r.create_component("ProtB",color=1.0)
    r.add_component_beads("ProtB", [(1,10)],incoord=(0,10,0))
    r.add_component_beads("ProtB", [(11,20)],incoord=(10,10,0))
    r.add_component_beads("ProtB", [(21,30)],incoord=(20,10,0))
    r.set_floppy_bodies()
    return r


def setup_crosslinks_complex(representation,mode):

    if mode=="single_category":
        columnmap={}
        columnmap["Protein1"]="pep1.accession"
        columnmap["Protein2"]="pep2.accession"
        columnmap["Residue1"]="pep1.xlinked_aa"
        columnmap["Residue2"]="pep2.xlinked_aa"
        columnmap["IDScore"]=None
        columnmap["XLUniqueID"]=None

        ids_map=IMP.pmi.tools.map()
        ids_map.set_map_element(1.0,1.0)

    with IMP.allow_deprecated():
        xl = IMP.pmi.restraints.crosslinking.ISDCrossLinkMS(representation,
                               IMP.pmi.get_data_path("polii_xlinks.csv"),
                               length=21.0,
                               slope=0.01,
                               columnmapping=columnmap,
                               ids_map=ids_map,
                               resolution=1.0,
                               label="XL",
                               csvfile=True)
    xl.add_to_model()
    xl.set_label("XL")
    psi=xl.get_psi(1.0)[0]
    psi.set_scale(0.05)
    sigma=xl.get_sigma(1)[0]
    sigma.set_scale(10.0)
    return xl

def setup_crosslinks_beads(representation,mode):

    restraints_beads=IMP.pmi.tools.get_random_cross_link_dataset(representation,
                                                number_of_cross_links=100,
                                                resolution=1.0,
                                                avoid_same_particles=True,
                                                ambiguity_probability=0.3,
                                                confidence_score_range=[0,100])

    ids_map=IMP.pmi.tools.map()
    ids_map.set_map_element(25.0,0.1)
    ids_map.set_map_element(75,0.01)

    with IMP.allow_deprecated():
        xl = IMP.pmi.restraints.crosslinking.ISDCrossLinkMS(
            representation, restraints_beads, 21, label="XL", ids_map=ids_map,
            resolution=1, inner_slope=0.01)

    sig = xl.get_sigma(1.0)[0]
    psi1 = xl.get_psi(25.0)[0]
    psi2 = xl.get_psi(75.0)[0]
    sig.set_scale(10.0)
    psi1.set_scale(0.1)
    psi2.set_scale(0.01)

    return xl,restraints_beads





class Tests(IMP.test.TestCase):

    def init_representation_complex(self, m):
        pdbfile = self.get_input_file_name("1WCM.pdb")
        fastafile = self.get_input_file_name("1WCM.fasta.txt")
        components = ["Rpb1","Rpb2","Rpb3","Rpb4"]
        chains = "ABCD"
        colors = [0.,0.1,0.5,1.0]
        beadsize = 20
        fastids = IMP.pmi.tools.get_ids_from_fasta_file(fastafile)

        r = IMP.pmi.representation.Representation(m)
        hierarchies = {}
        for n in range(len(components)):
            r.create_component(components[n], color=colors[n])
            r.add_component_sequence(components[n], fastafile,
                                     id="1WCM:"+chains[n])
            hierarchies[components[n]] = r.autobuild_model(
                components[n], pdbfile, chains[n],
                resolutions=[1, 10, 100], missingbeadsize=beadsize)
            r.setup_component_sequence_connectivity(components[n], 1)
        return r

    def test_restraint_probability_complex(self):
        m = IMP.Model()
        rcomplex=self.init_representation_complex(m)
        xlc=setup_crosslinks_complex(rcomplex,"single_category")

        # check all internals didn't change since last time
        o=IMP.pmi.output.Output()
        o.write_test("expensive_test_cross_link_ms_restraint.dat", [xlc])

        passed=o.test(self.get_input_file_name("expensive_test_cross_link_ms_restraint.dat"), [xlc])
        self.assertEqual(passed, True)
        rs=xlc.get_restraint()

        # check the probability of cross-links
        restraints=[]
        for p in xlc.pairs:
            p0 = p[0]
            p1 = p[1]
            prob = p[2].get_probability()
            resid1 = p[3]
            chain1 = p[4]
            resid2 = p[5]
            chain2 = p[6]
            attribute = p[7]
            d0 = IMP.core.XYZ(p0)
            d1 = IMP.core.XYZ(p1)
            dist=IMP.core.get_distance(d0, d1)

            sig1 = xlc.get_sigma(p[8])[0]
            sig2 = xlc.get_sigma(p[9])[0]
            psi = xlc.get_psi(p[10])[0]
            test_prob=get_probability([d0],[d1],[sig1],[sig2],[psi],21.0,0.0)
            restraints.append(p[2])


            # check that the probability is the same for
            # each cross-link
            self.assertAlmostEqual(prob, test_prob, delta=0.00001)

        # check the log_wrapper
        log_wrapper_score=rs.unprotected_evaluate(None)
        test_log_wrapper_score=log_evaluate(restraints)
        self.assertAlmostEqual(log_wrapper_score, test_log_wrapper_score, delta=0.00001)
        for output in ['excluded.None.xl.db',
                       'expensive_test_cross_link_ms_restraint.dat',
                       'included.None.xl.db', 'missing.None.xl.db']:
            os.unlink(output)

    def test_restraint_ambiguity(self):
        m=IMP.Model()
        r = IMP.pmi.representation.Representation(m)
        r.create_component("ProtA",color=1.0)
        r.add_component_beads("ProtA", [(1,10)],incoord=(0,0,0))
        r.create_component("ProtB",color=1.0)
        r.add_component_beads("ProtB", [(1,10)],incoord=(0,10,0))
        r.add_component_beads("ProtB", [(11,20)],incoord=(10,10,0))
        r.add_component_beads("ProtB", [(21,30)],incoord=(20,10,0))
        r.set_floppy_bodies()

        restraints_beads='''#
ProtA ProtB 1 1  94.81973271 1
ProtA ProtB 1 11 52.4259605298 1
ProtA ProtB 1 21 87.4778223289 1'''


        ids_map=IMP.pmi.tools.map()
        ids_map.set_map_element(25.0,0.1)
        ids_map.set_map_element(75,0.01)

        with IMP.allow_deprecated():
            xl = IMP.pmi.restraints.crosslinking.ISDCrossLinkMS(
                            r, restraints_beads, 21, label="XL", slope=0.0,
                            ids_map=ids_map, resolution=1, inner_slope=0.01)

        sig = xl.get_sigma(1.0)[0]
        psi1 = xl.get_psi(25.0)[0]
        psi2 = xl.get_psi(75.0)[0]
        sig.set_scale(10.0)
        psi1.set_scale(0.1)
        psi2.set_scale(0.01)


        for i in range(100):
            r.shuffle_configuration(max_translation=10)
            for p in xl.pairs:
                p0 = p[0]
                p1 = p[1]
                ln = p[2]
                resid1 = p[3]
                chain1 = p[4]
                resid2 = p[5]
                chain2 = p[6]
                attribute = p[7]
                xlid=p[11]
                d0 = IMP.core.XYZ(p0)
                d1 = IMP.core.XYZ(p1)
                sig1 = xl.get_sigma(p[8])[0]
                sig2 = xl.get_sigma(p[9])[0]
                psi =  xl.get_psi(p[10])[0]
                self.assertEqual(-log(ln.get_probability()),xl.rs.unprotected_evaluate(None))

    @IMP.test.expectedFailure
    def test_restraint_probability_beads(self):
        m = IMP.Model()
        rbeads=init_representation_beads(m)
        xlb,restraints_beads=setup_crosslinks_beads(rbeads,"single_category")

        # check internal data structure
        ds=xlb.pairs
        nxl=0
        for l in restraints_beads.split("\n"):
            if not l.strip(): continue
            if l[0]=="#": continue
            t=l.split()

            chain1 = t[0]
            chain2 = t[1]
            res1 =  int(t[2])
            res2 =  int(t[3])
            ids =   float(t[4])
            xlid =  int(t[5])
            dsres1 = ds[nxl][3]
            dschain1 = ds[nxl][4]
            dsres2 = ds[nxl][5]
            dschain2 = ds[nxl][6]
            dsxlid=    int(ds[nxl][11])
            '''
            self.assertEqual(chain1,dschain1)
            self.assertEqual(chain2,dschain2)
            self.assertEqual(res1,dsres1)
            self.assertEqual(res2,dsres2)
            self.assertEqual(xlid,dsxlid)
            '''
            nxl+=1

        # randomize coordinates and check that the probability is OK
        for i in range(100):
            rbeads.shuffle_configuration(max_translation=10)
            cross_link_dict={}
            for p in xlb.pairs:
                p0 = p[0]
                p1 = p[1]
                prob = p[2].get_probability()
                resid1 = p[3]
                chain1 = p[4]
                resid2 = p[5]
                chain2 = p[6]
                attribute = p[7]
                xlid=p[11]
                d0 = IMP.core.XYZ(p0)
                d1 = IMP.core.XYZ(p1)
                sig1 = xlb.get_sigma(p[8])[0]
                sig2 = xlb.get_sigma(p[9])[0]
                psi =  xlb.get_psi(p[10])[0]

                if xlid not in cross_link_dict:
                    cross_link_dict[xlid]=([d0],[d1],[sig1],[sig2],[psi],prob)
                else:
                    cross_link_dict[xlid][0].append(d0)
                    cross_link_dict[xlid][1].append(d1)
                    cross_link_dict[xlid][2].append(sig1)
                    cross_link_dict[xlid][3].append(sig2)
                    cross_link_dict[xlid][4].append(psi)

            for xlid in cross_link_dict:

                test_prob=get_probability(cross_link_dict[xlid][0],
                                               cross_link_dict[xlid][1],
                                               cross_link_dict[xlid][2],
                                               cross_link_dict[xlid][3],
                                               cross_link_dict[xlid][4],21.0,0.01)
                prob=cross_link_dict[xlid][5]

                self.assertAlmostEqual(prob/test_prob,1.0, delta=0.0001)
        for output in ['excluded.None.xl.db',
                       'included.None.xl.db', 'missing.None.xl.db']:
            os.unlink(output)

if __name__ == '__main__':
    IMP.test.main()
