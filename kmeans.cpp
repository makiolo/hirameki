#include <iostream>
#include <vector>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <algorithm>
#include <cmath>
#include <numeric>
#include <assert.h>
#include <stdexcept>

using namespace std;

using DataType = string;
// using DataType = double;
// using DataType = float;
// using DataType = long double;
// using DataType = int;
// using DataType = long;
// using DataType = long long;

template <typename T>
struct convert
{
	T operator()(const std::string& data)
	{
		throw std::runtime_error("not implemmented");
		return T();
	}
};

template <>
struct convert<float>
{
	float operator()(const std::string& data)
	{
		return std::stof(data);
	}
};

template <>
struct convert<long double>
{
	long double operator()(const std::string& data)
	{
		return std::stold(data);
	}
};

template <>
struct convert<double>
{
	double operator()(const std::string& data)
	{
		return std::stod(data);
	}
};

template <>
struct convert<int>
{
	int operator()(const std::string& data)
	{
		return std::stoi(data);
	}
};

template <>
struct convert<long>
{
	long operator()(const std::string& data)
	{
		return std::stol(data);
	}
};

template <>
struct convert<long long>
{
	long long operator()(const std::string& data)
	{
		return std::stoll(data);
	}
};

template <>
struct convert<std::string>
{
	std::string operator()(const std::string& data)
	{
		return data;
	}
};

template <typename T>
struct heuristhic
{
	int operator()(T a, T b)
	{
		throw std::runtime_error("not implemmented");
		return 0;
	}
};

template <>
struct heuristhic<std::string>
{
	int operator()(const string &s1, const string &s2)
	{
		int N1 = s1.size();
		int N2 = s2.size();
		int i, j;
		vector<int> T(N2+1);

		for ( i = 0; i <= N2; i++ )
		T[i] = i;

		for ( i = 0; i < N1; i++ ) 
		{
			T[0] = i+1;
			int corner = i;
			for (j = 0; j < N2; j++) 
			{
				int upper = T[j+1];
				if ( s1[i] == s2[j] )
					T[j+1] = corner;
				else
					T[j+1] = min(T[j], min(upper, corner)) + abs(s2[j] - s1[i]);
				corner = upper;
			}
		}
		return T[N2];
	}
};

template <>
struct heuristhic<double>
{
	int operator()(double n1, double n2)
	{
		return sqrt(pow(n1 - n2, 2.0));
	}
};

template <typename CONTAINER>
DataType median(CONTAINER&& ks)
{
	assert(ks.size() > 0);
	auto& ref = ks[0];
	std::sort(ks.begin(), ks.end(), [=](auto& str1, auto& str2){ return heuristhic<DataType>()(str1, ref) > heuristhic<DataType>()(str2, ref); });
	return ks[ks.size() / 2];
}

class Point
{
private:
	int id_point, id_cluster;
	vector<DataType> values;
	int total_values;
	string name;

public:
	Point(int id_point, vector<DataType>& values, string name = "")
	{
		this->id_point = id_point;
		total_values = values.size();

		for(int i = 0; i < total_values; i++)
			this->values.push_back(values[i]);

		this->name = name;
		id_cluster = -1;
	}

	int get_id()
	{
		return id_point;
	}

	void set_cluster(int id_cluster)
	{
		this->id_cluster = id_cluster;
	}

	int get_cluster()
	{
		return id_cluster;
	}

	DataType get(int index)
	{
		return values[index];
	}

	int getTotalValues()
	{
		return total_values;
	}

	void addValue(DataType value)
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
	vector<DataType> central_values;
	vector<Point> points;

public:
	Cluster(int id_cluster, Point point)
	{
		this->id_cluster = id_cluster;

		int total_values = point.getTotalValues();

		for(int i = 0; i < total_values; i++)
			central_values.push_back( point.get(i) );

		points.push_back(point);
	}

	void push_back(Point point)
	{
		points.push_back(point);
	}

	bool remove(int id_point)
	{
		int total_points = points.size();

		for(int i = 0; i < total_points; i++)
		{
			if(points[i].get_id() == id_point)
			{
				points.erase(points.begin() + i);
				return true;
			}
		}
		return false;
	}

	DataType get_center(int index)
	{
		return central_values[index];
	}

	void set_center(int index, DataType value)
	{
		central_values[index] = value;
	}

	Point get(int index)
	{
		return points[index];
	}

