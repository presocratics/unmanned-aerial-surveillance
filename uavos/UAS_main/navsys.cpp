/*
 * navsys.cpp
 *
 *  Created on: Mar 06, 2013
 *      Author: Shubham Gupta
 */

#include <navsys.hpp>

void* runNavsys(void*)
{
	cout <<"Gimbal angles>>\n";
	cout<<"Tilt\tPan\n";
	//tracking_started=true;
	while(true)
	{
		if(tracking_started==true){
			//cout<<"inside loop\n";
			centroid c=find_centroid();
			gimbal_angle g=gimbal_calculate(c.x,c.y,320,240,g_gimbal_input.tilt,g_gimbal_input.pan);
			//cout<<g.tilt<<"\t"<<g.pan<<"\n";
			g_gimbal_output.tilt=g.tilt;
			g_gimbal_output.pan=g.pan;
		}
		
	}
}