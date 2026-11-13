/* The start of the actual project. First is the road network*/

#include <cstdio>
#include <string>
#include <vector>
#include <random>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <thread>
#include <cstdlib>

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


// One lane of a horizontal street. If cars > 0, the number of cars is shown in the middle of the lane.
std::string lane(bool exists, bool goesRight, int cars = 0)
{
	if (!exists) return std::string(link_len, ' ');
	std::string s(link_len, road_char);

	if (cars > 0)
	{
		std::string text = std::to_string(cars);	
		s.replace((link_len - text.size()) / 2, text.size(), text);	// Replaces the dots in the middle with the number
	}

	if (goesRight) s.back() = '>';
	else s.front() = '<';
	return s;
}

char countChar(int n) 
{ 
	return n > 9 ? '+' : (char)('0' + n); 
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
	void printMap(const std::vector<int>& carCounts = std::vector<int>(), int cols = 3) const
	{

		// Cars on the street. | 0 if there are no cars on the street
		auto carsOn = [&](int a, int b)
			{
				int idx = findStreet(a, b);
				return(idx == -1 || carCounts.empty()) ? 0 : carCounts[idx];
			};


		int rows = nodeCount / cols;
		for (int r = 0; r < rows; ++r)
		{
			// One row of intersections. Lane going RIGHT gets drawn to the next intersection
			for (int c = 0; c < cols; ++c)
			{
				int n = r * cols + c + 1;
				std::printf("[%2d]", n);
				if (c < cols - 1)
					std::printf("%s", lane(hasEdge(n, n + 1), true, carsOn(n, n + 1)).c_str());
			}

			std::printf("\n");

			// Lane going left gets drawan
			for (int c = 0; c < cols; ++c)
			{
				int n = r * cols + c + 1;
				std::printf("    ");
				if (c < cols - 1)
					std::printf("%s", lane(hasEdge(n + 1, n), false, carsOn(n + 1, n)).c_str());
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
std::mt19937 rng(42);          // Random generator; fixed seed gives the same results every run, which helps benchmarking

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
const double a_max = 1.5;        // The max acceleration (m/s^2)
const double b_comf = 2.0;       // The comfortable braking (m/s^2)
const double t_headway = 1.5;    // Desired time headway (s)
const double gap_floor = 0.1;    // The smallest gap the formula uses (m). It prevents division by zero

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
	int arrived = 0;		// How many cars have reached their destination
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

// Is there room at the start of the street for a new car?
bool streetHasRoom(const std::vector<Vehicle>& cars, int streetIdx)
{
	for (const Vehicle& c : cars)
	{
		if (!c.active || c.street != streetIdx) continue;		// Ignmores ccars that left or on another street
		

		if (c.x < car_length + min_gap)		// The car is ccloser than the 6.5m gap
			return false;	// There is no free space / room for a new car.
	}
	
	
	return true;	// No cars were too close to eacch other, means there is room
}


void spawnStep(const Network& net, World& world, int tick)
{
	for (SpawnNode& s : world.spawners)
	{
		if (tick < s.nextSpawnTick) continue;    // The turn for this node hasn't come yet

		int dest = randomDestination(s.node, net.nodeCount);
		Route route = findRoute(net, s.node, dest);

		// Check whether the route exists and whether the first street has room.
		if (!route.streetIdx.empty() &&
			streetHasRoom(world.cars, route.streetIdx.front()))
		{
			Vehicle v;	// Creates the car

			
			v.id = world.nextId++;	// The unique ID
			v.spawnTick = tick;	// To note when the car was created
			v.destination = dest;
			v.route = route;
			v.routeStep = 0;		// Starts on the first street of its route (Spawn Point)
			v.street = route.streetIdx.front();
			v.x = 0;	// This is at the start of the street
			v.v = 0;	// Ccar standing still
			v.v0 = net.streets[v.street].limit_kmh / 3.6;
			v.active = true;

			world.cars.push_back(v);
			++s.spawned;
		}

		else
		{
			++world.missedSpawns;		// There is no room for the car, so spawn is skippepd.
		}

		// Draws the time of the next spawn.
		s.nextSpawnTick = tick + randomIntervalTicks(world.ratePerMinute);
	}
}

	// FR4 Vehicle movement
double idm(double v, double v0, bool hasLeader, double gap, double leaderV)
{
	double freeTerm = 1.0 - std::pow(v / v0, 4);

	// No car ahead: accelerate towards desired speed.
	if (!hasLeader)
		return a_max * freeTerm;

	// Prevent division by zero or an extremely small gap.
	gap = std::max(gap, gap_floor);

	double dv = v - leaderV;

	double sStar = min_gap + v * t_headway + (v * dv) / (2.0 * std::sqrt(a_max * b_comf));

	return a_max * (freeTerm - std::pow(sStar / gap, 2));
}



bool findLeader(const Network& net, const std::vector<Vehicle>& cars,
	int me, double& gap, double& leaderSpeed)
{
	const Vehicle& m = cars[me];	// Gets the car we are currently checking.

	int best = -1; // Stores the index of the closest car ahead.

	for (int i = 0; i < (int)cars.size(); ++i)		// Looks through all cars to find one ahead on the same street.
	{
		if (i == me) continue; // Skip the car itself.

		const Vehicle& c = cars[i];

		if (!c.active || c.street != m.street)	// Ignore cars that are inactive or on a different street.
			continue;

		if (c.x > m.x)		// Check if this car is ahead of our car.
		{
			if (best == -1 || c.x < cars[best].x)		// Keep the car if it is the first one found or closer than the previous one.
				best = i;
		}
	}

	// A leader was found on the same street.
	if (best != -1)
	{
		gap = cars[best].x - m.x - car_length;		// Calculate the distance between the two cars (bumper to bumper).

		leaderSpeed = cars[best].v;		// Get the speed of the car ahead.

		return true; // A leader was found.
	}

	// Check if there is another street after the current one in the route.
	// If not, there cannot be a leader on the next street.
	if (m.routeStep + 1 >= (int)m.route.streetIdx.size())
		return false;

	int nextStreet = m.route.streetIdx[m.routeStep + 1];	// Get the ID of the next street in the car's route.

	best = -1; // Reset the closest car index.

	for (int i = 0; i < (int)cars.size(); ++i)	// Look for cars on the next street.
	{
		const Vehicle& c = cars[i];

		if (!c.active || c.street != nextStreet)	// Ignore inactive cars and cars that are not on the next street.
			continue;

		if (best == -1 || c.x < cars[best].x)	// Ignore inactive cars and cars that are not on the next street.
			best = i;
	}

	// A leader was found on the next street.
	if (best != -1)
	{
		double streetLength = net.streets[m.street].length_m;		// Get the length of the street our car is currently on.
		gap = (streetLength - m.x) + cars[best].x - car_length;		// Calculate the distance to the car on the next street.
		leaderSpeed = cars[best].v;									// Get the speed of the car ahead.

		return true; // A leader was found.
	}

	// No leader was found on either street.
	return false;
}


void moveStep(const Network& net, World& world)
{
	std::vector<double> acc(world.cars.size(), 0.0);		// Store the acceleration of each car for this simulation step.

	// First, calculate acceleration for every active car.
	// Nobody moves during this loop, so all cars use the same initial state.
	for (int i = 0; i < (int)world.cars.size(); ++i)
	{
		Vehicle& c = world.cars[i];

		if (!c.active) continue;

		double gap = 0.0;
		double leaderSpeed = 0.0;

		bool hasLeader = findLeader(net, world.cars, i, gap, leaderSpeed); 

		// Calculate the acceleration using the IDM car-following model.
		acc[i] = idm(c.v, c.v0, hasLeader, gap, leaderSpeed);
	}

	for (int i = 0; i < (int)world.cars.size(); ++i)
	{
		Vehicle& c = world.cars[i];

		if (!c.active) continue;

		// Update speed based on the calculated acceleration.
		// The maximum function prevents the car from moving backwards.
		c.v = std::max(0.0, c.v + acc[i] * tick_s);

		c.x += c.v * tick_s;		// Move the car forward using its updated speed.
		
		double streetLength = net.streets[c.street].length_m;	// Check the length of the street the car is currently travelling on.

		if (c.x >= streetLength)
		{
			// The car has reached the end of its current street.
			if (c.routeStep + 1 >= (int)c.route.streetIdx.size())		// Check whether this was the last street in its route.
			{
				c.active = false;	// The car has completed its route and is marked as inactive.
				++world.arrived;
			}

			else
			{
				// Move onto the next street in the car's route.
				// Subtract the previous street length to preserve the remaining distance.
				c.x -= streetLength;
				++c.routeStep;		// Advance to the next street in the route.
				c.street = c.route.streetIdx[c.routeStep];
				c.v0 = net.streets[c.street].limit_kmh / 3.6;		// Set the desired speed according to the new street's speed limit.
			}
		}
	}

	// Remove all inactive cars (those that have completed their routes).
	// erase-remove is used to clean up the vector in one operation.
	world.cars.erase(std::remove_if(world.cars.begin(),world.cars.end(), [](const Vehicle& c) { return !c.active; }),
		world.cars.end());
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

// FR5 - Time Steps

struct RunResult
{
	int ticksRun = 0;
	double simulatedSeconds = 0;   // ticksRun times 0.1
	int spawned = 0;               // Cars created during the run, for all spawn nodes together
	int arrived = 0;               // Cars that have reached their final destination
	int missedSpawns = 0;
	int carsOnRoad = 0;            // Cars that are still driving when the run ended
};


RunResult runSimulation(const Network& net, double ratePerMinute, int seed, int totalTicks, bool verbose)
{
	RunResult r;		// The results of the current run are filled at the end of the run
	rng.seed(seed);		// The same seed gives the same random numbers

	World world = makeWorld(ratePerMinute);

	for (int tick = 0; tick < totalTicks; ++tick)
	{
		spawnStep(net, world, tick);		// The spawn nodes create the cars which are due
		moveStep(net, world);		// Accelerations, positions and speeds

		if (verbose && tick % 600 == 0)		// Runs once every 600 ticks or 60 simulated seconds
		{
			std::printf("Time: %.1f s | Cars on road: %zu | Arrived: %d | Missed spawns: %d\n", tick * tick_s, world.cars.size(), world.arrived, world.missedSpawns);
		}
	}


	r.ticksRun = totalTicks;		// Srote the results 
	r.simulatedSeconds = totalTicks * tick_s;		// Converts the ticks into simulated seconds

	// Adds up the number of cars that spawned at every spawn node
	r.spawned = 0;
	for (const SpawnNode& s : world.spawners)
	{
		r.spawned += s.spawned;
	}

	r.arrived = world.arrived;		// Storets the total number of cars that have reached their final destination
	r.missedSpawns = world.missedSpawns;	// Stores the number of cars that were not able to spwn
	r.carsOnRoad = (int)world.cars.size();		// Stores how many caras are still on the road.

	return r;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

 // This is the checker that sees if all functional requirements pass or fail
int failures = 0;
void check(const char* name, bool ok)
{
	std::printf("[%s] %s\n", ok ? "Pass" : "Fail", name);
	if (!ok) ++failures;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

// The Main Function
int main()
{
	// Network Setup

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

	city.printMap();    // This loads the road network on the console as output.

	std::printf("\n");

	//----------------------------------------------------------------------------
	//----------------------------------------------------------------------------

	std::printf("\n FR1: Road Representaion \n");
	// FR1 – Road Representation

	// Test to see if everything is met and in order.
	check("12 intersections", city.nodeCount == 12);
	check("26 directed edges", city.streets.size() == 26);
	check("Network is strongly connected", city.isStronglyConnected());
	check("2->3 exists (one-way)", city.hasEdge(2, 3));
	check("3->2 does not exist", !city.hasEdge(3, 2));

	// A negative test - 1->2->3 must NOT count as strongly connected
	Network chain(3);
	chain.addOneWay(1, 2, 500, 50);
	chain.addOneWay(2, 3, 500, 50);
	check("One-way chain is not strongly connected", !chain.isStronglyConnected());

	//----------------------------------------------------------------------------
	//----------------------------------------------------------------------------

	std::printf("\n FR2: Vehicle Spawn \n");
	// FR2 – Vehicle Spawn
	
	// Free-flow travel time checks from Table 1 in the Senior Thesis.
	check("Free-flow 1->2 = 45 s", std::lround(city.streets[city.findStreet(1, 2)].freeFlowSeconds()) == 45);
	check("Free-flow 2->3 = 72 s", std::lround(city.streets[city.findStreet(2, 3)].freeFlowSeconds()) == 72);
	check("Free-flow 3->6 = 60 s", std::lround(city.streets[city.findStreet(3, 6)].freeFlowSeconds()) == 60);
	check("Free-flow 6->9 = 36 s", std::lround(city.streets[city.findStreet(6, 9)].freeFlowSeconds()) == 36);
	check("Free-flow 5->6 = 120 s", std::lround(city.streets[city.findStreet(5, 6)].freeFlowSeconds()) == 120);

	// Test random vehicle spawn intervals.
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

	// Test random destination selection.
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
		if (n != 5 && (picked[n] < 9500 || picked[n] > 10500))
			evenlySpread = false;
	}

	check("Destination is never the spawn node", neverSelf);
	check("The other 11 nodes are picked about equally", evenlySpread);

	// Test whether a street has enough room for a new vehicle.
	std::vector<Vehicle> testCars;
	check("An empty street has room", streetHasRoom(testCars, 0));

	Vehicle blocker;        // Car that is already standing on street 0
	blocker.street = 0;
	blocker.active = true;
	blocker.x = 5.0;        // Put the car on the street
	testCars.push_back(blocker);

	check("A car 5 m from the start blocks the spawn", !streetHasRoom(testCars, 0));

	testCars[0].x = 6.5;    // Move the car to the minimum distance [4.5 meters car length + 2 meters safety gap]
	check("A car exactly 6.5 m away leaves room", streetHasRoom(testCars, 0));

	testCars[0].x = 1.0;
	check("A car on another street does not block", streetHasRoom(testCars, 1));    // Mustn't block street 1

	//----------------------------------------------------------------------------
	//----------------------------------------------------------------------------

	std::printf("\n FR2: Vehicle Spawn \n");
	// FR2 – Vehicle Spawn

	// One simulated hour, 10 cars per min per node.
	World hour = makeWorld(10.0);
	bool carsOk = true;

	for (int tick = 0; tick < 36000; ++tick)        // 36k ticks multiplied by 0.1 makes 1 hour
	{
		spawnStep(city, hour, tick);        // The spawn nodes create cars if the time to create a car has come

		for (const Vehicle& c : hour.cars)        // Looks at every car that was created
		{
			if (c.street < 0 || c.route.streetIdx.empty())
			{
				carsOk = false;
				continue;
			}

			int origin = city.streets[c.street].from;
			bool isSpawnNode = origin == 1 || origin == 5 || origin == 6 || origin == 12;

			if (!isSpawnNode || c.destination == origin || c.v != 0 || c.x != 0
				|| c.street != c.route.streetIdx.front()
				|| city.streets[c.route.streetIdx.back()].to != c.destination)
				carsOk = false;    // Route should end at the final destination
		}

		hour.cars.clear();    // Pretend that the cars drive away right away
	}

	bool countsOk = true;
	for (const SpawnNode& s : hour.spawners)
		if (s.spawned < 500 || s.spawned > 700)
			countsOk = false;    // 10 cars per minute times 60 minutes makes 600

	check("Each spawn node creates about 600 cars in one hour at 10 cars/min", countsOk);
	check("Every created car is valid", carsOk);
	check("No missed spawns while the streets are empty", hour.missedSpawns == 0);

	// This is for when cars never leave. The streets will fill up and the car spawns must be counted as skipped.
	World jam = makeWorld(60.0);

	for (int tick = 0; tick < 600; ++tick)
		spawnStep(city, jam, tick);    // For 1 min the cars stay where they spawned

	check("A full street makes spawns get skipped and counted", jam.missedSpawns > 0);
	check("At most one waiting car per first street", jam.cars.size() <= 8);

	//----------------------------------------------------------------------------
	//----------------------------------------------------------------------------
	
	std::printf("\n FR3: Route Planning \n");
	// FR3 - Route Planning 

	// Check expected routes from Table 1.
	check("Route 1->12", routeIs(city, 1, 12, { 1, 2, 5, 8, 11, 12 }, 240));
	check("Route 12->1", routeIs(city, 12, 1, { 12, 11, 10, 7, 4, 1 }, 225));
	check("Route 1->3", routeIs(city, 1, 3, { 1, 2, 3 }, 117));
	check("Route 3->1 (one-way streets force the long way)", routeIs(city, 3, 1, { 3, 6, 5, 2, 1 }, 270));
	check("Route 10->3", routeIs(city, 10, 3, { 10, 7, 4, 1, 2, 3 }, 225));

	// Check that all possible routes are valid chains of real streets.
	bool allValid = true;    // This is always true unless a route is broken

	for (int from = 1; from <= 12; ++from)
		for (int to = 1; to <= 12; ++to)
		{
			if (from == to) continue;        //// A car never goes from an intersection to itself

			Route r = findRoute(city, from, to);

			if (r.streetIdx.empty()) { allValid = false; continue; }            // If no route then failure
			if (city.streets[r.streetIdx.front()].from != from) allValid = false;    // The first street should start at the spawn point
			if (city.streets[r.streetIdx.back()].to != to) allValid = false;        // Last street should end at the final destination point

			for (size_t i = 1; i < r.streetIdx.size(); i++)
				if (city.streets[r.streetIdx[i]].from != city.streets[r.streetIdx[i - 1]].to)
					allValid = false;
		}

	check("All 132 routes are valid chains of real streets", allValid);

	//----------------------------------------------------------------------------
	//----------------------------------------------------------------------------

	std::printf("\n FR4: Vehicle Movement \n");
	// FR4 - Vehicle movement
	// IDM checks
	check("IDM: a standing car on an empty road accelerates at a_max", std::fabs(idm(0, 13.9, false, 0, 0) - a_max) < 1e-9);
	check("IDM: a car at its desired speed on an empty road does not accelerate", std::fabs(idm(13.9, 13.9, false, 0, 0)) < 1e-9);
	check("IDM: a car brakes when the gap ahead is small", idm(10, 13.9, true, 5, 10) < 0);
	check("IDM: a car far behind its leader still accelerates", idm(10, 13.9, true, 1000, 10) > 0);
	check("IDM: a gap of zero does not break the formula", std::isfinite(idm(5, 13.9, true, 0, 0)));

	
	// The leader on the next street: car at 990 m, standing car at 10 m on the next street
	Route r13 = findRoute(city, 1, 3);
	std::vector<Vehicle> look(2);
	look[0].route = r13;
	look[0].street = r13.streetIdx[0];		// look[0] is on the first street of the route
	look[0].x = 990;	// 10 m before the end of the 1000 m street
	look[0].v = 10;
	look[1].street = r13.streetIdx[1];	// [1] is on the next street
	look[1].x = 10;		// Car positioned 10 m into the next street
	look[1].v = 0;			//  Stationary car to test if stopped cars are beign detected
	double gap = 0, leaderSpeed = -1;
	bool seen = findLeader(city, look, 0, gap, leaderSpeed);

	check("A car sees a standing car on the next street (gap 15.5 m)", seen && std::fabs(gap - 15.5) < 1e-9 && leaderSpeed == 0);
	look[1].active = false;
	check("A car with nobody ahead sees no leader", !findLeader(city, look, 0, gap, leaderSpeed));

	// A car 0.1 meters before the end of its street
	World hop;
	Vehicle c;
	c.route = r13;
	c.street = r13.streetIdx[0];
	c.x = 999.9;	// 0.1 m before the end, so one tick is enough to cross it
	c.v = 10;	// 10 m/s means the car moves about 1 m in one tick (0.1 s), so it crosses the end by about 0.9 m
	c.v0 = 80 / 3.6;	// 80 km/h is the speed limit of the first street (1 towards 2), converted to m/s
	hop.cars.push_back(c);
	moveStep(city, hop);	// One tick

	check("At the end of a street a car continues on the next street of its route", hop.cars.size() == 1 && hop.cars[0].street == r13.streetIdx[1] && hop.cars[0].routeStep == 1);
	check("It keeps the leftover distance", hop.cars[0].x > 0 && hop.cars[0].x < 5);
	check("Its desired speed becomes the speed limit of the new street", std::fabs(hop.cars[0].v0 - 50 / 3.6) < 1e-9);

	// One way street test, a fast car approaches a slow one.
	Network line(2);
	line.addOneWay(1, 2, 1000, 50);
	World w;
	Vehicle slow, fast;
	slow.id = 1; slow.route.streetIdx = { 0 }; slow.street = 0; slow.x = 100; slow.v = 30 / 3.6; slow.v0 = 30 / 3.6;
	fast.id = 2; fast.route.streetIdx = { 0 }; fast.street = 0; fast.x = 0;   fast.v = 50 / 3.6; fast.v0 = 50 / 3.6;
	w.cars = { slow, fast };		// cars[0] is the slow car, cars[1] is the fast car
	double smallestGap = 1e9;
	for (int tick = 0; tick < 800; ++tick)	// 80 seconds: long enough for the fast car to catch up and settle, and the slow car (about 770 m by then) is still on the street
	{
		moveStep(line, w);
		double g = w.cars[0].x - w.cars[1].x - car_length;	// Bumper - to - bumper gap between the two cars
		if (g < smallestGap) smallestGap = g;	// Remember the closest distance they've ever got
	}
	double finalGap = w.cars[0].x - w.cars[1].x - car_length;

	check("A fast car behind a slow car never gets closer than the minimum gap", smallestGap > min_gap);
	check("The fast car ends up at the speed of the slow car", std::fabs(w.cars[1].v - w.cars[0].v) < 0.3);
	check("The gap between them ends up safe and stable", finalGap > 5 && finalGap < 40);

	// A single car drives its whole route and then disappears
	World trip;
	Route r = findRoute(city, 1, 12);
	Vehicle lone;
	lone.id = 1;
	lone.route = r;
	lone.street = r.streetIdx.front();
	lone.v0 = city.streets[lone.street].limit_kmh / 3.6;
	trip.cars.push_back(lone);
	int ticks = 0;

	while (!trip.cars.empty() && ticks < 20000) { moveStep(city, trip); ++ticks; }
	double seconds = ticks * tick_s;

	check("A lone car drives its whole route and disappears at its destination", trip.cars.empty() && trip.arrived == 1);
	check("The trip takes at least the free-flow time, but less than twice as long", seconds >= r.cost_s && seconds < 2 * r.cost_s);

	//----------------------------------------------------------------------------
	//----------------------------------------------------------------------------

	std::printf("\n FR5: Time Steps \n");
	// FR5 - Time Steps

	// Check to see if two runs are identical
	auto same = [](const RunResult& x, const RunResult& y)
		{
			return x.spawned == y.spawned && x.arrived == y.arrived && x.missedSpawns == y.missedSpawns && x.carsOnRoad == y.carsOnRoad;
		};

	RunResult shortRun = runSimulation(city, 10.0, 7, 6000, false);		// 6000 ticks are equal to 600 seconds in the simulation
	check("Simulated time equals ticks x 0.1 s", shortRun.ticksRun == 6000 && std::fabs(shortRun.simulatedSeconds - 600.0) < 1e-9);

	RunResult b1 = runSimulation(city, 10.0, 7, 12000, false);	// 20 simulated minutes
	RunResult b2 = runSimulation(city, 10.0, 7, 12000, false);	// The same settings and seed again

	check("Two runs with the same seed give identical results", same(b1, b2));

	RunResult otherSeed = runSimulation(city, 10.0, 8, 12000, false);		// Another seed
	check("A different seed gives different resutls", !same(b1, otherSeed));

	check("No cars are lost: spawned = arrived + still on the road", b1.spawned == b1.arrived + b1.carsOnRoad);
	check("Cars do arrive at their destinations", b1.arrived > 0);


	RunResult heavy = runSimulation(city, 15.0, 7, 6000, false);	// 15 cars per minute per node
	check("A heavy load (15 cars/min per node) causes missed spawns", heavy.missedSpawns > 0);

	std::printf("\n");
	std::printf("\n");
	runSimulation(city, 5.0, 1, 3600, true);	
	return failures;    // If it's 0 then all checks have passed
}