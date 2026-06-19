#include <iostream>
#include <chrono>
#include <vector>
#include <deque>
#include <random>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/bellman_ford_shortest_paths.hpp>
#include <boost/graph/dag_shortest_paths.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
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
	int dist; // distance
	int pred; // predecessor index
	default_color_type color;
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

void my_print_shortest_paths(Graph& g) {
	long long sum_dist = 0; // total distance
	int max_dist = 0; // maximum distance for connected vertices
	int num_edge = 0; // number of edges used for shortest paths
	int num_show = 16;
	for (int i = 0; i < num_vertices(g) && num_show > 0; ++i, --num_show) {
		int dist = g[i].dist;
		if (dist == std::numeric_limits<int>::max()) {
			cout << "\tvertex " << g[i].name << ": color=" << my_color2str(g[i].color) << ", not connected" << endl;
		} else {
			cout << "\tvertex " << g[i].name << ": color=" << my_color2str(g[i].color) << ", dist=" << dist;
			sum_dist += dist;
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
	if (boost::num_vertices(g) > 16) {
		cout << "\t. . ." << endl;
	}
	cout << "\t" << "num edges used = " << num_edge << endl;
	cout << "\t" << "max dist = " << max_dist << endl;
	cout << "\t" << "sum dist = " << sum_dist << endl;
}

template <class RandNumGen>
void my_gen_random_dag(Graph& g, int V, int E, RandNumGen& gen) {
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
	int M = N * (N / 4); // num edges
	int START = 0; // start vertex index, you can change it randomly
	// make a random graph
	std::random_device rd; // random device
	std::default_random_engine re(rd()); // random engine
	Graph g;
	cout << "random directed graph generation: " << endl;
	my_gen_random_dag(g, N, M, re);
	my_print(g);
	// bellman-ford algorithm
	if (N <= 4 * 1024) {
		cout << "Bellman-Ford algorithm:" << endl;
		for (int i = 0; i < N; ++i) {
			g[i].dist = std::numeric_limits<int>::max();
		}
		g[START].dist = 0; // set start veretx distance 0
		bool ret;
		{
			steady_clock::time_point start = steady_clock::now();
			ret = bellman_ford_shortest_paths(g, weight_map(boost::get(&EdgeProp::w, g))
			                                  .root_vertex(vertex(START, g))
			                                  .distance_map(boost::get(&VertProp::dist, g))
			                                  .predecessor_map(boost::get(&VertProp::pred, g))
			                                 );
			cout << "\t" << duration_cast<milliseconds>(steady_clock::now() - start).count() << " msec elapsed" << endl;
		}
		cout << "\tret = " << std::boolalpha << ret << endl;
		my_print_shortest_paths(g);
	}
	// dag shortest-path
	cout << "DAG shortest-path:" << endl;
	for (int i = 0; i < N; ++i) {
		g[i].dist = std::numeric_limits<int>::max();
	}
	g[START].dist = 0; // set start vertex distance 0
	{
		steady_clock::time_point start = steady_clock::now();
		dag_shortest_paths(g, vertex(START, g),
		                   weight_map(boost::get(&EdgeProp::w, g))
		                   .distance_map(boost::get(&VertProp::dist, g))
		                   .predecessor_map(boost::get(&VertProp::pred, g))
		                   .color_map(boost::get(&VertProp::color, g))
		                  );
		cout << "\t" << duration_cast<milliseconds>(steady_clock::now() - start).count() << " msec elapsed" << endl;
	}
	my_print_shortest_paths(g);
	// dijkstra shortest-path
	cout << "Dijkstra shortest-path:" << endl;
	for (int i = 0; i < N; ++i) {
		g[i].dist = std::numeric_limits<int>::max();
	}
	g[START].dist = 0; // set start vertex distance 0
	{
		steady_clock::time_point start = steady_clock::now();
		dijkstra_shortest_paths(g, vertex(START, g),
		                        weight_map(boost::get(&EdgeProp::w, g))
		                        .distance_map(boost::get(&VertProp::dist, g))
		                        .predecessor_map(boost::get(&VertProp::pred, g))
		                        .color_map(boost::get(&VertProp::color, g))
		                       );
		cout << "\t" << duration_cast<milliseconds>(steady_clock::now() - start).count() << " msec elapsed" << endl;
	}
	my_print_shortest_paths(g);
	// done
	return 0;
}
