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
typedef boost::adjacency_list<listS, vecS, directedS, VertProp> Graph;
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

void my_top_sort_rec(Graph& g, const VertDesc& vd, std::deque<int>& ans) {
	VertIndMap vertIndMap = boost::get(boost::vertex_index, g);
	VertColorMap vertColorMap = boost::get(boost::vertex_color, g);
	vertColorMap[vd] = boost::gray_color; // discovered
	auto adjp = adjacent_vertices(vd, g);
	for (auto it = adjp.first; it != adjp.second; ++it) {
		if (vertColorMap[*it] == boost::white_color) {
			my_top_sort_rec(g, *it, ans);
		}
	}
	vertColorMap[vd] = boost::black_color; // finished
	ans.push_front(vertIndMap[vd]);
}

void my_top_sort(Graph& g, std::deque<int>& ans) {
	VertColorMap vertColorMap = boost::get(boost::vertex_color, g);
	auto vp = boost::vertices(g);
	// reset all vertex colors to white
	for (auto it = vp.first; it != vp.second; ++it) {
		vertColorMap[*it] = boost::white_color; // white means not-visited
	}
	// recursive visit
	for (auto it = vp.first; it != vp.second; ++it) {
		if (vertColorMap[*it] == boost::white_color) {
			my_top_sort_rec(g, *it, ans);
		}
	}
}

int main(int argc, char* argv[]) {
	// make a directed graph
	const int N = 11; // num vert
	Graph g(N);
	add_edge(1, 2, g);
	add_edge(2, 3, g);
	add_edge(2, 5, g);
	add_edge(4, 1, g);
	add_edge(4, 7, g);
	add_edge(5, 8, g);
	add_edge(6, 3, g);
	add_edge(6, 9, g);
	add_edge(7, 8, g);
	add_edge(8, 9, g);
	add_edge(10, 8, g);
	my_print(g);
	// my topological sort
	cout << "my topological sort " << endl;
	std::deque<int> ans;
	{
		steady_clock::time_point start = steady_clock::now();
		my_top_sort(g, ans);
		cout << "\t" << duration_cast<milliseconds>(steady_clock::now() - start).count() << " msec elapsed" << endl;
	}
	cout << "\tans[" << ans.size() << "] = ";
	print(ans);
	// boost topological sort
	cout << "boost::topological_sort: " << endl;
	std::vector<VertDesc> ans2;
	{
		steady_clock::time_point start = steady_clock::now();
		boost::topological_sort(g, std::back_inserter(ans2));
		cout << "\t" << duration_cast<milliseconds>(steady_clock::now() - start).count() << " msec elapsed" << endl;
	}
	VertIndMap vertIndMap = boost::get(boost::vertex_index, g); // VertDesc --> index
	cout << "\tans[" << ans2.size() << "] = ";
	for (std::vector<VertDesc>::reverse_iterator it = ans2.rbegin(); it != ans2.rend(); ++it) {
		cout << vertIndMap[*it] << " ";
	}
	cout << endl;
	// done
	return 0;
}
