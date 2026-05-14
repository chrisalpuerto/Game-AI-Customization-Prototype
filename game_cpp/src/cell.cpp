

#include "cell.h"

cell::cell(POINT loc, int _id)
{
	max_speed = 30;
	radius = 20;
	sight_height = 500;
	sight_angle = 50;	//degrees
	sight_dir = Vector(1, 0, 0);
	view_loc = loc;
	part_loc = winPoint_to_myPoint(loc);
	type = "cell";
	alive = true;

	id = _id;

	//The max force this cell can muster at 100% effort.
	max_force = 1000;

	energy = 10000;

	hungry = false;

	feed_mode = false;
	pursuing_food = false;
	food_within_grasp = false;
	goal_point = myPoint(0, 0, 0);
	explore = true;
	hit_barrier = false;
	responding_to_barrier = false;
	response_ticks = 0;
	end_tick = 0;
	turn_maneuver = false;
	near_barrier = false;		//has same meaning as see_barrier. becoming obsolete.
	see_barrier = false;
	keep_distance_to_barrier = true;

	bored = true;
	pursuing_push = false;
	push_within_grasp = false;
	confused = false;

	do_observe = false;
	ooi = nullptr;
	
	analyze_observation = false;

}
cell::~cell()
{

}




//Body
void cell::energy_depletion(float delta)
{
	energy += delta;
	if (energy <= 0)
	{
		alive = false;
	}

	float prop = energy / 15000;
	if (prop <= .7)
	{
		//Cell is 70% full (30% empty)
		hungry = true;
		bored = false;
	}

}






//Atomic actions
void cell::rotate_eyes(float angle)
{
	myMatrix mat;
	mat.init_RotXY_3x3(angle);
	mat.Rotate_XY_3x3(&sight_dir, &sight_dir);
}
void cell::move_forward(float force)
{
	//100 or 1000 force is a good suggestion.

	energy_depletion(-1);
	Vector f = sight_dir.MultiplyVectorByScalar(force);
	applyForce(f);
}
void cell::force_self(Vector force)
{
	energy_depletion(-1);
	applyForce(force);
}
void cell::stop()
{
	force_stop();
}

//Complex actions
void cell::face_point(myPoint p)
{
	Vector stri_h;	//sight triangle height vector
	stri_h = sight_dir.MultiplyVectorByScalar(sight_height);
	Vector v_obj = p.SubtractPointFromPoint(&part_loc);
	v_obj.tuple[1] *= -1;
	stri_h.tuple[1] *= -1;
	float dist_to_obj = v_obj.get_size();
	float dot = v_obj.DotProduct(&stri_h);
	float cos_phi = dot / sight_height / dist_to_obj;
	if (abs(1 - cos_phi) < .0001)
	{
		cos_phi = 1;
	}
	float phi = acos(cos_phi);		//angle between sight_dir and object
	phi = phi * 180 / 3.14159; //convert to degrees.

	//But now, we still need to find whether phi (angle from sight_dir to obj_vec) is positive or negative.
	float alpha_vobj = v_obj.get_angle();
	float alpha_vsight = stri_h.get_angle();
	//the difference in normalized/standard coordinate system
	float diff = alpha_vsight - alpha_vobj;
	diff = diff * 180 / 3.14159;
	if (int(abs(diff)*10) <= int(phi*10))
	{
		if (diff > 0)
		{
			//Means you must go clockwise from vsight to vobj --> negative in standard coordinates
			phi *= -1;
		}
		else
		{
			//phi is already positive. this else statement is just expository.
		}
	}
	else
	{
		if (diff < 0)
		{
			phi *= -1;
		}
	}



	//temporary. this will not be fluid. turn will be instant. solution will be: update sight to follow direction of particle body. give particle body angular "amenities"
	//The matrix I currently use rotates in the opposite sense to standard. so clockwise is positive. Therefore pass -phi into rotate function.
	rotate_eyes(-1*phi);
	
	


}
void cell::turn_shift(int ticks)
{
	//Stops cell, sets turn_maneuver flag, resets response_ticks, and sets end_tick to parameter.
	stop();
	turn_maneuver = true;
	busy = true;
	response_ticks = 0;
	end_tick = ticks;
}

