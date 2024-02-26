// Implementation of the KMeans Algorithm

#include <iostream>
#include <vector>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <limits>
#include <tbb/tbb.h>
#include <tbb/cache_aligned_allocator.h>
#include <tbb/enumerable_thread_specific.h>
#include <tbb/concurrent_vector.h>

using namespace std;

class Point
{
private:
	int id_point, id_cluster;
	hash_map<int, vector<vector<double>>, tbb::cache_aligned_allocator> vals;
	vector<double, tbb::cache_aligned_allocator<double>> values;
	int total_values;
	string name;

public:
	Point(int id_point, vector<double>& values, string name = "")
	{
		this->id_point = id_point;
		total_values = values.size();

		for(int i = 0; i < total_values; i++)
			this->values.push_back(values[i]);

		this->name = name;
		id_cluster = -1;
	}

	int getID()
	{
		return id_point;
	}

	void setCluster(int id_cluster)
	{
		this->id_cluster = id_cluster;
	}

	int getCluster()
	{
		return id_cluster;
	}

	double getValue(int index)
	{
		return values[index];
	}

	int getTotalValues()
	{
		return total_values;
	}

	void addValue(double value)
	{
		values.push_back(value);
	}

	string getName()
	{
		return name;
	}

	vector<double> computeClusterSums(int K, int total_points)
	{
		vector<double> sums(K);
		for (int i = total_points)
	}	
};

// class Cluster
// {
// private:
// 	int id_cluster;
// 	vector<double, tbb::cache_aligned_allocator<double>> central_values;
// 	vector<Point> points;

// public:
// 	Cluster(int id_cluster, Point point)
// 	{
// 		this->id_cluster = id_cluster;

// 		int total_values = point.getTotalValues();

// 		for(int i = 0; i < total_values; i++)
// 			central_values.push_back(point.getValue(i));

// 		points.push_back(point);
// 	}

// 	void addPoint(Point point)
// 	{
// 		points.push_back(point);
// 	}

// 	bool removePoint(int id_point)
// 	{
// 		int total_points = points.size();

// 		for(int i = 0; i < total_points; i++)
// 		{
// 			if(points[i].getID() == id_point)
// 			{
// 				points.erase(points.begin() + i);
// 				return true;
// 			}
// 		}
// 		return false;
// 	}

// 	double getCentralValue(int index)
// 	{
// 		return central_values[index];
// 	}

// 	void setCentralValue(int index, double value)
// 	{
// 		central_values[index] = value;
// 	}

// 	Point getPoint(int index)
// 	{
// 		return points[index];
// 	}

// 	int getTotalPoints()
// 	{
// 		return points.size();
// 	}

// 	int getID()
// 	{
// 		return id_cluster;
// 	}
// };

class KMeans
{
private:
	int K; // number of clusters
	int total_values, total_points, max_iterations;
	vector<Point> centroids;
	// vector<Cluster> clusters;


	// return ID of nearest center (uses euclidean distance)
	int getIDNearestCenter(Point point)
	{

		double sum = 0.0; 
		double min_dist;
		int id_cluster_center = 0;

        // compute the Euclidean distance from each point to the center of the first cluster
		for (size_t i = 0; i < total_values; i++) {
			//sum += pow(clusters[0].getCentralValue(i) - point.getValue(i), 2.0);
			sum += pow(centroids[0].getValue(i) - point.getValue(i), 2.0);
		}

		min_dist = sum;

        // compute the distance from the point to the center of each cluster
		
		// tbb::concurrent_vector<pair<double, int>> d_c(K);

		// d_c.push_back(make_pair(min_dist, 0));

		tbb::parallel_for(tbb::blocked_range<int>(1, K), [&](const tbb::blocked_range<int>& r) {
			for (int i = r.begin(); i < r.end(); i++) {
				double dist = 0.0;

				for (int j = 0; j < total_values; j++) {
					// double diff = clusters[i].getCentralValue(j) - point.getValue(j);
					double diff = centroids[i].getValue(j) - point.getValue(j);
					dist += pow(diff, 2.0);
				};

				// d_c.push_back(make_pair(dist, i));

				if (dist < min_dist) {
					min_dist = dist;
					id_cluster_center = i;
				}
			}
		});

		// sort(d_c.begin(), d_c.end(),
		// 	[](const auto& a, const auto& b) {
		// 		return a.first < b.first;
		// 	}
		// );

		return id_cluster_center;		
		// return d_c[0].second;
	}

public:
	KMeans(int K, int total_points, int total_values, int max_iterations)
	{
		this->K = K;
		this->total_points = total_points;
		this->total_values = total_values;
		this->max_iterations = max_iterations;
	}

