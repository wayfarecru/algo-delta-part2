#include <iostream>
#include <iomanip>
#include <chrono>
#include <vector>
#include <deque>
#include <random>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/floyd_warshall_shortest.hpp>
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

std::string my_color2str(boost::default_color_type c) {
	switch (c) {
	case boost::white_color:	return std::string("white");
	case boost::gray_color:		return std::string("gray");
	case boost::black_color:	return std::string("black");
	case boost::green_color:	return std::string("green");
	case boost::red_color:		return std::string("red");
	}
	return std::string("unknown");
}

typedef struct {
	std::string name;
#if 0 // if needed, add anyone
	float dist; // distance
	int pred; // predecessor index
	default_color_type color;
#endif
} VertProp;
typedef struct {
	float w; // weight for each edge
} EdgeProp;
typedef boost::adjacency_list<vecS, vecS, bidirectionalS, VertProp, EdgeProp> Graph;
typedef boost::graph_traits<Graph>::vertex_descriptor VertDesc;
typedef boost::graph_traits<Graph>::edge_descriptor EdgeDesc;

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

template<typename Graph, typename TwoD, typename TYPE>
void print_mat(const int N, const Graph& g, const TwoD& dist, const TYPE inf = std::numeric_limits<TYPE>::max()) {
	cout << "\t";
	for (int i = 0; i < N; ++i) {
		cout << std::setw(3) << g[i].name << " ";
	}
	cout << endl;
	for (int i = 0; i < N; ++i) {
		cout << g[i].name << " ->\t";
		for (int j = 0; j < N; ++j) {
			if (dist[i][j] == inf) {
				cout << "inf ";
			} else {
				cout << std::setw(3) << dist[i][j] << " ";
			}
		}
		cout << endl;
	}
}

int main(int argc, char* argv[]) {
	// make a target graph : STAR-shape in CLRS
	enum { A, B, C, D, E, N }; // N is the number of vertices
	Graph g(N);
	g[A].name = "A";
	g[B].name = "B";
	g[C].name = "C";
	g[D].name = "D";
	g[E].name = "E";
	add_edge(A, B, EdgeProp { 3 }, g);
	add_edge(A, C, EdgeProp { 8 }, g);
	add_edge(A, E, EdgeProp { -4 }, g);
	add_edge(B, D, EdgeProp { 1 }, g);
	add_edge(B, E, EdgeProp { 7 }, g);
	add_edge(C, B, EdgeProp { 4 }, g);
	add_edge(D, A, EdgeProp { 2 }, g);
	add_edge(D, C, EdgeProp { -5 }, g);
	add_edge(E, D, EdgeProp { 6 }, g);
	my_print(g);
	// Floyd-Warshall algorithm
	cout << "Floyd algorithm:" << endl;
	float dist[N][N];
	bool ret = floyd_warshall_all_pairs_shortest_paths(g, dist, weight_map(boost::get(&EdgeProp::w, g)));
	if (ret == false) {
		cout << "ERROR: negative cycle detected" << endl;
		exit(0);
	}
	// output
	print_mat(N, g, dist, std::numeric_limits<float>::max());
	// done
	return 0;
}
