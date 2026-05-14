#include "world.h"

world::world(int cols, int rows, int width, int height, float _delta_time)
{
	num_columns = cols;
	num_rows = rows;
	world_width = width;
	world_height = height;


	SetRect(&brc, 0, 0, world_width, world_height);


	world_time = 0;
	delta_time = _delta_time;

	//Initialize land_plots.

	int column_rem = world_width % num_columns;
	int row_rem = world_height % num_rows;

	int plot_width = (world_width - column_rem) / num_columns;
	int plot_height = (world_height - row_rem) / num_rows;

	land_width = plot_width;
	land_height = plot_height;

	for (int x = 0; x < num_columns; x++)
	{
		for (int y = 0; y < num_rows; y++)
		{
			//(x, y) = top_left of current land plot

			int w = plot_width;
			int h = plot_height;

			POINT pt;
			pt.x = x * w;
			pt.y = y * h;


			if (x == num_columns - 1)
			{
				//On the last column. If there's a remainder, extend width to end.
				w += column_rem;
			}
			if (y == num_rows - 1)
			{
				//On the last row. If there's a remainder, extend height to end.
				h += row_rem;
			}


			land temp(pt, w, h);
			temp.nutrient_level = 100;
			temp.temperature = 50;
			temp.moisture_level = 50;			

			temp.nut_regen = 5.5;
			temp.mid_temp = 50;
			temp.temp_radius = 10;

			if (x % 2 == 0)
			{
				temp.temp_radius = 30;
			}
			if (y % 2 == 0)
			{
				temp.mid_temp = 30;
			}

			temp.calculate_fertility();

			land_plots.push_back(temp);

		}
	}





	//Initialize nature boxes

	//Sun
	SUN = new sun(world_width, world_height);






}


void world::simulate_wind()
{
	WIND.calculate_wind(world_time, world_width, world_height);
}

Vector world::get_wind(POINT pt)
{	
	if (WIND.inBox(pt))
	{
		Vector out;
		out = WIND.dir;
		out.MultiplyVectorByScalar(WIND.magnitude);
		return out;
	}
	else
	{
		return Vector(0, 0, 0);
	}
}

void world::simulate_rain()
{
	RAIN.calculate_rain(world_time, world_width, world_height);
	if (find_spore)
	{
		

	}
}

void world::simulate_sun()
{
	SUN->calculate_sun(world_time, world_width);
}

void world::simulate_land()
{
	//Nature boxes should be simulated before land, so that effects of nature on land can be calculated.



	//Assumes that rain has already been calculated, leaving only changing moisture if landplot intersects with rainbox.

	for (int i = 0; i < land_plots.size(); i++)
	{
		//Temperature
		land_plots[i].calculate_temperature(world_time);


		//Moisture
		if (isOverlapping(RAIN.brc, land_plots[i].brc))
		{
			//It's raining in this landplot.
			//Increase moisture in landplot according to the strength (i.e. magnitude) of the rain.
			land_plots[i].change_moisture(RAIN.magnitude);
		}

		for (int j = 0; j < rains.size(); j++)
		{
			if (isOverlapping(rains[j].brc, land_plots[i].brc))
			{
				//It's raining in this landplot.
				//Increase moisture in landplot according to the strength of the rain.
				land_plots[i].change_moisture(rains[j].magnitude);
			}
		}


		if (land_plots[i].temperature > 70)
		{

			land_plots[i].change_moisture(-.005 * (land_plots[i].temperature / MAX_TEMPERATURE));
		}
		if (land_plots[i].temperature > 95)
		{

			land_plots[i].change_moisture(-1 * (land_plots[i].temperature / MAX_TEMPERATURE));
		}

		//Nutrients
		land_plots[i].change_nutrients(land_plots[i].nut_regen * delta_time);


		//Sunlight
		if (isOverlapping(SUN->brc, land_plots[i].brc))
		{
			//Sun is shining in this landplot.
			//Set sunlight.
			land_plots[i].set_sunlight(SUN->magnitude);
		}
		else
		{
			land_plots[i].set_sunlight(50);
		}



		land_plots[i].calculate_fertility();
	}
}