//Actions on environment
void cell::try_eat(view_obj* object)
{
	//cell tries to eat object.

	if (object->type == "food")
	{
		food* pFood = reinterpret_cast<food*>(object);
		energy += pFood->energy;
		pFood->energy = 0;
		pFood->alive = false;


		float prop = energy / 15000;
		if (prop >= 1)
		{
			hungry = false;
			feed_mode = false;
			explore = false;
			bored = true;
		}

	}

}
void cell::try_push(view_obj* object)
{
	for (int i = 0; i < grasp_space.size(); i++)
	{
		if (object == grasp_space[i].pObj)
		{
			if (object->type == "food")
			{
				int test = 1;
			}

			//For now magic number. In future, cell should calculate how much energy it ought to use to push this object.
			float effort = 1000;
			Vector f = sight_dir;
			f = f.MultiplyVectorByScalar(effort);
			object->applyForce(f);
			//Reset motivation state, since operation was successful
			push_within_grasp = false;
		}
	}
}

void cell::observe_object(view_obj* object)
{
	for (int i = 0; i < awareness.size(); i++)
	{
		if (awareness[i].pObj == object)
		{
			observations.push_back(awareness[i]);
		}
	}
}





//Perception
void cell::feel_barrier()
{
	//The world has told cell that it has arrived at a barrier.
	clear_thinking();
}
void cell::see(DynQueue<view_obj*>* world, int world_size)
{
	awareness.clear();
	grasp_space.clear();

	//The cell becomes the origin point for the following analysis.

	Vector stri_h;	//sight triangle height vector
	Vector stri_l1; //leg one of sight triangle
	Vector stri_l2; //leg two of sight triangle
	
	stri_h = sight_dir.MultiplyVectorByScalar(sight_height);
	
	stri_l1 = sight_dir;
	stri_l2 = sight_dir;
	myMatrix mat;
	mat.init_RotXY_3x3(sight_angle);
	mat.Rotate_XY_3x3(&stri_l1, &stri_l1);
	mat.init_RotXY_3x3(-1*sight_angle);
	mat.Rotate_XY_3x3(&stri_l2, &stri_l2);
	float leg_length = sight_height / cos(sight_angle * 3.14159 / 180);
	stri_l1 = stri_l1.MultiplyVectorByScalar(leg_length);
	stri_l2 = stri_l2.MultiplyVectorByScalar(leg_length);

	float l1_alpha = stri_l1.get_angle();		//normalized angle
	float l2_alpha = stri_l2.get_angle();		//normalized angle


	for (int i = 0; i < world_size; i++)
	{
		DynQueue<view_obj*>::Node* curObj;
		world->jump_to_index(i, curObj);

		myPoint obj_point = curObj->data->part_loc;
		Vector v_obj = obj_point.SubtractPointFromPoint(&part_loc);		//vector from cell location to object location.
		v_obj.tuple[1] *= -1;		//switch from window coordinate (y grows downward) to standard coordinate (y grows upward).
		float obj_alpha = v_obj.get_angle();

		bool angle_within = false;
		bool distance_within_awareness = false;
		bool distance_within_grasp = false;

		float dist_to_obj = v_obj.get_size();
		Vector temp_stri_h = stri_h;
		temp_stri_h.tuple[1] *= -1;
		float dot = v_obj.DotProduct(&temp_stri_h);
		float cos_arg = dot / sight_height / dist_to_obj;
		if (abs(1 - cos_arg) < .0001)
		{
			cos_arg = 1;
		}
		float phi = acos(cos_arg);		//angle between sight_dir and object
		phi = phi * 180 / 3.14159; //convert to radians.
		
		if (phi <= sight_angle)
		{
			angle_within = true;
		}

		//I'm demoting the view triangle to viewing-sector (of a circle) to simplify the math.

		if (dist_to_obj <= sight_height)
		{
			distance_within_awareness = true;
			if (dist_to_obj <= radius + 10)
			{
				distance_within_grasp = true;
			}
		}

		if (angle_within && distance_within_awareness)
		{
			//In sight area -> add to awareness space.


			perceptual_object persobj;
			persobj.pObj = curObj->data;
			persobj.distance = dist_to_obj;
			persobj.dir = v_obj;
			persobj.dir.make_unit();
			persobj.type = curObj->data->type;
			persobj.itsDir = curObj->data->vel;
			persobj.itsDir.make_unit();
			persobj.itsSpeed = curObj->data->vel.get_size();
			if (int(persobj.itsSpeed*1000) > 0)
			{
				persobj.moving = true;
			}



			awareness.push_back(persobj);
			if (distance_within_grasp)
			{
				grasp_space.push_back(persobj);
			}
		}


	}

	for (int i = 0; i < awareness.size(); i++)
	{
		for (int j = i+1; j < awareness.size(); j++)
		{
			if (awareness[i].distance > awareness[j].distance)
			{
				perceptual_object temp = awareness[i];
				awareness[i] = awareness[j];
				awareness[j] = temp;
			}
		}
	}

	for (int i = 0; i < grasp_space.size(); i++)
	{
		for (int j = i + 1; j < grasp_space.size(); j++)
		{
			if (grasp_space[i].distance > grasp_space[j].distance)
			{
				perceptual_object temp = grasp_space[i];
				grasp_space[i] = grasp_space[j];
				grasp_space[j] = temp;
			}
		}
	}

	if (outside_rect(myPoint_to_winPoint(part_loc.AddVectorToPoint(&stri_h)), clirect))
	{
		near_barrier = true; //has same meaning as see_barrier. becoming obsolete.
		see_barrier = true;
	}
	else
	{
		near_barrier = false; //has same meaning as see_barrier. becoming obsolete.
		see_barrier = false;
	}

	//Perceiving takes energy in this simulation.
	energy_depletion(-2);

}








