/**
 *   Copyright 2007-2013 IMP Inventors. All rights reserved
 */
#include <IMP/kernel/Model.h>
#include <IMP/base/log.h>
#include <IMP/kernel/Particle.h>
#include <IMP/kernel/internal/pdb.h>
#include <sstream>

const char *data=
    "ATOM    331  C   GLU    41      30.658   1.906  20.427  \n" \
    "ATOM    332  O   GLU    41      31.209   0.890  20.838  \n" \
    "ATOM    333  CB  GLU    41      28.461   2.190  21.522  \n" \
    "ATOM    334  CG  GLU    41      27.648   2.725  22.678  \n" \
    "ATOM    335  CD  GLU    41      26.298   2.048  22.772  \n" \
    "ATOM    336  OE1 GLU    41      25.420   2.340  21.930  \n" \
    "ATOM    337  OE2 GLU    41      26.125   1.206  23.678  \n" \
    "ATOM    338  N   LYS    42      30.696   2.286  19.151  \n" \
    "ATOM    339  CA  LYS    42      31.421   1.522  18.136  \n" \
    "ATOM    340  C   LYS    42      32.934   1.611  18.273  \n" \
    "ATOM    341  O   LYS    42      33.661   0.750  17.787  \n" \
    "ATOM    342  CB  LYS    42      31.007   1.987  16.739  \n";


int main(int, char*[]) {
  IMP::base::set_log_level(IMP::base::MEMORY);
  IMP_NEW(IMP::kernel::Model, m, ());
  std::istringstream in(data);
  IMP::kernel::ParticlesTemp ps=
    IMP::kernel::internal::create_particles_from_pdb(in, m);
  IMP_USAGE_CHECK(std::abs(m->get_sphere(ps[3]->get_index()).get_center()[2]
                           -22.678)
                  < .002, "Bad coordinates: "
                  << m->get_sphere(ps[3]->get_index()));
  return 0;
}
