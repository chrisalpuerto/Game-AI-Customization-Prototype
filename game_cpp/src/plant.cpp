#include "plant.h"


plant::plant(POINT loc, Vector dir, int segLength, world* WORLD)
{


	//DNA variables
	spawn_point = loc;
	growth_dir = dir;
	segment_length = segLength;
	water_suction = 5;
	nutrient_suction = 3;
	eng_gr_req = 2;

	//Physical variables
	water = 0;
	nutrients = 0;
	energy = 0;
	vine_length = 0;
	mature = false;


	pWorld = WORLD;
	pLand = nullptr;
	//What landplot is this plant in?
	pLand = WORLD->getLand(spawn_point);

	//Simulation/representation variables
	alive = true;
	max_speed = 30;
	radius = 7;			//vine radius
	dxdv = 10;			//pixels/vine length
	view_loc = loc;
	part_loc = winPoint_to_myPoint(loc);
	type = "plant";
}

void plant::osmosis()
{
	//absorbs water.
	if (pLand)
	{
		float moisture_initial = pLand->moisture_level;
		pLand->change_moisture(-1 * water_suction);
		float moisture_final = pLand->moisture_level;
		float delta_water = moisture_final - moisture_initial;
		float absorbed_water = -1 * delta_water;
		water += absorbed_water;
	}
	else
	{
		//Error: lost reference to land plot.
	}


	//The following allows the plant to absorb water at every one of its fruit nodes (at 10% effectiveness).
	for (int i = 0; i < fnodes.size(); i++)
	{
		if (fnodes[i].pLand)
		{
			float moisture_initial = fnodes[i].pLand->moisture_level;
			fnodes[i].pLand->change_moisture(-1 * water_suction/10);
			float moisture_final = fnodes[i].pLand->moisture_level;
			float delta_water = moisture_final - moisture_initial;
			float absorbed_water = -1 * delta_water;
			water += absorbed_water;
		}
		else
		{
			//This fruit node's land pointer is null.
		}
	}

}
void plant::absorb_nutrients()
{
	if (pLand)
	{
		float nutrient_initial = pLand->nutrient_level;
		pLand->change_nutrients(-1 * nutrient_suction);
		float nutrient_final = pLand->nutrient_level;
		float delta_nutrient = nutrient_final - nutrient_initial;
		float absorbed_nutrient = -1 * delta_nutrient;
		nutrients += absorbed_nutrient;
	}
	else
	{
		//Error: lost reference to land plot.
	}

	//The following allows the plant to absorb nutrients at every one of its fruit nodes (at 10% effectiveness).
	for (int i = 0; i < fnodes.size(); i++)
	{
		if (fnodes[i].pLand)
		{
			float nutrient_initial = fnodes[i].pLand->nutrient_level;
			fnodes[i].pLand->change_nutrients(-1 * nutrient_suction/10);
			float nutrient_final = fnodes[i].pLand->nutrient_level;
			float delta_nutrient = nutrient_final - nutrient_initial;
			float absorbed_nutrient = -1 * delta_nutrient;
			nutrients += absorbed_nutrient;
		}
		else
		{
			//This fruit node's land pointers is null.
		}
	}


}
void plant::photosynthesis()
{
	//converts water and nutrients into energy, given sufficient sunlight.
	if (pLand)
	{
		if (pLand->sunlight > 60)
		{
			//Sunlight sufficient.

			//3 parts water, 5 part nutrient -> 10 energy.
			if (water - 3 >= 0)
			{
				if (nutrients - 5 >= 0)
				{
					water -= 3;
					nutrients -= 5;
					energy += 10;
				}

			}

		}
	}
	else
	{
		//Error: lost reference to land plot.
	}
}
void plant::grow()
{
	//Main simulation logic of plant.
	update_locations();

	osmosis();
	absorb_nutrients();
	//Suggestion for future improvement: give plant a sense function which activates photosynthesis when there's sunlight, etc.
	photosynthesis();

	if (!mature)
	{
		if (energy - eng_gr_req >= 0)
		{
			//energy sufficient for 1 vine length of growth.

			//But, is there room to grow?
			bool enough_room = false;

			Vector dx = growth_dir;
			dx = dx.MultiplyVectorByScalar((vine_length + 1) * dxdv);
			POINT nLoc = myPoint_to_winPoint(part_loc.AddVectorToPoint(&dx));

			enough_room = PtInRect(&pWorld->brc, nLoc);

			if (enough_room)
			{
				energy -= eng_gr_req;
				vine_length++;
				int segs = vine_length / segment_length;
				if (segs > num_segments)
				{
					//New segment. New fruit node.
					num_segments++;

					//to do: shift nLoc perpendicularly to vine direction.
					food* nFruit = produce_fruit(nLoc);

					land* ptrLand = nullptr;
					for (int i = 0; i < pWorld->land_plots.size(); i++)
					{
						if (pWorld->land_plots[i].point_in_land(nLoc))
						{
							ptrLand = &pWorld->land_plots[i];
							break;
						}
					}

					fruit_node nNode(nLoc, nFruit, 600, 0, ptrLand);
					fnodes.push_back(nNode);
				}
			}
			else
			{
				mature = true;
				//Temporarily disabling plant reproduction (simulation slows when too many spores/plants/fruits)
				//produce_spores();
			}
		}
	}

	for (int i = 0; i < fnodes.size(); i++)
	{		
		if (fnodes[i].pFruit->alive == false)
		{
			if (energy >= 2)
			{
				energy -= 2;
				fnodes[i].energyInvested += 2;
				if (fnodes[i].energyInvested >= fnodes[i].energyRequired)
				{
					fnodes[i].energyInvested = 0;
					food* nFruit = produce_fruit(fnodes[i].location);
					fnodes[i].pFruit = nFruit;
				}
			}
		}
	}


}

