#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_utility.hpp> // print_vertices(), ...
using namespace std;
using namespace boost;
#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wsign-compare"
#endif
#if defined(_MSC_VER)
#pragma warning(disable: 4388 4389)
#endif

typedef boost::adjacency_list<boost::listS, boost::vecS, boost::bidirectionalS> Graph;
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
		cout << "\t" << vertIndMap[*it] << " --> ";
		auto adjp = boost::adjacent_vertices(*it, g); // all adjacent vertices
		for (auto jt = adjp.first; jt != adjp.second; ++jt) {
			cout << vertIndMap[*jt] << " ";
		}
		cout << endl;
	}
	if (boost::num_vertices(g) > 16) {
		cout << "\t. . ." << endl;
	}
}

int main(int argc, char* argv[]) {
	// make a graph
	const int N = 8; // number of vertices
	Graph g(N); // and 10 edges
	add_edge(0, 1, g);
	add_edge(0, 2, g);
	add_edge(0, 4, g);
	add_edge(1, 3, g);
	add_edge(2, 3, g);
	add_edge(2, 4, g);
	add_edge(2, 5, g);
	add_edge(3, 5, g);
	add_edge(4, 5, g);
	add_edge(6, 7, g);
	// now print
	cout << "graph: ";
	cout << "num vert = " << boost::num_vertices(g);
	cout << ", num edge = " << boost::num_edges(g) << endl;
	// for a vertex
	const int ind = 2;
	cout << "selecting vertex number " << ind << ":" << endl;
	VertDesc vd = boost::vertex(ind, g);
	cout << "in_degree = " << boost::in_degree(vd, g) << endl; // unsupported for <directredS> graph
	cout << "out_degree = " << boost::out_degree(vd, g) << endl;
	// utility functions
	std::string name[] = { "A", "B", "C", "D", "E", "F", "G", "H" };
	std::cout << "vertex set: ";
	boost::print_vertices(g, name);
	std::cout << "edge set: ";
	boost::print_edges(g, name); // unsupported for directed graph
	std::cout << "out-edges: " << std::endl;
	boost::print_graph(g, name);
	// my print function
	my_print(g, "my graph");
	// done 
	return 0;
}
