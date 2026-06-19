#include <iostream>
#include <chrono>
#include <vector>
#include <random>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/depth_first_search.hpp>
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

template<typename T>
void print(typename std::vector<T>::const_iterator first, typename std::vector<T>::const_iterator last) {
	const int nElemPrint = 16; // number of printed elements, divided by 2
	int n = std::distance(first, last);
	if (n <= 2 * nElemPrint) {
		std::copy(first, last, std::ostream_iterator<T>(std::cout, " "));
	} else {  // 0 1 2 3 . . . 996 997 998 999
		std::copy(first, std::next(first, nElemPrint), std::ostream_iterator<T>(std::cout, " "));
		cout << ". . . ";
		std::copy(std::prev(last, nElemPrint), last, std::ostream_iterator<T>(std::cout, " "));
	}
}

template<typename T>
void print(const std::vector<T>& v) {
	print<T>(v.begin(), v.end());
	cout << endl;
}

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
	boost::default_color_type color; // color
	int comp; // component number
	int d; // discovery time
	int f; // finish time
} VertProp;
typedef struct {
	boost::default_color_type ec; // edge color
} EdgeProp;
typedef boost::adjacency_list<vecS, vecS, bidirectionalS, VertProp, EdgeProp> Graph;
typedef boost::graph_traits<Graph>::vertex_descriptor VertDesc;
typedef boost::property_map<Graph, boost::vertex_index_t>::type VertIndMap;

std::string my_color(boost::default_color_type color) {
	switch (color) {
	case boost::white_color:	return std::string("white");
	case boost::gray_color:		return std::string("gray");
	case boost::black_color:	return std::string("black");
	case boost::green_color:	return std::string("green");
	case boost::red_color:		return std::string("red");
	}
	return std::string("unknown");
}

void my_print(Graph& g, const char* name = nullptr) {
	cout << ((name == nullptr) ? "graph" : name) << ": ";
	cout << "num vert = " << boost::num_vertices(g);
	cout << ", num edge = " << boost::num_edges(g) << endl;
	VertIndMap vertIndMap = boost::get(boost::vertex_index, g); // VertDesc --> index
	// print vertices
	std::pair<Graph::vertex_iterator, Graph::vertex_iterator> vp = boost::vertices(g); // all vertices
	int num_show = 16;
	for (Graph::vertex_iterator it = vp.first; it != vp.second && num_show > 0; ++it, --num_show) {
		auto adjp = boost::adjacent_vertices(*it, g); // all adjacent vertices
		int num = distance(adjp.first, adjp.second); // number of adjacent vertices
		cout << "\t" << vertIndMap[*it] << " (" << g[*it].comp << ") " << my_color(g[*it].color);
		cout << " [" << g[*it].d << "," << g[*it].f << "] --> [" << num << "] ";
		int num_show = 16;
		for (auto jt = adjp.first; jt != adjp.second && num_show > 0; ++jt, --num_show) {
			cout << vertIndMap[*jt] << " ";
		}
		cout << ((num > 16) ? ". . ." : "") << endl;
	}
	if (boost::num_vertices(g) > 16) {
		cout << "\t. . ." << endl;
	}
	// print edges
	std::pair<Graph::edge_iterator, Graph::edge_iterator> ep = boost::edges(g); // all edges
	num_show = 16;
	for (Graph::edge_iterator it = ep.first; it != ep.second && num_show > 0; ++it, --num_show) {
		cout << "\t(" << boost::source(*it, g) << "," << boost::target(*it, g) << ") " << my_color(g[*it].ec) << endl;
	}
	if (boost::num_edges(g) > 16) {
		cout << "\t. . ." << endl;
	}
}

static int my_dfs_timestamp = 0;
static int my_dfs_comp_num = 0;

void my_dfs_time_rec(Graph& g, const VertDesc& vd) {
	g[vd].color = boost::gray_color; // discovered
	g[vd].comp = my_dfs_comp_num;
	g[vd].d = ++my_dfs_timestamp;
	auto outedgep = out_edges(vd, g);
	for (auto it = outedgep.first; it != outedgep.second; ++it) {
		assert(g[*it].ec == red_color);
		VertDesc target_vd = target(*it, g);
		switch (g[target_vd].color) {
		case boost::white_color:	// tree edge
			g[*it].ec = white_color;
			my_dfs_time_rec(g, target_vd);
			break;
		case boost::gray_color: 	// back edge
			g[*it].ec = gray_color;
			break;
		case boost::black_color:	// forward or cross edge
			if (g[target_vd].comp == my_dfs_comp_num) {
				if (g[vd].d < g[target_vd].d) { // current vertex is ancesstor of target vertex
					g[*it].ec = black_color;
				} else {
					g[*it].ec = green_color; // crossing to sibling vertex
				}
			} else { // crossing different connected components
				g[*it].ec = green_color;
			}
			break;
		default:
			assert(0);
			break;
		}
	}
	g[vd].color = boost::black_color; // finished
	g[vd].f = ++my_dfs_timestamp;
}

void my_reset(Graph& g) {
	my_dfs_timestamp = 0;
	// reset all vertex colors to white, time intervals to zero
	auto vp = boost::vertices(g);
	for (auto it = vp.first; it != vp.second; ++it) {
		g[*it].color = boost::white_color; // unvisited vertex
		g[*it].comp = 0;
		g[*it].d = 0;
		g[*it].f = 0;
	}
	// reset all edge colors to red
	auto ep = boost::edges(g);
	for (auto it = ep.first; it != ep.second; ++it) {
		g[*it].ec = boost::red_color; // unclassified edge
	}
}