	void run(vector<Point> & points)
	{
		vector<atomic<int>> cluster_sizes(K);
        auto begin = chrono::high_resolution_clock::now();
        
		if(K > total_points)
			return;

		vector<int> prohibited_indexes;

		// choose K distinct values for the centers of the clusters
		for(int i = 0; i < K; i++)
		{
            srand(i); // seed the execution for now to standardize execution in testing
            // Stop the loop when a new cluster center is chosen
			while(true)
			{
				int index_point = rand() % total_points;

				if(find(prohibited_indexes.begin(), prohibited_indexes.end(), index_point) == prohibited_indexes.end())
				{
					prohibited_indexes.push_back(index_point);
					points[index_point].setCluster(i);
					cluster_sizes[i]++;
					//Cluster cluster(i, points[index_point]);
					//clusters.push_back(cluster);
					break;
				}
			}
		}
        auto end_phase1 = chrono::high_resolution_clock::now();
        
		int iter = 1;

        // Stop the loop when the maximum number of iterations is reached or the points are assigned to the nearest cluster center
		while(true)
		{
			bool done = true;

			// associates each point to the nearest center
			for(int i = 0; i < total_points; i++)
			{
				int id_old_cluster = points[i].getCluster(); // get the cluster designation of point i
				int id_nearest_center = getIDNearestCenter(points[i]); // calculate the nearest cluster by Euclidian distance of point i

                // if the cluster is anything other than the nearest cluster, remove the point from the old cluster and add it to the nearest cluster
				if(id_old_cluster != id_nearest_center)
				{
					// if(id_old_cluster != -1) // this will == -1 when the point has not been assigned a cluster 
					// {
					// 	// if the point has already been assigned a cluster, remove it from the old cluster
					// 	clusters[id_old_cluster].removePoint(points[i].getID());
					// }
                        
					points[i].setCluster(id_nearest_center); // assign the point to a cluster
					cluster_sizes[id_old_cluster]--;
					cluster_sizes[id_nearest_center];

					//clusters[id_nearest_center].addPoint(points[i]); // add the point to the nearest cluster
                    done = false; // set done to false to continue the loop as the clusters were not finalized in successive iterations
				}
			}



			// recalculating the center of each cluster
			tbb::parallel_for(tbb::blocked_range<size_t>(0, K),
				[&](const tbb::blocked_range<size_t>& r) {
					for (size_t i = r.begin(); i < r.end(); i++) {
						for(int j = 0; j < total_values; j++)
						{
							//int total_points_cluster = clusters[i].getTotalPoints();
							int total_points_cluster = cluster_sizes[i];
							double sum = 0.0;
							
							if(total_points_cluster > 0)
							{
								//compute the Euclidean distance from each point to the center of the cluster
								for(int p = 0; p < total_points_cluster; p++) {
									// sum += clusters[i].getPoint(p).getValue(j);
									sum += 
								}

								// set the central value of a cluster
								clusters[i].setCentralValue(j, sum / total_points_cluster);
							}
						}
					}
				}
			);

			if(done == true || iter >= max_iterations)
			{
				cout << "Break in iteration " << iter << "\n\n";
				break;
			}

			iter++;
		}
        auto end = chrono::high_resolution_clock::now();

		// shows elements of clusters
		for(int i = 0; i < K; i++)
		{
			int total_points_cluster =  clusters[i].getTotalPoints();

			// cout << "Cluster " << clusters[i].getID() + 1 << endl;
			// for(int j = 0; j < total_points_cluster; j++)
			// {
			// 	cout << "Point " << clusters[i].getPoint(j).getID() + 1 << ": ";
			// 	for(int p = 0; p < total_values; p++)
			// 		cout << clusters[i].getPoint(j).getValue(p) << " ";

			// 	string point_name = clusters[i].getPoint(j).getName();

			// 	if(point_name != "")
			// 		cout << "- " << point_name;

			// 	cout << endl;
			// }

			cout << "Cluster centroid values: ";

			for(int j = 0; j < total_values; j++) {
				// cout << clusters[i].getCentralValue(j) << " ";
				cout << centroids[i].getValue(j) << " ";
			}

			cout << "\n\n";
            cout << "TOTAL EXECUTION TIME = "<<std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count()<<"\n";
            
            cout << "TIME PHASE 1 = "<<std::chrono::duration_cast<std::chrono::microseconds>(end_phase1-begin).count()<<"\n";
            
            cout << "TIME PHASE 2 = "<<std::chrono::duration_cast<std::chrono::microseconds>(end-end_phase1).count()<<"\n";
		}
	}
};

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

	vector<Point> points;
	string point_name;

	for(int i = 0; i < total_points; i++)
	{
		vector<double> values;

		for(int j = 0; j < total_values; j++)
		{
			double value;
			inputFile >> value;
			values.push_back(value);
		}

		if(has_name)
		{
			inputFile >> point_name;
			Point p(i, values, point_name);
			points.push_back(p);
		}
		else
		{
			Point p(i, values);
			points.push_back(p);
		}
	}

	KMeans kmeans(K, total_points, total_values, max_iterations);
	kmeans.run(points);

	return 0;
}
