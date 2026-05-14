#pragma once
#include "view_obj.h"
#include "food.h"
#include "windows.h"
#include <vector>
#include "myDynQueue.h"


typedef struct perceptual_object
{
	view_obj* pObj;
		
	float distance;	//How far was the object from me?
	Vector dir;		//In what direction, from me, was the object
	string type;	//The object's type
	bool moving;	//Was the object moving?
	Vector itsDir;	//In what direction was it moving?
	float itsSpeed; //At what speed was it moving?

	perceptual_object()
	{
		distance = 0;
		dir = Vector(0, 0, 0);
		type = "\0";
		moving = false;
		itsDir = Vector(0, 0, 0);
		itsSpeed = 0;

		pObj = nullptr;
	}
};

typedef struct analysis_object
{
	bool changed_moving = false;
	bool stopped = false;
	bool moved_it = false;
	bool got_further = false;
	bool got_closer = false;
	bool sped_up = false;
	bool slowed_down = false;
};


typedef struct conclusion
{
	int action_type;		//0 ~ push
	string action_name;	
	string object_type;		//e.g. spore, cell, etc.
	string subject;			//I, a cell, etc.

	vector<string> result;

	conclusion()
	{
		action_type = -1;
		action_name = "";
		object_type = "";
		subject = "";
	}
};

typedef struct pushability_summary
{
	string object_type;
	float pushability;
	float confidence;
	int num_trials;
};

typedef struct cause_effect
{
	int action_idx;
	string action_name;
	string subject_name;
	string object_name;

	float mv;
	float st;
	float f;
	float c;
	float su;
	float sd;

	int num_trials;
};


class cell : public view_obj
{
public:
	RECT clirect;

	int radius;
	int sight_height;
	float sight_angle;
	Vector sight_dir;	//should always be a unit vector whose origin is the location of the cell.
	std::vector<perceptual_object> awareness;
	std::vector<perceptual_object> grasp_space;

	float energy;

	int id;

	cell(POINT loc, int _id);
	~cell();
	void draw(HDC hdc, HWND hwnd);

	void simulate();





	//Body
	void energy_depletion(float delta);
	bool hungry;
	float max_force;


	//Perception...............
	void see(DynQueue<view_obj*>* world, int world_size);
	void feel_barrier();


	//Motor controls............
	
	//atomic actions (but perhaps more appropriately named "reflexive actions"("reflexive" as in the object of the subject's action is itself))
	void rotate_eyes(float angle);
	void move_forward(float force);
	void force_self(Vector force);
	void stop();

	//more complex actions
	void face_point(myPoint p); //rotate to face point.
	void turn_shift(int ticks);


	//actions on external things
	void try_eat(view_obj* object);
	void try_push(view_obj* object);
	void observe_object(view_obj* object);
	vector<perceptual_object> observations;
	vector<conclusion> conclusions;

	vector<pushability_summary> push_db;



	//Mind............
	bool pursuing_food;//obsolete
	bool food_within_grasp;//obsolete
	bool hit_barrier;
	bool responding_to_barrier;
	int response_ticks;
	int end_tick;
	bool turn_maneuver;
	bool near_barrier;
	bool see_barrier;

	bool keep_distance_to_barrier;
	//mindset variable, busy signifies that the cell's mind is coordinating a long-term action, such as turning.
	bool busy;






	//bool bored;
	bool pursuing_push;//obsolete
	perceptual_object push_goal;//obsolete
	bool push_within_grasp;//obsolete

	bool confused;


	view_obj* ooi;		//object of interest
	bool do_observe;
	bool analyze_observation;


	/////////////////////////////////////////////////////////////////////

	bool feed_mode;
	bool bored;
	bool explore;

	//Pursuit variables
	bool pursuing_goal_point;
	bool goal_obj_within_grasp;
	myPoint goal_point;
	perceptual_object pursuit_goal_perception;







	void thinkC();
	void thinkB();
	void think();
	void clear_thinking();
	void clear_thinkingB();

	void mind_set();
	void frame_perception();
	void frame_push_curious();
	void frame_push_optimize();
	void coordinate_action();
	void set_push_target(perceptual_object po);

	void analyze();
	bool in_push_db(string item);

	void motor_control(myPoint gPoint, float effort);













	bool outside_rect(POINT pt, RECT rect);

};