void my_analyze(Graph& g) {
	// count vertex colors
	std::vector<int> num(5); // in <properties.hpp>, white(0), gray(1), green(2), red(3), black(4)
	std::vector<int> comp_count;
	std::fill(num.begin(), num.end(), 0);
	auto vp = boost::vertices(g);
	for (auto it = vp.first; it != vp.second; ++it) {
		default_color_type color = g[*it].color;
		assert(white_color <= color && color <= black_color);
		num[color]++;
		int comp = g[*it].comp;
		if (comp >= comp_count.size()) {
			comp_count.resize(comp + 1);
		}
		comp_count[comp]++;
	}
	cout << "connected componnets: " << comp_count.size() << endl;
	for (int i = 0; i < comp_count.size(); ++i) {
		cout << "\t" << i << "\t" << comp_count[i] << endl;
	}
	cout << "vertex colors:" << endl;
	for (int i = 0; i < num.size(); ++i) {
		cout << "\t" << my_color(static_cast<default_color_type>(i)) << ":\t" << num[i] << endl;
	}
	cout << "\ttotal:\t" << std::accumulate(num.begin(), num.end(), 0) << endl;
	// count edge colors
	std::fill(num.begin(), num.end(), 0);
	auto ep = boost::edges(g);
	for (auto it = ep.first; it != ep.second; ++it) {
		default_color_type color = g[*it].ec;
		assert(white_color <= color && color <= black_color);
		num[color]++;
	}
	cout << "edge colors:" << endl;
	for (int i = 0; i < num.size(); ++i) {
		cout << "\t" << my_color(static_cast<default_color_type>(i)) << ":\t" << num[i] << endl;
	}
	cout << "\ttotal:\t" << std::accumulate(num.begin(), num.end(), 0) << endl;
}

void my_dfs_time(Graph& g) {
	my_reset(g);
	// recursive visit
	my_dfs_comp_num = 0;
	auto vp = boost::vertices(g);
	for (auto it = vp.first; it != vp.second; ++it) {
		if (g[*it].color == boost::white_color) {
			++my_dfs_comp_num;
			my_dfs_time_rec(g, *it);
		}
	}
}

struct Visitor : public boost::dfs_visitor<> {
	Graph& targetGraph; // this graph has bundled property
	int comp_num;
	int timestamp;
	// ctor saves the target graph
	Visitor(Graph& g) : targetGraph(g), comp_num(0), timestamp(0) { }
	// visitor event points
	template <class Vertex, class Graph>
	void start_vertex(const Vertex& v, const Graph& g) {
		++comp_num;
	}
	template <class Vertex, class Graph>
	void discover_vertex(const Vertex& v, const Graph& g) {
		targetGraph[v].comp = comp_num;
		targetGraph[v].d = ++timestamp;
	}
	template <class Vertex, class Graph>
	void finish_vertex(const Vertex& v, const Graph& g) {
		targetGraph[v].f = ++timestamp;
	}
	template <class Edge, class Graph>
	void tree_edge(const Edge& e, const Graph& g) {
		targetGraph[e].ec = white_color;
	}
	template <class Edge, class Graph>
	void back_edge(const Edge& e, const Graph& g) {
		targetGraph[e].ec = gray_color;
	}
	template <class Edge, class Graph>
	void forward_or_cross_edge(const Edge& e, const Graph& g) {
		targetGraph[e].ec = black_color;
	}
};

template <class RandNumGen>
void my_gen_random_graph(Graph& g, int V, int E, RandNumGen& gen) {
	for (int i = 0; i < V; ++i) {
		add_vertex(VertProp { white_color, 0, 0, 0 }, g);
	}
	for (int j = 0; j < E; ) {
		VertDesc a = random_vertex(g, gen);
		VertDesc b;
		do {
			b = random_vertex(g, gen);
		} while (a == b);
		auto p = edge(a, b, g);
		if (p.second == false) {
			add_edge(a, b, EdgeProp { red_color }, g);
			++j;
		}
	}
}

typedef boost::adjacency_list<vecS, vecS, bidirectionalS> SimpleGraph;
typedef boost::property_map<SimpleGraph, boost::vertex_index_t>::type SimpleVertIndMap;

int main(int argc, char* argv[]) {
	int N = parse_num(argc, argv, 1024); // num vert
	int M = N * static_cast<int>(sqrt(sqrt(N))); // num edges
/*
	int M = 4 * N;
*/
	// make a random graph
	cout << "random graph generation: " << endl;
	std::random_device rd; // random device
	std::default_random_engine re(rd()); // random engine
/*
	SimpleGraph h;
	boost::generate_random_graph(h, N, M, re, false, false); // no parallel edge, no self edge
	SimpleVertIndMap vertIndMap = boost::get(boost::vertex_index, h); // VertDesc --> index
	Graph g(N);
	auto ep = boost::edges(h);
	for (auto it = ep.first; it != ep.second; ++it) {
		add_edge(vertIndMap[source(*it, h)], vertIndMap[target(*it, h)], EdgeProp { red_color }, g);
	}
*/
	Graph g;
	my_gen_random_graph(g, N, M, re);
	my_reset(g);
	my_print(g);
#if 0
	// my depth first search, with time intervals
	cout << "my depth-first search with time: " << endl;
	my_dfs_time(g);
	my_analyze(g);
#endif
	// boost depth first search
	cout << "boost::depth_first_search with time: " << endl;
	my_reset(g);
	Visitor vis(g);
	boost::depth_first_search(g, vis, boost::get(&VertProp::color, g));
	my_analyze(g);
	// done
	return 0;
}
