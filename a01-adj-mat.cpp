#include <iostream>
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/graph_utility.hpp> // print_vertices(), ...
using namespace std;
using namespace boost;
#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wsign-compare"
#endif
#if defined(_MSC_VER)
#pragma warning(disable: 4388 4389)
#endif

typedef boost::adjacency_matrix<boost::undirectedS> Graph;
typedef boost::graph_traits<Graph>::vertex_descriptor VertDesc;

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
	cout << "num vert = " << boost::num_vertices(g) << endl;
	cout << "num edge = " << boost::num_edges(g) << endl;
	const int ind = 2;
	cout << "selecting vertex number " << ind << ":" << endl;
	VertDesc vd = boost::vertex(ind, g);
	cout << "in_degree = " << boost::in_degree(vd, g) << endl;
	cout << "out_degree = " << boost::out_degree(vd, g) << endl;
	// utility functions
	std::string name[] = { "A", "B", "C", "D", "E", "F", "G", "H" };
	std::cout << "vertex set: ";
	boost::print_vertices(g, name);
	std::cout << "edge set: ";
	boost::print_edges(g, name);
	std::cout << "out-edges: " << std::endl;
	boost::print_graph(g, name);
	// done
	return 0;
}
