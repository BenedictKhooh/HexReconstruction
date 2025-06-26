#ifndef RECONSTRUCTION_ENGINE_H
#define RECONSTRUCTION_ENGINE_H

#include <vector>
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <limits>
#include <QVector3D>
#include <QDebug>
#include <QSet>

// --- Type Definitions ---
using Vector3 = QVector3D;

/**
 * @struct MeshPoint
 * @brief Represents a point in the mesh with a constraint on its connectivity.
 */
struct MeshPoint {
    Vector3 pos;
    int required_neighbors;
};

// Represents the connectivity graph.
using AdjacencyGraph = std::unordered_map<int, std::unordered_set<int>>;

// Represents a quadrilateral face.
using QuadFace = std::array<int, 4>;

// Represents a hexahedral cell.
using Hexahedron = std::array<int, 8>;


// --- Helper Functions ---

/**
 * @brief Checks if four points are coplanar within a given tolerance.
 */
inline bool arePointsCoplanar(const std::vector<MeshPoint>& points, const QuadFace& face, float tolerance = 1e-3f) {
    if (face[0] >= (int)points.size() || face[1] >= (int)points.size() ||
        face[2] >= (int)points.size() || face[3] >= (int)points.size()) return false;
    const Vector3& p0 = points[face[0]].pos;
    const Vector3& p1 = points[face[1]].pos;
    const Vector3& p2 = points[face[2]].pos;
    const Vector3& p3 = points[face[3]].pos;
    Vector3 v1 = p1 - p0;
    Vector3 v2 = p2 - p0;
    Vector3 v3 = p3 - p0;
    float volume = QVector3D::dotProduct(v1, QVector3D::crossProduct(v2, v3));
    return std::abs(volume) < tolerance;
}

namespace ReconstructionEngine {
    /**
     * @brief Step 1: Build the adjacency graph based on precise neighbor constraints.
     */
    inline AdjacencyGraph buildAdjacencyGraph(const std::vector<MeshPoint>& points) {
        AdjacencyGraph adjGraph;
        if (points.empty()) return adjGraph;

        for (size_t i = 0; i < points.size(); ++i) {
            std::vector<std::pair<float, int>> distances;
            for (size_t j = 0; j < points.size(); ++j) {
                if (i == j) continue;
                distances.push_back({points[i].pos.distanceToPoint(points[j].pos), (int)j});
            }
            std::sort(distances.begin(), distances.end());

            adjGraph[(int)i] = {};
            int k_neighbors = points[i].required_neighbors;
            for (int k = 0; k < k_neighbors && k < (int)distances.size(); ++k) {
                adjGraph[(int)i].insert(distances[k].second);
            }
        }
        return adjGraph;
    }

