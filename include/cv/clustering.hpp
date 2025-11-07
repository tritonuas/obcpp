#ifndef INCLUDE_CV_CLUSTER_HPP_
#define INCLUDE_CV_CLUSTER_HPP_
#include <vector>
#include "protos/obc.pb.h"


class Clustering {
    
  public: std::vector<GPSCoord> FindClustersCenter(const std::vector<std::vector<GPSCoord>>& points);
};

#endif