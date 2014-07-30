// This file is part of MatrixPilot.
//
//    http://code.google.com/p/gentlenav/
//
// Copyright 2009-2011 MatrixPilot Team
// See the AUTHORS.TXT file for a list of authors of MatrixPilot.
//
// MatrixPilot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// MatrixPilot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with MatrixPilot.  If not, see <http://www.gnu.org/licenses/>.


#include "defines.h"
#include "navigate.h"
#include "behaviour.h"
#include "servoPrepare.h"
#include "../libDCM/mathlibNAV.h"

int16_t tiltError[3] ;
//int16_t desiredTilt[3] = { RMAX , 0 , RMAX } ;  // test case, desired tilt is 45 degrees to the left
//int16_t desiredTilt[3] = { RMAX , 0 , 0 } ;  // test case, desired tilt is 90 degrees to the left
int16_t desiredTilt[3] = { 0 , 0 , RMAX } ;  // test case, level
int16_t desiredRotationRate[3] = { 0 , 0 , 0 } ;
//int16_t actualRotationRate[3] = { 0 , 0 , 0 } ;
int16_t rotationRateError[3] = { 0 , 0 , 0 } ;

// helicalTurnCntrl determines the values of the elements of the bottom row of rmat
// as well as the required rotation rates in the body frame that are required to make a coordinated turn.
// The required values for the bottom row of rmat are placed in the vector desiredTilt.
// Desired tilt is computed from the helical turn parameters from desired climb rate, turn rate, and airspeed.
// The cross product of rmat[6,7,8] with the desiredTilt produces the orientation error.
// The desired rotation rate in the body frame is computed by multiplying desired turn rate times actual tilt vector.
// The rotation rate error is the actual rotation rate vector in the body frame minus the desired rotation rate vector.
// The tilt and rate vectors are then used by roll, pitch, and yaw control to deflect control surfaces.

void helicalTurnCntrl( void )
{
	vector3_normalize( desiredTilt , desiredTilt ) ; // make sure tilt vector has magnitude RMAX
	VectorCross( tiltError , &rmat[6] , desiredTilt ) ; // compute tilt orientation error
	if ( VectorDotProduct( 3 , &rmat[6] , desiredTilt ) < 0 ) // more than 90 degree error
	{
		vector3_normalize( tiltError , tiltError ) ; // for more than 90 degrees, make the tilt error vector parallel to desired axis, with magnitude RMAX
	}

	VectorSubtract( 3 , rotationRateError , omegaAccum , desiredRotationRate ) ;

}