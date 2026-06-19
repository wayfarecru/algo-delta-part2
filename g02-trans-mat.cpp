#include <iostream>
#include <iomanip>
#include <chrono>
#include <vector>
#include <deque>
#include <random>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/exterior_property.hpp>
#include <boost/graph/floyd_warshall_shortest.hpp>
#include <boost/graph/johnson_all_pairs_shortest.hpp>
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
} VertProp;
typedef struct {
	int w; // weight for each edge
} EdgeProp;
typedef boost::adjacency_list<vecS, vecS, bidirectionalS, VertProp, EdgeProp> Graph;

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
			cout << g[*jt].name << " ";
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

template <typename T>
struct my_logical_or { // modified from boost::closed_plus()
	T inf; // infinity value
	my_logical_or() : inf(std::numeric_limits<T>::max()) { }
	my_logical_or(T infty) : inf(infty) { }
	T operator()(const T& lhs, const T& rhs) const {
		if (lhs == inf) return inf;
		if (rhs == inf) return inf;
		return static_cast<T>((lhs > 0) || (rhs > 0));
	}
};

int main(int argc, char* argv[]) {
	// make a target graph : STAR-shape in CLRS
	enum { A, B, C, D, N }; // N is the number of vertices
	Graph g(N);
	g[A].name = "A";
	g[B].name = "B";
	g[C].name = "C";
	g[D].name = "D";
	add_edge(A, A, EdgeProp { 1 }, g);
	add_edge(B, B, EdgeProp { 1 }, g);
	add_edge(C, C, EdgeProp { 1 }, g);
	add_edge(D, D, EdgeProp { 1 }, g);
	add_edge(B, C, EdgeProp { 1 }, g);
	add_edge(B, D, EdgeProp { 1 }, g);
	add_edge(C, B, EdgeProp { 1 }, g);
	add_edge(D, A, EdgeProp { 1 }, g);
	add_edge(D, C, EdgeProp { 1 }, g);
	my_print(g);
	// Floyd-Warshall algorithm
	const int inf = std::numeric_limits<int>::max(); // infinity
	cout << "Floyd algorithm:" << endl;
	boost::exterior_vertex_property<Graph, int>::matrix_type dist_mat_f(N); // NxN square matrix
	boost::exterior_vertex_property<Graph, int>::matrix_map_type dist_f(dist_mat_f, g); // connect to g
	floyd_warshall_all_pairs_shortest_paths(g, dist_f,
	                                        weight_map(boost::get(&EdgeProp::w, g))
	                                        .distance_zero(1)
	                                        .distance_combine(my_logical_or(inf))
	                                       );
	print_mat(N, g, dist_f, std::numeric_limits<int>::max());
	// Johnson's algorithm
	cout << "Johnson's algorithm:" << endl;
	boost::exterior_vertex_property<Graph, int>::matrix_type dist_mat_j(N); // NxN square matrix
	boost::exterior_vertex_property<Graph, int>::matrix_map_type dist_j(dist_mat_j, g); // connect to g
	johnson_all_pairs_shortest_paths(g, dist_j,
	                                 weight_map(boost::get(&EdgeProp::w, g))
	                                 .distance_zero(1)
	                                 .distance_combine(my_logical_or(inf))
	                                ); // caution: checking return value may cause many warnings
	print_mat(N, g, dist_j, inf);
	// done
	return 0;
}