//mind
void cell::motor_control(myPoint gPoint, float effort)
{
	//effort is a percentage here. Max force * effort is what will be passed to the force_body function.
	if (effort > 0 && int(effort * 1000) <= 1000)
	{

		Vector vcurr = vel;
		vcurr.make_unit();
		Vector vgoal = gPoint - part_loc;
		vgoal.make_unit();

		Vector verr = vgoal - vcurr;
		//notes [] is overloaded for vectors. the number in the brackets means nothing (even if it isn't 0).
		float sz = verr[0];

		verr.make_unit();
		Vector Fcorrective = verr * (max_force * effort);
		applyForce(Fcorrective);
	}
}
void cell::clear_thinking()
{
	pursuing_food = false;
	pursuing_push = false;
	food_within_grasp = false;
	push_within_grasp = false;
	explore = true;
	confused = false;
	bored = false;
	do_observe = false;
	analyze_observation = false;
	observations.clear();
}

void cell::clear_thinkingB()
{
	pursuing_food = false;
	pursuing_push = false;
	food_within_grasp = false;
	push_within_grasp = false;
	explore = false;
	confused = false;
	bored = false;
	do_observe = false;
	analyze_observation = false;
	observations.clear();
}

void cell::mind_set()
{
	if (see_barrier && keep_distance_to_barrier)
	{
		//Cell knows it's approaching a barrier and its paradigm includes "keep distance to barrier".
		//Therefore, after calculating whether its velocity will soon lead it to collision with barrier, it will set up a flag.

		clear_thinkingB();

		Vector dxdt = vel;
		Vector dx = dxdt * 16.65;		//displacement after __ seconds (50 steps in the simulation)


		if (outside_rect(myPoint_to_winPoint(part_loc.AddVectorToPoint(&dx)), clirect))
		{
			//Then it would collide with the barrier after 10 steps under current conditions.
			//Initiate turn maneuver.
			turn_shift(20);
		}
	}

	//If hungry, should look for food.

	if (hungry)
	{
		//At present, there's no reason not to look for food if hungry. But maybe in the future, certain priorities, threats, etc. could take precedence.
		feed_mode = true;
		explore = true;
	}
	else if (!busy)
	{
		bored = true;
		explore = true;
	}

}
void cell::frame_perception()
{

}
void cell::frame_push_curious()
{
	for (int i = 0; i < awareness.size(); i++)
	{
		if (!in_push_db(awareness[i].type))
		{
			set_push_target(awareness[i]);
		}
		else
		{
			for (int j = 0; j < push_db.size(); j++)
			{
				if ((push_db[j].object_type == awareness[i].type) && (push_db[j].confidence < .7))
				{
					//When framing push while curious, cell only wants to push things it doesn't know very well.
					set_push_target(awareness[i]);
					break;
				}
			}			
		}
		if (pursuing_push)
		{
			break;
		}
	}
}
void cell::frame_push_optimize()
{
	//------------------------------[paradigm: frame perception based on trying to maximize the effectiveness of the push action]-----------------------------------
	//The following chooses a push target based on what is most pushable and currently in sight. So, we could say the cell's concern here is to make its push work best.
	//--------------------------------------------------------------------------------------------------------------------------------------------------------------
	// 
	//This searches all items in the push_db to find the most pushable object. Index stored in best_push_idx.
	int best_push_idx = -1;
	float best_rank = 0;
	for (int i = 0; i < push_db.size(); i++)
	{
		if (push_db[i].pushability > best_rank)
		{
			best_rank = push_db[i].pushability;
			best_push_idx = i;
		}
	}

	for (int i = 0; i < awareness.size(); i++)
	{
		//If the above check did not find a pushable item in the push_db (i.e. this cell knows nothing about anything's pushability), the very first object will be a suitable target.
		if (best_push_idx == -1)
		{
			set_push_target(awareness[i]);
			break;	//this cell has set its focus on pushing the first thing in its awareness.
		}
		else
		{
			//In this case, the cell knows a type of object and will prioritize that.
			if (awareness[i].type == push_db[best_push_idx].object_type)
			{
				set_push_target(awareness[i]);
				break;
			}
		}
	}
	if (!pursuing_push)
	{
		//Couldn't find an object of best type, so pick anything available.
		for (int i = 0; i < awareness.size(); i++)
		{
			set_push_target(awareness[i]);
			break;	//this cell has set its focus on pushing the first thing in its awareness.
		}
	}



}
void cell::coordinate_action()
{

	//This turn_maneuver routine is specifically designed to prevent hitting the barrier.
	if (turn_maneuver)
	{
		rotate_eyes(4);
		response_ticks++;
		if (response_ticks == end_tick)
		{
			response_ticks = 0;
			turn_maneuver = false;
			//Since, routine has completed, busy is set to false.
			busy = false;
		}
	}




}
void cell::set_push_target(perceptual_object po)
{
	stop();
	face_point(po.pObj->part_loc);
	goal_point = po.pObj->part_loc;
	push_goal = po;
	pursuing_push = true;
	turn_maneuver = false;
	explore = false;
}

