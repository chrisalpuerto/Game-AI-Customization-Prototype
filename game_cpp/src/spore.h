#pragma once
#include "particle.h"
#include "world.h"
#include <vector>

enum class spore_mode
{
	DORMANT, GROWING
};

typedef struct root
{
	POINT location;
	land* pLand;
	float water_suction;
	float nutrient_suction;

	root(POINT loc, land* _pLand, float wSuction, float nSuction)
	{
		location = loc;
		pLand = _pLand;
		water_suction = wSuction;
		nutrient_suction = nSuction;
	}
};

class world;

class spore : public view_obj
{
public:
	spore_mode mode;	//current mode


	float water;
	float nutrients;
	float energy;
	float required_energy;		//energy required to create plant.
	float root_energy;			//energy required to create root.

	int seg_length;				//DNA variable. the segment length of the plant that'll grow from this spore.

	int root_prop_dist;			//max root propagation distance.
	float water_suction;
	float nutrient_suction;

	float should_root;				//tracks how much this spore should grow a new root.
	bool attempted_root_failed;		//an attempted grow root operation failed.

	float life_time;

	world* pWorld;		//pointer to spore's world
	land* pLand;		//land where the spore has taken root.


	Vector growth_dir;		//direction which the spore is "facing". roots would grow in that direction. plant will inherit this direction when spore spawns it.	
	vector<root> roots;

	spore(POINT loc, world* WORLD);
	void simulate();
	void germinate(land* curLand);		//begins lifecycle by taking first root into land
	bool grow_root();		//grows new root

	void absorb_nutrients();
	void osmosis();
	void produce_energy();




	void draw(HDC hdc, HWND hwnd);

};

