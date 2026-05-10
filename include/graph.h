#pragma once
// NanoDB Graph Optimizer Model
// Represents join-cost graphs and provides MST-based join-order optimization.

namespace nanodb {

// Weighted edge for graph
struct Edge {
    int from;
    int to;
    double weight;
    
    Edge(int f = -1, int t = -1, double w = 0.0) : from(f), to(t), weight(w) {}
    
    bool operator<(const Edge& other) const {
        return weight < other.weight;
    }
    
    bool operator>(const Edge& other) const {
        return weight > other.weight;
    }
};

// Simple Graph for query optimizer (Minimal Spanning Tree)
class Graph {
private:
    int num_vertices_;
    Edge* edges_;
    int edge_count_;
    int max_edges_;
    
    // Disjoint set for Kruskal's algorithm
    int* parent_;
    int* rank_;
    
    int find(int x) {
        if (parent_[x] != x) {
            parent_[x] = find(parent_[x]); // path compression
        }
        return parent_[x];
    }
    
    void unite(int x, int y) {
        int px = find(x);
        int py = find(y);
        
        if (px == py) return;
        
        if (rank_[px] < rank_[py]) {
            parent_[px] = py;
        } else if (rank_[px] > rank_[py]) {
            parent_[py] = px;
        } else {
            parent_[py] = px;
            ++rank_[px];
        }
    }
    
public:
    Graph(int num_vertices) : num_vertices_(num_vertices), edge_count_(0) {
        max_edges_ = num_vertices * (num_vertices - 1) / 2 + 100;
        edges_ = new Edge[max_edges_];
        parent_ = new int[num_vertices_];
        rank_ = new int[num_vertices_];
        
        for (int i = 0; i < num_vertices_; ++i) {
            parent_[i] = i;
            rank_[i] = 0;
        }
    }
    
    ~Graph() {
        delete[] edges_;
        delete[] parent_;
        delete[] rank_;
    }
    
    void add_edge(int from, int to, double weight) {
        if (edge_count_ >= max_edges_) return;
        edges_[edge_count_++] = Edge(from, to, weight);
    }
    
    // Kruskal's algorithm: find Minimal Spanning Tree
    Edge* find_mst(int& mst_edge_count) {
        mst_edge_count = 0;
        
        // Sort edges by weight (simple bubble sort for small graphs)
        for (int i = 0; i < edge_count_ - 1; ++i) {
            for (int j = 0; j < edge_count_ - i - 1; ++j) {
                if (edges_[j].weight > edges_[j+1].weight) {
                    Edge tmp = edges_[j];
                    edges_[j] = edges_[j+1];
                    edges_[j+1] = tmp;
                }
            }
        }
        
        // Reset parent/rank
        for (int i = 0; i < num_vertices_; ++i) {
            parent_[i] = i;
            rank_[i] = 0;
        }
        
        Edge* mst = new Edge[num_vertices_];
        
        for (int i = 0; i < edge_count_ && mst_edge_count < num_vertices_ - 1; ++i) {
            int u = edges_[i].from;
            int v = edges_[i].to;
            
            if (find(u) != find(v)) {
                mst[mst_edge_count++] = edges_[i];
                unite(u, v);
            }
        }
        
        return mst;
    }
    
    int vertex_count() const { return num_vertices_; }
    int edge_count() const { return edge_count_; }
};

} // namespace nanodb