void cell::think()
{

	if (confused)
	{
		stop();
		clear_thinking();
	}

	if (!pursuing_food && !food_within_grasp && !pursuing_push && !push_within_grasp)
	{
		if (hit_barrier && !responding_to_barrier)
		{
			bool right = false;
			bool left = false;
			bool top = false;
			bool bottom = false;

			if (view_loc.x >= clirect.right)
			{
				right = true;
			}
			if (view_loc.x <= clirect.left)
			{
				left = true;
			}
			if (view_loc.y > clirect.bottom)
			{
				bottom = true;
			}
			if (view_loc.y < clirect.top)
			{
				top = true;

			}





			rotate_eyes(110);
			hit_barrier = false;
			responding_to_barrier = true;
		}
		else if (responding_to_barrier)
		{
			hit_barrier = false;
			response_ticks++;
			if (response_ticks == 50)
			{
				response_ticks = 0;
				responding_to_barrier = false;
			}
		}
		else if (near_barrier)
		{
			turn_shift(20);
		}
	}
	if (pursuing_food)
	{
		Vector vfood = goal_point.SubtractPointFromPoint(&part_loc);
		float dist_to_food = vfood.get_size();


		for (int i = 0; i < awareness.size(); i++)
		{
			if (awareness[i].type == "food")
			{
				
				if (awareness[i].distance < dist_to_food)
				{
					//If this food in awareness is closer than what it was previously seeking, switch to it.
					stop();
					face_point(awareness[i].pObj->part_loc);
					goal_point = awareness[i].pObj->part_loc;
					pursuing_food = true;
					break;
				}
			}
		}

		move_forward(100);
		if (dist_to_food <= radius + 10)
		{
			stop();
			food_within_grasp = true;
			pursuing_food = false;
		}
	}
	else if (food_within_grasp)
	{
		stop();

		for (int i = 0; i < grasp_space.size(); i++)
		{
			if (grasp_space[i].type == "food")
			{
				//food within grasp.
				face_point(grasp_space[i].pObj->part_loc);
				try_eat(grasp_space[i].pObj);
			}
		}
		food_within_grasp = false;
	}

	else if (pursuing_push)
	{
		//Last time it saw the goal, it had a certain perception about it.
		//Since circumstances could've changed, it needs to find the closest match to what it was just pursuing.
		//If it can't, it will be "confused," which will let it reset behavior.

		bool match_found = false;
		int cat = 0;

		float radius = 10;		//a default error margin of 10


		for (int i = 0; i < awareness.size(); i++)
		{
			if (awareness[i].type != push_goal.type)
			{
				//clear mismatch.
				continue;
			}

			Vector dx = awareness[i].pObj->part_loc.SubtractPointFromPoint(&goal_point);

			if (dx.get_size() <= radius)
			{
				//This object is the right type and it's within the expected distance.
				//It's a good guess that this object is the same object cell was just pursuing.
				//Possible future improvement: check for reasonable changes of direction and speed as well.

				if (push_goal.pObj != awareness[i].pObj)
				{
					int test = 1;
				}
				goal_point = awareness[i].pObj->part_loc;
				push_goal = awareness[i];
				match_found = true;
				break;
			}
		}

		//match_found will be true iff an object in the awareness is of the right type as the original pursuit goal and is within radius distance of the original pursuit point.

		if (!match_found)
		{
			confused = true;
		}

		face_point(goal_point);
		if (push_goal.moving)
		{
			move_forward(300);
		}
		else
		{
			move_forward(100);
		}
		if (push_goal.distance <= radius + 20)
		{
			push_within_grasp = true;
			pursuing_push = false;
		}

	}
	else if (push_within_grasp)
	{
		string prediction = "";
		bool i_know_this = false;
		for (int i = 0; i < grasp_space.size(); i++)
		{
			if (grasp_space[i].type == push_goal.type)
			{
				for (int j = 0; j < conclusions.size(); j++)
				{
					if (conclusions[j].object_type == push_goal.type)
					{
						for (int k = 0; k < conclusions[j].result.size(); k++)
						{
							i_know_this = true;
							prediction += conclusions[j].subject + " " + conclusions[j].action_name + " " + conclusions[j].object_type + " will cause " + conclusions[j].object_type + " " + conclusions[j].result[k] + "\n";
						}
					}
				}

				cout << "Cell " << id << " says: Prediction:\n" << prediction << endl;
				if (!i_know_this)
				{
					cout << "I don't know what will happen.\n";
				}
				
				//Let's just assume it's the right one...
				face_point(grasp_space[i].pObj->part_loc);
				observe_object(grasp_space[i].pObj);
				do_observe = true;
				bored = false;
				ooi = grasp_space[i].pObj;
				response_ticks = 0;
				end_tick = 30;
				try_push(grasp_space[i].pObj);				

				//this indicates a successful push
				Vector da = grasp_space[i].pObj->vel - (push_goal.itsDir * push_goal.itsSpeed);



				break;
			
			}
		}
		push_within_grasp = false;
	}

	else
	{

		if (hungry)
		{
			//At present, there's no reason not to look for food if hungry. But maybe in the future, certain priorities, threats, etc. could take precedence.
			feed_mode = true;
			explore = true;
		}


		//--------------------------------------------------------------
		//Framing perception.		
		//--------------------------------------------------------------

		if (bored)
		{
			frame_push_curious();
		}
		if (feed_mode)
		{
			for (int i = 0; i < awareness.size(); i++)
			{
				if (awareness[i].type == "food")
				{
					stop();
					face_point(awareness[i].pObj->part_loc);
					goal_point = awareness[i].pObj->part_loc;
					pursuing_food = true;
					turn_maneuver = false;
					break; //stops searching for anything else. basically focuses.
				}
			}
		}


		
		if (do_observe && !near_barrier)
		{
			//Cell chooses to observe an object.
			stop();
			if (ooi)
			{
				face_point(ooi->part_loc);
				observe_object(ooi);
			}
			else
			{
				confused = true;
			}
			response_ticks++;
			if (response_ticks == end_tick)
			{
				do_observe = false;
				analyze_observation = true;
			}
		}
		else if (analyze_observation)
		{
			analyze();
			analyze_observation = false;
		}
		else
		if (explore)
		{
			if (turn_maneuver)
			{
				rotate_eyes(4);
				move_forward(120);
				response_ticks++;
				if (response_ticks == 20)
				{
					response_ticks = 0;
					turn_maneuver = false;
				}
			}
			else
			{
				move_forward(100);
				bored = true;
			}
		}
		else if (bored)
		{
			//Cells like to push things when they are bored.
			explore = true;
		}
		else
		{
			//Just chilling.
			bored = true;
		}
	}


	//Thinking takes up energy.
	energy_depletion(-2);
}


