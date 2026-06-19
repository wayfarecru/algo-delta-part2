#include <iostream>
#include <iomanip>
#include <chrono>
#include <vector>
#include <deque>
#include <random>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/transitive_closure.hpp>
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

// --------------------------------------------------------------------------------

template <typename G>
void my_warren_transitive_closure(G& g) {
	using namespace boost;
	typedef typename graph_traits<G>::vertex_iterator vertex_iterator;
	BOOST_CONCEPT_ASSERT((AdjacencyMatrixConcept<G>));
	BOOST_CONCEPT_ASSERT((EdgeMutableGraphConcept<G>));
	// Make sure second loop will work
	if (num_vertices(g) == 0)
		return;
	// for i = 2 to n
	//    for k = 1 to i - 1
	//      if A[i,k]
	//        for j = 1 to n
	//          A[i,j] = A[i,j] | A[k,j]
	vertex_iterator ic, ie, jc, je, kc, ke;
	for (boost::tie(ic, ie) = vertices(g), ++ic; ic != ie; ++ic)
		for (boost::tie(kc, ke) = vertices(g); *kc != *ic; ++kc)
			if (edge(*ic, *kc, g).second)
				for (boost::tie(jc, je) = vertices(g); jc != je; ++jc)
					if (!edge(*ic, *jc, g).second && edge(*kc, *jc, g).second) {
						add_edge(*ic, *jc, g);
					}
	//  for i = 1 to n - 1
	//    for k = i + 1 to n
	//      if A[i,k]
	//        for j = 1 to n
	//          A[i,j] = A[i,j] | A[k,j]
	for (boost::tie(ic, ie) = vertices(g) /*, --ie */; ic != ie; ++ic) // ``--ie'' is removed !
		for (kc = ic, ke = ie, ++kc; kc != ke; ++kc)
			if (edge(*ic, *kc, g).second)
				for (boost::tie(jc, je) = vertices(g); jc != je; ++jc)
					if (!edge(*ic, *jc, g).second && edge(*kc, *jc, g).second) {
						add_edge(*ic, *jc, g);
					}
}


typedef struct {
	std::string name;
} VertProp;
typedef boost::adjacency_list<vecS, vecS, bidirectionalS, VertProp, no_property> Graph;

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

int main(int argc, char* argv[]) {
	// make a target graph : STAR-shape in CLRS
	enum { A, B, C, D, N }; // N is the number of vertices
	Graph g(N);
	g[A].name = "A";
	g[B].name = "B";
	g[C].name = "C";
	g[D].name = "D";
	add_edge(A, A, g);
	add_edge(B, B, g);
	add_edge(C, C, g);
	add_edge(D, D, g);
	add_edge(B, C, g);
	add_edge(B, D, g);
	add_edge(C, B, g);
	add_edge(D, A, g);
	add_edge(D, C, g);
	my_print(g);
	// transitive closure
	cout << "transitive closure algorithm:" << endl;
	Graph tc;
	boost::transitive_closure(g, tc);
	tc[A].name = "A";
	tc[B].name = "B";
	tc[C].name = "C";
	tc[D].name = "D";
	my_print(tc);
	// warshall transitive closure
	cout << "warshall transitive closure algorithm:" << endl;
	tc.clear();
	boost::copy_graph(g, tc);
	my_print(tc);
	boost::warshall_transitive_closure(tc);
	my_print(tc);
	// warren transitive closure
	cout << "warren transitive closure algorithm:" << endl;
	tc.clear();
	boost::copy_graph(g, tc);
	my_print(tc);
	boost::warren_transitive_closure(tc);
	my_print(tc);
	// corrected warren transitive closure
	cout << "corrected my warren transitive closure algorithm:" << endl;
	tc.clear();
	boost::copy_graph(g, tc);
	my_print(tc);
	my_warren_transitive_closure(tc);
	my_print(tc);
	// done
	return 0;
}
