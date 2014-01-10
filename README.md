Add multiresolution as parallel hierarchies to IMP.atom

Add the IMP.atom.Resolution decorator which implements multiresolution
as parallel hierarchies. And add support to selection also. This
removes the radius criteria from Selection, but no one seemed to be
able to figure that one out. We can add a backwards compat method if
needed for that, but not having it is way simpler.

close #647
close #648

Todo:
- synchronize RMF and atom names?
- check argument orders in RMF
- move RMF to RMF::Enum