	int get_totals()
	{
		return points.size();
	}

	int get_id()
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

	// return ID of nearest center
	int get_nearest_center(Point point)
	{
		double sum = 0.0;
		double min_dist = numeric_limits<double>::max();
		int id_cluster_center = 0;

		for(int i = 0; i < K; i++)
		{
			sum = 0.0;
			for(int j = 0; j < total_values; j++)
			{
				sum += heuristhic<DataType>()(clusters[i].get_center(j), point.get(j));
			}

			if(sum < min_dist)
			{
				min_dist = sum;
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
		if(K > total_points)
			return;

		vector<int> prohibited_indexes;

		// choose K distinct values for the centers of the clusters
		for(int i = 0; i < K; i++)
		{
			while(true)
			{
				int index_point = rand() % total_points;

				if(find(prohibited_indexes.begin(), prohibited_indexes.end(),
						index_point) == prohibited_indexes.end())
				{
					prohibited_indexes.push_back(index_point);
					points[index_point].set_cluster(i);
					Cluster cluster(i, points[index_point]);
					clusters.push_back(cluster);
					break;
				}
			}
		}

		int iter = 1;

		while(true)
		{
			bool done = true;

			// associates each point to the nearest center
			for(int i = 0; i < total_points; i++)
			{
				int id_old_cluster = points[i].get_cluster();
				int id_nearest_center = get_nearest_center(points[i]);

				if(id_old_cluster != id_nearest_center)
				{
					if(id_old_cluster != -1)
						clusters[id_old_cluster].remove(points[i].get_id());

					points[i].set_cluster(id_nearest_center);
					clusters[id_nearest_center].push_back(points[i]);
					done = false;
				}
			}

			// recalculating the center of each cluster
			for(int i = 0; i < K; i++)
			{
				for(int j = 0; j < total_values; j++)
				{
					int total_points_cluster = clusters[i].get_totals();
					if(total_points_cluster > 0)
					{
						// calculate centroid string ?
						vector<DataType> ks;
						for(int p = 0; p < total_points_cluster; p++)
						{
							ks.push_back( clusters[i].get(p).get(j) );
						}
						clusters[i].set_center(j, median(ks) );
					}
				}
			}

			if(done == true || iter >= max_iterations)
			{
				cout << "Break in iteration " << iter << "\n\n";
				break;
			}
			else
			{
				std::cout << "trying iter: " << iter << std::endl;
			}

			iter++;
		}

		// shows elements of clusters
		for(int i = 0; i < K; i++)
		{
			int total_points_cluster =  clusters[i].get_totals();

			cout << "Cluster " << clusters[i].get_id() + 1 << endl;
			for(int j = 0; j < total_points_cluster; j++)
			{
				cout << "Point " << clusters[i].get(j).get_id() + 1 << ": ";
				for(int p = 0; p < total_values; p++)
					cout << clusters[i].get(j).get(p) << " ";

				string point_name = clusters[i].get(j).getName();

				if(point_name != "")
					cout << "- " << point_name;

				cout << endl;
			}

			cout << "Cluster values: ";

			for(int j = 0; j < total_values; j++)
				cout << clusters[i].get_center(j) << " ";

			cout << "\n\n";
		}
	}
};

int main(int argc, char *argv[])
{
	// srand(time(NULL));
	srand(123);

	bool has_name = false;
	int max_iterations = 100;
	int total_values = 1;
	int K = 2;

	vector<Point> points;
	string point_name;

	int total_points = 0;
	for (std::string line; std::getline(std::cin, line);)
	{
		if(line.empty())
			continue;

		vector<DataType> values;

		// for(int j = 0; j < total_values; j++)
		// {
		// TODO: cut for total_values != 1 ?
		//
		values.push_back( convert<DataType>()(line) );
		//
		// }

		// if(has_name)
		// {
		// 	cin >> point_name;
		// 	Point p(total_points, values, point_name);
		// 	points.push_back(p);
		// }
		// else
		// {
		//
		Point p(total_points, values);
		points.push_back(p);
		//
		// }
		total_points += 1;
	}

	std::cout << "K = " << K << std::endl;
	std::cout << "total_points = " << total_points << std::endl;
	std::cout << "total_values = " << total_values << std::endl;
	std::cout << "max_iterations = " << max_iterations << std::endl;
	KMeans kmeans(K, total_points, total_values, max_iterations);
	kmeans.run(points);

	return 0;
}
