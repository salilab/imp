/**
 *  \file KMCentersTree.cpp   \brief A tree that handles point partition
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/statistics/internal/KMCentersTree.h>
#include <IMP/algebra/utility.h>
IMPSTATISTICS_BEGIN_INTERNAL_NAMESPACE


KMCentersTree::KMCentersTree( KMData *data_points,KMCenters *centers,
  KMPoint *bb_lo, KMPoint *bb_hi) : centers_(centers){
  data_points_ = data_points;
  Ints pid;
  skeleton_tree(pid,bb_lo, bb_hi);
  root_ = build_tree(0, data_points_->get_number_of_points()-1,0);
  IMP_LOG_VERBOSE("KMCentersTree const end build tree "<< std::endl);
  root_->compute_sums();
  IMP_LOG_VERBOSE("KMCentersTree const end compute sums "<< std::endl);
  //TODO - should we use the ignore stuff
  //  root_->compute_sums(ignoreMe1, ignoreMe2, ignoreMe3);
  //  IMP_INTERNAL_CHECK(ignoreMe1 == data_points_->get_number_of_points(),
  // "calculate sums should have included all of the points");
}
//TODO: check if we need to add anything
KMCentersTree::~KMCentersTree()
{
  if (root_ != nullptr) delete root_;
}


KMPoint KMCentersTree::sample_center()
{
  KMPoint p = root_->sample_center();
  //TODO - should we have this ?
//   for (int i = 0; i < dim_; i++) {
//     IMP_INTERNAL_CHECK( bb_save.lo[i] == bnd_box_.lo[i] &&
//     bb_save.hi[i] == bnd_box_.hi[i],
//     "the bounding box was changed during sample_center");
//  }
  return p;
}

void KMCentersTree::show(std::ostream &out) {
  //TODO - consider uncommenting
  //out << "    Points:\n";
  //print_points("Points:\n",*(data_points_->get_points()),out);
  if (root_ == nullptr)
  out << "    Null tree.\n";
 else {
   root_->show(out);
 }
}
void KMCentersTree::get_assignments(Ints &close_center)
{
  IMP_LOG_VERBOSE("KMCentersTree::get_assignments for "
  << centers_->get_number_of_centers() << " centers "<<std::endl);
  close_center.clear();
  Ints candidate_centers;
  for (int j = 0; j < centers_->get_number_of_centers(); j++) {
    candidate_centers.push_back(j);
  }
  close_center.clear();
  for(int i=0;i<data_points_->get_number_of_points();i++) {
    close_center.push_back(0);
  }
  root_->get_assignments(candidate_centers,close_center);
}

void KMCentersTree::skeleton_tree(const Ints &p_id,
  KMPoint *bb_lo,KMPoint *bb_hi) {
  //TODO: where do get n from ?
  IMP_INTERNAL_CHECK(data_points_ != nullptr,
                     "Points must be supplied to construct tree.");
  if (p_id.size() == 0) {
    for (int i = 0; i < data_points_->get_number_of_points(); i++)
      p_id_.push_back(i);
  }
  else {
    for (int i = 0; i < data_points_->get_number_of_points(); i++)
      p_id_.push_back(p_id[i]);
  }
  if (bb_lo == nullptr || bb_hi == nullptr) {
    bnd_box_ = bounding_rectangle(0,data_points_->get_number_of_points()-1);;
  }
  // if points are provided, use it
  if (bb_lo != nullptr) {
    copy_point(bb_lo,bnd_box_->get_point(0));
  }
  if (bb_hi != nullptr) {
    copy_point(bb_hi,bnd_box_->get_point(1));
  }
  root_ = nullptr;
}

KMCentersNode *KMCentersTree::build_tree(int start_ind,int end_ind,
  int level) {
  IMP_LOG_VERBOSE("build tree for point indexes: " <<
          start_ind << " to " << end_ind << std::endl);
  if (end_ind-start_ind<=1){
    Ints curr_inds;
    for(int i=start_ind;i<=end_ind;i++) {
      curr_inds.push_back(i);
    }
    return new KMCentersNodeLeaf(level,*bnd_box_,centers_,curr_inds);
  }
  int cd=0; //the cutting dimension
  double cv;//the cutting value
  int n_lo; // number on low side of cut
  KMCentersNode *lo, *hi; // low and high children
  //split the data points along a dimension. The split data is stored in pidx
  split_by_mid_point(start_ind, end_ind, cd, cv, n_lo);
  IMP_LOG_VERBOSE("splitting points with indexes : " << start_ind << " to "
   << end_ind << " the splitting dimension is: " << cd << " with value: "<< cv
   << " the last point for the left side is: " << n_lo << std::endl);
  KMPoint *lo_p,*hi_p;
  lo_p = bnd_box_->get_point(0);
  hi_p = bnd_box_->get_point(1);
  double lv = (*lo_p)[cd];
  double hv = (*hi_p)[cd];
  (*hi_p)[cd] = cv;
  //build left subtree from p_id[0,...,n_lo-1]
  lo = build_tree(start_ind,n_lo-1,level+1);
  (*hi_p)[cd] = hv;// restore bounds
  (*lo_p)[cd] = cv;// modify bounds for right subtree
  // build right subtree from p_id[n_lo..n-1]
  hi = build_tree(n_lo, end_ind,level+1);
  (*lo_p)[cd] = lv;
  // create the splitting node
  KMCentersNodeSplit *ptr =
    new KMCentersNodeSplit(level,*bnd_box_, centers_,cd, cv, lv, hv, lo, hi);
  return ptr;
}
void KMCentersTree::get_neighbors(KMPointArray *sums,
   Floats *sum_sqs,Ints *weights) {
  Ints cand_ind;
  IMP_LOG_VERBOSE("KMCentersTree::get_neighbors start number of centers: "
         << centers_->get_number_of_centers() << "\n");
  for (int j = 0; j < centers_->get_number_of_centers(); j++) {
    cand_ind.push_back(j);
  }
  root_->get_neighbors(cand_ind,sums,sum_sqs,weights);
  IMP_LOG_VERBOSE("KMCentersTree::get_neighbors end\n");
}
std::pair<int,int> KMCentersTree::split_by_plane(
  int start_ind, int end_ind, int dim, double cv) {
  int l = start_ind;
  int r = end_ind;
  //switch indexes of p_id_ such that
  //data_points_[start_ind..x-1] < cv <= data_points_[x..end_ind]
  for(;;) {
    while (l <= end_ind && get_value(l,dim) < cv) l++;
    while (r >= start_ind && get_value(r,dim) >= cv) r--;
    if (l > r) break;
    //swap points
    int tmp = p_id_[l];
    p_id_[l] = p_id_[r];
    p_id_[r] = tmp;
    l++; r--;
  }
  std::pair<int,int> break_ind;
  break_ind.first = l;
  r = end_ind;
  // partition data_points_[x..end_ind] about cv
  for(;;) {
    while (l <end_ind && get_value(l,dim) <= cv) l++;
    while (r >= break_ind.first &&  get_value(r,dim) > cv) r--;
    if (l > r) break;
    int tmp = p_id_[l];
    p_id_[l] = p_id_[r];
    p_id_[r] = tmp;
    l++; r--;
  }
  break_ind.second = l;
  return break_ind;
}

void KMCentersTree::split_by_mid_point(
  int start_ind, int end_ind, int &cut_dim, double &cut_val, int &n_lo) {
  KMPoint *lo,*hi;
  lo = bnd_box_->get_point(0);
  hi = bnd_box_->get_point(1);
  // find the long side with the largest spread (the cutting dimension)
  double max_length = bnd_box_->max_length();
  double max_spread = -1;
  for (int d = 0; d < data_points_->get_dim(); d++) {
    if (std::abs((*hi)[d] - (*lo)[d]-max_length) <1E-6){
      double spr = spread(start_ind,end_ind,d);
      if (spr > max_spread) {
        max_spread = spr;
        cut_dim = d;
      }
    }
  }
  // find the splitting value
  double ideal_cut_val = ((*lo)[cut_dim] + (*hi)[cut_dim])/2;
  //min_max represent the minimal and maximal
  //values of points along the cutting dimension
  std::pair<double,double> min_max =
    limits_along_dimension(start_ind,end_ind, cut_dim);
  //slide to min or max as needed
  if (ideal_cut_val < min_max.first)
    cut_val = min_max.first;
  else if (ideal_cut_val > min_max.second)
    cut_val = min_max.second;
  else
    cut_val = ideal_cut_val;
  // permute points accordingly
  std::pair<int,int>
    break_ind = split_by_plane(start_ind,end_ind,cut_dim, cut_val);
  IMP_LOG_VERBOSE( "split by mid point for indexes: "
          << start_ind << " to " << end_ind << "break index: "
          << break_ind.first << " to " << break_ind.second << std::endl);
  //set n_lo such that each side of the split will contain at least one point
  n_lo = (start_ind+end_ind)/2;
  // if ideal_cut_val < min (y >= 1), we set n_lo = 1 (so there is one
  // point on left)
  if (ideal_cut_val < min_max.first) n_lo = start_ind+1;
  // if ideal_cut_val > max (x <= n-1), we set n_lo = n-1 (so there is one
  // point on right).
  else if (ideal_cut_val > min_max.second) n_lo = end_ind;
  // Otherwise, we select n_lo as close to the middle of  [x..y] as possbile
  else if (break_ind.first > n_lo) n_lo = break_ind.first;
  else if (break_ind.second < n_lo) n_lo = break_ind.second;
}
double KMCentersTree::get_value(int p_id, int dim) const
{
  return (*((*data_points_)[p_id_[p_id]]))[dim];
}


KMRectangle* KMCentersTree::bounding_rectangle(int start_ind,int end_ind) {
  KMPoint l,h;
  for (int d = 0; d < data_points_->get_dim(); d++) {
    l.push_back(get_value(start_ind,d));
    h.push_back(get_value(start_ind,d));
  }
  for (int d = 0; d < data_points_->get_dim(); d++) {
    for (int i = start_ind+1; i <= end_ind; i++) {
      if (get_value(i,d) < l[d]) l[d] = get_value(i,d);
      else if (get_value(i,d) > h[d]) h[d] = get_value(i,d);
    }
  }
  return new KMRectangle(l,h);
}

double KMCentersTree::spread(int start_ind, int end_ind,int dim)
{
  std::pair<double,double> l= limits_along_dimension(start_ind,end_ind,dim);
  return (l.second - l.first);
}
std::pair<double,double>
KMCentersTree::limits_along_dimension(int start_ind, int end_ind, int dim) {
  double min,max;
  min = get_value(start_ind,dim);
  max = get_value(start_ind,dim);
  for (int i = start_ind+1; i <= end_ind; i++) {
    double c = get_value(i,dim);
    if (c < min) min = c;
    else if (c > max) max = c;
  }
  return std::pair<double,double>(min,max);
}
IMPSTATISTICS_END_INTERNAL_NAMESPACE
