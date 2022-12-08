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
    authors=['Russel, D.', 'Lasker, K.', 'Webb, B.', 'Velázquez-Muriel, J.',
             'Tjioe, E.', 'Schneidman-Duhovny, D.', 'Peterson, B.',
             'Sali, A.'],
    doi='10.1371/journal.pbio.1001244')


pmi = ihm.Citation(
    pmid='31396911',
    title='Modeling Biological Complexes Using Integrative Modeling Platform.',
    journal='Methods Mol Biol', volume=2022, page_range=(353, 377), year=2019,
    authors=['Saltzberg, D.', 'Greenberg, C.H.', 'Viswanath, S.',
             'Chemmama, I.', 'Webb, B.', 'Pellarin, R.', 'Echeverria, I.',
             'Sali, A.'],
    doi='10.1007/978-1-4939-9608-7_15')


modeller = ihm.Citation(
    pmid='8254673',
    title='Comparative protein modelling by satisfaction of '
          'spatial restraints.',
    journal='J Mol Biol', volume=234, page_range=(779, 815), year=1993,
    authors=['Sali, A.', 'Blundell, T.L.'], doi='10.1006/jmbi.1993.1626')


psipred = ihm.Citation(
    pmid='10493868',
    title='Protein secondary structure prediction based on position-specific '
          'scoring matrices.',
    journal='J Mol Biol', volume=292, page_range=(195, 202), year=1999,
    authors=['Jones, D.T.'], doi='10.1006/jmbi.1999.3091')


disopred = ihm.Citation(
    pmid='25391399',
    title='DISOPRED3: precise disordered region predictions with annotated '
          'protein-binding activity.',
    journal='Bioinformatics', volume=31, page_range=(857, 863), year=2015,
    authors=['Jones, D.T.', 'Cozzetto D'], doi='10.1093/bioinformatics/btu744')


hhpred = ihm.Citation(
    pmid='15980461',
    title='The HHpred interactive server for protein homology detection '
          'and structure prediction.',
    journal='Nucleic Acids Res', volume=33, page_range=('W244', 'W248'),
    year=2005, authors=['Söding, J.', 'Biegert, A.', 'Lupas, A.N.'],
    doi='10.1093/nar/gki408')


relion = ihm.Citation(
    pmid='23000701',
    title='RELION: implementation of a Bayesian approach to cryo-EM '
          'structure determination.',
    journal='J Struct Biol', volume=180, page_range=(519, 530), year=2012,
    authors=['Scheres, S.H.'], doi='10.1016/j.jsb.2012.09.006')


phyre2 = ihm.Citation(
    pmid='25950237',
    title='The Phyre2 web portal for protein modeling, prediction '
          'and analysis.',
    journal='Nat Protoc', volume=10, page_range=('845', '858'), year=2015,
    authors=['Kelley, L.A.', 'Mezulis, S.', 'Yates, C.M.', 'Wass, M.N.',
             'Sternberg, M.J.'],
    doi='10.1038/nprot.2015.053')


swiss_model = ihm.Citation(
    pmid='29788355',
    title='SWISS-MODEL: homology modelling of protein structures '
          'and complexes.',
    journal='Nucleic Acids Res', volume=46, page_range=('W296', 'W303'),
    year=2018,
    authors=['Waterhouse, A.', 'Bertoni, M.', 'Bienert, S.', 'Studer, G.',
             'Tauriello, G.', 'Gumienny, R.', 'Heer, F.T.', 'de Beer, T.A.P.',
             'Rempfer, C.', 'Bordoli, L.', 'Lepore, R.', 'Schwede, T.'],
    doi='10.1093/nar/gky427')


alphafold2 = ihm.Citation(
    pmid='34265844',
    title='Highly accurate protein structure prediction with AlphaFold.',
    journal='Nature', volume=596, page_range=(583, 589), year=2021,
    authors=['Jumper, J.', 'Evans, R.', 'Pritzel, A.', 'Green, T.',
             'Figurnov, M.', 'Ronneberger, O.', 'Tunyasuvunakool, K.',
             'Bates, R.', 'Zidek, A.', 'Potapenko, A.', 'Bridgland, A.',
             'Meyer, C.', 'Kohl, S.A.A.', 'Ballard, A.J.', 'Cowie, A.',
             'Romera-Paredes, B.', 'Nikolov, S.', 'Jain, R.', 'Adler, J.',
             'Back, T.', 'Petersen, S.', 'Reiman, D.', 'Clancy, E.',
             'Zielinski, M.', 'Steinegger, M.', 'Pacholska, M.',
             'Berghammer, T.', 'Bodenstein, S.', 'Silver, D.', 'Vinyals, O.',
             'Senior, A.W.', 'Kavukcuoglu, K.', 'Kohli, P.', 'Hassabis, D.'],
    doi='10.1038/s41586-021-03819-2')


colabfold = ihm.Citation(
    pmid='35637307',
    title='ColabFold: making protein folding accessible to all.',
    journal='Nature Methods', volume=19, page_range=(679, 682), year=2022,
    authors=['Mirdita, M.', 'Schuetze, K.', 'Moriwaki, Y.', 'Heo, L.',
             'Ovchinnikov, S.', 'Steinegger, M.'],
    doi='10.1038/s41592-022-01488-1')

qmeandisco = ihm.Citation(
    pmid='31697312',
    title='QMEANDisCo-distance constraints applied on model quality '
          'estimation.',
    journal='Bioinformatics',
    volume=36,
    page_range=(1765, 1771),
    year=2019,
    authors=['Studer, G.', 'Rempfer, C.', 'Waterhouse, A.M.', 'Gumienny, R.',
             'Haas, J.', 'Schwede, T.'],
    doi='10.1093/bioinformatics/btz828')

mmseqs2 = ihm.Citation(
    pmid='30615063',
    title='MMseqs2 desktop and local web server app for fast, interactive '
          'sequence searches.',
    journal='Bioinformatics',
    volume=35,
    page_range=(2856, 2858),
    year=2019,
    authors=['Mirdita, M.', 'Steinegger, M.', 'Soeding, J.'],
    doi='10.1093/bioinformatics/bty1057')
