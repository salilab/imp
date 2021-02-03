"""Chemical descriptors of commonly-used cross-linkers.

   Each of these is an instance of the :class:`ihm.ChemDescriptor` class,
   and so can be used anywhere these objects are required, generally for
   :class:`ihm.restraint.CrossLinkRestraint`.
"""

import ihm

dss = ihm.ChemDescriptor(
    'DSS', chemical_name='disuccinimidyl suberate',
    smiles='C1CC(=O)N(C1=O)OC(=O)CCCCCCC(=O)ON2C(=O)CCC2=O',
    inchi='1S/C16H20N2O8/c19-11-7-8-12(20)17(11)25-15(23)5-'
          '3-1-2-4-6-16(24)26-18-13(21)9-10-14(18)22/h1-10H2',
    inchi_key='ZWIBGKZDAWNIFC-UHFFFAOYSA-N')

dsg = ihm.ChemDescriptor(
    'DSG', chemical_name='disuccinimidyl glutarate',
    smiles='C1CC(=O)N(C1=O)OC(=O)CCCC(=O)ON2C(=O)CCC2=O',
    inchi='1S/C13H14N2O8/c16-8-4-5-9(17)14(8)22-12(20)2-1-3-'
          '13(21)23-15-10(18)6-7-11(15)19/h1-7H2',
    inchi_key='LNQHREYHFRFJAU-UHFFFAOYSA-N')

bs3 = ihm.ChemDescriptor(
    'BS3', chemical_name='bissulfosuccinimidyl suberate',
    smiles='C1C(C(=O)N(C1=O)OC(=O)CCCCCCC(=O)ON2C(=O)CC(C2=O)S(=O)'
           '(=O)O)S(=O)(=O)O',
    inchi='1S/C16H20N2O14S2/c19-11-7-9(33(25,26)27)15(23)17(11)31'
          '-13(21)5-3-1-2-4-6-14(22)32-18-12(20)8-10(16(18)24)'
          '34(28,29)30/h9-10H,1-8H2,(H,25,26,27)(H,28,29,30)',
    inchi_key='VYLDEYYOISNGST-UHFFFAOYSA-N')

dsso = ihm.ChemDescriptor(
    'DSSO', chemical_name='disuccinimidyl sulfoxide',
    smiles='O=C(CCS(CCC(ON1C(CCC1=O)=O)=O)=O)ON2C(CCC2=O)=O',
    inchi='1S/C14H16N2O9S/c17-9-1-2-10(18)15(9)24-13(21)5-7-'
          '26(23)8-6-14(22)25-16-11(19)3-4-12(16)20/h1-8H2',
    inchi_key='XJSVVHDQSGMHAJ-UHFFFAOYSA-N')

edc = ihm.ChemDescriptor(
    'EDC', chemical_name='1-ethyl-3-(3-dimethylaminopropyl)carbodiimide',
    smiles='CCN=C=NCCCN(C)C',
    inchi='1S/C8H17N3/c1-4-9-8-10-6-5-7-11(2)3/h4-7H2,1-3H3',
    inchi_key='LMDZBCPBFSXMTL-UHFFFAOYSA-N')

dhso = ihm.ChemDescriptor(
    'DHSO', chemical_name='dihydrazide sulfoxide',
    smiles='NNC(=O)CC[S](=O)CCC(=O)NN',
    inchi='1S/C6H14N4O3S/c7-9-5(11)1-3-14(13)4-2-6(12)10-8'
          '/h1-4,7-8H2,(H,9,11)(H,10,12)',
    inchi_key='XTCXQISMAWBOOT-UHFFFAOYSA-N')

bmso = ihm.ChemDescriptor(
    'BMSO', chemical_name='bismaleimide sulfoxide',
    smiles='O=C(CC[S](=O)CCC(=O)NCCN1C(=O)C=CC1=O)NCCN2C(=O)C=CC2=O',
    inchi='1S/C18H22N4O7S/c23-13(19-7-9-21-15(25)1-2-16(21)26)5-'
          '11-30(29)12-6-14(24)20-8-10-22-17(27)3-4-18(22)28/h1-'
          '4H,5-12H2,(H,19,23)(H,20,24)',
    inchi_key='PUNDHDZIOGBGHG-UHFFFAOYSA-N')

sda = ihm.ChemDescriptor(
    'SDA', chemical_name="succinimidyl 4,4'-azipentanoate",
    smiles='CC1(N=N1)CCC(ON2C(CCC2=O)=O)=O',
    inchi='1S/C9H11N3O4/c1-9(10-11-9)5-4-8(15)16-12-6(13)2-3-'
          '7(12)14/h2-5H2,1H3',
    inchi_key=' SYYLQNPWAPHRFV-UHFFFAOYSA-N')
