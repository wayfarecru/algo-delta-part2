#include <iostream>
#include <chrono>
#include <vector>
#include <random>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/undirected_dfs.hpp>
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
	boost::default_color_type c; // color
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
		cout << "\t" << vertIndMap[*it] << " (" << g[*it].comp << ") " << my_color2str(g[*it].c);
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
		cout << "\t(" << boost::source(*it, g) << "," << boost::target(*it, g) << ") ";
		cout << my_color2str(g[*it].ec) << endl;
	}
	if (boost::num_edges(g) > 16) {
		cout << "\t. . ." << endl;
	}
}

void my_reset(Graph& g) {
	// reset all vertex colors to white, time intervals to zero
	auto vp = boost::vertices(g);
	for (auto it = vp.first; it != vp.second; ++it) {
		g[*it].c = boost::white_color; // unvisited vertex
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

struct Visitor : public boost::dfs_visitor<> {
	Graph& targetGraph; // this graph has bundled property
	int comp_num; // component number
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
		cout << "(" << v << ":" << g[v].d << " ";
	}
	template <class Vertex, class Graph>
	void finish_vertex(const Vertex& v, const Graph& g) {
		targetGraph[v].f = ++timestamp;
		cout << v << ":" << g[v].f << ") ";
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
		VertDesc source = boost::source(e, g);
		VertDesc target = boost::target(e, g);
		if (g[source].comp == g[target].comp) {
			if (g[source].d < g[target].d) { // source is an ancestor of target
				targetGraph[e].ec = black_color;
			} else { // crossing to sibling vertex
				targetGraph[e].ec = green_color;
			}
		} else { // crossing difference connected components
			targetGraph[e].ec = green_color;
		}
	}
};

int main(int argc, char* argv[]) {
	// make a graph
	const int N = 8; // number of vertices
	Graph g(N); // and 13 edges
	add_edge(0, 4, EdgeProp { red_color }, g);
	add_edge(0, 7, EdgeProp { red_color }, g);
	add_edge(1, 2, EdgeProp { red_color }, g);
	add_edge(1, 3, EdgeProp { red_color }, g);
	add_edge(2, 0, EdgeProp { red_color }, g);
	add_edge(2, 7, EdgeProp { red_color }, g);
	add_edge(3, 1, EdgeProp { red_color }, g);
	add_edge(3, 2, EdgeProp { red_color }, g);
	add_edge(4, 5, EdgeProp { red_color }, g);
	add_edge(4, 7, EdgeProp { red_color }, g);
	add_edge(5, 6, EdgeProp { red_color }, g);
	add_edge(6, 4, EdgeProp { red_color }, g);
	add_edge(7, 6, EdgeProp { red_color }, g);
	my_print(g);
	// boost depth first search
	cout << "boost::depth_first_search with time: " << endl;
	my_reset(g);
	Visitor vis(g);
	cout << "\t";
	boost::depth_first_search(g, vis, boost::get(&VertProp::c, g));
	cout << endl;
	my_print(g);
	// done
	return 0;
}