void cell::analyze()
{


	std::vector<analysis_object> analysis_list;	

	for (int i = 0; i < observations.size()-1; i++)
	{
		analysis_object a1;

		perceptual_object before = observations[i];
		perceptual_object after = observations[i + 1];

		if (before.moving != after.moving)
		{
			a1.changed_moving = true;
			if (before.moving && !after.moving)
			{
				a1.stopped = true;
			}
			else
			{
				a1.moved_it = true;
			}
		}

		float dist_i = before.distance;
		float dist_f = after.distance;
		int delta_dist = int((dist_f - dist_i)*1000);
		if (delta_dist > 0)
		{
			a1.got_further = true;
		}
		if (delta_dist < 0)
		{
			a1.got_closer = true;
		}


		float speed_i = before.itsSpeed;
		float speed_f = after.itsSpeed;
		int delta_speed = int((speed_f - speed_i) * 1000);
		if (delta_speed > 0)
		{
			a1.sped_up = true;
		}
		if (delta_speed < 0)
		{
			a1.slowed_down = true;
		}
		analysis_list.push_back(a1);
	}

	int count_further = 0;
	int count_closer = 0;
	int count_no_change = 0;

	
	bool event_caused_movement = false;
	bool object_reached_stop = false;

	for (int i = 0; i < analysis_list.size(); i++)
	{
		if (analysis_list[i].moved_it == true)
		{
			//This means the observed object began moving while being observed.
			event_caused_movement = true;
		}
		if (event_caused_movement)
		{
			if (analysis_list[i].got_further)
			{
				count_further++;
			}
			if (analysis_list[i].got_closer)
			{
				count_closer++;
			}
		}
		if (analysis_list[i].stopped)
		{
			//This means the observed object stopped while being observed.
			object_reached_stop = true;
		}
		if (object_reached_stop)
		{
			if (event_caused_movement)
			{
				//We can conclude that the object stopped after it had been moving.
			}
		}

		if (analysis_list[i].changed_moving == false && analysis_list[i].got_closer == false && analysis_list[i].got_further == false && analysis_list[i].slowed_down == false && analysis_list[i].sped_up == false)
		{
			//No change is observed
			count_no_change++;
		}
	}



	float num_obs = analysis_list.size();
	float prop_further = float(count_further) / num_obs;
	float prop_closer = float(count_closer) / num_obs;
	float prop_NC = float(count_no_change) / num_obs;


	bool ps_entry_exists = false;
	for (int i = 0; i < push_db.size(); i++)
	{
		if (ooi->type == push_db[i].object_type)
		{
			ps_entry_exists = true;
			float nPush = (push_db[i].num_trials * push_db[i].pushability + prop_further) / (push_db[i].num_trials + 1);
			push_db[i].pushability = nPush;
			push_db[i].num_trials++;
			push_db[i].confidence = float(push_db[i].num_trials) / 100;
		}
	}
	if (!ps_entry_exists)
	{
		//This is a new object. Add new item to push_db.
		pushability_summary ps;
		ps.object_type = ooi->type;
		ps.num_trials = 1;
		ps.pushability = prop_further;
		ps.confidence = 1 / 100;
		push_db.push_back(ps);
	}

	conclusion con;
	con.subject = "I";
	con.action_type = 0;
	if (con.action_type == 0)
	{
		con.action_name = "push";
	}
	con.object_type = ooi->type;

	if (event_caused_movement)
	{
		con.result.push_back("to move");
	}
	if (object_reached_stop)
	{
		con.result.push_back("to stop");
	}
	if (!event_caused_movement && !object_reached_stop)
	{
		con.result.push_back("no result");
	}
	conclusions.push_back(con);



	//Temporary measure. As much of this is, of course.
	observations.clear();

}

