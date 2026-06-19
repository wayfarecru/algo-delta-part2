#include <iostream>
#include <chrono>
#include <vector>
#include <random>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/properties.hpp> // white_color, gray_color, black_color
#include <boost/graph/depth_first_search.hpp> // for depth_first_search()
#include <boost/graph/random.hpp> // for generate_random_graph()
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
	boost::default_color_type c; // color
	int d; // discovery time
	int f; // finish time
} VertProp;
typedef boost::adjacency_list<vecS, vecS, bidirectionalS, VertProp> Graph;
typedef boost::graph_traits<Graph>::vertex_descriptor VertDesc;
typedef boost::property_map<Graph, boost::vertex_index_t>::type VertIndMap;

std::string my_color(boost::default_color_type c) {
	switch (c) {
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
	std::pair<Graph::vertex_iterator, Graph::vertex_iterator> vp = boost::vertices(g); // all vertices
	int num_show = 16;
	for (Graph::vertex_iterator it = vp.first; it != vp.second && num_show > 0; ++it, --num_show) {
		auto adjp = boost::adjacent_vertices(*it, g); // all adjacent vertices
		int num = distance(adjp.first, adjp.second); // number of adjacent vertices
		cout << "\t" << vertIndMap[*it] << " " << my_color(g[*it].c);
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
}

static int my_dfs_timestamp = 0;

void my_dfs_time_rec(Graph& g, const VertDesc& vd) {
	VertIndMap vertIndMap = boost::get(boost::vertex_index, g);
	g[vd].c = boost::gray_color; // discovered
	g[vd].d = ++my_dfs_timestamp;
	cout << "(" << vertIndMap[vd] << ":" << g[vd].d << " ";
	auto adjp = adjacent_vertices(vd, g);
	for (auto it = adjp.first; it != adjp.second; ++it) {
		if (g[*it].c == boost::white_color) {
			my_dfs_time_rec(g, *it);
		}
	}
	g[vd].c = boost::black_color; // finished
	g[vd].f = ++my_dfs_timestamp;
	cout << vertIndMap[vd] << ":" << g[vd].f << ") ";
}

void my_dfs_time(Graph& g) {
	my_dfs_timestamp = 0;
	auto vp = boost::vertices(g);
	// reset all vertex colors to white, time intervals to zero
	for (auto it = vp.first; it != vp.second; ++it) {
		g[*it].c = boost::white_color; // unvisited
		g[*it].d = 0;
		g[*it].f = 0;
	}
	// recursive visit
	for (auto it = vp.first; it != vp.second; ++it) {
		if (g[*it].c == boost::white_color) {
			my_dfs_time_rec(g, *it);
		}
	}
}

struct Visitor : public boost::dfs_visitor<> {
	Graph& targetGraph; // this graph has bundled property
	int timestamp;
	// ctor saves the target graph
	Visitor(Graph& g) : targetGraph(g), timestamp(0) { }
	// visitor event points
	template <class Vertex, class Graph>
	void initialize_vertex(const Vertex& v, const Graph& g) {
		targetGraph[v].d = 0;
		targetGraph[v].f = 0;
	}
	template <class Vertex, class Graph>
	void discover_vertex(const Vertex& v, const Graph& g) {
		targetGraph[v].d = ++timestamp;
		cout << "(" << v << ":" << g[v].d << " ";
	}
	template <class Vertex, class Graph>
	void finish_vertex(const Vertex& v, const Graph& g) {
		targetGraph[v].f = ++timestamp;
		cout << v << ":" << g[v].f << ") ";
	}
};

int main(int argc, char* argv[]) {
	// make a graph
	const int N = 8; // number of vertices
	Graph g(N); // and 13 edges
	add_edge(0, 4, g);
	add_edge(0, 7, g);
	add_edge(1, 2, g);
	add_edge(1, 3, g);
	add_edge(2, 0, g);
	add_edge(2, 7, g);
	add_edge(3, 1, g);
	add_edge(3, 2, g);
	add_edge(4, 5, g);
	add_edge(4, 7, g);
	add_edge(5, 6, g);
	add_edge(6, 4, g);
	add_edge(7, 6, g);
	my_print(g);
	// my depth first search, with time intervals
	cout << "my depth-first search with time: " << endl;
	cout << "\t";
	my_dfs_time(g);
	cout << endl;
	my_print(g);
	// boost depth first search
	cout << "boost::depth_first_search with time: " << endl;
	Visitor vis(g);
	cout << "\t";
	boost::depth_first_search(g, vis, boost::get(&VertProp::c, g));
	cout << endl;
	my_print(g);
	// done
	return 0;
}