void plant::update_locations()
{
	//call this only when a plant is moved.
	//The location of all its nodes and the nodes' attachements must be updated as well.

	bool moving = (int(vel.get_size() * 1000) > 0);

	if (moving)
	{
		for (int i = 0; i < fnodes.size(); i++)
		{
			//update node locations
			myPoint node_loc = part_loc + (growth_dir * ((i+1)*segment_length*dxdv));
			fnodes[i].location = myPoint_to_winPoint(node_loc);

			if (fnodes[i].pFruit->alive)
			{
				//update fruit attachements
				//A loop is necessary, though at most 1 att. is expected, because attachements could be broken.
				for (int j = 0; j < attachements.size(); j++)
				{
					fnodes[i].pFruit->attachements[j].pt = node_loc;
				}
			}
		}
	}
}

food* plant::produce_fruit(POINT pt)
{

	return pWorld->spawn_food(pt, true);
}

void plant::produce_spores()
{
	//Cloud-burst spore production (produces spores in a circle pattern)
	//One-time event.
	float angle = 30;	
	int max = int(360 / angle);
	for (int i = 0; i < max; i++)
	{
		Vector cursor(1, 0, 0);
		myMatrix mat;
		mat.init_RotXY_3x3(i*angle);
		mat.Rotate_XY_3x3(&cursor, &cursor);
		Vector dx = cursor.MultiplyVectorByScalar(segment_length);
		POINT nLoc = myPoint_to_winPoint(part_loc.AddVectorToPoint(&dx));
		pWorld->spawn_spore(nLoc, cursor, 1);
	}
}

void plant::draw(HDC hdc, HWND hwnd)
{
	HPEN brownPen = CreatePen(PS_SOLID, radius, COLORREF(0x082945));
	SelectObject(hdc, brownPen);

	Vector dx = growth_dir;
	dx = dx.MultiplyVectorByScalar(vine_length * dxdv);
	POINT nLoc = myPoint_to_winPoint(part_loc.AddVectorToPoint(&dx));

	MoveToEx(hdc, view_loc.x, view_loc.y, NULL);
	LineTo(hdc, nLoc.x, nLoc.y);
	DeleteObject(brownPen);

}

