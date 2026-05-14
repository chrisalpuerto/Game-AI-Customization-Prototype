#pragma once
#include "world.h"
#include "view_obj.h"
#include "food.h"
#include "Vector.h"


#include <Windows.h>
#include <vector>
#include "myDynQueue.h"

typedef struct fruit_node
{
	POINT location;
	land* pLand;
	food* pFruit;
	int energyRequired;
	int energyInvested;

	fruit_node(POINT loc, food* fPtr, int eReq, int eInv, land* _pLand)
	{
		location = loc;
		pFruit = fPtr;
		energyRequired = eReq;
		energyInvested = eInv;
		pLand = _pLand;
	}
};

class world;


class plant : public view_obj
{
public:
	float water;			//current water storage. a "physical variable".
	float nutrients;		//current nutrient storage. a "physical variable".
	float energy;			//current energy storage. a "physical variable".
	int vine_length;		//current length of vine. a "physical variable".
	int num_segments;		//current number of complete segments. "physical"

	vector<fruit_node> fnodes;

	POINT spawn_point;		//the point which the plant originates from. a "physical variable".

	Vector growth_dir;		//the direction the vine will grow in. a "DNA variable".
	int segment_length;		//the length of each segment. a "DNA variable".
	float water_suction;		//"strength" of root suction of water. a "DNA variable".
	float nutrient_suction;		//"strength" of root suction of nutrients. a "DNA variable".
	float eng_gr_req;			//energy growth requirement per 1 vine length. "dna"

	world* pWorld;			//Pointer to the plant's world.
	land* pLand;			//Pointer to the land plot the plant is in.

	bool mature;

	plant(POINT loc, Vector dir, int segLength, world* WORLD);


	void osmosis();				//absorb water
	void absorb_nutrients();	//absorb nutrients
	void photosynthesis();		// given sufficient sunlight, turn nutrients and water into energy
	void grow();				// energy into vine length and fruit growth
	void produce_spores();		//one-time, cloud-burst spore production event

	food* produce_fruit(POINT pt);


	void update_locations();


	int dxdv;		//rate of pixels with respect to vine length.
	void draw(HDC hdc, HWND hwnd);



};

