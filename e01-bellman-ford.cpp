#include <iostream>
#include <chrono>
#include <vector>
#include <deque>
#include <random>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/bellman_ford_shortest_paths.hpp>
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
	int pred; // predecessor index
} VertProp;
typedef struct {
	int w; // weight for each edge
} EdgeProp;
typedef boost::adjacency_list<vecS, vecS, bidirectionalS, VertProp, EdgeProp> Graph;
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

struct Visitor : public boost::bellman_visitor<> {
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
	enum { S, T, X, Y, Z, N }; // N is 6, number of vertices
	Graph g(N);
	g[S].name = "s";
	g[T].name = "t";
	g[X].name = "x";
	g[Y].name = "y";
	g[Z].name = "z";
	add_edge(S, T, EdgeProp { 6 }, g);
	add_edge(S, Y, EdgeProp { 7 }, g);
	add_edge(T, X, EdgeProp { 5 }, g);
	add_edge(T, Y, EdgeProp { 8 }, g);
	add_edge(T, Z, EdgeProp { -4 }, g);
	add_edge(X, T, EdgeProp { -2 }, g);
	add_edge(Y, X, EdgeProp { -3 }, g);
	add_edge(Y, Z, EdgeProp { 9 }, g);
	add_edge(Z, S, EdgeProp { 2 }, g);
	add_edge(Z, X, EdgeProp { 7 }, g);
	my_print(g);
	// bellman-ford algorithm
	cout << "Bellman-Ford algorithm:" << endl;
	g[0].dist = 0; // set start veretx 0
	for (int i = 1; i < N; ++i) {
		g[i].dist = std::numeric_limits<int>::max();
	}
	Visitor vis;
	bool ret = bellman_ford_shortest_paths(g, weight_map(boost::get(&EdgeProp::w, g))
	                                       .root_vertex(vertex(S, g))
	                                       .distance_map(boost::get(&VertProp::dist, g))
	                                       .predecessor_map(boost::get(&VertProp::pred, g))
	                                       .visitor(vis)
	                                      );
	// output
	cout << "\tret = " << std::boolalpha << ret << endl;
	int max_dist = 0; // maximum distance for connected vertices
	int num_edge = 0; // number of edges used for shortest paths
	for (int i = 0; i < num_vertices(g); ++i) {
		int dist = g[i].dist;
		if (dist == std::numeric_limits<int>::max()) {
			cout << "\tvertex " << g[i].name << ": not connected" << endl;
		} else {
			cout << "\tvertex " << g[i].name << ": dist=" << dist;
			max_dist = std::max(max_dist, dist);
			if (g[i].pred != i) {
				++num_edge;
			}
			// print the shortest path for this vertex
			int j = i;
			cout << " " << g[j].name;
			while (g[j].pred != j) {
				cout << "<-" << g[g[j].pred].name;
				j = g[j].pred;
			}
			cout << endl;
		}
	}
	cout << "\t" << "num edges used = " << num_edge << endl;
	cout << "\t" << "max dist = " << max_dist << endl;
	// done
	return 0;
}
