#include "spore.h"

spore::spore(POINT loc, world* WORLD)
{
	water = 0;
	nutrients = 0;
	energy = 0;
	required_energy = 250;
	root_energy = 100;
	water_suction = 5;
	nutrient_suction = 5;
	should_root = 0;
	attempted_root_failed = false;
	seg_length = 4;
	root_prop_dist = 10;

	//growth direction is becomes direction of velocity when spore germinates.
	growth_dir = Vector(1, 1, 0);


	pWorld = WORLD;
	pLand = nullptr;
	life_time = 0;


	alive = true;
	view_loc = loc;
	part_loc = winPoint_to_myPoint(loc);
	max_speed = 30;
	radius = 5;
	type = "spore";
}
void spore::simulate()
{
	if (life_time > 1000)
	{
		//spore has reached the end of its lifespan.
		alive = false;
	}
	else
	{
		if (mode == spore_mode::DORMANT)
		{
			//Search for suitable conditions, take root when such are found.
			land* curLand = pWorld->getLand(view_loc);
			if (curLand)
			{
				if (curLand->fertility > .7)
				{
					//Suitable conditions.
					germinate(curLand);
				}
			}
		}
		else if (mode == spore_mode::GROWING)
		{
			//Absorb nutrients/water, produce energy, spawn plant when possible.
			//Consider this a simulation of an attachment to ground.
			force_stop();	//prevent wind from pushing spore.

			absorb_nutrients();
			osmosis();
			produce_energy();

			//Try to spawn plant.
			int diff = energy - required_energy;
			if (diff >= 0)
			{
				//energy sufficient. -> forget about rooting and grow plant.
				attempted_root_failed = false;
				pWorld->spawn_plant(view_loc, growth_dir, seg_length);
				alive = false;	//set this spore to dead so it will be removed by the world.
			}

			//Setting it to less than zero for now so that it won't do it because the mechanics need work.
			//Roots definitely help it to gather energy faster, however, when it keeps making roots instead of just saving energy, it'll never spawn a plant.
			if (pLand->fertility < 0)
			{
				float prop = energy / required_energy;
				if (prop < .5)
				{
					should_root += .05;
				}
				if (should_root > .75)
				{
					//Fertility of land has been very low while spore has been far from the requirement 17 times.
					//Growing a new root calculated as a good idea.
					if (!grow_root())
					{
						attempted_root_failed = true;
					}
				}
			}
			//The following represents the intention of the spore to generate a new root once it has gathered enough energy/found room.
			if (attempted_root_failed)
			{
				if (grow_root())
				{
					attempted_root_failed = false;
					should_root = 0;
				}
			}





		}
	}
}
void spore::germinate(land* curLand)
{
	if (mode == spore_mode::DORMANT)
	{
		mode = spore_mode::GROWING;
		pLand = curLand;

		//Spore also attaches itself to the land via the particle point.
		attach_to_point(part_loc, 1000, 0);

		if (vel.get_size() > 0)
		{
			growth_dir = vel;	//growth_direction equals current velocity.
			growth_dir.make_unit();
			//If velocity happens to be zero, then we'll leave the growth dir at the default.
			//Perhaps in future, it will be in the direction of the most fertile adjacent land plot.
		}
		force_stop();

	}
}
bool spore::grow_root()
{
	//Costs energy, will delay plant spawning. Spore should calculate when to use this.
	if (energy >= root_energy)
	{	
		//Sufficient energy.
		Vector dx = growth_dir;
		dx = dx.MultiplyVectorByScalar(root_prop_dist);
		POINT nLoc = myPoint_to_winPoint(part_loc.AddVectorToPoint(&dx));

		bool enough_room = PtInRect(&pWorld->brc, nLoc);

		if (enough_room)
		{
			//Sufficient room.
			//Then it's possible to form root.	
			energy -= root_energy;
			land* nLand = pWorld->getLand(nLoc);
			root nRoot(nLoc, nLand, water_suction, nutrient_suction);
			roots.push_back(nRoot);

			//Rotate growth_direction (so that next roots won't go to same place... one way or another, plants do (smth. like) this, with spiralling as well -- though I'll ommit that detail for now).
			myMatrix mat;
			mat.init_RotXY_3x3(5);
			mat.Rotate_XY_3x3(&growth_dir, &growth_dir);

			return true;


		}

		//Rotate growth_direction (so that next roots won't go to same place... one way or another, plants do (smth. like) this, with spiralling as well -- though I'll ommit that detail for now).
		myMatrix mat;
		mat.init_RotXY_3x3(5);
		mat.Rotate_XY_3x3(&growth_dir, &growth_dir);
		return false;
	}
	return false;
}

void spore::absorb_nutrients()
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
		//Error: no land.
	}

	//absorb with roots
	for (int i = 0; i < roots.size(); i++)
	{
		if (roots[i].pLand)
		{
			float nutrient_initial = roots[i].pLand->nutrient_level;
			roots[i].pLand->change_nutrients(-1 * roots[i].nutrient_suction);
			float nutrient_final = roots[i].pLand->nutrient_level;
			float delta_nutrient = nutrient_final - nutrient_initial;
			float absorbed_nutrient = -1 * delta_nutrient;
			nutrients += absorbed_nutrient;
		}
		else
		{
			//This root's land pointer is null.
		}
	}
}
void spore::osmosis()
{
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
		//Error: no land.
	}
	//absorb with roots.
	for (int i = 0; i < roots.size(); i++)
	{
		if (roots[i].pLand)
		{
			float moisture_initial = roots[i].pLand->moisture_level;
			roots[i].pLand->change_moisture(-1 * roots[i].water_suction);
			float moisture_final = roots[i].pLand->moisture_level;
			float delta_water = moisture_final - moisture_initial;
			float absorbed_water = -1 * delta_water;
			water += absorbed_water;
		}
		else
		{
			//This root's land pointer is null.
		}
	}
}
void spore::produce_energy()
{
	//10wat + 15nut --> 5ener

	if (water - 10 >= 0)
	{
		if (nutrients - 15 >= 0)
		{
			water -= 10;
			nutrients -= 15;
			energy += 5;
		}
	}

}


void spore::draw(HDC hdc, HWND hwnd)
{
	HPEN whitePen = CreatePen(PS_SOLID, 2, COLORREF(0xFFFF00));
	HBRUSH whiteBrush = CreateSolidBrush(COLORREF(0xFFFF00));
	RECT brc;
	brc.left = view_loc.x - radius;
	brc.right = view_loc.x + radius;
	brc.top = view_loc.y - radius;
	brc.bottom = view_loc.y + radius;
	SelectObject(hdc, whitePen);
	SelectObject(hdc, whiteBrush);
	Ellipse(hdc, brc.left, brc.top, brc.right, brc.bottom);
	DeleteObject(whitePen);
	DeleteObject(whiteBrush);

	for (int i = 0; i < roots.size(); i++)
	{
		HPEN brownPen = CreatePen(PS_SOLID, radius, COLORREF(0x082945));
		SelectObject(hdc, brownPen);

		MoveToEx(hdc, view_loc.x, view_loc.y, NULL);
		LineTo(hdc, roots[i].location.x, roots[i].location.y);
		DeleteObject(brownPen);
	}

}