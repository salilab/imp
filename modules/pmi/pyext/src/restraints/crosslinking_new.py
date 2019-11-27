"""@namespace IMP.pmi.restraints.crosslinking_new
Restraints for handling crosslinking data. This temporary module has been
merged with IMP.pmi.restraints.crosslinking and will be removed in the next
IMP release.
"""

import IMP
from IMP.pmi.restraints.crosslinking import DisulfideCrossLinkRestraint

IMP.deprecated_module("2.12", __name__,
                      "Use IMP.pmi.restraints.crosslinking instead")
