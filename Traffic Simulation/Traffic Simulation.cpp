/* The start of the actual project. First is the road network*/

#include <cstdio>
#include <string>
#include <vector>
#include <random>
#include <algorithm>
#include <cmath>

// This is for the size of the streets once the program runs and 
const int link_len = 24;   // Length of a horizontal street using characters.
const int link_rows = 6;   // Length of a vertical street, uses lines as length.
const char road_char = '.';   // What the road uses to be drawn.

struct Street // Only one directed edge of the graph
{
	int from, to;		// The numbers of the intersections.
	double length_m;	// 500m or 1km depending which road
	double limit_kmh;	// The speed limit of the streets
	double freeFlowSeconds() const { return length_m / (limit_kmh / 3.6); }
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

	// How many intersections can be reached beginning form the spawn node
	int reachableFrom(int start) const
	{
		std::vector<bool> seen(nodeCount + 1, false);   // "seen[n]" becomes true once intersection n has been found
		std::vector<int> toVisit;                       // Intersections that need to be looked at.
		toVisit.push_back(start);
		seen[start] = true;
		int count = 0;

		while (!toVisit.empty())
		{
			int u = toVisit.back();      // Takes one intersection from the list
			toVisit.pop_back();
			++count;                     // It's reachable, so count it

			for (int idx : outgoing[u])		//Looks at every street before leaving it
			{
				int next = streets[idx].to;		// Where the street leads to?
				if (!seen[next])                
				{
					seen[next] = true;
					toVisit.push_back(next);
				}
			}
		}

		return count;
	}

	 // Are the streets strongly connected - every intersection can be rerached from every other one.
	bool isStronglyConnected() const
	{
		for (int n = 1; n <= nodeCount; n++)	// Tries every intersecion as a start point
			if (reachableFrom(n) != nodeCount) return false;	// One of them can't reach everything, so no.
		return true;
	}

	// A street that goes from a to be?
	bool hasEdge(int a, int b) const
	{
		return findStreet(a, b) != -1;
	}

