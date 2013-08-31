import IMP
import IMP.core
import IMP.algebra
import IMP.atom
import IMP.isd
import gzip
from math import exp,log,pi

'''This program generate a Table of score values for the cross-link restraint varying the
Crosslink fraction, the noise, epsilon, the distance between particle and the weights.
It currently creates only data for the two state case.'''

m=IMP.Model()
betatuple=(0.03, 0.1,30)
disttuple=(0.0,25.0, 1000)
omegatuple=(1.0, 1000.0, 50)
sigmatuple=(1.0,1.0,1)
epsilontuple=(0.01,0.99)

#pfm file from cysteine_FES.dat
cysteine_FES_dat=[(0.230424407, 7.0381e-28), (0.2365070296, 7.25201e-28), (0.2425896523, 7.80539e-28), (0.2486722749, 9.2144e-28), (0.2547548975, 1.30004e-27),
              (0.2608375202, 2.50574e-27), (0.2669201428, 7.91145e-27), (0.2730027655, 5.03747e-26), (0.2790853881, 7.78829e-25), (0.2851680107, 3.21356e-23),
              (0.2912506334, 3.32407e-21), (0.297333256, 6.71683e-19), (0.3034158786, 1.76968e-16), (0.3094985013, 3.81682e-14), (0.3155811239, 4.50004e-12),
              (0.3216637465, 2.28107e-10), (0.3277463692, 4.79114e-09), (0.3338289918, 4.76432e-08), (0.3399116145, 2.7977e-07), (0.3459942371, 1.20566e-06),
              (0.3520768597, 4.42649e-06), (0.3581594824, 1.47042e-05), (0.364242105, 4.3745e-05), (0.3703247276, 0.000111944), (0.3764073503, 0.000239032),
              (0.3824899729, 0.000427014), (0.3885725955, 0.00066164), (0.3946552182, 0.000937414), (0.4007378408, 0.00127473), (0.4068204635, 0.00171443),
              (0.4129030861, 0.0023014), (0.4189857087, 0.00306689), (0.4250683314, 0.00401459), (0.431150954, 0.00511908), (0.4372335766, 0.00634095),
              (0.4433161993, 0.00764802), (0.4493988219, 0.00902362), (0.4554814445, 0.0104585), (0.4615640672, 0.0119454), (0.4676466898, 0.0134918),
              (0.4737293125, 0.0151342), (0.4798119351, 0.0169161), (0.4858945577, 0.0188141), (0.4919771804, 0.0206602), (0.498059803, 0.0221492),
              (0.5041424256, 0.0229906), (0.5102250483, 0.0231181), (0.5163076709, 0.0227579), (0.5223902935, 0.0222833), (0.5284729162, 0.0220038),
              (0.5345555388, 0.0220545), (0.5406381615, 0.0224249), (0.5467207841, 0.0230785), (0.5528034067, 0.0240845), (0.5588860294, 0.0256641),
              (0.564968652, 0.0280903), (0.5710512746, 0.0314531), (0.5771338973, 0.0353664), (0.5832165199, 0.0388484), (0.5892991425, 0.0407352),
              (0.5953817652, 0.0405242), (0.6014643878, 0.0387727), (0.6075470105, 0.0365693), (0.6136296331, 0.0347555), (0.6197122557, 0.0336085),
              (0.6257948784, 0.0329487), (0.631877501, 0.0323771), (0.6379601236, 0.0314711), (0.6440427463, 0.0298918), (0.6501253689, 0.02743),
              (0.6562079915, 0.0240525), (0.6622906142, 0.0199454), (0.6683732368, 0.0154716), (0.6744558595, 0.0110465), (0.6805384821, 0.00706293),
              (0.6866211047, 0.00388943), (0.6927037274, 0.0017753), (0.69878635, 0.000660821), (0.7048689726, 0.000204988), (0.7109515953, 5.58067e-05),
              (0.7170342179, 1.39326e-05), (0.7231168405, 3.13325e-06), (0.7291994632, 5.63055e-07), (0.7352820858, 6.50158e-08), (0.7413647085, 3.73039e-09),
              (0.7474473311, 8.73345e-11), (0.7535299537, 8.04334e-13), (0.7596125764, 3.4897e-15), (0.765695199, 1.04182e-17), (0.7717778216, 3.48028e-20),
              (0.7778604443, 2.0696e-22), (0.7839430669, 3.03873e-24), (0.7900256895, 1.25666e-25), (0.7961083122, 1.3886e-26), (0.8021909348, 3.43959e-27),
              (0.8082735575, 1.53018e-27), (0.8143561801, 9.94758e-28), (0.8204388027, 8.0657e-28), (0.8265214254, 7.34554e-28), (0.832604048, 7.07026e-28)]


def SetupNuisance(initialvalue,minvalue,maxvalue,isoptimized=True):
    nuisance=IMP.isd.Scale.setup_particle(IMP.kernel.Particle(m),initialvalue)
    nuisance.set_lower(minvalue)
    nuisance.set_upper(maxvalue)

    nuisance.set_is_optimized(nuisance.get_nuisance_key(),isoptimized)
    return nuisance

def SetupWeight(isoptimized=True):
    pw=IMP.kernel.Particle(m)
    weight=IMP.isd.Weight.setup_particle(pw)
    weight.set_weights_are_optimized(True)
    return weight


