/* The start of the actual project. First is the road network*/

#include <cstdio>
#include <string>
#include <vector>

// This is for the size of the streets once the program runs and 
const int link_len = 24;   // Length of a horizontal street using characters.
const int link_rows = 6;   // Length of a vertical street, uses lines as length.
const char road_char = '.';   // What the road uses to be drawn.

struct Street // Only one directed edge of the graph
{
	int from, to;		// The numbers of the intersections.
	double length_m;	// 500m or 1km depending which road
	double limit_kmh;	// The speed limit of the streets

	/* FOR ME TO DO: add a function for the free flow, and speed from Table 1.*/
};

// This is for the street directions, depending whether it goes LEFR or RIGHT.
std::string lane(bool exists, bool goestRight)
{
	if (!exists) return std::string(link_len, ' ');   // No street exists in this direction.
	std::string s(link_len, road_char);

	if(goestRight) s.back() = '>';     // Arrow to signal that the street direction is going RIGHT.
	else s.front() = '<';		// Arrow to signal that the street direction is going LEFT.
	return s;
}

struct Network
{
	int nodeCount;
	std::vector<Street> streets;                // This is for every directed edge of the graph.
	std::vector<std::vector<int>> outgoing;     // Outgoing[u] = indexes (into streets) of the streets leaving u.

	explicit Network(int n) : nodeCount(n), outgoing(n + 1) {}   // n + 1 allows us to use numbers from 1 to 12 and ignore 0.

	// One-way street = one directed edge
	void addOneWay(int a, int b, double length_m, double limit_kmh)
	{
		streets.push_back({ a, b, length_m, limit_kmh });    // This stores the street.
		outgoing[a].push_back((int)streets.size() - 1);    // This remembers the indesx for intersection a.
	}

	// Two way street = two directed edges
	void addTwoWay(int a, int b, double length_m, double limit_kmh)
	{
		addOneWay(a, b, length_m, limit_kmh);	// a goes to b
		addOneWay(b, a, length_m, limit_kmh);	// b goes to a
	}

	// FOR ME TO DO LATER OR TMRW: How many intersections can be reached beginning form the spawn node
	int reachableFrom(int start) const
	{
		return 0;
	}

	// FOR ME TO DO LATER OR TMRW: Are the streets strongly connected - every intersection can be rerached from every other one
	bool isStronglyConnected() const
	{
		return false;
	}

	// A street that goes from a to be?
	bool hasEdge(int a, int b) const
	{
		for (int idx : outgoing[a])
			if (streets[idx].to == b) return true;
		return false;
	}

	// This here will draw/ display the road network on the console once the program starts to run
	void printMap(int cols = 3) const
	{
		int rows = nodeCount / cols;
		for (int r = 0; r < rows; ++r)
		{
			// One row of intersections. Lane going RIGHT gets drawn to the next intersection
			for (int c = 0; c < cols; ++c)
			{
				int n = r * cols + c + 1;
				std::printf("[%2d]", n);
				if (c < cols - 1)
					std::printf("%s", lane(hasEdge(n, n + 1), true).c_str());
			}

			std::printf("\n");

			// Lane going left gets drawan
			for (int c = 0; c < cols; ++c)
			{
				int n = r * cols + c + 1;
				std::printf("    ");
				if (c < cols - 1)
					std::printf("%s", lane(hasEdge(n + 1, n), false).c_str());
			}

			std::printf("\n");

			if (r < rows - 1)	// Vertical streets
			{
				for (int line = 0; line < link_rows; ++line)
				{
					for (int c = 0; c < cols; ++c)
					{
						int n = r * cols + c + 1;
						bool down = hasEdge(n, n + cols);       
						bool up = hasEdge(n + cols, n);       
						char d = !down ? ' ' : (line == link_rows - 1 ? 'v' : road_char);   
						char u = !up ? ' ' : (line == 0 ? '^' : road_char);               
						std::printf(" %c%c ", d, u);            
						if (c < cols - 1) std::printf("%*s", link_len, "");
					}
					std::printf("\n");
				}
			}
		}


	}
};
	