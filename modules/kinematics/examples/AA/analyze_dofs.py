import numpy
#import sklearn
#import pandas
from matplotlib import pyplot as plt

# Simple script to plot degrees of freedom
# distributions from a DOF dat file

infile = "dofs_0.1.dat"
infile2 = "./AA_0.78_4/dofs.dat"
data = numpy.genfromtxt(infile, delimiter=" ")
#data2 = numpy.genfromtxt(infile2, delimiter=",")

#hm, xe, ye = numpy.histogram2d(data[:,0],data[:,1], bins=50)
#extent = [xe[0], xe[-1], ye[0], ye[-1]]
print data.shape, len(data[:,0])
#plt.clf()
#plt.imshow(hm.T, extent=extent, origin='lower')
plt.tripcolor(data[:,0], data[:,1], data[:,2],15)
#plt.pcolor(data)
plt.show()

'''
U, s, V = numpy.linalg.svd(data)


fig, ax = plt.subplots(1,3, figsize=(12,5))


ax[0].plot(U)
ax[1].plot(s)
ax[2].plot(V.T)
plt.plot(numpy.transpose(U[0:3]))
plt.show()

'''

#fig,ax = plt.subplots(1,2, figsize = (12,6))
#print data.shape
#ax[0].scatter(data[:,0], data[:,1], s=0.2)

#ax[1].scatter(data2[:,0], data2[:,1], s=0.2)
#plt.show()

