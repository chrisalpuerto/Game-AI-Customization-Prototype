#pragma once
#include "Vector.h"
#include "myPoint.h"
#include <Windows.h>

#include <vector>

typedef struct attachement
{
	//specifically a string attachement.

	myPoint pt;			//Point of attachment
	float strength;		//measured in force units
	float length;		//The length/distance at which the attachment becomes active.
};

class particle
{
public:

	int id;

	int radius;
	float mass;
	myPoint part_loc;
	Vector vel;
	Vector acceleration;
	float charge;
	COLORREF color;

	bool attached;	//this indicates the particle is attached to something
	std::vector<attachement> attachements;


	float max_speed;		//terminal velocity.


	bool gravity_exception = false; //if true then gravity doesn't apply to this object
	bool ignore_collision = false; //if true, then ignore the effect of a collision on this object.
	bool ignore_electrity = false;
	bool gentle_electric_mode = false;

	void move(float dt);
	void accelerate(float dt);
	void applyForce(Vector force);
	void Collision(float mass_collide, float speed_x_collide, float speed_y_collide);
	void applyGravity(particle b);
	void applyElectricity(particle b);
	bool inParticle(myPoint pt);

	void attach_to_point(myPoint pt, float str, float length);

	void force_stop();
};

