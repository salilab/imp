.. highlight:: rest

.. _cross_linkers_module:

The :mod:`ihm.cross_linkers` Python module
==========================================

.. automodule:: ihm.cross_linkers

.. data:: dss

   DSS cross-linker that links a primary amine with another primary amine
   (non-water-soluble).

.. data:: dsg

   DSG cross-linker that links a primary amine with another primary amine
   (non-water-soluble).

.. data:: bs3

   BS3 cross-linker that links a primary amine with another primary amine
   (water-soluble).

.. data:: dsso

   DSSO cross-linker that links a primary amine with another primary amine
   (non-water-soluble). It is similar to DSS but can be cleaved in the gas
   phase using collision-induced dissociation.

.. data:: edc

   EDC cross-linker that links a carboxyl group with a primary amine.

.. data:: dhso

   DHSO (dihydrazide sulfoxide) MS-cleavable cross-linker that links
   carboxyl groups, described in
   `Gutierrez et al, 2016 <https://dx.doi.org/10.1021/acs.analchem.6b02240>`_.

.. data:: bmso

   BMSO (bismaleimide sulfoxide) MS-cleavable cross-linker that links
   cysteines, described in
   `Gutierrez et al, 2018 <https://dx.doi.org/10.1021/acs.analchem.8b01287>`_.

.. data:: sda

   SDA (NHS-Diazirine) (succinimidyl 4,4′-azipentanoate) cross-linker that
   links primary amines with nearly any other functional group via
   long-wave UV-light activation.

.. data:: photo_leucine

   L-photo-leucine. Non-canonical amino acid incorporated at leucine
   positions that links leucine to any other functional group via long-wave
   UV-light activation.
   See `Suchanek et al, 2005 <https://doi.org/10.1038/nmeth752>`_.

.. data:: dsbu

   dsbu (disuccinimidyl dibutyric urea) cross-linker that links a primary
   amine with another primary amine (non-water-soluble).
   Cleavable in the gas phase using collision-induced dissociation.
   See `Müller et al, 2011 <https://doi.org/10.1021/ac101241t>`_.

.. data:: phoX

   PhoX cross-linker that links a primary amine with another primary amine.
   The spacer group contains a phosphonate group, making the cross-linker
   IMAC-enrichable. Also known by the name DSPP. See
   `Steigenberger et al, 2019 <https://doi.org/10.1021/acscentsci.9b00416>`_.

.. data:: tbuphoX

   Tert-butyl PhoX cross-linker. Similar to PhoX, but containing a tert-butyl
   group that renders the cross-linker cell permeable.
   See `Jiang et al, 2021 <https://doi.org/10.1002/anie.202113937>`_.
