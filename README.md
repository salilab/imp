Remove RMF code and use ExternalProject to get in instead

This has the advantage of avoiding the frequent sync conflicts that arise between the RMF repository and the copy of RMF in IMP. At the moment, it has the following disadvantages
- RMF won't be automatically built with a scons build-- I don't think we care
- installation of the internally built RMF doesn't work as ExternalProject_Add installs at build time which doesn't work with the in-place usage of IMP. One can always install RMF manually if you are installing IMP, but we should probably provide better error messages.
