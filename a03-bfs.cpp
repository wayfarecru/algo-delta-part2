#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/properties.hpp> // white_color, gray_color, black_color
#include <boost/graph/breadth_first_search.hpp> // for breadth_first_search()
using namespace std;
using namespace boost;
#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wsign-compare"
#endif
#if defined(_MSC_VER)
#pragma warning(disable: 4388 4389)
#endif

typedef boost::property<boost::vertex_color_t, boost::default_color_type> VertProp;
typedef boost::adjacency_list<listS, vecS, undirectedS, VertProp> Graph;
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

void my_bfs(const Graph& g, int s) {
	VertIndMap vertIndMap = boost::get(boost::vertex_index, g);
	int n = boost::num_vertices(g);
	std::deque<int> viq; // vertex index queue
	std::deque<bool> visit(n, false); // vert_ind -> visited or not
	// start with s vertex
	visit[s] = true;
	viq.push_back(s);
	cout << s << " ";
	// main loop
	while (! viq.empty()) {
		int vi = viq.front();
		viq.pop_front();
		auto vp = boost::adjacent_vertices(vi, g);
		for (auto it = vp.first; it != vp.second; ++it) {
			int ind = vertIndMap[*it];
			if (visit[ind] == false) {
				visit[ind] = true;
				viq.push_back(ind);
				cout << ind << " ";
			}
		}
	}
}

void my_bfs_color(Graph& g, int s) {
	VertIndMap vertIndMap = boost::get(boost::vertex_index, g);
	VertColorMap vertColorMap = boost::get(boost::vertex_color, g);
	// reset all colors to white
	std::pair<Graph::vertex_iterator, Graph::vertex_iterator> vp = boost::vertices(g);
	for (Graph::vertex_iterator it = vp.first; it != vp.second; ++it) {
		vertColorMap[*it] = boost::white_color; // white means not processed
	}
	// process vertices
	VertDesc vd = boost::vertex(s, g);
	std::deque<VertDesc> vdq; // vertex description queue
	vdq.push_back(vd);
	cout << vertIndMap[vd] << " ";
	vertColorMap[vd] = boost::gray_color; // gray means in the queue
	while (! vdq.empty()) {
		VertDesc vd = vdq.front();
		vdq.pop_front();
		auto adjp = boost::adjacent_vertices(vd, g); // adjacent vertex pair
		for (auto it = adjp.first; it != adjp.second; ++it) {
			if (vertColorMap[*it] == boost::white_color) {
				vdq.push_back(*it);
				cout << vertIndMap[*it] << " ";
				vertColorMap[*it] = boost::gray_color; // gray means in the queue
			}
		}
		vertColorMap[vd] = boost::black_color; // black means out of the queue
	}
}

struct Visitor : public boost::default_bfs_visitor {
	template <class Vertex, class Graph>
	void discover_vertex(const Vertex& v, const Graph& g) {
		cout << v << " ";
	}
};

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
	// show the graph
	my_print(g);
	// my breadth first search
	cout << "my breadth first search: " << endl;
	my_bfs(g, 0);
	cout << endl;
	// my breadth first search, with vertex color property
	cout << "my breadth first search, with vertex color property: " << endl;
	my_bfs_color(g, 0);
	cout << endl;
	// boost breadth first search
	cout << "boost::breadth_first_search: " << endl;
	Visitor vis;
	boost::breadth_first_search(g, boost::vertex(0, g), boost::visitor(vis));
	cout << endl;
	// done
	return 0;
}
