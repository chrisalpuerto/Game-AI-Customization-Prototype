#include "particle.h"



void particle::move(float dt)
{
	Vector dx = vel.MultiplyVectorByScalar(dt);
	part_loc = part_loc.AddVectorToPoint(&dx);
}

void particle::force_stop()
{
	vel = Vector(0, 0, 0);
}

void particle::accelerate(float dt)
{
	Vector dacc = acceleration.MultiplyVectorByScalar(dt);
	vel = vel.AddVectorToVector(&dacc);
	acceleration = Vector(0, 0, 0);

	for (int i = 0; i < attachements.size(); i++)
	{
		Vector dx = attachements[i].pt.SubtractPointFromPoint(&part_loc); //points from particle to att. pt.
		float distance = dx.get_size();
		if (distance >= attachements[i].length)
		{
			Vector R_vec = dx;
			R_vec.make_unit();//Unit of Resistance vector of particle because of attachement				
			Vector neg_R = R_vec*-1;
			//If the particle has any velocity, at this point the attachement brings it to a sudden halt.
			//Of course, this doesn't seem to arise very well from first principles.
			//It should be more dynamic. At every time step, the attachement should exert enough force to stop the particle.
			//As a temporary fix, this is ok.
			float vopp = vel.DotProduct(&neg_R);
			Vector vel_opp = neg_R * vopp;
			vel = vel - vel_opp;
		}
	}
}
void particle::applyForce(Vector force)
{
	if (vel.get_size() < max_speed)
	{
		

		float rem_force = 0;
		std::vector<int> active_attachements;

		for (int i = 0; i < attachements.size(); i++)
		{
			//Apply counter force for each attachement.

			
			Vector dx = attachements[i].pt.SubtractPointFromPoint(&part_loc); //points from particle to att. pt.
			float distance = dx.get_size();

			if (distance >= attachements[i].length)
			{
				//Since the particle is at the length of the attachment, the attachment might be active.
				//Check the force vector to see if it opposes attachement.
				Vector R_vec = dx;
				R_vec.make_unit();//Unit of Resistance vector of particle because of attachement				
				Vector neg_R = R_vec.MultiplyVectorByScalar(-1);
				float F_opp = force.DotProduct(&neg_R);							//Amount of opposing force along attachment axis

				if (F_opp > 0)
				{
					//Attachement is active.

					float diff = attachements[i].strength - F_opp;
					if (diff >= 0)
					{
						//Attachement holds. Opposing force is cancelled.
						Vector fres = R_vec.MultiplyVectorByScalar(F_opp);
						force += fres;
						//If the particle has any velocity, at this point the attachement brings it to a sudden halt.
						//Of course, this doesn't seem to arise very well from first principles.
						//It should be more dynamic. At every time step, the attachement should exert enough force to stop the particle.
						//As a temporary fix, this is ok.
						float vopp = vel.DotProduct(&neg_R);
						Vector vel_opp = neg_R * vopp;						
						vel = vel - vel_opp;
						rem_force = 0;
						break;
					}
					else
					{
						//Attachement is overcome.
						Vector fres = R_vec.MultiplyVectorByScalar(attachements[i].strength);
						force += fres;
						rem_force = -diff;
						active_attachements.push_back(i);
					}
				}

			}
			//Attachements break only if the force is able to overcome their combined strength.
			if (rem_force > 0)
			{
				//After all atachements have been evaluated, some force still remains. -> The active_attachements break.
				for (int i = 0; i < active_attachements.size(); i++)
				{
					int idx = active_attachements[i];
					if (idx > 0)
					{
						copy(attachements.begin() + idx, attachements.end(), attachements.begin() + idx - 1);
						attachements.resize(attachements.size() - 1);
					}
					else
					{
						attachements.clear();
					}
				}
			}

		}
		Vector da = force.DivideVectorByScalar(mass);
		acceleration = acceleration.AddVectorToVector(&da);
		if (int(acceleration.get_size() * 1000) == 0)
		{

			acceleration = acceleration * 0;
		}
	}
}

void particle::Collision(float mass_collide, float speed_x_collide, float speed_y_collide)
{
	if (!ignore_collision)
	{
		float final_speed_x = (vel.tuple[0] * mass + speed_x_collide * mass_collide - mass_collide * (vel.tuple[0] - speed_x_collide)) / (mass_collide + mass);
		float final_speed_y = (vel.tuple[1] * mass + speed_y_collide * mass_collide - mass_collide * (vel.tuple[1] - speed_y_collide)) / (mass_collide + mass);
		//final_speed_x = final_speed_x * -1;

		vel.tuple[0] = final_speed_x;
		vel.tuple[1] = final_speed_y;
	}

}

void particle::applyGravity(particle b)
{
	if (!gravity_exception)
	{
		float G = 200000.0;
		Vector disp = b.part_loc.SubtractPointFromPoint(&part_loc); //points from a to b.
		float distance = disp.get_size();
		disp = disp.DivideVectorByScalar(distance);
		float f = G * mass * b.mass / (distance * distance);
		float acc = f / mass;
		Vector a = disp.MultiplyVectorByScalar(acc);
		acceleration = acceleration.AddVectorToVector(&a);
	}

}

void particle::applyElectricity(particle b)
{
	if (!ignore_electrity)
	{
		Vector disp = b.part_loc.SubtractPointFromPoint(&part_loc);
		float distance = disp.get_size();
		disp = disp.DivideVectorByScalar(distance); //unit vector
		if (distance < (radius + b.radius))
		{
			float charge_product = charge * b.charge;
			if (charge_product > 0)
			{
				//then they have equipolar charge. Apply repellent force. The point of checking this is that I don't want to apply the force unless it's repellent. In the future, I'm sure I'll just use the regular equation to determine direction.

				if (gentle_electric_mode)
				{
					float E = 1.0f;
					float a_scalar = E * charge_product / (distance * distance) / mass;
					Vector a = disp.MultiplyVectorByScalar(a_scalar);

					vel = Vector(0, 0, 0);
					acceleration = acceleration.AddVectorToVector(&a);
				}
				else
				{
					float E = 100.0;
					float a_scalar = E * charge_product / (distance * distance) / mass;
					Vector a = disp.MultiplyVectorByScalar(a_scalar);
					acceleration = acceleration.AddVectorToVector(&a);
				}

			}
		}
	}

}




void particle::attach_to_point(myPoint pt, float str, float length)
{
	attachement nAtt;
	nAtt.pt = pt;
	nAtt.strength = str;
	nAtt.length = length;

	attachements.push_back(nAtt);
}



bool particle::inParticle(myPoint pt)
{
	Vector disp = pt.SubtractPointFromPoint(&part_loc);
	float dist = disp.get_size();
	if (dist <= radius)
	{
		return true;
	}
	else
	{
		return false;
	}
}