#include <iostream>
#include <chrono>
#include <vector>
#include <deque>
#include <random>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/kruskal_min_spanning_tree.hpp>
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

typedef struct {
	std::string name;
} VertProp;
typedef struct {
	int w; // weight for each edge
} EdgeProp;
typedef boost::adjacency_list<vecS, vecS, undirectedS, VertProp, EdgeProp> Graph;
typedef boost::graph_traits<Graph>::vertex_descriptor VertDesc;
typedef boost::graph_traits<Graph>::edge_descriptor EdgeDesc;
typedef boost::property_map<Graph, boost::vertex_index_t>::type VertIndMap;

typedef std::map<VertDesc, int> Rank;
typedef std::map<VertDesc, VertDesc> Parent;
typedef boost::associative_property_map<Rank> RankPropMap;
typedef boost::associative_property_map<Parent> ParentPropMap;

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
			auto e = edge(*it, *jt, g); // it returns (edge_descriptor, bool)
			cout << g[*jt].name << "(" << g[e.first].w << ") ";
		}
		cout << ((num > 16) ? ". . ." : "") << endl;
	}
	if (boost::num_vertices(g) > 16) {
		cout << "\t. . ." << endl;
	}
}

int main(int argc, char* argv[]) {
	// make a target graph
	enum { A, B, C, D, E, F, G, H, I, N }; // N is 9, number of vertices
	Graph g(N);
	g[A].name = "a";
	g[B].name = "b";
	g[C].name = "c";
	g[D].name = "d";
	g[E].name = "e";
	g[F].name = "f";
	g[G].name = "g";
	g[H].name = "h";
	g[I].name = "i";
	add_edge(A, B, EdgeProp { 4 }, g);
	add_edge(A, H, EdgeProp { 8 }, g);
	add_edge(B, C, EdgeProp { 8 }, g);
	add_edge(B, H, EdgeProp { 11 }, g);
	add_edge(C, D, EdgeProp { 7 }, g);
	add_edge(C, F, EdgeProp { 4 }, g);
	add_edge(C, I, EdgeProp { 2 }, g);
	add_edge(D, E, EdgeProp { 9 }, g);
	add_edge(D, F, EdgeProp { 14 }, g);
	add_edge(E, F, EdgeProp { 10 }, g);
	add_edge(F, G, EdgeProp { 2 }, g);
	add_edge(G, H, EdgeProp { 1 }, g);
	add_edge(G, I, EdgeProp { 6 }, g);
	my_print(g);
	// kruskal's algorithm
	cout << "Kruskal's MST algorithm:" << endl;
	std::vector<EdgeDesc> mstree;
	Rank rank_map;
	Parent parent_map;
	RankPropMap rank_pmap(rank_map);
	ParentPropMap parent_pmap(parent_map);
	kruskal_minimum_spanning_tree(g, std::back_inserter(mstree),
	                              weight_map(boost::get(&EdgeProp::w, g))
	                              .rank_map(rank_pmap)
	                              .predecessor_map(parent_pmap)
	                              .vertex_index_map(boost::get(vertex_index, g))
	                             );
	cout << "\t" << mstree.size() << " edges: ";
	int num_show = 16;
	int sum_w = 0;
	for (auto it = mstree.begin(); it != mstree.end() && num_show > 0; ++it, --num_show) {
		cout << "(" << g[source(*it, g)].name << "," << g[target(*it, g)].name << ") ";
		sum_w += g[*it].w;
	}
	cout << ((mstree.size() > num_show) ? ". . ." : "") << endl;
	cout << "total weight for MST = " << sum_w << endl;
	// done
	return 0;
}
