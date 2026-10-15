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
	const Vehicle* best = nullptr;		// closes car ahead found
	for (const auto& c : cars) {
		if (!c.active || c.id == me.id || c.x <= me.x) continue;
		if (!best || c.x < best ->x) best = &c;
	}
	return best;
}

int main()
{
	std::vector<Vehicle> cars = {
		{0, 100.0, 50.0 / 3.6, 50.0 / 3.6, true}, // car number one, moves slow at 50km/h
		{1,   0.0, 70.0 / 3.6, 70.0 / 3.6, true}, // car number 2, moves fast at 70 km/h
	};

	int accidents = 0;
	bool collided = false;

	for (double t = 0.0; t < t_end; t += dt)
	{
		// Here the program will compute all accelerations from the current state
		std::vector<double> acc(cars.size(), 0.0);
		for (size_t i = 0; i < cars.size(); ++i)
			if (cars[i].active) acc[i] = idm(cars[i], findLead(cars, cars[i]));


		// Applies the states from above. Each car moves every tick.
		for (size_t i = 0; i < cars.size(); ++i)
		{
			if (!cars[i].active) continue;  // This will skip the alreadt despaawned cars
			cars[i].v = std::max(0.0, cars[i].v + acc[i] * dt);		// new speed = old speed + acceleration
			cars[i].x += cars[i].v * dt;	// new position = old position

			if (cars[i].x >= road_length) cars[i].active = false;	// Here the car despawns once it reaches the end
		}

		// This is for collision check
		const Vehicle* lead = findLead(cars, cars[1]);
		if (cars[1].active && lead && lead->x - cars[1].x - length <= 0 && !collided)
		{
			collided = true;		// Remembers if the collision happened so that the future ticks do not count the same crash/collision
			++accidents;		// Just adds one to the accident counter
			std::printf("*** Accident occured at t=%.1fs, cars 0 and 1 ***\n", t);
		}

	}
