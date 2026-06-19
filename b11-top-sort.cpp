#include <iostream>
#include <chrono>
#include <vector>
#include <deque>
#include <random>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/properties.hpp> // white_color, gray_color, black_color
#include <boost/graph/topological_sort.hpp> // for topological_sort()
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

template<typename T>
void print(typename std::deque<T>::const_iterator first, typename std::deque<T>::const_iterator last) {
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
void print(const std::deque<T>& v) {
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
	boost::default_color_type color;
	std::string name;
} VertProp;
typedef boost::adjacency_list<vecS, vecS, bidirectionalS, VertProp> Graph;
typedef boost::graph_traits<Graph>::vertex_descriptor VertDesc;
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
			cout << g[*jt].name << " ";
		}
		cout << ((num > 16) ? ". . ." : "") << endl;
	}
	if (boost::num_vertices(g) > 16) {
		cout << "\t. . ." << endl;
	}
}

struct Visitor : public boost::dfs_visitor<> {
	int timestamp;
	std::deque<int>& ans; // answer
	VertIndMap vertIndMap;
	Visitor(Graph& g, std::deque<int>& _ans) : timestamp(0), ans(_ans) {
		vertIndMap = boost::get(boost::vertex_index, g);
	}
	template <class Vertex, class Graph>
	void discover_vertex(const Vertex& v, const Graph& g) {
		++timestamp;
	}
	template <class Vertex, class Graph>
	void finish_vertex(const Vertex& v, const Graph& g) {
		ans.push_front(vertIndMap[v]);
		++timestamp;
		cout << "vertex " << vertIndMap[v] << " finished at " << timestamp << endl;
	}
	template <class Edge, class Graph>
	void back_edge(const Edge& e, const Graph& g) {
		cout << "cycle detected at the edge [" << boost::source(e, g) << "," << boost::target(e, g) << "]" << endl;
	}
};

int main(int argc, char* argv[]) {
	// make a directed graph
	const int N = 9; // num vert
	Graph g(N);
	g[0].name = "shirt";
	g[1].name = "tie";
	g[2].name = "jacket";
	g[3].name = "belt";
	g[4].name = "watch";
	g[5].name = "undershorts";
	g[6].name = "pants";
	g[7].name = "shoes";
	g[8].name = "socks";
	add_edge(0, 1, g);
	add_edge(0, 3, g);
	add_edge(1, 2, g);
	add_edge(3, 2, g);
	add_edge(5, 6, g);
	add_edge(5, 7, g);
	add_edge(6, 3, g);
	add_edge(6, 7, g);
	add_edge(8, 7, g);
	my_print(g);
	// my topological sort, based on the boost::DFS
	cout << "my topological sort:" << endl;
	std::deque<int> my_ans;
	Visitor vis(g, my_ans);
	boost::depth_first_search(g, vis, boost::get(&VertProp::color, g));
	cout << "\tans[" << my_ans.size() << "] = ";
	for (std::deque<int>::iterator it = my_ans.begin(); it != my_ans.end(); ++it) {
		cout << g[*it].name << " ";
	}
	cout << endl;
	// boost topological sort
	cout << "boost::topological_sort: " << endl;
	std::deque<VertDesc> ans;
	boost::topological_sort(g, std::front_inserter(ans));
	cout << "\tans[" << ans.size() << "] = ";
	for (std::deque<VertDesc>::iterator it = ans.begin(); it != ans.end(); ++it) {
		cout << g[*it].name << " ";
	}
	cout << endl;
	// done
	return 0;
}
