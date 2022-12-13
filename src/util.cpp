#include <algorithm>
#include <ctime>
#include <iostream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <vector>

#include "file_io.h"

namespace Util
{
    /**
     * Gets the current date and time, formatted as a string.
     * 
     * @return the current date and time, formatted as a string.
     * 
     * @throws 
     */
    std::string now()
    {
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);

        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y_%m_%d-%H_%M_%S");
        std::string str = oss.str();

        return str;
    }

    /**
     * Returns the given double as a string, with the given number of decimal places.
     * 
     * @param d The double to convert to a string.
     * @param n Te number of decimal places to include in the string.
     * 
     * @return The given double as a string, with the given number of decimal places.
     */
    std::string to_string(double d, int n)
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(n) << d;
        return oss.str();
    }

    /**
     * Parse command line arguments.
     * args[0]: argv[0]
     * args[1]: graph G file
     * args[2]: graph H file
     * args[3]: biological data file
     * args[4]: GDV - edge weight balancer
     * args[5]: topological - biological balancer
     * args[6]: alignment cost threshold
     * args[7]: merged (whether to create the merged graph)
     * 
     * @param argc The number of command line arguments.
     * @param argv The command line arguments.
     * 
     * @return A list with values for all command line arguments, in a certain order.
     * 
     * @throws std::invalid_argument if an argument is misformatted.
     */
    std::vector<std::string> parse_args(int argc, char* argv[])
    {
        std::vector<std::string> args = {"", "", "", "", "1", "1", "0", "0"};

        if (argc < 3 || argc > 8)
        {
            throw std::invalid_argument("Invalid number of arguments.\nUsage: ./minaaa.exe <G.csv> <H.csv> [-B=bio_costs.csv] [-a=alpha] [-b=beta] [-g=gamma]");
        }

        if (!FileIO::is_accessible(argv[1]))
        {
            throw std::invalid_argument("The first file specified cannot be read.");
        }
        if (!FileIO::is_accessible(argv[2]))
        {
            throw std::invalid_argument("The second file specified cannot be read.");
        }

        args[0] = argv[0];
        args[1] = argv[1];
        args[2] = argv[2];

        for (auto i = 3; i < argc; ++i)
        {
            std::string arg = std::string(argv[i]);
            if (arg.find("-B=") != std::string::npos)
            {
                args[3] = arg.substr(3);
                if (!FileIO::is_accessible(args[3]))
                {
                    throw std::invalid_argument("The biological data file cannot be read.");
                }
            }
            else if (arg.find("-a=") != std::string::npos)
            {
                args[4] = arg.substr(3);

                if (std::stod(args[4]) < 0 || std::stod(args[4]) > 1)
                {
                    throw std::invalid_argument("The alpha argument must be in range [0, 1].");
                }
            }
            else if (arg.find("-b=") != std::string::npos)
            {
                args[5] = arg.substr(3);

                if (std::stod(args[5]) < 0 || std::stod(args[5]) > 1)
                {
                    throw std::invalid_argument("The beta argument must be in range [0, 1].");
                }
            }
            else if (arg.find("-g=") != std::string::npos)
            {
                args[6] = arg.substr(3);
                // We check the validity of the gamma string later.
            }
            else if (arg.find("-merge") != std::string::npos)
            {
                args[7] = "1";
            }
            else
            {
                throw std::invalid_argument("Invalid argument: " + arg);
            }
        }

        return args;
    }

    /**
     * Parse a comma-separated string to a vector of doubles.
     * 
     * @param gamma The comma-separated string.
     * 
     * @returns A vector of doubles.
     * 
     * @throws std::invalid_argument if the string is not comma-separated or the values are out of range.
     */
    std::vector<double> parse_gammas(std::string gamma_str)
    {
        std::vector<double> gamma;
        std::stringstream ss(gamma_str);

        while(ss.good())
        {
            std::string substr;
            getline(ss, substr, ',');
            try
            {
                double gi = std::stod(substr);
                if (gi < 0 || gi > 1)
                {
                    throw std::out_of_range("Each gamma value must be in range [0, 1].");
                }
                gamma.push_back(gi);
            }
            catch (const std::invalid_argument& e)
            {
                throw std::invalid_argument("Could not parse the gamma string.");
            }
            catch (const std::out_of_range& e)
            {
                throw std::invalid_argument("Each gamma value must be in range [0, 1].");
            }  
        }

        if (gamma.size() > 10)
        {
            throw std::invalid_argument("It is not permitted to calculate more than 10 alignments in batch");
        }

        return gamma;
    }

    /**
     * Normalize the entries of the given matrix to be in range [0, 1].
     * 
     * @param matrix The matrix to normalize.
     * 
     * @return The normalized matrix.
     * 
     * @throws 
     */
    std::vector<std::vector<double>> normalize(std::vector<std::vector<double>> matrix)
    {
        // Find the max and min values in the matrix
        double min = std::numeric_limits<double>::max();
        double max = std::numeric_limits<double>::min();
        for (unsigned i = 0; i < matrix.size(); ++i)
        {
            std::vector<double> row;
            for (unsigned j = 0; j < matrix[i].size(); ++j)
            {
                if (matrix[i][j] < min)
                {
                    min = matrix[i][j];
                }
                if (matrix[i][j] > max)
                {
                    max = matrix[i][j];
                }
            }
        }

        // Make nonnegative
        if (min < 0)
        {
            // Shift all values up by the min value
            for (unsigned i = 0; i < matrix.size(); ++i)
            {
                for (unsigned j = 0; j < matrix[i].size(); ++j)
                {
                    matrix[i][j] += std::abs(min);
                }
            }
            max += std::abs(min);
            min = 0;
        }

        // Normalize
        std::vector<std::vector<double>> norm_matrix;
        for (unsigned i = 0; i < matrix.size(); ++i)
        {
            std::vector<double> row;
            for (unsigned j = 0; j < matrix[i].size(); ++j)
            {
                row.push_back(matrix[i][j] / max);
            }
            norm_matrix.push_back(row);
        }

        return norm_matrix;
    }

    /**
     * Combine the topological and biological cost matrices.
     * 
     * @param topological_costs The topological cost matrix.
     * @param biological_costs The biological cost matrix.
     * @param beta of the weight goes to topological similarity, (1 - beta) goes to biological similarity.
     * 
     * @return The combined cost matrix.
     * 
     * @throws 
     */
    std::vector<std::vector<double>> combine(
        std::vector<std::vector<double>> topological_costs, std::vector<std::vector<double>> biological_costs, double beta) {
        // Handle absent biological costs
        if (biological_costs.empty()) {
            return topological_costs;
        }
        // Handle invalid beta
        if (beta < 0 || beta > 1) {
            std::cerr << "Beta must be between 0 and 1. Defaulting to beta = 1." << std::endl;
            return topological_costs;
        }
        
        std::vector<std::vector<double>> overall_costs;

        for (unsigned i = 0; i < topological_costs.size(); ++i)
        {
            std::vector<double> row;
            for (unsigned j = 0; j < topological_costs[i].size(); ++j)
            {
                row.push_back(beta * topological_costs[i][j] + (1 - beta) * biological_costs[i][j]);
            }
            overall_costs.push_back(row);
        }

        return overall_costs;
    }
    
    /**
     * Bridge graphs G and H, with respect to the alignment.
     * Returns simply:
     * [    G    ][A(G, H)]
     * [A(G, H)^T][   H   ] 
     * 
     * @param g_graph The graph G.
     * @param h_graph The graph H.
     * @param alignment The alignment between G and H.
     * @param gamma Gamma.
     * 
     * @return A new graph containing G and H, and the edges between them, according to the alignment.
     * 
     * @throws 
     */
    std::vector<std::vector<double>> bridge(
        std::vector<std::vector<unsigned>> g_graph,
        std::vector<std::vector<unsigned>> h_graph,
        std::vector<std::vector<double>> alignment,
        double gamma)
    {
        // Initialize bridged with 0s
        std::vector<std::vector<double>> bridged(g_graph.size() + h_graph.size(), std::vector<double>(g_graph.size() + h_graph.size(), 0));

        // Iterate through the first G rows
        for (unsigned i = 0; i < g_graph.size(); ++i)
        {
            // Iterate through the first G columns
            for (unsigned j = 0; j < g_graph.size(); ++j)
            {
                if (g_graph[i][j] > 0 && i != j) // ignoring self loops
                {
                    bridged[i][j] = 1;
                }
            }
            // Iterate through the last H columns
            for (unsigned j = 0; j < h_graph.size(); ++j)
            {
                if (alignment[i][j] > 0 && alignment[i][j] >= gamma)
                {
                    bridged[i][g_graph.size() + j] = 1;
                }
            }
        }
        
        // Iterate through the last H rows
        for (unsigned i = 0; i < h_graph.size(); ++i)
        {
            // Iterate through the first G columns
            for (unsigned j = 0; j < g_graph.size(); ++j)
            {
                if (alignment[j][i] > 0 && alignment[j][i] >= gamma)
                {
                    bridged[g_graph.size() + i][j] = 1;
                }
            }
            // Iterate through the last H columns
            for (unsigned j = 0; j < h_graph.size(); ++j)
            {
                if (h_graph[i][j] > 0 && i != j) // ignoring self loops
                {
                    bridged[g_graph.size() + i][g_graph.size() + j] = 1;
                }
            }
        }

        return bridged;
    }

    /**
     * Map the labels of the will-be merged matrix to the indices of it.
     * 
     * @param alignment The alignment between G and H.
     * @param g_labels The labels of G.
     * @param h_labels The labels of H.
     * @param gamma Gamma.
     * 
     * @return The lables of the merged matrix.
     * 
     * @throws 
     */
    std::vector<std::string> merge_labels(
        std::vector<std::vector<double>> alignment,
        std::vector<std::string> g_labels,
        std::vector<std::string> h_labels,
        double gamma)
    {
        std::vector<std::string> merged_labels;
        bool aligned = false;

        // For each gi in G, label index i in merged with gi, or gi + hj if gi is aligned with hj
        for (unsigned i = 0; i < alignment.size(); ++i)
        {
            aligned = false;
            for (unsigned j = 0; j < alignment[0].size(); ++j)
            {
                if (alignment[i][j] > 0 && alignment[i][j] >= gamma)
                {
                    merged_labels.push_back(g_labels[i] + h_labels[j]);
                    aligned = true;
                    break;
                }
            }
            if (!aligned)
            {
                merged_labels.push_back(g_labels[i]);
            }
        }

        // For each hj in H that isn't aligned with gi, label the next index with hj
        for (unsigned j = 0; j < alignment[0].size(); ++j)
        {
            aligned = false;
            for (unsigned i = 0; i < alignment.size(); ++i)
            {
                if (alignment[i][j] > 0 && alignment[i][j] >= gamma)
                {
                    aligned = true;
                    break;
                }
            }
            if (!aligned)
            {
                merged_labels.push_back(h_labels[j]);
            }
        }

        return merged_labels;
    }

    /**
     * Assigns the entry at index (label1, label2) the merged matrix the given value.
     * 
     * @param merged The merged matrix.
     * @param merged_labels The labels of the merged matrix.
     * @param label1 The first label.
     * @param label2 The second label.
     * @param value The value to assign.
     * 
     * @return The merged matrix with the new value.
     * 
     * @throws 
     */
    std::vector<std::vector<double>> assign(
        std::vector<std::vector<double>> merged,
        std::vector<std::string> merged_labels,
        std::string label1,
        std::string label2,
        unsigned value)
    {
        int i = std::distance(std::begin(merged_labels), std::find(std::begin(merged_labels), std::end(merged_labels), label1));
        int j = std::distance(std::begin(merged_labels), std::find(std::begin(merged_labels), std::end(merged_labels), label2));
        // If the label is not found, do something

        merged[i][j] = value;
        merged[j][i] = value;

        return merged;
    }

    /**
     * Merge graph H onto graph G, with respect to the alignment.
     * 
     * @param g_graph The graph G.
     * @param h_graph The graph H.
     * @param alignment The alignment between G and H.
     * @param g_labels The labels of G.
     * @param h_labels The labels of H.
     * @param merged_labels The labels of the merged matrix.
     * @param gamma Gamma.
     * 
     * @return The merged matrix.
     * 
     * @throws 
     */
    std::vector<std::vector<double>> merge(
        std::vector<std::vector<unsigned>> g_graph,
        std::vector<std::vector<unsigned>> h_graph,
        std::vector<std::vector<double>> alignment,
        std::vector<std::string> g_labels,
        std::vector<std::string> h_labels,
        std::vector<std::string> merged_labels,
        double gamma)
    {
        // merged_i,j = 0 iff there is no edge between nodes i and j
        // merged_i,j = 1 iff only G draws an edge between nodes i and j
        // merged_i,j = 2 iff only H draws an edge between nodes i and j
        // merged_i,j = 3 iff both G and H draw an edge between nodes i and j

        // Initialize merged with 0s
        std::vector<std::vector<double>> merged(merged_labels.size(), std::vector<double>(merged_labels.size(), 0));

        // Iterate through all nodes gi in G, aligned and unaligned
        for (unsigned gi = 0; gi < g_graph.size(); ++gi)
        {   
            unsigned hj = 0;
            for (; hj < alignment[0].size(); ++hj)
            {
                if (alignment[gi][hj] > 0 && alignment[gi][hj] >= gamma) // gi and hj are aligned
                {
                    // Iterate through all nodes gk adjacent to gi
                    for (unsigned gk = 0; gk < g_graph[0].size(); ++gk)
                    {
                        if (g_graph[gi][gk] > 0 && gi != gk) // gi and gk are adjacent, ignoring self loops
                        {
                            unsigned hl = 0;
                            for (; hl < alignment[0].size(); ++hl)
                            {
                                if (alignment[gk][hl] > 0 && alignment[gk][hl] >= gamma)
                                {
                                    break; // this gk is aligned with only this hl
                                }
                            }
                            if (hl < alignment[0].size()) // gk and hl are aligned
                            {
                                auto label1 = g_labels[gi] + h_labels[hj];
                                auto label2 = g_labels[gk] + h_labels[hl];
                                if (h_graph[hj][hl] > 0) // hj is adjacent to hl
                                {
                                    merged = assign(merged, merged_labels, label1, label2, 3);
                                }
                                else // hj is not adjacent to hl
                                {
                                    merged = assign(merged, merged_labels, label1, label2, 1);
                                }
                            }
                            else // gk is unaligned
                            {
                                auto label1 = g_labels[gi] + h_labels[hj];
                                auto label2 = g_labels[gk];
                                merged = assign(merged, merged_labels, label1, label2, 1);
                            }
                        }
                    }
                    // Iterate through all nodes hk adjacent to hj
                    for (unsigned hk = 0; hk < h_graph[0].size(); ++hk)
                    {
                        if (h_graph[hj][hk] > 0 && hj != hk) // hj and hk are adjacent, ignopring self loops
                        {
                            unsigned gl = 0;
                            for (; gl < alignment.size(); ++gl)
                            {
                                if (alignment[gl][hk] > 0 && alignment[gl][hk] >= gamma)
                                {
                                    break; // this gl is aligned with only this hk
                                }
                            }
                            if (gl < alignment.size()) // hk and gl are aligned
                            {
                                auto label1 = g_labels[gi] + h_labels[hj];
                                auto label2 = g_labels[gl] + h_labels[hk];
                                if (g_graph[gi][gl] > 0) // gi is adjacent to gl 
                                {
                                    // merged = assign(merged, merged_labels, label1, label2, 3);
                                    continue; // we already recorded this merge
                                }
                                else // gi is not adjacent to gl
                                {
                                    merged = assign(merged, merged_labels, label1, label2, 2);
                                }
                            }
                            else // hk is unaligned
                            {
                                auto label1 = g_labels[gi] + h_labels[hj];
                                auto label2 = h_labels[hk];
                                merged = assign(merged, merged_labels, label1, label2, 2);
                            }
                        }
                    }
                    break; // this gi is aligned with only this hj
                }
            }
            if (hj == alignment[0].size()) // gi was aligned to no hj
            {
                // Iterate through all nodes gj adjacent to gi
                for (unsigned gj = 0; gj < g_graph[0].size(); ++gj)
                {
                    if (g_graph[gi][gj] > 0  && gi != gj) // gi and gj are adjacent, ignoring self loops
                    {
                        unsigned hk = 0;
                        for (; hk < alignment[0].size(); ++hk)
                        {
                            if (alignment[gj][hk] > 0 && alignment[gj][hk] >= gamma)
                            {
                                break; // this gj is aligned with only this hk
                            }
                        }
                        if (hk < alignment[0].size()) // gj and hk are aligned
                        {
                            auto label1 = g_labels[gi];
                            auto label2 = g_labels[gj] + h_labels[hk];
                            merged = assign(merged, merged_labels, label1, label2, 1);
                        }
                        else // gj is unaligned
                        {
                            auto label1 = g_labels[gi];
                            auto label2 = g_labels[gj];
                            merged = assign(merged, merged_labels, label1, label2, 1);
                        }
                    }
                }
            }
        }

        // Iterate through the nodes hi in H that are not aligned to any node in G
        for (unsigned hi = 0; hi < h_graph.size(); ++hi)
        {   
            unsigned gj = 0;
            for (; gj < alignment.size(); ++gj)
            {
                if (alignment[gj][hi] > 0 && alignment[gj][hi] >= gamma) // gj and hi are aligned
                {
                    break; // skip any aligned hi
                }
            }
            if (gj == alignment.size()) // hi was not aligned to any gj
            {
                // Iterate through all nodes hk adjacent to hi
                for (unsigned hk = 0; hk < h_graph[0].size(); ++hk)
                {
                    if (h_graph[hi][hk] > 0 && hi != hk) // hi and hk are adjacent, ignoring self loops
                    {
                        unsigned gl = 0;
                        for (; gl < alignment.size(); ++gl)
                        {
                            if (alignment[gl][hk] > 0 && alignment[gl][hk] >= gamma)
                            {
                                break; // this hk is aligned with only this gl
                            }
                        }
                        if (gl < alignment.size()) // hk and gl are aligned
                        {
                            auto label1 = h_labels[hi];
                            auto label2 = g_labels[gl] + h_labels[hk];
                            merged = assign(merged, merged_labels, label1, label2, 2);
                        }
                        else // hk is unaligned
                        {
                            auto label1 = h_labels[hi];
                            auto label2 = h_labels[hk];
                            merged = assign(merged, merged_labels, label1, label2, 2);
                        }
                    }
                }
            }
        }

        return merged;
    }
}