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

typedef boost::disjoint_sets<int*,int*> DisjSets;

void print(DisjSets& ds, int N) {
	std::vector<std::vector<int>> aads(N); // array of array representations of disjoint sets
	for (int i = 0; i < N; ++i) {
		aads[ds.find_set(i)].push_back(i);
	}
	for (int i = 0; i < N; ++i) {
		if (aads[i].size() > 0) {
			cout << "{ " << aads[i][0];
			for (int j = 1; j < aads[i].size(); ++j) {
				cout << "," << aads[i][j];
			}
			cout << " } ";
		}
	}
}

int main(int argc, char* argv[]) {
	const int N = parse_num(argc, argv, 16); // number of disjoint sets
	std::vector<int> rank(N);
	std::vector<int> parent(N);
	DisjSets ds(&rank[0], &parent[0]); // disjoint sets
	// make sets
	for (int i = 0; i < N; ++i) {
		ds.make_set(i);
	}
	// union, randomly
	std::random_device rd; // random device
	std::default_random_engine re(rd()); // random engine
	std::uniform_int_distribution<int> unif_dist(0, N - 1);
	for (int k = 0; k < N; ++k) {
		int i = unif_dist(re);
		int j = unif_dist(re);
		ds.union_set(i, j); // even i == j, no problem
	}
	// output
	cout << "disjoint sets:" << endl;
	cout << "\t";
	print(ds, N);
	cout << endl;
	cout << "rank:" << endl;
	cout << "\t";
	print(rank);
	cout << "parent:" << endl;
	cout << "\t";
	print(parent);
	// done
	return 0;
}