void world::simulate_world(float step_time)
{
	//step_time is how much time should be simulated since last call to simulate_world
	//delta_time is saved and will be used.
	//reconcile world_time (simulation variable) with running_time (app variable)??

	float final_time = world_time + step_time;


	while (world_time < final_time)
	{
		world_time += delta_time;
		//Think of all the nature boxes as sort of like the forces on the land. 
		//When they're all calculated, the land can be determined and the order of nature box calculations doesn't matter.
		simulate_wind();
		simulate_rain();
		simulate_sun();
		//Think of the land as one of the factors that affect entities (nature boxes do as well)
		simulate_land();


		


		//Now, simulate entities
		DynQueue<view_obj*>::Node* curObj = nullptr;
		objects.traverse(&curObj);
		while (curObj)
		{
			if (curObj->data->alive == false)
			{
				DynQueue<view_obj*>::Node* temp = curObj;
				objects.traverse(&curObj);
				objects.deleteNode(temp);
			}
			else
			{
				curObj->data->accelerate(delta_time);
				curObj->data->view_move(delta_time);
				
				barrier* bar = nullptr;
				if (checkBarrierCollisions(curObj->data->view_loc, &bar))
				{
					curObj->data->Collision(50000, 0, 0);
					if (curObj->data->type == "cell")
					{
						cell* pCell = reinterpret_cast<cell*>(curObj->data);
						pCell->feel_barrier();
					}
				}
				

				if (!PtInRect(&brc, curObj->data->view_loc))
				{
					//World-boundary collision
					curObj->data->Collision(300, 0, 0);
				}

				if (curObj->data->type == "cell")
				{
					int num_objects = objects.getCount();
					cell* pCell = reinterpret_cast<cell*>(curObj->data);
					pCell->see(&objects, num_objects);
					if (pCell->view_loc.x >= world_width || pCell->view_loc.x <= 0 || pCell->view_loc.y >= world_height || pCell->view_loc.y <= 0)
					{
						//pCell->force_stop();
						//pCell->Collision(100, 0, 0);
						pCell->feel_barrier();
					}
					pCell->simulate();
				}
				if (curObj->data->type == "plant")
				{
					plant* pPlant = reinterpret_cast<plant*>(curObj->data);
					pPlant->grow();
				}
				if (curObj->data->type == "spore")
				{
					spore* pSpore = reinterpret_cast<spore*>(curObj->data);

					//Hacky temporary solution. Apply wind.
					//I consider it hacky because, really, wind should apply to all particles.. But I'm worried how that will affect the simulation.

					if (PtInRect(&WIND.brc, pSpore->view_loc))
					{
						Vector force = WIND.dir;
						force = force.MultiplyVectorByScalar(WIND.magnitude);
						pSpore->applyForce(force);
					}
					
					//Should aging be applied to an object externally in world simulation function? Or should aging be part of the object's internal simulation function?
					pSpore->life_time += delta_time;
					pSpore->simulate();
				}
				if (curObj->data->type == "food")
				{
					food* pFood = reinterpret_cast<food*>(curObj->data);
					if (PtInRect(&WIND.brc, pFood->view_loc))
					{
						Vector force = WIND.dir;
						force = force.MultiplyVectorByScalar(WIND.magnitude);
						pFood->applyForce(force);
					}
				}




				objects.traverse(&curObj);
			}
		}




	}


}




food* world::spawn_food(POINT pt, bool attach)
{
	food* fd1 = new food(pt);
	fd1->mass = 20;
	if (attach)
		fd1->attach_to_point(winPoint_to_myPoint(pt), 1000, 25);
	objects.enqueue(reinterpret_cast<view_obj*>(fd1));
	return fd1;
}

void world::spawn_cell(POINT pt, int id)
{
	cell* c1 = new cell(pt, id);
	c1->mass = 50;
	objects.enqueue(reinterpret_cast<view_obj*>(c1));
}

