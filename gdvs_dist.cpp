// gdvs_dist.cpp
// Topological Similarity Calculator (from GRAAL)
// Reed Nelson

#include <array>
#include <cmath>
#include <vector>

namespace GDVs_Dist
{
    const double O[73] = {
    1, 2, 2, 2, 3, 4, 3, 3, 4, 3, 
    4, 4, 4, 4, 3, 4, 6, 5, 4, 5, 
    6, 6, 4, 4, 4, 5, 7, 4, 6, 6, 
    7, 4, 6, 6, 6, 5, 6, 7, 7, 5, 
    7, 6, 7, 6, 5, 5, 6, 8, 7, 6, 
    6, 8, 6, 9, 5, 6, 4, 6, 6, 7, 
    8, 6, 6, 8, 7, 6, 7, 7, 8, 5, 
    6, 6, 4};
    double alpha;

    /*
    * The weight of orbit i, accounting for dependencies between orbits.
    */
    double weight(unsigned i)
    {
        return 1 - log10(O[i]) / log10(73);
    }

    /*
    * The distance between the ith orbits of nodes v and u.
    */
    double distance(unsigned vi, unsigned ui, unsigned i)
    {

        //printf("vi: %i; ui: %i;\n", vi, ui); // debug

        double ret;
        ret = log10(vi + 1) - log10(ui + 1);
        ret = abs(ret);
        ret /= log10(std::max(vi, ui) + 2);
        ret *= weight(i);

        return ret;
    }

    /*
    * The signature similarity between nodes v and u. (1 - the distance between v and u).
    */
    double similarity(std::array<unsigned, 73> v, std::array<unsigned, 73> u)
    {
        double dist = 0;
        for (unsigned i = 0; i < 73; ++i)
        {
            dist += distance(v[i], u[i], i);
        }

        double wei = 0;
        for (unsigned i = 0; i < 73; ++i)
        {
            wei += weight(i);
        }

        return 1 - dist / wei;
    }

    /*
    * The cost of aligning nodes v and u.
    */
    double cost(std::array<unsigned, 73> v, std::array<unsigned, 73> u, unsigned g_max_deg, unsigned h_max_deg)
    {
        double node_degs;
        node_degs = (v[0] + u[0]) / (g_max_deg + h_max_deg);

        return 1 - ((1 - alpha) * node_degs + alpha * similarity(v, u)); // originally 2 - ...
    }

    /*
    * The maximum degree of all the nodes in the given graph.
    */
    unsigned max_deg(std::vector<std::array<unsigned, 73>> gdvs)
    {
        unsigned max = 0;
        for (unsigned i = 0; i < gdvs.size(); ++i)
        { 
            if (max < gdvs[i][0])
            {
                max = gdvs[i][0];
            }
        }

        return max;
    }

    /*
     * Calculate the topological similarity between the graphs at the given paths.
     */
    std::vector<std::vector<double>> gdvs_dist(
        std::vector<std::array<unsigned, 73>> g_gdvs, std::vector<std::array<unsigned, 73>> h_gdvs, double alpha)
    {
        GDVs_Dist::alpha = alpha;
        
        // Calculate the highest degree among all the nodes in G, H
        unsigned g_max_deg = max_deg(g_gdvs);
        unsigned h_max_deg = max_deg(h_gdvs); 

        // Initialize the cost matrix to the right dimensions
        std::vector<std::vector<double>> costs(g_gdvs.size(), std::vector<double>(h_gdvs.size())); 

        // Calculate the cost matrix between G and H
        for (unsigned i = 0; i < g_gdvs.size(); ++i)
        {
            for (unsigned j = 0; j < h_gdvs.size(); ++j)
            {
                costs[i][j] = cost(g_gdvs[i], h_gdvs[j], g_max_deg, h_max_deg);
            } 
        }

        return costs;
    }
}