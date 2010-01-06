/**
 * \file element.h \brief
 *
 * Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */
#ifndef IMPATOM_ELEMENT_H
#define IMPATOM_ELEMENT_H

#include "config.h"
#include <IMP/base_types.h>

#include <IMP/log.h>
#include <IMP/exception.h>

#include <boost/algorithm/string.hpp>

#include <string>
#include <map>

IMPATOM_BEGIN_NAMESPACE

//! The various elements currently supported/known?
enum Element {UNKNOWN_ELEMENT=0, OH=-1, H2O=-2,
              H= 1, He= 2, Li= 3, Be= 4, B= 5, C= 6, N= 7,
              O= 8, F= 9, Ne= 10, Na= 11, Mg= 12, Al= 13, Si= 14,
              P= 15, S= 16, Cl= 17, Ar= 18, K= 19, Ca= 20, Sc= 21,
              Ti= 22, V= 23, Cr= 24, Mn= 25, Fe= 26, Co= 27, Ni= 28,
              Cu= 29, Zn= 30, Ga= 31, Ge= 32, As= 33, Se= 34, Br= 35,
              Kr= 36, Rb= 37, Sr= 38, Y= 39, Zr= 40, Nb= 41, Mo= 42,
              Tc= 43, Ru= 44, Rh= 45, Pd= 46, Ag= 47, Cd= 48, In= 49,
              Sn= 50, Sb= 51, Te= 52, I= 53, Xe= 54, Cs= 55, Ba= 56,
              La= 57, Ce= 58, Pr= 59, Nd= 60, Pm= 61, Sm= 62, Eu= 63,
              Gd= 64, Tb= 65, Dy= 66, Ho= 67, Er= 68, Tm= 69, Yb= 70,
              Lu= 71, Hf= 72, Ta= 73, W= 74, Re= 75, Os= 76, Ir= 77,
              Pt= 78, Au= 79, Hg= 80, Tl= 81, Pb= 82, Bi= 83, Po= 84,
              At= 85, Rn= 86, Fr= 87, Ra= 88, Ac= 89, Th= 90, Pa= 91,
              U= 92, Np= 93, Pu= 94, Am= 95, Cm= 96, Bk= 97, Cf= 98,
              Es= 99, Fm= 100, Md= 101, No= 102, Lr= 103, Db= 104, Jl= 105,
              Rf= 106, NUMBER_OF_ELEMENTS = 107
};


class IMPATOMEXPORT ElementTable {
public:
  ElementTable();

  //! get element from the string name (PDB columns 77-78)
  Element get_element(const std::string& s) const;

  //! get string name for Element
  std::string get_name(Element e) const;

  //! get mass for Element
  Float get_mass(Element e) const { return mass_[e]; }

 private:
  struct ElementString {
    std::string name;
    Element e;
  };

  static ElementString element_strings_[];
  static Float mass_[];
  static std::map<std::string, Element> string_2_element_;
  static std::map<Element, std::string> element_2_string_;
};

IMPATOMEXPORT ElementTable &get_element_table();

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_ELEMENT_H */
