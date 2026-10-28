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

	/* FOR ME TO DO: add a function for the free flow, and speed from Table 1*/
};

// This is for the street directions, depending whether it goes LEFR or RIGHT.
std::string lane(bool exists, bool goestRight)
{
	if (!exists) return std::string(link_len, ' ');   // No street exists in this direction
	std::string s(link_len, road_char);

	if(goestRight) s.back() = '>';     // Arrow to signal that the street direction is going RIGHT
	else s.front() = '<';		// Arrow to signal that the street direction is going LEFT
	return s;
}