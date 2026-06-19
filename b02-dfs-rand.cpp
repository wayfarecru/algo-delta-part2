#include <iostream>
#include <chrono>
#include <vector>
#include <random>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/properties.hpp> // white_color, gray_color, black_color
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/connected_components.hpp>
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

typedef boost::property<vertex_color_t, boost::default_color_type> VertProp;
typedef boost::adjacency_list<vecS, vecS, undirectedS, VertProp> Graph;
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

void my_dfs_rec(const Graph& g, int s, std::deque<bool>& visit, std::vector<int>& ans) {
	VertIndMap vertIndMap = boost::get(boost::vertex_index, g);
	visit[s] = true;
	ans.push_back(s);
	auto vp = boost::adjacent_vertices(s, g);
	for (auto it = vp.first; it != vp.second; ++it) {
		int ind = vertIndMap[*it];
		if (visit[ind] == false) {
			my_dfs_rec(g, ind, visit, ans);
		}
	}
}

void my_dfs(const Graph& g, int s, std::vector<int>& ans) {
	int n = num_vertices(g);
	std::deque<bool> visit(n, false); // vert_ind -> visited or not
	my_dfs_rec(g, s, visit, ans);
}

void my_dfs_prop_rec(Graph& g, const VertDesc& vd, std::vector<int>& ans) {
	VertIndMap vertIndMap = boost::get(boost::vertex_index, g);
	VertColorMap vertColorMap = boost::get(boost::vertex_color, g);
	vertColorMap[vd] = boost::gray_color; // discovered
	ans.push_back(vertIndMap[vd]);
	auto adjp = adjacent_vertices(vd, g);
	for (auto it = adjp.first; it != adjp.second; ++it) {
		if (vertColorMap[*it] == boost::white_color) {
			my_dfs_prop_rec(g, *it, ans);
		}
	}
	vertColorMap[vd] = boost::black_color; // finished
}

void my_dfs_prop(Graph& g, int s, std::vector<int>& ans) {
	VertColorMap vertColorMap = boost::get(boost::vertex_color, g);
	auto vp = boost::vertices(g);
	// reset all vertex colors to white
	for (auto it = vp.first; it != vp.second; ++it) {
		vertColorMap[*it] = boost::white_color; // white means not-visited
	}
	// recursive visit
	my_dfs_prop_rec(g, vertex(s, g), ans);
}

struct Visitor : public boost::dfs_visitor<> {
	static std::vector<int> ans;
	template <class Vertex, class Graph>
	void discover_vertex(const Vertex& v, const Graph& g) {
		ans.push_back(v);
	}
};
std::vector<int> Visitor::ans;

int main(int argc, char* argv[]) {
	int N = parse_num(argc, argv, 16);
	int M = N * static_cast<int>(sqrt(sqrt(N))); // num edges
	// make a random graph
	std::random_device rd; // random device
	std::default_random_engine re(rd()); // random engine
	Graph g;
	cout << "random graph generation: " << endl;
	{
		steady_clock::time_point start = steady_clock::now();
		boost::generate_random_graph(g, N, M, re, false, false); // no parallel edge, no self edge
		cout << "\t" << duration_cast<milliseconds>(steady_clock::now() - start).count() << " msec elapsed" << endl;
	}
	my_print(g);
	// my depth first search
	cout << "my depth-first search: " << endl;
	std::vector<int> ans;
	{
		steady_clock::time_point start = steady_clock::now();
		my_dfs(g, 0, ans);
		cout << "\t" << duration_cast<milliseconds>(steady_clock::now() - start).count() << " msec elapsed" << endl;
	}
	cout << "\tans[" << ans.size() << "] = ";
	print(ans);
	// my depth first search, with vertex color property
	cout << "my depth-first search, with vertex color property: " << endl;
	ans.clear();
	{
		steady_clock::time_point start = steady_clock::now();
		my_dfs_prop(g, 0, ans);
		cout << "\t" << duration_cast<milliseconds>(steady_clock::now() - start).count() << " msec elapsed" << endl;
	}
	cout << "\tans[" << ans.size() << "] = ";
	print(ans);
	// boost depth first search
	cout << "boost::depth_first_search: " << endl;
	Visitor vis;
	{
		steady_clock::time_point start = steady_clock::now();
		boost::depth_first_search(g, vis, boost::get(boost::vertex_color, g));
		cout << "\t" << duration_cast<milliseconds>(steady_clock::now() - start).count() << " msec elapsed" << endl;
	}
	cout << "\tans[" << Visitor::ans.size() << "] = ";
	print(Visitor::ans);
	// number of connected components
	std::vector<int> comp(boost::num_vertices(g));
	int num = boost::connected_components(g, &comp[0]);
	cout << "total number of components = " << num << endl;
	// done
	return 0;
}
