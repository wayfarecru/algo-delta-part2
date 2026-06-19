#include <iostream>
#include <chrono>
#include <vector>
#include <deque>
#include <random>
#include <boost/pending/disjoint_sets.hpp>
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

typedef std::map<std::string,int> Rank;
typedef std::map<std::string,std::string> Parent;
typedef boost::associative_property_map<Rank> RankPropMap;
typedef boost::associative_property_map<Parent> ParentPropMap;
typedef boost::disjoint_sets<RankPropMap,ParentPropMap> DisjSets;

void print(DisjSets& ds, const std::vector<std::string>& elems) {
	std::deque<bool> visit(elems.size(), false);
	for (int i = 0; i < elems.size(); ++i) {
		if (visit[i] == false) {
			visit[i] = true;
			std::string rep = ds.find_set(elems[i]); // representative member for i
			cout << "{ " << elems[i];
			for (int j = i + 1; j < elems.size(); ++j) {
				std::string rep_j = ds.find_set(elems[j]); // representative member for j
				if (rep == rep_j) {
					visit[j] = true;
					cout << "," << elems[j];
				}
			}
			cout << " } ";
		}
	}
}

int main(int argc, char* argv[]) {
	Rank rank_map;
	Parent parent_map;
	RankPropMap rank_pmap(rank_map);
	ParentPropMap parent_pmap(parent_map);
	DisjSets ds(rank_pmap, parent_pmap); // disjoint sets
	// make sets
	std::vector<std::string> elems = { "a", "b", "c", "d", "e", "f", "g" };
	for (auto it = elems.begin(); it != elems.end(); ++it) {
		ds.make_set(*it);
	}
	ds.union_set(elems[0], elems[1]);
	ds.union_set(elems[3], elems[4]);
	ds.union_set(elems[4], elems[5]);
	// output
	cout << "elements:" << endl;
	cout << "\t";
	print(elems);
	cout << "disjoint sets:" << endl;
	cout << "\t";
	print(ds, elems);
	cout << endl;
	// done
	return 0;
}