bool cell::in_push_db(string item)
{
	for (int i = 0; i < push_db.size(); i++)
	{
		if (item == push_db[i].object_type)
		{
			return true;
		}
	}
	return false;
}

void cell::simulate()
{
	think();
}










//Representation
void cell::draw(HDC hdc, HWND hwnd)
{
	GetClientRect(hwnd, &clirect);


	Vector stri_h;	//sight triangle height vector
	Vector stri_l1; //leg one of sight triangle
	Vector stri_l2; //leg two of sight triangle

	stri_h = sight_dir.MultiplyVectorByScalar(sight_height);
	myPoint sight_end_point = part_loc.AddVectorToPoint(&stri_h);
	stri_l1 = sight_dir;
	stri_l2 = sight_dir;
	myMatrix mat;
	mat.init_RotXY_3x3(sight_angle);
	mat.Rotate_XY_3x3(&stri_l1, &stri_l1);
	mat.init_RotXY_3x3(-1 * sight_angle);
	mat.Rotate_XY_3x3(&stri_l2, &stri_l2);
	
	Vector a1 = stri_l1;
	Vector a2 = stri_l2;
	
	float leg_length = sight_height / cos(sight_angle * 3.14159 / 180);
	stri_l1 = stri_l1.MultiplyVectorByScalar(leg_length);
	stri_l2 = stri_l2.MultiplyVectorByScalar(leg_length);

	myPoint tri_one = part_loc.AddVectorToPoint(&stri_l1);
	myPoint tri_two = part_loc.AddVectorToPoint(&stri_l2);

	
	a1 = a1 * sight_height;
	a2 = a2 * sight_height;

	myPoint arc_one = part_loc + a1;
	myPoint arc_two = part_loc + a2;


	HPEN blackPen = CreatePen(PS_SOLID, 2, COLORREF(0x000000));


	COLORREF bodyColor(0x000000);

	if (hungry)
	{
		bodyColor = COLORREF(0x00FF00);
	}
	else if (bored)
	{
		bodyColor = COLORREF(0xFF0000);
	}
	if (pursuing_push)
	{
		bodyColor = COLORREF(0x0000FF);
	}
	if (confused)
	{
		bodyColor = COLORREF(0x00FFFF);
	}

	HBRUSH bodyBrush = CreateSolidBrush(bodyColor);

	RECT cell_brc;
	cell_brc.left = view_loc.x - radius;
	cell_brc.right = view_loc.x + radius;
	cell_brc.top = view_loc.y - radius;
	cell_brc.bottom = view_loc.y + radius;
	SelectObject(hdc, bodyBrush);
	SelectObject(hdc, blackPen);
	Ellipse(hdc, cell_brc.left, cell_brc.top, cell_brc.right, cell_brc.bottom);
	DeleteObject(blackPen);

	HPEN yellowPen = CreatePen(PS_SOLID, 2, COLORREF(0x00FFFF));
	SelectObject(hdc, yellowPen);
	MoveToEx(hdc, view_loc.x, view_loc.y, NULL);
	LineTo(hdc, sight_end_point.tuple[0], sight_end_point.tuple[1]);
	MoveToEx(hdc, view_loc.x, view_loc.y, NULL);
	LineTo(hdc, arc_one.tuple[0], arc_one.tuple[1]);
	MoveToEx(hdc, view_loc.x, view_loc.y, NULL);
	LineTo(hdc, arc_two.tuple[0], arc_two.tuple[1]);
	RECT arc_brc;
	arc_brc.left = view_loc.x - sight_height;
	arc_brc.right = view_loc.x + sight_height;
	arc_brc.top = view_loc.y - sight_height;
	arc_brc.bottom = view_loc.y + sight_height;
	Arc(hdc, arc_brc.left, arc_brc.top, arc_brc.right, arc_brc.bottom, arc_one.tuple[0], arc_one.tuple[1], arc_two.tuple[0], arc_two.tuple[1]);


	DeleteObject(yellowPen);
	DeleteObject(bodyBrush);

}
bool cell::outside_rect(POINT pt, RECT rect)
{
	if (pt.x >= clirect.right || pt.x <= clirect.left || pt.y >= clirect.bottom || pt.y <= clirect.top)
	{
		return true;
	}
	return false;
}