	int findStreet(int a, int b) const
	{
		for (int idx : outgoing[a])
			if (streets[idx].to == b) return idx;
		return -1;
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
					for (int c = 0; c < cols; ++c)	// Goes through the columns left to right
					{
						int n = r * cols + c + 1;	// Number of the intersection in this column of the current row
						bool down = hasEdge(n, n + cols);      // If there is a street going down to the intersection below
						bool up = hasEdge(n + cols, n);		// If there is a street coming up from the intersection below
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


// Route planning FR3
struct Route
{
	std::vector<int> streetIdx;
		double cost_s = 0;	// Overall free flow time of the route set in seconds.
};


Route findRoute(const Network& net, int origin, int destination)
{
	std::vector<double> cost(net.nodeCount + 1, 1e18);        // Cheapest known cost in order to reach each node.
	std::vector<int> previousStreet(net.nodeCount + 1, -1);   // The street that is used to reacch each node.
	std::vector<bool> done(net.nodeCount + 1, false);         // The finalized node.
	cost[origin] = 0;

	while (true)
	{
		int u = -1;		// The node that was chosen this round. Its negatvie beccause none are found yet
		double lowestCost = 1e18;		// Lowest cost found so far
		for (int n = 1; n <= net.nodeCount; n++)	// Looks at every interseccton
		{
			if (!done[n] && cost[n] < lowestCost)
			{
				lowestCost = cost[n];
				u = n;	// New best ccandidate
			}
		}


		if (u == -1) break;		// Nothing reachable is left, meaning the search is finished
		done[u] = true;			// The cost is final and there's no cheaper way to exists in order to reacch it
		if (u == destination) break;	// The destination has the cheapest cost, meaning we can stop early

		for (int idx : net.outgoing[u])		// Looks at every street that is leaving u
		{
			int next = net.streets[idx].to;
			double newCost = cost[u] + net.streets[idx].freeFlowSeconds();

			if (newCost < cost[next])	// We might have new cheaper cost
			{
				cost[next] = newCost;		// Remember the new ccost
				previousStreet[next] = idx;
			}
		}
	}

		Route r;

		if (cost[destination] >= 1e18)
		{
			return r;		// Desintation never reached; empty route
		}

		int node = destination;

		while (node != origin)
		{
			int street = previousStreet[node];
			r.streetIdx.push_back(street);
			node = net.streets[street].from;
		}

		std::reverse(r.streetIdx.begin(), r.streetIdx.end());
		r.cost_s = cost[destination];
		return r;
}


bool routeIs(const Network& net, int from, int to, const std::vector<int>& expectedNodes, long expectedSeconds)
{
	Route r = findRoute(net, from, to);
	std::vector<int> nodes = { from };                        // The intersections that the route passes, starting at the origin
	for (int idx : r.streetIdx) nodes.push_back(net.streets[idx].to);   // Each street adds the intersection which it leads to
	return nodes == expectedNodes && std::lround(r.cost_s) == expectedSeconds;
}


const double tick_s = 0.1;     // One tick is equal to 0.1 seconds
std::mt19937 rng(42);          // Random generator; FOR NOW MIGHT BE CHANGED LATERRRRRRRRRRRRRRRRRRRRRRR a fixed seed gives the same results every run, which helps benchmarking

// This is how many ticccks passed before the next car appears.
int randomIntervalTicks(double ratePerMinute)
{
	std::exponential_distribution<double> gap(ratePerMinute / 60.0);	// This is because the distibution needs to have the rate per second.
	int ticks = (int)std::lround(gap(rng) / tick_s);	// This is raandom gap converted into seconds.. The ticks are rounded to the nearest tick
	return ticks < 1 ? 1 : ticks;		// It's never less than 1 ticck
}

// Random final destination that is not a spawn node.
int randomDestination(int spawnNode, int nodeCount)
{
	std::uniform_int_distribution<int> pick(1, nodeCount - 1);	// One number less than the max amt of nodes
	int d = pick(rng);			// Picks random node from the 12
	return d >= spawnNode ? d + 1 : d;
}

struct SpawnNode
{
	int node;		// The intersections - 1, 5, 6, or 12
	int nextSpawnTick;		// This is for when the next car is due
	int spawned = 0;	// How many cars got created by the node
};


const double car_length = 4.5;	// The car length in meters
const double min_gap = 2.0;	// The safe gap between 2 cars

struct Vehicle
{
	int id = 0;
	int spawnTick = 0;
	int destination = 0;
	Route route;	// The streets that the car needs to drive.
	int routeStep = 0;	// The street that the car is on. 0 for the first one.
	int street = -1;
	double x = 0;            // The position of the car along that street in meters (0 = the start of the street)
	double v = 0;            // The speed set in meters per second m/s
	double v0 = 0;           // The Desired speed in m/s; the street speed limiit of the current street
	bool active = true;
};

struct World
{
	std::vector<Vehicle> cars;
	std::vector<SpawnNode> spawners;
	double ratePerMinute = 10.0;	// The spawn rate will be the same for all spawn nodes
	int nextId = 1;
	int missedSpawns = 0;	// If spawns are missed they will be counted
};

// This creates the fixed 4 spawned nodes, each has its own first random spawn time.
World makeWorld(double ratePerMinute)
{
	World w;
	w.ratePerMinute = ratePerMinute;
	for (int node : {1, 5, 6, 12})
		w.spawners.push_back({ node, randomIntervalTicks(ratePerMinute), 0 });
	return w;
}

// TO DO TOMORROWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW OR MONDAT
bool streetHasRoom(const std::vector<Vehicle>& cars, int streetIdx)
{
	return false; fhfghgf
}

// ALSO FOR TODO TMRWWWWWWW OR MONDAY
void spawnStep(const Network& net, World& world, int tick)
{
	fghfghfg
}


 // This is the checker that sees if all functional requirements pass or fail
int failures = 0;
void check(const char* name, bool ok)
{
	std::printf("[%s] %s\n", ok ? "Pass" : "Fail", name);
	if (!ok) ++failures;
}

// SOME MOVINGS AND REORGANIZATION NEEDS TO HAPPEN HERE BEFORE THE FULL PRJCT IS SUBMITTED.
int main()
{
	Network city(12);

	// These are the 17 streets each with their connection as in Table 1.
	city.addTwoWay(1, 2, 1000, 80);
	city.addOneWay(2, 3, 1000, 50);
	city.addOneWay(3, 6, 500, 30);
	city.addOneWay(6, 9, 500, 50);
	city.addOneWay(9, 12, 500, 30);
	city.addTwoWay(12, 11, 1000, 80);
	city.addOneWay(11, 10, 1000, 50);
	city.addOneWay(10, 7, 500, 50);
	city.addOneWay(7, 4, 500, 50);
	city.addOneWay(4, 1, 500, 50);
	city.addTwoWay(2, 5, 500, 40);
	city.addTwoWay(4, 5, 1000, 30);
	city.addTwoWay(5, 6, 1000, 30);
	city.addTwoWay(5, 8, 500, 30);
	city.addTwoWay(7, 8, 1000, 30);
	city.addTwoWay(8, 9, 1000, 30);
	city.addTwoWay(8, 11, 500, 40);

	city.printMap();	// This loads the road network on the console as output.

	std::printf("\n");

	// FR1 (Road Network Representation) test to see if everything is met and in order.
	check("12 intersections", city.nodeCount == 12);
	check("26 directed edges", city.streets.size() == 26);
	check("Network is strongly connected", city.isStronglyConnected());
	check("2->3 exists (one-way)", city.hasEdge(2, 3));
	check("3->2 does not exist", !city.hasEdge(3, 2));

	// Checks for FR2(Vehicle Spawn) from Table 1 in the Senior Thesis
	check("Free-flow 1->2 = 45 s", std::lround(city.streets[city.findStreet(1, 2)].freeFlowSeconds()) == 45);
	check("Free-flow 2->3 = 72 s", std::lround(city.streets[city.findStreet(2, 3)].freeFlowSeconds()) == 72);
	check("Free-flow 3->6 = 60 s", std::lround(city.streets[city.findStreet(3, 6)].freeFlowSeconds()) == 60);
	check("Free-flow 6->9 = 36 s", std::lround(city.streets[city.findStreet(6, 9)].freeFlowSeconds()) == 36);
	check("Free-flow 5->6 = 120 s", std::lround(city.streets[city.findStreet(5, 6)].freeFlowSeconds()) == 120);
	
	// A negative test - 1->2->3 must NOT count as strongly connected
	Network chain(3);
	chain.addOneWay(1, 2, 500, 50);
	chain.addOneWay(2, 3, 500, 50);
	check("One-way chain is not strongly connected", !chain.isStronglyConnected());

	// Tests for FR2 (Vehicle Spawn).
	double total = 0;
	int shortest = 1000000;

	for (int i = 0; i < 100000; ++i)
	{
		int t = randomIntervalTicks(10.0);       // 10 cars per minute
		total += t;
		if (t < shortest) shortest = t;
	}

	double meanSeconds = total / 100000 * tick_s;
	check("Mean gap is about 6 s at 10 cars/min", meanSeconds > 5.8 && meanSeconds < 6.2);
	check("A gap is never shorter than 1 tick", shortest >= 1);

	bool neverSelf = true;
	std::vector<int> picked(13, 0);
	for (int i = 0; i < 110000; ++i)
	{
		int d = randomDestination(5, city.nodeCount);
		if (d == 5 || d < 1 || d > 12) neverSelf = false;
		else ++picked[d];
	}

	bool evenlySpread = true;
	for (int n = 1; n <= 12; ++n)
	{
		if (n != 5 && (picked[n] < 9500 || picked[n] > 10500)) evenlySpread = false;
	}

	check("Destination is never the spawn node", neverSelf);
	check("The other 11 nodes are picked about equally", evenlySpread);

	// Checks for FR3(Route Planning) from Table 1
	check("Route 1->12", routeIs(city, 1, 12, { 1, 2, 5, 8, 11, 12 }, 240));
	check("Route 12->1", routeIs(city, 12, 1, { 12, 11, 10, 7, 4, 1 }, 225));
	check("Route 1->3", routeIs(city, 1, 3, { 1, 2, 3 }, 117));
	check("Route 3->1 (one-way streets force the long way)", routeIs(city, 3, 1, { 3, 6, 5, 2, 1 }, 270));
	check("Route 10->3", routeIs(city, 10, 3, { 10, 7, 4, 1, 2, 3 }, 225));

	bool allValid = true;	// This is always true unless a route is broken
	for (int from = 1; from <= 12; ++from)
		for (int to = 1; to <= 12; ++to)
		{
			if (from == to) continue;		//// A car never goes from an intersection to itself
			Route r = findRoute(city, from, to);
			if (r.streetIdx.empty()) { allValid = false; continue; }			// If no route then failure
			if (city.streets[r.streetIdx.front()].from != from) allValid = false;		// The first street should start at the spawn point
			if (city.streets[r.streetIdx.back()].to != to) allValid = false;		// Last street should end at the final destination point
			for(size_t i =1; i < r.streetIdx.size(); i++)
				if (city.streets[r.streetIdx[i]].from != city.streets[r.streetIdx[i - 1]].to) allValid = false;
		}
	check("All 132 routes are valid chains of real streets", allValid);


	// Additional FR2 (Vehicle spawn) tests. Room check
	std::vector<Vehicle> testCars;
	check("An empty street has room", streetHasRoom(testCars, 0));
	Vehicle blocker;		// CCar that is already standing on street 0
	blocker.street = 0;
	blocker.active = true;
	blocker.x = 5.0;	// Put the car on the street
	testCars.push_back(blocker);	
	check("A car 5 m from the start blocks the spawn", !streetHasRoom(testCars, 0));
	testCars[0].x = 6.5;	// Move the car to the minimum distance [4.5 meters car length + 2 meters safety gap]
	check("A car exactly 6.5 m away leaves room", streetHasRoom(testCars, 0));
	testCars[0].x = 1.0;
	check("A car on another street does not block", streetHasRoom(testCars, 1));	// Mustn't blocck street 1


	// One simulated hour, 10 cars per min per node.
	World hour = makeWorld(10.0);
	bool carsOk = true;	
	for (int tick = 0; tick < 36000; ++tick)		// 36k ticks multipled by 0.1 makes 1 hour
	{
		spawnStep(city, hour, tick);		// The spawn nodes create cars if the time to create a car has come
		for (const Vehicle& c : hour.cars)			// Looks at every ccar that was ccreated
		{
			if (c.street < 0 || c.route.streetIdx.empty()) { carsOk = false; continue; }
			int origin = city.streets[c.street].from;
			bool isSpawnNode = origin == 1 || origin == 5 || origin == 6 || origin == 12;
			if (!isSpawnNode || c.destination == origin || c.v != 0 || c.x != 0		// Must start at a spawn node, with a different destination
				|| c.street != c.route.streetIdx.front()
				|| city.streets[c.route.streetIdx.back()].to != c.destination) carsOk = false;	// Route should end at the final destinaation
		}
		hour.cars.clear();     // Pretend that the cars drive away right away

	}
	
	bool countsOk = true;
	for (const SpawnNode& s : hour.spawners)
		if (s.spawned < 500 || s.spawned > 700) countsOk = false;	// 10 cars per minute times 60 minutes makes 600

	check("Each spawn node creates about 600 cars in one hour at 10 cars/min", countsOk);
	check("Each spawn node creates about 600 cars in one hour at 10 cars/min", carsOk);
	check("Each spawn node creates about 600 cars in one hour at 10 cars/min", hour.missedSpawns == 0);

	// This is is for when cars never leave. The streets will fill up and the ccar spawns must be ccounted as skipped
	World jam = makeWorld(60.0);
	for (int tick = 0; tick < 600; ++tick) spawnStep(city, jam, tick);	// For 1 min the cars stay where they spaned
	check("A full street makes spawns get skipped and counted", jam.missedSpawns > 0);
	check("At most one waiting car per first street", jam.cars.size() <= 8);


	return failures;	// If it's a 0 then all checks have passed
}
	