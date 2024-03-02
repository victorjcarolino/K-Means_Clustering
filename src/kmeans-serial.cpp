// Implementation of the KMeans Algorithm
// reference: https://github.com/marcoscastro/kmeans

#include <iostream>
#include <vector>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <numeric>
#include <tbb/tbb.h>
#include <mutex>

using namespace std;

class Point
{
private:
	int id_point, id_cluster;
	vector<double> values;
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
};

class Cluster
{
private:
	int id_cluster;
	vector<double> central_values;
	vector<Point> points;
	int total_values;

public:
	Cluster(int id_cluster, Point point)
	{
		this->id_cluster = id_cluster;

		total_values = point.getTotalValues();

		for(int i = 0; i < total_values; i++)
			this->central_values.push_back(point.getValue(i));

		this->points.push_back(point);
	}

	void addPoint(Point point)
	{
		this->points.push_back(point);
	}

	bool removePoint(int id_point)
	{
		int total_points = this->points.size();
		for(int i = 0; i < total_points; i++)
		{
			if(this->points[i].getID() == id_point)
			{
				this->points.erase(this->points.begin() + i);
				return true;
			}
		}
		return false;
	}

	double getCentralValue(int index)
	{
		return this->central_values[index];
	}

	void setCentralValue(int index, double value)
	{
		this->central_values[index] = value;
	}

	Point getPoint(int index)
	{
		return this->points[index];
	}

	// void getAllCentralValues() {
	// 	cout << "Central Values: ";
	// 	for (int i = 0; i < total_values; i++) {
	// 		cout << this->central_values[i] << " ";
	// 	}
	// 	cout << endl;
	// }

	int getTotalPoints()
	{
		return this->points.size();
	}

	int getID()
	{
		return id_cluster;
	}
};

class KMeans
{
private:
	int K; // number of clusters
	int total_values, total_points, max_iterations;
	vector<Cluster> clusters;

	// return ID of nearest center (uses euclidean distance)
	int getIDNearestCenter(Point point)
	{
		double sum = 0.0, min_dist;
		int id_cluster_center = 0;

        // compute the Euclidean distance from each point to the center of the first cluster
		for(int i = 0; i < total_values; i++)
		{
			sum += pow(clusters[0].getCentralValue(i) - point.getValue(i), 2.0);
		}
		min_dist = sqrt(sum);
        
        // compute the distance from the point to the center of each cluster
		for(int i = 1; i < K; i++)
		{
			double dist;
			sum = 0.0;

            // compute the Euclidean distance from each point to the center of the cluster
			for(int j = 0; j < total_values; j++)
			{
				sum += pow(clusters[i].getCentralValue(j) - point.getValue(j), 2.0);
			}
			dist = sqrt(sum);

            // if the distance is less than the minimum distance, update the minimum distance and the ID of the cluster center
			if(dist < min_dist)
			{
				min_dist = dist;
				id_cluster_center = i;
			}
		}

		return id_cluster_center;
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
		vector<std::chrono::microseconds> times1;
		vector<std::chrono::microseconds> times2;
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
					Cluster cluster(i, points[index_point]);
					clusters.push_back(cluster);
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

			auto end_phase2 = chrono::high_resolution_clock::now();

			for(int i = 0; i < total_points; i++)
			{
				int id_old_cluster = points[i].getCluster(); // get the cluster designation of point i
				int id_nearest_center = getIDNearestCenter(points[i]); // calculate the nearest cluster by Euclidian distance of point i

                // if the cluster is anything other than the nearest cluster, remove the point from the old cluster and add it to the nearest cluster
				if(id_old_cluster != id_nearest_center)
				{
					if(id_old_cluster != -1) // this will == -1 when the point has not been assigned a cluster 
					{
						// if the point has already been assigned a cluster, remove it from the old cluster
						// cout << "hit" << endl;
						clusters[id_old_cluster].removePoint(points[i].getID());
					}
					points[i].setCluster(id_nearest_center); // assign the point to a cluster
					clusters[id_nearest_center].addPoint(points[i]); // add the point to the nearest cluster
                    done = false; // set done to false to continue the loop as the clusters were not finalized in successive iterations
				}
			}

			auto end_phase3 = chrono::high_resolution_clock::now();

			// recalculating the center of each cluster
			for(int i = 0; i < K; i++)
			{
				// cout << "Before recalculating central values for cluster " << i << " on iter " << iter << endl;
				// clusters[i].getAllCentralValues();
				for(int j = 0; j < total_values; j++)
				{
					int total_points_cluster = clusters[i].getTotalPoints();
					double sum = 0.0;

					// cout << "Total Points check: " << total_points_cluster << endl;
					if(total_points_cluster > 0)
					{
						for(int p = 0; p < total_points_cluster; p++) {
                            sum += clusters[i].getPoint(p).getValue(j);
                        }
						// cout << "Intermediate value " << j << " before reassigning: " << sum << endl;
						clusters[i].setCentralValue(j, sum / total_points_cluster);
					}
				}
				// cout << "After recalculating central values for cluster " << i << " on iter " << iter << endl;
				// clusters[i].getAllCentralValues();
				// cout << '\n';
			}

			auto end_phase4 = chrono::high_resolution_clock::now();

			if(done == true || iter >= max_iterations)
			{
				cout << "Break in iteration " << iter << "\n\n";
				break;
			}
			
			// cout << "\n";
			// cout << "Iteration: " << iter << endl;
			// cout << "Time to associate each point with the nearest cluster: " << std::chrono::duration_cast<std::chrono::microseconds>(end_phase3-end_phase2).count()<<"\n";
			// times1.push_back(std::chrono::duration_cast<std::chrono::microseconds>(end_phase3-end_phase2));
			// cout << "Time to recalculate the center of each point: " << std::chrono::duration_cast<std::chrono::microseconds>(end_phase4-end_phase3).count()<<"\n";
			// times2.push_back(std::chrono::duration_cast<std::chrono::microseconds>(end_phase4-end_phase3));
			// cout << "\n";

			iter++;
		}
        auto end = chrono::high_resolution_clock::now();
		cout << "TOTAL EXECUTION TIME = "<<std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count()<<"\n\n";

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

			cout << "Cluster values: ";

			for(int j = 0; j < total_values; j++)
				cout << clusters[i].getCentralValue(j) << " ";

			cout << "\n\n";
            // cout << "TIME PHASE 1 = "<<std::chrono::duration_cast<std::chrono::microseconds>(end_phase1-begin).count()<<"\n";
            
            // cout << "TIME PHASE 2 = "<<std::chrono::duration_cast<std::chrono::microseconds>(end-end_phase1).count()<<"\n";

			// cout << "TIME PHASE ASSOCIATE = " << time_phase_associate.count() << endl;
			// cout << "TIME PHASE RECALCULATE = " <<  time_phase_recalculate.count() << endl;
		}
	}
};

int main(int argc, char *argv[])
{
	int total_points, total_values, K, max_iterations, has_name;

	string filename = argv[1];
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
