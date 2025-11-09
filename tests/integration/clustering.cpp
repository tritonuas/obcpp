#include "cv/clustering.hpp"
#include "protos/obc.pb.h"
#include <matplot/matplot.h>
#define NUM_CLUSTERS 2
#define CLUSTER_SIZE 3
#define POINT_DISTANCE 10
#define NUM_OUTLIERS 5
#define NUM_RUNS 5
//redeclaring method to not have fixed seed so the test is deterministic 
unsigned int seed = time(NULL);
double random(double min, double max) {
    return min + static_cast<double>(rand_r(&seed)) / RAND_MAX * (max - min);
}

GPSCoord randomWithin(const GPSCoord &bottom, const GPSCoord &top)
{
    GPSCoord out;
    out.set_latitude(random(bottom.latitude(), top.latitude()));

    out.set_longitude(random(bottom.longitude(), top.longitude()));

    out.set_altitude(random(bottom.altitude(), top.altitude()));
    return out;
}
GPSCoord randomAround(const GPSCoord &center, double dist)
{
    GPSCoord out;
    out.set_longitude(center.longitude() + random(-dist, dist));
    out.set_latitude(center.latitude() + random(-dist, dist));
    return out;
}
int main()
{
    for (int run = 0; run < NUM_RUNS; run++)
    {
        using namespace matplot;
        GPSCoord bottom;
        GPSCoord top;
        bottom.set_latitude(0);
        bottom.set_altitude(100);
        bottom.set_longitude(0);

        top.set_latitude(500);
        top.set_altitude(100);
        top.set_longitude(500);
        std::vector<GPSCoord> true_clusters;
        std::vector<std::vector<GPSCoord>> clusters;
        for (int i = 0; i < NUM_CLUSTERS; i++)
        {
            std::vector<GPSCoord> cluster;
            GPSCoord center = randomWithin(bottom, top);
            for (int j = 0; j < CLUSTER_SIZE; j++)
            {
                cluster.push_back(randomAround(center, POINT_DISTANCE));
            }
            for (int j = 0; j < NUM_OUTLIERS; j++)
            {
                cluster.push_back(randomAround(center, POINT_DISTANCE * 20));
            }
            clusters.push_back(cluster);
        }
        // output vectors
        std::vector<double> x;
        std::vector<double> y;

        std::vector<double> cluster_x;
        std::vector<double> cluster_y;
        std::vector<double> c;
        double current_c_val;
        Clustering cluster;
        std::vector<GPSCoord> centers = cluster.FindClustersCenter(clusters);
        for (int i = 0; i < clusters.size(); i++)
        {
            auto cluster = clusters.at(i);
            for (auto &point : cluster)
            {
                x.push_back(point.longitude());
                y.push_back(point.latitude());
                c.push_back(current_c_val);
            }
            cluster_x.push_back(centers.at(i).longitude());

            cluster_y.push_back(centers.at(i).latitude());
            current_c_val += 10.0 / NUM_CLUSTERS;
        }
        scatter(x, y, std::vector<double>(), c);
        hold(on);
        auto plot = scatter(cluster_x, cluster_y);
        plot->marker_style(line_spec::marker_style::diamond);
        std::ostringstream stringStream;
        stringStream << "run" << run << ".png";
        std::string copyOfStr = stringStream.str();
     //  std::filesystem::remove(copyOfStr);
        save(copyOfStr);
        hold(off);

    }
    std::cout << "Images outputted to build folder" << std::endl;
}