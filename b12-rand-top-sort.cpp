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

typedef boost::property<vertex_color_t, boost::default_color_type> VertProp;
typedef boost::adjacency_list<vecS, vecS, bidirectionalS, VertProp> Graph;
typedef boost::graph_traits<Graph>::vertex_descriptor VertDesc;
typedef boost::property_map<Graph, boost::vertex_index_t>::type VertIndMap;
typedef boost::property_map<Graph, boost::vertex_color_t>::type VertColorMap;

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
		cout << "\t" << vertIndMap[*it] << " --> [" << num << "] ";
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

template <class RandNumGen>
void my_gen_random_graph(Graph& g, int V, int E, RandNumGen& gen) {
	for (int i = 0; i < V; ++i) {
		add_vertex(g);
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
			add_edge(a, b, g);
			++j;
		}
	}
}

struct Visitor : public boost::dfs_visitor<> {
	std::deque<int>& ans; // answer
	VertIndMap vertIndMap;
	Visitor(Graph& g, std::deque<int>& _ans) : ans(_ans) {
		vertIndMap = boost::get(boost::vertex_index, g);
	}
	template <class Vertex, class Graph>
	void finish_vertex(const Vertex& v, const Graph& g) {
		ans.push_front(vertIndMap[v]);
	}
	template <class Edge, class Graph>
	void back_edge(const Edge& e, const Graph& g) {
		cout << "cycle detected at the edge [" << boost::source(e, g) << "," << boost::target(e, g) << "]" << endl;
	}
};

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
	// my topological sort, based on the boost::DFS
	cout << "my topological sort:" << endl;
	std::deque<int> my_ans;
	Visitor vis(g, my_ans);
	boost::depth_first_search(g, vis, boost::get(boost::vertex_color, g));
	cout << "\tans[" << my_ans.size() << "] = ";
	print(my_ans);
	// boost topological sort
	cout << "boost::topological_sort: " << endl;
	std::deque<VertDesc> ans;
	boost::topological_sort(g, std::front_inserter(ans));
	cout << "\tans[" << ans.size() << "] = ";
	print(ans);
	// done
	return 0;
}
