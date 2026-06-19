#include <iostream>
#include <chrono>
#include <vector>
#include <random>
#include <limits>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/properties.hpp> // white_color, gray_color, black_color
#include <boost/graph/breadth_first_search.hpp> // for breadth_first_search()
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
	boost::default_color_type color;
	int dist; // distance
	int parent; // my parent index (-1 for NULL)
} VertProp;
typedef boost::adjacency_list<listS, vecS, undirectedS, VertProp> Graph;
typedef boost::graph_traits<Graph>::vertex_descriptor VertDesc;
typedef boost::property_map<Graph, boost::vertex_index_t>::type VertIndMap;

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

int my_bfs_shortest_path(Graph& g, int src, int dst, std::vector<int>& ans) {
	VertIndMap vertIndMap = boost::get(boost::vertex_index, g);
	std::pair<Graph::vertex_iterator, Graph::vertex_iterator> vp = vertices(g);
	// reset all bundled properties
	for (Graph::vertex_iterator it = vp.first; it != vp.second; ++it) {
		g[*it].color = boost::white_color; // white means not processed
		g[*it].dist = std::numeric_limits<int>::max(); // as infinite distance
		g[*it].parent = -1; // as NULL
	}
	// breadth-first search
	VertDesc vd = boost::vertex(src, g);
	std::deque<VertDesc> vdq; // vertex description queue
	vdq.push_back(vd);
	g[vd].color = boost::gray_color; // gray means in the queue
	g[vd].dist = 0;
	while (! vdq.empty()) {
		VertDesc vd = vdq.front();
		vdq.pop_front();
		auto adjp = boost::adjacent_vertices(vd, g); // adjacent vertex pair
		for (auto it = adjp.first; it != adjp.second; ++it) {
			if (g[*it].color == boost::white_color) {
				vdq.push_back(*it);
				g[*it].color = boost::gray_color; 
				g[*it].dist = g[vd].dist + 1;
				g[*it].parent = vertIndMap[vd];
			}
		}
		g[vd].color = boost::black_color;
		if (vertIndMap[vd] == dst) break;
	}
#if 0
	// dump the bundled properties, if you needed
	for (int i = 0, Graph::vertex_iterator it = vp.first; it != vp.second; ++it, ++i) {
		cout << i << ": color=" << g[*it].color << " dist=" << g[*it].dist << " parent=" << g[*it].parent << endl;
	}
#endif
	// back-trace from dst
	int dist = g[vertex(dst, g)].dist;
	do {
		VertDesc vd = boost::vertex(dst, g);
		ans.push_back(vertIndMap[vd]);
		dst = g[vd].parent;
	} while (dst != -1);
	return dist;
}

int main(int argc, char* argv[]) {
	int N = parse_num(argc, argv, 16); // num vert
	int M = N * static_cast<int>(sqrt(sqrt(N))); // num edges
	// make a random graph
	std::random_device rd; // random device
	std::default_random_engine re(rd()); // random engine
	Graph g;
	cout << "random graph generation: " << endl;
	{
		steady_clock::time_point start = steady_clock::now();
		generate_random_graph(g, N, M, re, false, false); // no parallel edge, no self edge
		cout << "\t" << duration_cast<milliseconds>(steady_clock::now() - start).count() << " msec elapsed" << endl;
	}
	my_print(g);
	// my depth first search, with vertex colors
	cout << "my shortest path-first search: " << endl;
	int dist;
	std::vector<int> ans;
	{
		steady_clock::time_point start = steady_clock::now();
		dist = my_bfs_shortest_path(g, 0, N - 1, ans);
		cout << "\t" << duration_cast<milliseconds>(steady_clock::now() - start).count() << " msec elapsed" << endl;
	}
	cout << "\tdist = " << dist << endl;
	cout << "\tans[" << ans.size() << "] = ";
	print(ans);
}