    /**
     * @brief Step 2: Identify all valid quadrilateral faces from the graph.
     */
    inline std::vector<QuadFace> findValidFaces(const std::vector<MeshPoint>& points, const AdjacencyGraph& adjGraph) {
        std::vector<QuadFace> validFaces;
        QSet<QVector<int>> uniqueFaces;

        for (int p0_idx = 0; p0_idx < (int)points.size(); ++p0_idx) {
            if (!adjGraph.count(p0_idx)) continue;

            std::vector<int> neighbors(adjGraph.at(p0_idx).begin(), adjGraph.at(p0_idx).end());

            for (size_t i = 0; i < neighbors.size(); ++i) {
                for (size_t j = i + 1; j < neighbors.size(); ++j) {
                    int p1_idx = neighbors[i];
                    int p3_idx = neighbors[j];

                    if (!adjGraph.count(p1_idx) || !adjGraph.count(p3_idx)) continue;
                    for (int p2_idx : adjGraph.at(p1_idx)) {
                        if (p2_idx != p0_idx && adjGraph.at(p3_idx).count(p2_idx)) {
                            QuadFace potentialFace = {p0_idx, p1_idx, p2_idx, p3_idx};

                            if (arePointsCoplanar(points, potentialFace)) {
                                float edge01_sq = (points[p0_idx].pos - points[p1_idx].pos).lengthSquared();
                                float edge12_sq = (points[p1_idx].pos - points[p2_idx].pos).lengthSquared();
                                float edge23_sq = (points[p2_idx].pos - points[p3_idx].pos).lengthSquared();
                                float edge30_sq = (points[p3_idx].pos - points[p0_idx].pos).lengthSquared();

                                float diag02_sq = (points[p0_idx].pos - points[p2_idx].pos).lengthSquared();
                                float diag13_sq = (points[p1_idx].pos - points[p3_idx].pos).lengthSquared();

                                float max_edge_sq = std::max({edge01_sq, edge12_sq, edge23_sq, edge30_sq});

                                if (diag02_sq > max_edge_sq * 1.01f && diag13_sq > max_edge_sq * 1.01f) {
                                    QVector<int> sortedFace = {p0_idx, p1_idx, p2_idx, p3_idx};
                                    std::sort(sortedFace.begin(), sortedFace.end());

                                    if (!uniqueFaces.contains(sortedFace)) {
                                        validFaces.push_back(potentialFace);
                                        uniqueFaces.insert(sortedFace);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        return validFaces;
    }

    /**
     * @brief Step 3: Build hexahedral cells from the list of valid faces using a robust face-pairing strategy.
     */
    inline std::vector<Hexahedron> buildHexahedra(const std::vector<QuadFace>& validFaces, const AdjacencyGraph& adjGraph) {
        std::vector<Hexahedron> candidateHexahedra;

        // Iterate through all possible pairs of faces to find opposite pairs.
        for (size_t i = 0; i < validFaces.size(); ++i) {
            for (size_t j = i + 1; j < validFaces.size(); ++j) {
                const auto& face1 = validFaces[i];
                const auto& face2 = validFaces[j];

                // --- Check 1: Faces must be disjoint (no shared vertices).
                QSet<int> face1_pts;
                for(int p : face1) face1_pts.insert(p);
                bool disjoint = true;
                for(int p : face2) {
                    if (face1_pts.contains(p)) {
                        disjoint = false;
                        break;
                    }
                }
                if (!disjoint) continue;

                // --- Check 2: There must be exactly 4 connecting edges between them.
                std::vector<std::pair<int, int>> connecting_edges;
                for (int p1 : face1) {
                    if (!adjGraph.count(p1)) continue;
                    for (int p2 : face2) {
                        if (adjGraph.at(p1).count(p2)) {
                            connecting_edges.push_back({p1, p2});
                        }
                    }
                }

                if (connecting_edges.size() == 4) {
                    // --- Check 3: Verify that each vertex is used exactly once in the connections.
                    QSet<int> f1_check, f2_check;
                    for(const auto& edge : connecting_edges) {
                        f1_check.insert(edge.first);
                        f2_check.insert(edge.second);
                    }

                    if (f1_check.size() == 4 && f2_check.size() == 4) {
                        // We found a valid hexahedron candidate.
                        Hexahedron hex;
                        for(int k=0; k<4; ++k) hex[k] = connecting_edges[k].first;
                        for(int k=0; k<4; ++k) hex[k+4] = connecting_edges[k].second;
                        candidateHexahedra.push_back(hex);
                    }
                }
            }
        }

        // Deduplicate the results.
        std::vector<Hexahedron> finalHexahedra;
        QSet<QVector<int>> uniqueHexes;
        for (const auto& hex : candidateHexahedra) {
            QVector<int> sortedHex(8);
            for(int k=0; k<8; ++k) sortedHex[k] = hex[k];
            std::sort(sortedHex.begin(), sortedHex.end());

            QSet<int> pointSet;
            for(int p_idx : sortedHex) pointSet.insert(p_idx);
            if(pointSet.size() != 8) continue;

            if (!uniqueHexes.contains(sortedHex)) {
                finalHexahedra.push_back(hex);
                uniqueHexes.insert(sortedHex);
            }
        }

        return finalHexahedra;
    }
} // namespace ReconstructionEngine

#endif // RECONSTRUCTION_ENGINE_H