void world::spawn_plant(POINT pt, Vector dir, int seg_length)
{
	plant* p1 = new plant(pt, dir, seg_length, this);
	p1->mass = 100;
	p1->attach_to_point(winPoint_to_myPoint(pt), 1000, 1);
	objects.enqueue(reinterpret_cast<view_obj*>(p1));
}

void world::spawn_spore(POINT pt, Vector dir, float initial_speed)
{
	if (int(dir.get_size()*10) > 10)
	{
		//not a unit vector or a 0 vector.
		return;
	}
	
	//dir may be left as 0, if desired, but it must be either 0 or a unit vector.
	spore* s1 = new spore(pt, this);
	s1->growth_dir = dir;
	s1->mass = 5;
	s1->vel = s1->growth_dir.MultiplyVectorByScalar(initial_speed);
	objects.enqueue(reinterpret_cast<view_obj*>(s1));
}

void world::spawn_linear_barrier(POINT origin, float c0, float c1,float xmin, float xmax)
{
	float* coeff = new float[2];
	coeff[0] = c0;
	coeff[1] = c1;

	barrier* b1 = new barrier(coeff, 1, xmin, xmax, 5, 20, origin);
	b1->mass = 500;
	objects.enqueue(reinterpret_cast<view_obj*>(b1));
	barriers.enqueue(b1);

	POINT tp;
	tp.x = 392;
	tp.y = 700;
	bool test = b1->isCollision(tp);

}

void world::spawn_parabolic_barrier(POINT origin, float c0, float c1, float c2, float xmin, float xmax)
{
	float* coeff = new float[3];
	coeff[0] = c0;
	coeff[1] = c1;
	coeff[2] = c2;

	barrier* b1 = new barrier(coeff, 2, xmin, xmax, 5, 20, origin);
	b1->mass = 500;
	objects.enqueue(reinterpret_cast<view_obj*>(b1));
	barriers.enqueue(b1);
}

void world::spawn_cubic_barrier(POINT origin, float c0, float c1, float c2, float c3, float xmin, float xmax)
{
	float* coeff = new float[4];
	coeff[0] = c0;
	coeff[1] = c1;
	coeff[2] = c2;
	coeff[3] = c3;

	barrier* b1 = new barrier(coeff, 3, xmin, xmax, 5, 20, origin);
	b1->mass = 500;
	objects.enqueue(reinterpret_cast<view_obj*>(b1));
	barriers.enqueue(b1);
}


bool world::checkBarrierCollisions(POINT pt, barrier** bar)
{
	//If at least one barrier collides, returns true;.
	//Saves the address of the first barrier that collides in bar parameter.


	DynQueue<barrier*>::Node* curObj = nullptr;
	barriers.traverse(&curObj);
	while (curObj)
	{
		if (curObj->data->alive)
		{
			if (curObj->data->isCollision(pt))
			{
				*bar = curObj->data;
				return true;
			}
		}
		barriers.traverse(&curObj);
	}
	*bar = nullptr;
	return false;
}

bool world::isOverlapping(RECT r1, RECT r2)
{
	bool p = (r1.left <= r2.left && r2.left <= r1.right);
	bool q = (r1.top <= r2.bottom && r2.bottom <= r1.bottom);
	bool r = (r1.top <= r2.top && r2.top <= r1.bottom);
	bool s = (r1.left <= r2.right && r2.right <= r1.right);

	bool vert = q || r;

	return vert && (p || s);
}

land* world::getLand(POINT pt)
{
//	for (int i = 0; i < land_plots.size(); i++)
//	{
//		if (land_plots[i].point_in_land(pt))
//		{
//			return &land_plots[i];
//		}
//	}

	//check that pt is in world.
	if (PtInRect(&brc, pt))
	{
		int i = pt.x / land_width;
		int j = pt.y / land_height;

		if (i == num_columns)
		{
			//Accounting for the remainder of world_width which may have been included in the border plots.
			i = num_columns - 1;
		}
		if (j == num_rows)
		{
			//Accounting for the remainder of world_height which may have been included in the border plots.
			j = num_rows - 1;
		}
		int idx = i * num_rows + j;
		return &land_plots[idx];

	}
	return nullptr;
}