#include <iostream>
#include <chrono>
#include <vector>
#include <deque>
#include <random>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/prim_minimum_spanning_tree.hpp>
#include <boost/graph/random.hpp>
using namespace std;
using namespace std::chrono;
using namespace boost;
#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wsign-compare"
#endif
#if defined(_MSC_VER)
#pragma warning(disable: 4388 4389)
#endif

// --------------------------------------------------------------------------------

typedef struct {
	std::string name;
	int dist; // distance
} VertProp;
typedef struct {
	int w; // weight for each edge
} EdgeProp;
typedef boost::adjacency_list<vecS, vecS, undirectedS, VertProp, EdgeProp> Graph;
typedef boost::graph_traits<Graph>::vertex_descriptor VertDesc;
typedef boost::graph_traits<Graph>::edge_descriptor EdgeDesc;
typedef boost::property_map<Graph, boost::vertex_index_t>::type VertIndMap;

void my_print(Graph& g, const char* name = nullptr) {
	cout << ((name == nullptr) ? "graph" : name) << ": ";
	cout << "num vert = " << boost::num_vertices(g);
	cout << ", num edge = " << boost::num_edges(g) << endl;
	std::pair<Graph::vertex_iterator, Graph::vertex_iterator> vp = boost::vertices(g); // all vertices
	int num_show = 16;
	for (Graph::vertex_iterator it = vp.first; it != vp.second && num_show > 0; ++it, --num_show) {
		auto adjp = boost::adjacent_vertices(*it, g); // all adjacent vertices
		int num = distance(adjp.first, adjp.second); // number of adjacent vertices
		cout << "\t" << g[*it].name << " --> [" << num << "] ";
		int num_show = 16;
		for (auto jt = adjp.first; jt != adjp.second && num_show > 0; ++jt, --num_show) {
			auto e = edge(*it, *jt, g); // it returns (edge_descriptor, bool)
			cout << g[*jt].name << "(" << g[e.first].w << ") ";
		}
		cout << ((num > 16) ? ". . ." : "") << endl;
	}
	if (boost::num_vertices(g) > 16) {
		cout << "\t. . ." << endl;
	}
}

struct Visitor : public boost::dijkstra_visitor<> {
	template <class Edge, class Graph>
	void edge_relaxed(const Edge& e, const Graph& g) {
		cout << "edge (" << boost::source(e, g) << "," << boost::target(e, g) << ") : relaxed" << endl;
	}
	template <class Edge, class Graph>
	void edge_not_relaxed(const Edge& e, const Graph& g) {
		cout << "edge (" << boost::source(e, g) << "," << boost::target(e, g) << ") : not relaxed" << endl;
	}
};

int main(int argc, char* argv[]) {
	// make a target graph
	enum { A, B, C, D, E, F, G, H, I, N }; // N is 9, number of vertices
	Graph g(N);
	g[A].name = "a";
	g[B].name = "b";
	g[C].name = "c";
	g[D].name = "d";
	g[E].name = "e";
	g[F].name = "f";
	g[G].name = "g";
	g[H].name = "h";
	g[I].name = "i";
	add_edge(A, B, EdgeProp { 4 }, g);
	add_edge(A, H, EdgeProp { 8 }, g);
	add_edge(B, C, EdgeProp { 8 }, g);
	add_edge(B, H, EdgeProp { 11 }, g);
	add_edge(C, D, EdgeProp { 7 }, g);
	add_edge(C, F, EdgeProp { 4 }, g);
	add_edge(C, I, EdgeProp { 2 }, g);
	add_edge(D, E, EdgeProp { 9 }, g);
	add_edge(D, F, EdgeProp { 14 }, g);
	add_edge(E, F, EdgeProp { 10 }, g);
	add_edge(F, G, EdgeProp { 2 }, g);
	add_edge(G, H, EdgeProp { 1 }, g);
	add_edge(G, I, EdgeProp { 6 }, g);
	my_print(g);
	// prim's algorithm
	cout << "Prim's MST algorithm:" << endl;
	std::vector<VertDesc> pred(num_vertices(g));
	Visitor vis;
	prim_minimum_spanning_tree(g, &pred[0],
	                           weight_map(get(&EdgeProp::w, g))
	                           .distance_map(get(&VertProp::dist, g))
	                           .root_vertex(*(vertices(g).first))
	                           .visitor(vis)
	                          );
	cout << "\t";
	int num_edge = 0;
	int sum_w = 0;
	for (int i = 0; i < num_vertices(g); ++i) {
		if (pred[i] != i) {
			cout << "(" << g[pred[i]].name << "," << g[i].name << ") ";
			sum_w += g[boost::edge(i, pred[i], g).first].w;
			++num_edge;
		}
	}
	cout << ": " << num_edge << " edges" << endl;
	cout << "total weight for MST = " << sum_w << endl;
	// extra information
	for (int i = 0; i < num_vertices(g); ++i) {
		cout << "vertex " << g[i].name << " : select weight " << g[i].dist << " edge to " << g[pred[i]].name << endl;
	}
	// done
	return 0;
}
