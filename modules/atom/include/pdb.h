/**
 *  \file atom/pdb.h
 *  \brief Functions to read pdbs
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */
#ifndef IMPATOM_PDB_H
#define IMPATOM_PDB_H

#include "config.h"
#include "internal/pdb.h"
#include "selectors.h"
#include "Hierarchy.h"

#include <IMP/Model.h>
#include <IMP/Particle.h>

IMPATOM_BEGIN_NAMESPACE

/** @name PDB IO
*/
//!@{

/** Selector objects can be used to define which atoms to read.
    \see write_pdb
 */
IMPATOMEXPORT Hierarchy
read_pdb(std::istream &in,
         Model* model,
         const Selector& selector = Selector(),
         bool select_first_model = true,
         bool ignore_alternatives = true);

/** Selector objects can be used to define which atoms to read.
    \see write_pdb
 */
IMPATOMEXPORT Hierarchy
read_pdb(std::string pdb_file_name,
         Model* model,
         const Selector& selector = Selector(),
         bool select_first_model = true,
         bool ignore_alternatives = true);

/** Add bonds using definitions from topology file
    if topology file is not given, default file is used
*/
IMPATOMEXPORT void add_bonds(Hierarchy d,
                             std::string topology_file_name = std::string());

/** \note This function produces files that are not valid PDB files,
    i.e. only ATOM/HETATM lines are printed for all Atom particles
    in the hierarchy. Complain if your favorite program can't read them and
    we might fix it.
   \see read_pdb
   \see write_pdb(const Hierarchys& mhd,std::string file_name)
*/
IMPATOMEXPORT void write_pdb(Hierarchy mhd,
                             std::ostream &out);

/**
\copydetails write_pdb(Hierarchy mhd,std::ostream &out)
*/
IMPATOMEXPORT void write_pdb(Hierarchy mhd,
                             std::string file_name);

/**
\copydetails write_pdb(Hierarchy mhd,std::ostream &out)
*/
IMPATOMEXPORT void write_pdb(const Hierarchys &mhd,
                             std::ostream &out);

/**
\copydetails write_pdb(Hierarchy mhd,std::ostream &out)
*/
IMPATOMEXPORT void write_pdb(const Hierarchys &mhd,
                             std::string file_name);

/**
\copydetails write_pdb(Hierarchy mhd,std::ostream &out)
*/
IMPATOMEXPORT void write_pdb(const Particles& ps, std::ostream &out);

/**
\copydetails write_pdb(Hierarchy mhd,std::ostream &out)
*/
IMPATOMEXPORT void write_pdb(const Particles& ps, std::string file_name);


//!@}

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_PDB_H */
