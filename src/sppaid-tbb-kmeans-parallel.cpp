// Implementation of the KMeans Algorithm

#include <iostream>
#include <vector>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <tbb/tbb.h>
#include <tbb/cache_aligned_allocator.h>

using namespace std;

/*

Need a point_addr class which has:
- constructor which constructs an identity element for point addition
- q+=p operator which adds the two operands
- p/n operator which returns a point q such that if n copies of q are added together, the result will be p 

- 
*/

int TOTAL_VALS;

class Point {
  public:
	vector<double, tbb::cache_aligned_allocator<double>> values;

	Point(int total_vals) : values(total_vals) {};

	Point& operator+=(Point p) {
		if (values.size() != p.values.size()) {
			throw std::invalid_argument("Points must have the same dimensionality");
		}

		tbb::parallel_for(tbb::blocked_range<size_t>(0, values.size()),
            [&](const tbb::blocked_range<size_t>& r) {
                for(size_t i = r.begin(); i != r.end(); ++i) {
                    values[i] += p.values[i];
                }
            }
        );

        return *this;
	}	

	Point& operator/(size_t n) {
		tbb::parallel_for(tbb::blocked_range<size_t> (0, values.size()),
		[&](const tbb::blocked_range<size_t>& r) {
			for (size_t i = r.begin(); i != r.end(); ++i) {
				values[i] /= n;
			}
		});

		return *this;
	}
};

double distance2(Point centroid, Point p) {
	double sum = 0.0;
	for (size_t i = 0; i < p.values.size(); i++) {
		sum += pow(centroid.values[i] - p.values[i], 2.0);
	}
	return sum;
}

struct sum_and_count {
	sum_and_count() : sum(TOTAL_VALS), count(0) {}
	Point sum;
	size_t count;

	void clear() {
		sum = Point(TOTAL_VALS);
		count = 0;
	}

	void tally(const Point& p) {
		sum += p;
		++count;
	}

	Point mean() {
		return sum / count;
	}

	void operator+=(const sum_and_count& other) {
		sum += other.sum;
		count += other.count;
	}
};

class View {
	View(const View& v);
	void operator+=(const View& v);
  public:
	sum_and_count* array;
	size_t change;
	View(size_t k) : array(new sum_and_count[k]), change(0) {}
	~View() {
		delete[] array;
	}
};

typedef tbb::enumerable_thread_specific<View> tls_type;

void reduce_local_counts_to_global_counts(tls_type& tls, View& global) {
	global.change = 0;
	for (auto i = tls.begin(); i != tls.end(); ++i) {
		View& v = *i;
		global.change += v.change;
		v.change = 0;
	}
}

void reduce_local_sums_to_global_sum(size_t k, tls_type& tls, View& global) {
	for (auto i = tls.begin(); i != tls.end(); ++i) {
		View& v = *i;
		for (size_t j = 0; j < k; ++j) {
			global.array[j] += v.array[j];
			v.array[j].clear();
		}
	}
}

int reduce_min_ind(const Point centroid[], size_t k, Point value) {
	int min = -1;
	double min_d = std::numeric_limits<double>::max();
	for (int j = 0; j < k; ++j) {
		double dist = distance2(centroid[j], value);
		if (dist < min_d) {
			min_d = dist;
			min = j;
		}
	}
	return min;
}

void repair_empty_clusters(int n, Point points[], int cluster_ids[], int k, Point centroids[], sum_and_count array[]) {
// check for empty cluster
	// check all cluster_ids and find how many distinct ones are found
	// if distinct clusters in cluster_ids < k
		// find the greatest distance from one random point in the largest cluster to another point in the largest cluster
		// assign the second point to the empty cluster
		// repeat until all clusters are non-empty
	vector<int> cluster_element_count(k);
	vector<int> empty_clusters(k);
	int max_cluster = 0;
	for (int i = 0; i < cluster_ids.size(); i++) {
		cluster_element_count[cluster_ids[i]]++;
	}
	for (auto i = 0; i != cluster_element_count.size(); i++) {
		if (cluster_element_count[i] == 0) {
			empty_clusters[i] = 
		}
	}

	
}

void compute_k_means(size_t n, const Point points[], size_t k, int cluster_ids[], Point centroids[]) {
	tls_type tls([&]{return k;});
	View global(k);

	// create initial clusters and compute their sums
	tbb::parallel_for(
		tbb::blocked_range<size_t>(0,n),
		[=, &tls, &global](tbb::blocked_range<size_t> r) {
			View& v = tls.local();
			for (size_t i = r.begin(); i != r.end(); ++i) {
				cluster_ids[i] = i % k;
				v.array[cluster_ids[i]].tally(points[i]);
			}
		}
	);

	// loop until ids do not change
	size_t change;
	do {
		// reduce local sums to global sum
		reduce_local_sums_to_global_sum(k, tls, global);

		// repair any empty clusters

	} while (global.change != 0);
}

int main(int argc, char *argv[])
{
	int total_points, total_values, K, max_iterations, has_name;

	string filename = "datasets/";
    string dataset = argv[1];
    filename.insert(filename.end(), dataset.begin(), dataset.end());

	ifstream inputFile(filename);

	if (!inputFile) {
		std::cerr << "Unable to open file";
		exit(1);
	}

	inputFile >> total_points;
	inputFile >> total_values;
	inputFile >> K;
	inputFile >> max_iterations;
	inputFile >> has_name;

	/* da content */
	return 0;
}
