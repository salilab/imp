"""Chemical descriptors of commonly-used cross-linkers.

   Each of these is an instance of the :class:`ihm.ChemDescriptor` class,
   and so can be used anywhere these objects are required, generally for
   :class:`ihm.restraint.CrossLinkRestraint`.
"""

import ihm

dss = ihm.ChemDescriptor('DSS', chemical_name='disuccinimidyl suberate',
              smiles='C1CC(=O)N(C1=O)OC(=O)CCCCCCC(=O)ON2C(=O)CCC2=O',
              inchi='1S/C16H20N2O8/c19-11-7-8-12(20)17(11)25-15(23)5-'
                    '3-1-2-4-6-16(24)26-18-13(21)9-10-14(18)22/h1-10H2',
              inchi_key='ZWIBGKZDAWNIFC-UHFFFAOYSA-N')

edc = ihm.ChemDescriptor('EDC',
              chemical_name='1-ethyl-3-(3-dimethylaminopropyl)carbodiimide',
              smiles='CCN=C=NCCCN(C)C',
              inchi='1S/C8H17N3/c1-4-9-8-10-6-5-7-11(2)3/h4-7H2,1-3H3',
              inchi_key='LMDZBCPBFSXMTL-UHFFFAOYSA-N')