def get_cross_link_data(directory,filename,(distmin,distmax,ndist),
                                             (omegamin,omegamax,nomega),
                                            (sigmamin,sigmamax,nsigma)):



    dist_grid=get_grid(distmin, distmax, ndist, False)
    omega_grid=get_log_grid(omegamin, omegamax, nomega)
    sigma_grid=get_log_grid(sigmamin, sigmamax, nsigma)
    try:
        x=1/0
        #try to open the cross-link database
        filen=IMP.isd.get_data_path("CrossLinkPMFs.dict")
        xlpot=open(filen)

        for line in xlpot:
            dictionary=eval(line)
            break
        xpot=dictionary[directory][filename]["distance"]
        pot=dictionary[directory][filename]["gofr"]

        xlmsdata=IMP.isd.CrossLinkData(dist_grid,omega_grid,sigma_grid,xpot,pot,10.0,20.0)
    except:
        #if current IMP version does not support it, use cysteine pmf file directly
        xpot=[]
        pot=[]
        for ele in cysteine_FES_dat:
            x=10*ele[0]
            xpot.append(x)
            pot.append(ele[1]/4.0/pi/x/x)
        xlmsdata=IMP.isd.CrossLinkData(dist_grid,omega_grid,sigma_grid,xpot,pot,10.0,20.0)
    return xlmsdata



def get_grid(gmin,gmax,ngrid,boundaries):
    grid=[]
    dx = ( gmax - gmin ) / float(ngrid)
    for i in range(0,ngrid+1):
        if(not boundaries and i==0): continue
        if(not boundaries and i==ngrid): continue
        grid.append( gmin + float(i) * dx )
    return grid

def get_log_grid(gmin,gmax,ngrid):
    grid=[]
    for i in range(0,ngrid+1):
        grid.append( gmin*exp(float(i)/ngrid*log(gmax/gmin)) )
    return grid

ps=[]
#setting up ps
p0=IMP.kernel.Particle(m)
xyz=IMP.core.XYZR.setup_particle(p0,IMP.algebra.Sphere3D(IMP.algebra.Vector3D(0,0,0),3.0))
xyz.set_coordinates_are_optimized(True)
ps.append(p0)

p0=IMP.kernel.Particle(m)
xyz0=IMP.core.XYZR.setup_particle(p0,IMP.algebra.Sphere3D(IMP.algebra.Vector3D(0,0,0),3.0))
xyz0.set_coordinates_are_optimized(True)
ps.append(p0)

p0=IMP.kernel.Particle(m)
xyz0=IMP.core.XYZR.setup_particle(p0,IMP.algebra.Sphere3D(IMP.algebra.Vector3D(0,0,0),3.0))
xyz0.set_coordinates_are_optimized(True)
ps.append(p0)

crossdata=get_cross_link_data("cysteine","cysteine_CA_FES.txt.standard",
                  disttuple,omegatuple,sigmatuple)


# beta
beta=SetupNuisance(betatuple[0],betatuple[0],betatuple[1],True)
# sigma
sigma=SetupNuisance(sigmatuple[0],sigmatuple[0],sigmatuple[1],True)
#population particle
weight=SetupWeight(True)
#epsilon
epsilon=SetupNuisance(epsilontuple[0],epsilontuple[0],epsilontuple[1],True)


# create grids needed by CysteineCrossLinkData
fmod_grid=get_grid(0.0, 1.0, 300, True)
omega2_grid=get_log_grid(0.001, 10000.0, 100)
beta_grid=get_log_grid(betatuple[0],betatuple[1],betatuple[2])



x1list=get_grid(1.0,30.0,10,True)
x2list=get_grid(1.0,30.0,10,True)
betalist=get_log_grid(betatuple[0],betatuple[1],10)
weightlist=get_grid(0,1,5,True)
epsilonlist=get_grid(epsilontuple[0],epsilontuple[1],5,True)

output_long=""

for fexp in [0,0.25,0.5,0.75,1.0]:

    #erf1dataunbiased=IMP.isd.CrossLinkMSData(dist_grid,omega_grid,sigma_grid,10.0)
    datacyst=IMP.isd.CysteineCrossLinkData(fexp,fmod_grid,omega2_grid,beta_grid)

    cystrest=IMP.isd.CysteineCrossLinkRestraint(beta,sigma,epsilon,weight,crossdata,datacyst)
    cystrest.add_contribution(ps[0],ps[1])
    cystrest.add_contribution(ps[0],ps[2])

    for x1 in x1list:
        for x2 in x2list:
            for ibeta in betalist:
                for iweight in weightlist:
                    for iepsilon in epsilonlist:
                        IMP.core.XYZ(ps[1]).set_coordinates(IMP.algebra.Vector3D(x1,0.0,0.0))
                        IMP.core.XYZ(ps[2]).set_coordinates(IMP.algebra.Vector3D(x2,0.0,0.0))
                        beta.set_scale(ibeta)
                        weight.set_weights([iweight,1-iweight])
                        epsilon.set_scale(iepsilon)
                        s=str((fexp,x1,x2,ibeta,iweight,iepsilon,cystrest.unprotected_evaluate(None),cystrest.get_model_frequency(),
                        cystrest.get_probability(),cystrest.get_standard_error(),cystrest.get_frequencies()))+"\n"
                        output_long+=s

f = gzip.open('test_CysteineCrosslink.data.gz', 'wb')
f.write(output_long)
f.close()

