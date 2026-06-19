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

int parse_num(int argc, char* argv[], int num) {
	if (argc >= 2) { // get the argv[1] to long int
		char* ptr = NULL;
		long val = strtol(argv[1], &ptr, 0);
		if (ptr != NULL && *ptr != '\0') {
			switch (*ptr) {
			case 'k': case 'K': val *= 1024; break; // kilo
			case 'm': case 'M': val *= (1024 * 1024); break; // mega
			case 'g': case 'G': val *= (1024 * 1024 * 1024); break; // giga
			}
		}
		if (val > 0) {
			return val;
		}
	}
	return num;
}

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
	int dist; // distance
	int pred; // predecessor index
	default_color_type color;
#endif
} VertProp;
typedef struct {
	int w; // weight for each edge
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

template <class RandNumGen>
void my_gen_random_dag(Graph& g, int V, int E, RandNumGen& gen) {
	std::uniform_int_distribution<int> unif_dist(1, std::max(V, E));
	for (int i = 0; i < V; ++i) {
		add_vertex(g);
		g[i].name = std::to_string(i);
	}
	for (int j = 0; j < E; ) {
		VertDesc a = random_vertex(g, gen);
		VertDesc b;
		do {
			b = random_vertex(g, gen);
		} while (a == b);
		auto p = edge(a, b, g);
		if (p.second == false) {
			int w = unif_dist(gen);
			add_edge(a, b, EdgeProp { w }, g);
			++j;
		}
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
	int N = parse_num(argc, argv, 20);
	int M = 2 * N * static_cast<int>(sqrt(sqrt(N))); // num edges
	// int M = N * (N / 4); // num edges
	// make a random graph
	std::random_device rd; // random device
	std::default_random_engine re(rd()); // random engine
	Graph g;
	cout << "random directed graph generation: " << endl;
	my_gen_random_dag(g, N, M, re);
	my_print(g);
	// Floyd-Warshall algorithm
	cout << "Floyd algorithm:" << endl;
	boost::exterior_vertex_property<Graph, int>::matrix_type dist_mat_f(N); // NxN square matrix
	boost::exterior_vertex_property<Graph, int>::matrix_map_type dist_f(dist_mat_f, g); // connect to g
	bool ret;
	{
		steady_clock::time_point start = steady_clock::now();
		ret = floyd_warshall_all_pairs_shortest_paths(g, dist_f,
		      weight_map(boost::get(&EdgeProp::w, g))
		                                             );
		cout << "\t" << duration_cast<milliseconds>(steady_clock::now() - start).count() << " msec elapsed" << endl;
	}
	if (ret == false) {
		cout << "ERROR: negative cycle detected" << endl;
		exit(0);
	}
	// Johnson's algorithm
	cout << "Johnson's algorithm:" << endl;
	boost::exterior_vertex_property<Graph, int>::matrix_type dist_mat_j(N); // NxN square matrix
	boost::exterior_vertex_property<Graph, int>::matrix_map_type dist_j(dist_mat_j, g); // connect to g
	{
		steady_clock::time_point start = steady_clock::now();
		johnson_all_pairs_shortest_paths(g, dist_j,
		                                 weight_map(boost::get(&EdgeProp::w, g))
		                                ); // caution: checking return value may cause many warnings
		cout << "\t" << duration_cast<milliseconds>(steady_clock::now() - start).count() << " msec elapsed" << endl;
	}
	// output
	if (N <= 20) {
		print_mat(N, g, dist_f, std::numeric_limits<int>::max());
		print_mat(N, g, dist_j, std::numeric_limits<int>::max());
	}
	long long sum_f = 0;
	long long sum_j = 0;
	for (int i = 0; i < N; ++i) {
		for (int j = 0; j < N; ++j) {
			sum_f += dist_f[i][j];
			sum_j += dist_j[i][j];
		}
	}
	cout << "\tsum_f = " << sum_f << endl;
	cout << "\tsum_j = " << sum_j << endl;
	// done
	return 0;
}
