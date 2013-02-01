/**
 * \file Element \brief
 *
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/element.h>
#include <boost/algorithm/string.hpp>

IMPATOM_BEGIN_NAMESPACE

ElementTable::ElementString ElementTable::element_strings_[] = {
  {"H", H},   {"HE", He}, {"LI", Li}, {"BE", Be}, {"B", B},
  {"C", C},   {"N", N},   {"O", O},   {"F", F},   {"NE", Ne},
  {"NA", Na}, {"MG", Mg}, {"AL", Al}, {"SI", Si}, {"P", P},
  {"S", S},   {"CL", Cl}, {"AR", Ar}, {"K", K},   {"CA", Ca},
  {"SC", Sc}, {"TI", Ti}, {"V", V},   {"CR", Cr}, {"MN", Mn},
  {"FE", Fe}, {"CO", Co}, {"NI", Ni}, {"CU", Cu}, {"ZN", Zn},
  {"GA", Ga}, {"GE", Ge}, {"AS", As}, {"SE", Se}, {"BR", Br},
  {"KR", Kr}, {"RB", Rb}, {"SR", Sr}, {"Y", Y},   {"ZR", Zr},
  {"NB", Nb}, {"MO", Mo}, {"TC", Tc}, {"RU", Ru}, {"RH", Rh},
  {"PD", Pd}, {"AG", Ag}, {"CD", Cd}, {"IN", In}, {"SN", Sn},
  {"SB", Sb}, {"TE", Te}, {"I", I},   {"XE", Xe}, {"CS", Cs},
  {"BA", Ba}, {"LA", La}, {"CE", Ce}, {"PR", Pr}, {"ND", Nd},
  {"PM", Pm}, {"SM", Sm}, {"EU", Eu}, {"GD", Gd}, {"TB", Tb},
  {"DY", Dy}, {"HO", Ho}, {"ER", Er}, {"TM", Tm}, {"YB", Yb},
  {"LU", Lu}, {"HF", Hf}, {"TA", Ta}, {"W", W},   {"RE", Re},
  {"OS", Os}, {"IR", Ir}, {"PT", Pt}, {"AU", Au}, {"HG", Hg},
  {"TL", Tl}, {"PB", Pb}, {"BI", Bi}, {"PO", Po}, {"AT", At},
  {"RN", Rn}, {"FR", Fr}, {"RA", Ra}, {"AC", Ac}, {"TH", Th},
  {"PA", Pa}, {"U", U},   {"NP", Np}, {"PU", Pu}, {"AM", Am},
  {"CM", Cm}, {"BK", Bk}, {"CF", Cf}, {"ES", Es}, {"FM", Fm},
  {"MD", Md}, {"NO", No}, {"LR", Lr}, {"Db", Db}, {"JL", Jl},
  {"RF", Rf},
  {"UNKNOWN_ELEMENT", UNKNOWN_ELEMENT}
};

IMP::base::map<std::string, Element> ElementTable::string_2_element_;
IMP::base::map<Element, std::string> ElementTable::element_2_string_;

// from http://physics.nist.gov/cgi-bin/Compositions/stand_alone.pl
Float ElementTable::mass_[] = {
  0.0, 1.007, 4.002, 6.941, 9.012, 10.811, 12.0107, 14.0067,
  //UNKNOWN_ELEMENT=0, H=1, He=2, Li=3, Be=4, B=5, C=6, N=7,
  15.9994, 18.998, 20.1797, 22.989, 24.3050, 26.981, 28.0855,
  //O=8, F=9, Ne=10, Na=11, Mg=12, Al=13, Si=14,
  30.973, 32.065, 35.453, 39.948, 39.0983, 40.078, 44.955,
  //P=15, S=16, Cl=17, Ar=18, K=19, Ca=20, Sc=21,
  47.867, 50.9415, 51.9961, 54.938, 55.845, 58.933, 58.6934,
  //Ti=22, V=23, Cr=24, Mn=25, Fe=26, Co=27, Ni=28,
  63.546, 65.409, 69.723, 72.64, 74.921, 78.96, 79.904,
  //Cu=29, Zn=30, Ga=31, Ge=32, As=33, Se=34, Br=35,
  83.798, 85.4678, 87.62, 88.905, 91.224, 92.906, 95.94,
  //Kr=36, Rb=37, Sr=38, Y=39, Zr=40, Nb=41, Mo=42,
  97.907, 101.07, 102.905, 106.42, 107.8682, 112.411, 114.818,
  //Tc=43, Ru=44, Rh=45, Pd=46, Ag=47, Cd=48, In=49,
  118.71, 121.76, 127.60, 126.90, 131.29, 132.91, 137.33,
  //Sn=50, Sb=51, Te=52, I=53, Xe=54, Cs=55, Ba=56,
  138.91, 140.12, 140.91, 144.24, 145, 150.36, 151.96,
  //La=57, Ce=58, Pr=59, Nd=60, Pm=61, Sm=62, Eu=63,
  157.25, 158.93, 162.50, 164.93, 167.26, 168.93, 173.04,
  //Gd=64, Tb=65, Dy=66, Ho=67, Er=68, Tm=69, Yb=70,
  174.97, 178.49, 180.95, 183.84, 186.21, 190.23, 192.22,
  //Lu=71, Hf=72, Ta=73, W=74, Re=75, Os=76, Ir=77,
  195.08, 196.97, 200.59, 204.38, 207.2, 208.98, 208.98,
  //Pt=78, Au=79, Hg=80, Tl=81, Pb=82, Bi=83, Po=84,
  209.99, 222.02, 223.0, 226.0, 227.0, 232.04, 231.04,
  //At=85, Rn=86, Fr=87, Ra=88, Ac=89, Th=90, Pa=91,
  238.03, 237.0, 244.0, 243.0, 247.0, 247.0, 251.0,
  //U=92, Np=93, Pu=94, Am=95, Cm=96, Bk=97, Cf=98,
  252.0,  257.0, 258.0, 259.0, 262.0, 262.0, 262.0, 262.0
  //Es=99,Fm=100,Md=101,No=102,Lr=103,Db=104,Jl=105,Rf=106
};

ElementTable::ElementTable() {
  for (unsigned int i=0; element_strings_[i].e != UNKNOWN_ELEMENT; ++i) {
    string_2_element_[element_strings_[i].name] = element_strings_[i].e;
    element_2_string_[element_strings_[i].e] = element_strings_[i].name;
  }
}

Element ElementTable::get_element(const std::string& s) const {
  IMP_IF_CHECK(USAGE) {
    std::string ts=s;
    boost::trim(ts);
    IMP_USAGE_CHECK(ts == s, "The string passed to get_element"
                  << " should not contain spaces.");
  }
  std::string copy_s = s;
  boost::to_upper(copy_s);
  if(string_2_element_.find(copy_s) == string_2_element_.end())
    return UNKNOWN_ELEMENT;
  return string_2_element_.find(copy_s)->second;
}

std::string ElementTable::get_name(Element e) const {
  if (e== UNKNOWN_ELEMENT) return "Un";
  return element_2_string_.find(e)->second;
}

ElementTable &get_element_table() {
  static ElementTable element_table;
  // TODO: add tests for table size
  return element_table;
}

IMPATOM_END_NAMESPACE
