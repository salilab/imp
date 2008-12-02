# script to calculate SAXS spectrum of a model
from modeller import *
from modeller.scripts import complete_pdb

env=environ()
env.libs.topology.read(file='$(LIB)/top_heav.lib')
env.libs.parameters.read(file='$(LIB)/par.lib')
log.verbose()
### Calculate SAXS spectrum of correct structure
mdl = complete_pdb(env, 'single_protein.pdb')
atmsel = selection(mdl)
saxs = saxsdata(env)

saxs.ini_saxs(atmsel=atmsel, s_min= 0.009, s_max=0.325, maxs=100, nmesh=505,
   natomtyp=15, represtyp='heav',
   filename='$(LIB)/formfactors-int_tab_solvation.lib',
   wswitch = 'uniform', s_low=0.0, s_hi=0.5, s_hybrid=0.0,
   spaceflag= 'real', use_lookup=False)

#saxs.ini_saxs(atmsel=atmsel, s_min= 0.0, s_max=0.5, maxs=100, nmesh=100,
#   natomtyp=15, represtyp='heav',
#   filename='$(LIB)/formfactors-int_tab_solvation.lib',
#   wswitch = 'uniform', s_low=0.0, s_hi=0.5, s_hybrid=0.0,
#   spaceflag= 'real', use_lookup=False)

### append SAXS data to energy structure
mdl.env.edat.saxsdata.append(saxs)
mdl.saxs_intens(saxsd=saxs, filename='i_s.txt', fitflag=False)
saxs.saxs_read(filename='i_s.txt')
# create some restraints
#mdl.restraints.make(atmsel, restraint_type='stereo', spline_on_site=False)
#edat = atmsel.energy()
#atmsel.randomize_xyz(deviation=2.)
#edat = atmsel.energy()
#mdl.saxs_intens(saxsd=saxs, filename='i_s_fit.txt',fitflag=True)
