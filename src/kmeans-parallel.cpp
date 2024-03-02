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
#include <tbb/tbb_allocator.h>
#include <tbb/enumerable_thread_specific.h>
#include <atomic>


using namespace std;

const int constK = 32;
const int const_total_values = 3;

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
		this->id_cluster = -1;
	}

	int getID()
	{
		return this->id_point;
	}

	void setCluster(int id_cluster)
	{
		this->id_cluster = id_cluster;
	}

	int getCluster()
	{
		return this->id_cluster;
	}

	double getValue(int index)
	{
		return this->values[index];
	}

	int getTotalValues()
	{
		return total_values;
	}

	void addValue(double value)
	{
		this->values.push_back(value);
	}

	string getName()
	{
		return this->name;
	}
};

class View {
  private:
	struct view {
		vector<int> total_points; // keep track of total points;
		int change; // keep track of the number of changes

		// keep track of intermediate_central_values
			// 1 vector level for every cluster a thread 
			// may encounter and another vector level for multiple values of a point 
		vector<vector<double>> intermediate_central_values; 
	};
	view view;
  public:
	View() {
		this->view.total_points = vector<int>(constK);
		for (int i = 0; i < constK; i++) {
			this->view.total_points[i] = 0;
		}
		this->view.change = 0;
		this->view.intermediate_central_values = vector<vector<double>>(constK);
		for (int i = 0; i < constK; i++) {
			this->view.intermediate_central_values[i] = vector<double>(const_total_values, 0);
		}
	}

	void getAllIntermediateValues() {
		for(int i = 0; i < constK; i++) {
			for(int j = 0; j < const_total_values; j++) {
				cout << this->view.intermediate_central_values[i][j] << " ";
			}
		}
	}

	vector<double> getIntermediateCentralValues(int index) {
		return this->view.intermediate_central_values[index];
	}

	int getTotalPoints(int index) {
		return this->view.total_points[index];
	}

	void addPoint(Point point, int clusterId) {
		this->view.total_points[clusterId]++;
		for (int i = 0; i < const_total_values; i++) {
			this->view.intermediate_central_values[clusterId][i] += point.getValue(i);
		}
		this->view.change++;
	}

	void removePoint(Point point, int clusterId) {
		this->view.total_points[clusterId]--;
		for (int i = 0; i < const_total_values; i++) {
			this->view.intermediate_central_values[clusterId][i] -= point.getValue(i);
		}
		this->view.change++;
	}

	int getChange() {
		return this->view.change;
	}

	void reset() {
		for (int i = 0; i < constK; i++) {
			this->view.total_points[i] = 0;
			for (int j = 0; j < const_total_values; j++) {
				this->view.intermediate_central_values[i][j] = 0;
			}
		}
		this->view.change = 0;
	}

};

class Cluster
{
private:
	int id_cluster;
	vector<double, tbb::tbb_allocator<double>> central_values;
	vector<double> intermediate_central_values;
	int total_points = 0;
	int total_values;
	int change;

public:
	Cluster(int id_cluster, Point point)
	{
		this->id_cluster = id_cluster;
		this->change = 0;

		total_values = point.getTotalValues();
		
		this->total_points++;

		for(int i = 0; i < total_values; i++) {
			this->intermediate_central_values.push_back(point.getValue(i));
			this->central_values.push_back(point.getValue(i));
		}
	}

	double getCentralValue(int index) {
		return this->central_values[index];
	}

	void getAllIntermediateValues() {
		for (int i = 0; i < total_values; i++) {
			cout << this->intermediate_central_values[i] << " ";
		}
		cout << "\n\n";
	}

	void getAllCentralValues() {
		for (int i = 0; i < total_values; i++) {
			cout << this->central_values[i] << " ";
		}
		cout << "\n\n";
	}

	void setCentralValues() {
		for (int i = 0; i < total_values; i++) {
			this->central_values[i] = this->intermediate_central_values[i] / this->total_points;
		}
	}

	int getTotalPoints()
	{
		return this->total_points;
	}

	int getID()
	{
		return this->id_cluster;
	}

	Cluster& operator+=(View view) {
		int index = this->id_cluster;
		this->total_points += view.getTotalPoints(index);
		this->change += view.getChange();
		vector<double> intermediates = view.getIntermediateCentralValues(index);
		for (int j = 0; j < total_values; j++) {
			this->intermediate_central_values[j] += intermediates[j];
		}
		return *this;
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
		min_dist = sum;
        
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
			dist = sum;

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
		//tbb::enumerable_thread_specific<vector<Cluster>> cluster_tls(clusters.begin(), clusters.end());
		tbb::enumerable_thread_specific<int> id_old_cluster_tls;
		tbb::enumerable_thread_specific<int> id_nearest_center_tls;
		tbb::enumerable_thread_specific<View> tls_views;
		bool not_done = true;
	
        // Stop the loop when the maximum number of iterations is reached or the points are assigned to the nearest cluster center
		do
		{
			// resolve intermediate cluster sums to global cluster sums	
			for(auto i = tls_views.begin(); i != tls_views.end(); i++) {
				View v = *i;
				for(int j = 0; j < K; j++) {
					clusters[j] += v;
				}
			}

			for(auto& v : tls_views) {
				v.reset();
			}	

			// recalculating the center of each cluster
			for(int i = 0; i < K; i++) {
				clusters[i].setCentralValues();
			}

			// associates each point to the nearest center
			auto end_phase2 = chrono::high_resolution_clock::now();
			tbb::parallel_for(tbb::blocked_range<size_t>(0, total_points),
				[&](tbb::blocked_range<size_t>& r) {
					for(int i = r.begin(); i < r.end(); i++) {

						id_old_cluster_tls.local() = points[i].getCluster(); // get the cluster designation of point i
						id_nearest_center_tls.local() = getIDNearestCenter(points[i]); // calculate the nearest cluster by Euclidian distance of point i
						
						// if the cluster is anything other than the nearest cluster, remove the point from the old cluster and add it to the nearest cluster
						if(id_old_cluster_tls.local() != id_nearest_center_tls.local()) {
							if(id_old_cluster_tls.local() != -1) {
								tls_views.local().removePoint(points[i], id_old_cluster_tls.local());
							}
						points[i].setCluster(id_nearest_center_tls.local()); // assign the point to a cluster
						tls_views.local().addPoint(points[i], id_nearest_center_tls.local()); //add the point to the nearest cluster

						}
					}
				}
			);

			auto end_phase3 = chrono::high_resolution_clock::now();

			// resolve change count and reset cluster_change_tls
			for (auto i = tls_views.begin(); i != tls_views.end(); ++i) {
				View v = *i;
				int s = v.getChange();
				if (s > 0) {
					not_done = true;
					break;
				}
				not_done = false;
			}

			if(not_done == false || iter >= max_iterations)
			{
				cout << "Break in iteration " << iter << "\n\n";
				break;
			}

			iter++;
		} while(not_done);
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
