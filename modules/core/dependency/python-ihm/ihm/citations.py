# -*- coding: utf-8 -*-

"""Citations for some commonly-used software packages.

   Each of these is an instance of the :class:`ihm.Citation` class,
   and so can be used anywhere these objects are required, generally for
   :class:`ihm.Software`.
"""

import ihm


imp = ihm.Citation(
    pmid='22272186',
    title='Putting the pieces together: integrative modeling platform '
          'software for structure determination of macromolecular assemblies',
    journal='PLoS Biol', volume=10, page_range='e1001244', year=2012,
    authors=['Russel D', 'Lasker K', 'Webb B', 'Velázquez-Muriel J', 'Tjioe E',
             'Schneidman-Duhovny D', 'Peterson B', 'Sali A'],
    doi='10.1371/journal.pbio.1001244')


pmi = ihm.Citation(
    pmid='31396911',
    title='Modeling Biological Complexes Using Integrative Modeling Platform.',
    journal='Methods Mol Biol', volume=2022, page_range=(353, 377), year=2019,
    authors=['Saltzberg D', 'Greenberg CH', 'Viswanath S', 'Chemmama I',
             'Webb B', 'Pellarin R', 'Echeverria I', 'Sali A'],
    doi='10.1007/978-1-4939-9608-7_15')


modeller = ihm.Citation(
    pmid='8254673',
    title='Comparative protein modelling by satisfaction of '
          'spatial restraints.',
    journal='J Mol Biol', volume=234, page_range=(779, 815), year=1993,
    authors=['Sali A', 'Blundell TL'], doi='10.1006/jmbi.1993.1626')


psipred = ihm.Citation(
    pmid='10493868',
    title='Protein secondary structure prediction based on position-specific '
          'scoring matrices.',
    journal='J Mol Biol', volume=292, page_range=(195, 202), year=1999,
    authors=['Jones DT'], doi='10.1006/jmbi.1999.3091')


disopred = ihm.Citation(
    pmid='25391399',
    title='DISOPRED3: precise disordered region predictions with annotated '
          'protein-binding activity.',
    journal='Bioinformatics', volume=31, page_range=(857, 863), year=2015,
    authors=['Jones DT', 'Cozzetto D'], doi='10.1093/bioinformatics/btu744')


hhpred = ihm.Citation(
    pmid='15980461',
    title='The HHpred interactive server for protein homology detection '
          'and structure prediction.',
    journal='Nucleic Acids Res', volume=33, page_range=('W244', 'W248'),
    year=2005, authors=['Söding J', 'Biegert A', 'Lupas AN'],
    doi='10.1093/nar/gki408')


relion = ihm.Citation(
    pmid='23000701',
    title='RELION: implementation of a Bayesian approach to cryo-EM '
          'structure determination.',
    journal='J Struct Biol', volume=180, page_range=(519, 530), year=2012,
    authors=['Scheres SH'], doi='10.1016/j.jsb.2012.09.006')


phyre2 = ihm.Citation(
    pmid='25950237',
    title='The Phyre2 web portal for protein modeling, prediction '
          'and analysis.',
    journal='Nat Protoc', volume=10, page_range=('845', '858'), year=2015,
    authors=['Kelley LA', 'Mezulis S', 'Yates CM', 'Wass MN', 'Sternberg MJ'],
    doi='10.1038/nprot.2015.053')


swiss_model = ihm.Citation(
    pmid='29788355',
    title='SWISS-MODEL: homology modelling of protein structures '
          'and complexes.',
    journal='Nucleic Acids Res', volume=46, page_range=('W296', 'W303'),
    year=2018,
    authors=['Waterhouse A', 'Bertoni M', 'Bienert S', 'Studer G',
             'Tauriello G', 'Gumienny R', 'Heer FT', 'de Beer TAP',
             'Rempfer C', 'Bordoli L', 'Lepore R', 'Schwede T'],
    doi='10.1093/nar/gky427')
