#include <iostream>
#include <chrono>
#include <vector>
#include <deque>
#include <random>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/kruskal_min_spanning_tree.hpp>
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

typedef struct {
	std::string name;
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

template <class RandNumGen>
void my_gen_random_graph(Graph& g, int V, int E, RandNumGen& gen) {
	std::uniform_int_distribution<int> unif_dist(1, std::max(V, E));
	for (int i = 0; i < V; ++i) {
		add_vertex(g);
		g[i].name = std::to_string(i);
	}
	VertIndMap vertIndMap = boost::get(boost::vertex_index, g); // VertDesc --> index
	for (int j = 0; j < E; ) {
		VertDesc a = random_vertex(g, gen);
		VertDesc b;
		do {
			b = random_vertex(g, gen);
		} while (a == b);
		if (vertIndMap[a] > vertIndMap[b]) {
			std::swap(a, b);
		}
		auto p = edge(a, b, g);
		if (p.second == false) {
			int w = unif_dist(gen);
			add_edge(a, b, EdgeProp { w }, g);
			++j;
		}
	}
}

int main(int argc, char* argv[]) {
	int N = parse_num(argc, argv, 16);
	int M = N * static_cast<int>(sqrt(sqrt(N))); // num edges
	// make a random graph
	std::random_device rd; // random device
	std::default_random_engine re(rd()); // random engine
	Graph g;
	cout << "random graph generation: " << endl;
	my_gen_random_graph(g, N, M, re);
	my_print(g);
	// kruskal's algorithm
	cout << "Kruskal's MST algorithm:" << endl;
	std::vector<EdgeDesc> mstree;
	{
		steady_clock::time_point start = steady_clock::now();
		kruskal_minimum_spanning_tree(g, std::back_inserter(mstree), weight_map(boost::get(&EdgeProp::w, g)));
		cout << "\t" << duration_cast<milliseconds>(steady_clock::now() - start).count() << " msec elapsed" << endl;
	}
	cout << "\t" << mstree.size() << " edges: ";
	long long sum_w = 0;
	for (auto it = mstree.begin(); it != mstree.end(); ++it) {
		sum_w += g[*it].w;
	}
	cout << "weight for MST = " << sum_w << endl;
	// prim's algorithm
	cout << "Prim's MST algorithm:" << endl;
	std::vector<VertDesc> pred(num_vertices(g));
	{
		steady_clock::time_point start = steady_clock::now();
		prim_minimum_spanning_tree(g, &pred[0], weight_map(get(&EdgeProp::w, g)));
		cout << "\t" << duration_cast<milliseconds>(steady_clock::now() - start).count() << " msec elapsed" << endl;
	}
	int num_edge = 0;
	long long sum_w2 = 0;
	for (int i = 0; i < num_vertices(g); ++i) {
		if (pred[i] != i) {
			sum_w2 += g[boost::edge(i, pred[i], g).first].w;
			++num_edge;
		}
	}
	cout << "\t" << num_edge << " edges: ";
	cout << "weight for MST = " << sum_w2 << endl;
	// compare
	cout << "result is " << ((sum_w == sum_w2) ? "same" : "different") << endl;
	// done
	return 0;
}
