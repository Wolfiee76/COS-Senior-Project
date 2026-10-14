/* This is a one-way street test with a small animation if I should say. It tests weather the car acccelerates and breaks accordingly
There will be aanother test about collisions, just to see if things work out as intented.*/

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <thread>
#include <vector>


// Parameters for the Driver Model (IDM)
const double a_max = 1.5;	// This is the max acceleration of the car
const double b = 2.0;	// This is for the breaking of the car
const double s0 = 2.0;	// This is the min gap when the car is stopped in meters
const double t_hw = 1.5;	// This is desired time headway, which is the gap that the driver wants to keep to the car in the front
const double length = 4.5;	// This is the length of the car in meters
const double road_length = 1000.0;	// for this test the one-waay street is 1km long
const double dt = 0.1;		// one tick equals 0.1 seconds in the simmulation. DT - delta time
const double t_end = 120.0;		// This is the end time of the simulatio | might be increased, further test needed and decision

struct Vehicle
{
	int id;
	double x;	// the position of the car on the road
	double v;	// speed of the car
	double v0;	// the desired speed of the car
	bool active;
};

double kmh(double ms) { return ms * 3.6; }

/* This part of the test is for the Driver model, specifically about the acceleration from the car's own speed, desired speed and the car in the front if any. */
double idm(const Vehicle& me, const Vehicle* lead)
{
	double freeterm = 1.0 - std::pow(me.v / me.v0, 4);
	if (!lead) return a_max * freeterm;

	double gap = std::max(lead->x - me.x - length, 00.1);
	double dv = me.v - lead->v;	// positive = closing in
	double sStar = s0 + me.v * t_hw + me.v * dv / (2.0 * std::sqrt(a_max * b));
	return a_max * (freeterm - std::pow(sStar / gap, 2));
}

// This is for the nearest car which is ahead of me on the same lane/road
const Vehicle* findLead(const std::vector<Vehicle>& cars, const Vehicle& me) {
	const Vehicle* best = nullptr;
	for (const auto& c : cars) {
		if (!c.active || c.id == me.id || c.x <= me.x) continue;
		if (!best || c.x < best->x) best = &c;
	}
	return best;
}

int main()
{

}

// Just aa test to see if updating the repo works
// dfgdfgfdgdhdfhgdfhgfh