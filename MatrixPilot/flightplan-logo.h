// This file is part of MatrixPilot.
//
//    http://code.google.com/p/gentlenav/
//
// Copyright 2009-2012 MatrixPilot Team
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


////////////////////////////////////////////////////////////////////////////////
// UDB LOGO Waypoint handling

// Move on to the next waypoint when getting within this distance of the current goal (in meters)
#define WAYPOINT_RADIUS 		25

// Origin Location
// When using relative waypoints, the default is to interpret those waypoints as relative to the
// plane's power-up location.  Here you can choose to use any specific, fixed 3D location as the
// origin point for your relative waypoints.
//
// USE_FIXED_ORIGIN should be 0 to use the power-up location as the origin for relative waypoints.
// Set it to 1 to use a fixed location as the origin, no matter where you power up.
// FIXED_ORIGIN_LOCATION is the location to use as the origin for relative waypoints.  It uses the
// format { X, Y, Z } where:
// X is Logitude in degrees * 10^7
// Y is Latitude in degrees * 10^7
// Z is altitude above sea level, in meters, as a floating point value.
// 
// If you are using waypoints for an autonomous landing, it is a good idea to set the altitude value
// to be the altitude of the landing point, and then express the heights of all of the waypoints with
// respect to the landing point.
// If you are using OpenLog, an easy way to determine the altitude of your landing point is to
// examine the telemetry after a flight, take a look in the .csv file, it will be easy to spot the
// altitude, expressed in meters.

#define USE_FIXED_ORIGIN		0
// #define FIXED_ORIGIN_LOCATION	{ -1219950467, 374124664, 2.00 }	// A point in Baylands Park in Sunnyvale, CA
// #define FIXED_ORIGIN_LOCATION	{ -797866871, 435341708, 182.0 }	// North Field field center launch point 168m above sea lev 
// #define FIXED_ORIGIN_LOCATION	{-797279923, 434894384, 170.0  }	// OMFC South Field field NW offset DC 
#define FIXED_ORIGIN_LOCATION	{-797278708, 434893087, 170.0}			// OMFC South Field field dead center 
// #define FIXED_ORIGIN_LOCATION	{-801586991, 429692064, 226.2 }		// Bennet Field, Springvale East field 
// #define FIXED_ORIGIN_LOCATION	{-801605927, 429691278, 226.2 }		// Bennet Field, Springvale West field 
// #define FIXED_ORIGIN_LOCATION	{-801595895, 429693672,  226.2}		// Bennet Field, Springvale Center field 
// #define FIXED_ORIGIN_LOCATION	{-801595895, 429693672,  226.2}		// Bennet Field, Springvale Center field 

////////////////////////////////////////////////////////////////////////////////
// UDB LOGO Flight Planning definitions
// 
// The UDB Logo flight plan language lets you use a language similar to Logo, aka Turtle graphics, to
// control your plane.  You are commanding an imaginary "turtle" to move to specific locations, and the
// plane will head towards the turtle.
// 
// You can also control the camera targeting code by switching from the plane turtle, to the camera turtle
// by using the SET_TURTLE(CAMERA) command.  Then logo commands will move the location that the camera
// is targeting, instead of the location to aim the plane.
// 
// Each time you enter waypoint mode, the state is reset and your logo program starts from the top.  If
// you enter RTL mode, the state is reset and your rtlInstructions[] logo program is run instead.
// The following state is cleared when entering waypoint mode or RTL mode: (but not when your program
// ends and starts over)
//   - The plane turtle and camera turtle begin at the plane's current position and altitude.
//   - Both turtles begin pointing in the plane's current heading.
//   - The flags are all turned off.
//   - The pen is down, and the PLANE turtle is active.
// 
// To use UDB Logo, set FLIGHT_PLAN_TYPE to FP_LOGO in options.h.

////////////////////////////////////////////////////////////////////////////////
// Commands
// 
// Use the following commands to create your logo paths:
// 
// HOME				- Return the turtle to the origin, aiming North.
// 
// FD(x)			- Move the turtle forward x meters, in the turtle's current direction.
// BK(x)			- Move the turtle backwards x meters, in the turtle's current direction.
// USE_CURRENT_POS	- Move the turtle to the plane's current {X,Y} position.  Mostly useful
//					  while being sneaky using PEN_UP.

// RT(x)			- Rotate the turtle to the right by x degrees.
// LT(x)			- Rotate the turtle to the left by x degrees.
// SET_ANGLE(x)		- Set the turtle to point x degrees clockwise from N.
// USE_CURRENT_ANGLE- Aim the turtle in the direction that the plane is currently headed.
// USE_ANGLE_TO_GOAL- Aim the turtle in the direction of the goal from the location of the plane.

// EAST(x)			- Move the turtle x meters East.
// WEST(x)			- Move the turtle x meters West.
// SET_X_POS(x)		- Set the X value of the turtle (meters East of the origin) to x.
// 
// NORTH(y)			- Move the turtle y meters North.
// SOUTH(y)			- Move the turtle y meters South.
// SET_Y_POS(y)		- Set the Y value of the turtle (meters North of the origin) to y.
// 
// SET_POS(x, y)	- Set both x and y at the same time.
// SET_ABS_POS(x, y)- Set absolute X,Y location (long,lat) in degrees * 10^7

// ALT_UP(z)		- Gain z meters of altitude.
// ALT_DOWN(z)		- Drop z meters of altitude.
// SET_ALT(z)		- Set altitude to z.

// SPEED_INCREASE(x)- Increases the target speed by x m/s
// SPEED_DECREASE(x)- Decreases the target speed by x m/s
// SET_SPEED(x)		- Sets the target speed to x m/s

// REPEAT(n)		- Repeat all of the instructions until the matching END, n times
// REPEAT_FOREVER	- Repeat all of the instructions until the matching END, forever
// END				- End the current REPEAT loop or Subroutine definition

// IF_EQ(val, x)	- Looks up a system value (listed below) and checks if it's equal to x.
// 					  If so, runs commands until reaching ELSE or END.  If not, skips to ELSE 
//					  and runs until END, or just skips to END if there's no ELSE.
//					  Available IF commands: IF_EQ(equal), IF_NE(not equal), 
//					  IF_GT(val>x), IF_LT(val<x),IF_GE(val>=x), IF_LE(val<=x).
// ELSE				- Starts a list of commands that get run if the preceding IF failed.

// PEN_UP			- While the pen is up, logo code execution does not stop to wait for the
// 					  plane to move to each new position of the turtle before continuing.
//					  This allows you to use multiple logo instructions to get the turtle to
//					  the next goal location before commanding the plane to fly there by 
//					  putting the pen back down.
// PEN_DOWN			- When the pen is down, the plane moves to each new position of the turtle
//					  before more logo instructions are interpereted.
// PEN_TOGGLE		- Toggle the pen between up and down.

// SET_TURTLE(T)	- Choose to control either the plane's turtle, or the camera turtle.
//					  Use either SET_TURTLE(PLANE) or SET_TURTLE(CAMERA).


// Commands for Modifying Flags
// 
// FLAG_ON(F)		- Turn on flag F.  (See below for a list of flags.)
// FLAG_OFF(F)		- Turn off flag F.
// FLAG_TOGGLE(F)	- Toggle flag F.
// 
// The supported flags are the following:
// 
// F_TAKEOFF		- More quickly gain altitude at takeoff.
// F_INVERTED		- Fly with the plane upside down. (only if STABILIZE_INVERTED_FLIGHT is set to 1 in options.h)
// F_HOVER			- Hover the plane with the nose up. (only if STABILIZE_HOVER is set to 1 in options.h)
//					  NOTE: while hovering, no navigation is performed, and throttle is under manual control.
// F_TRIGGER		- Trigger an action to happen at this point in the flight.  (See the Trigger Action section of the options.h file.) 
// F_ALTITUDE_GOAL	- Climb or descend to the given altitude.
// F_CROSS_TRACK	- Navigate using cross-tracking.  Best used for longer flight legs.
// F_LAND			- Fly with the throttle off.


// Commands for Creating and Calling Subroutines
//
// TO(FUNC)			- Begin defining subroutine FUNC (requires #define FUNC N where N is an
//					  integer, unique among your defined subroutines.  End each subroutine
//					  definition with an END.
// DO(FUNC)			- Run subroutine FUNC.  When it finishes, control returns to the line after
//					  the DO() instruction.
// EXEC(FUNC)		- Call FUNC as though it were the beginning of the logo program.  This will never return.
//					  When/if FUNC finishes, logo will start back at the beginning of the program.
// DO_ARG(FUNC, PARAM) - Run subroutine FUNC, using an integer value as a parameter.
// EXEC_ARG(FUNC, PARAM) - Exec subroutine FUNC, using an integer value as a parameter.
// 
// FD_PARAM			- From within a subroutine, call the FD command using the parameter
//					  passed to this subroutine as the distance.
// RT_PARAM			- From within a subroutine, call the RT command using the parameter
//					  passed to this subroutine as the angle.
// REPEAT_PARAM		- Start a REPEAT block, using the current subroutine's parameter as the
//					  number of times to repeat.
// DO_PARAM(FUNC)	- Call subroutine FUNC with a parameter equal to the current subroutine's
//					  parameter value.
// 
// PARAM_ADD(x)		- Adds x to the current subroutine's current parameter value.  Fun
//					  inside repeats inside subroutines!
// PARAM_SUB(x)		- Subtracts x from the current subroutine's current parameter value.
// PARAM_MUL(x)		- Multiplies the current subroutine's current parameter value by x.
// PARAM_DIV(x)		- Divides the current subroutine's current parameter value by x.
// PARAM_SET(x)		- Sets the current subroutine's current parameter value to x.
// 
// LOAD_TO_PARAM(val) - Loads a system value (listed below) into the current subroutine's parameter value.
// 
// All parameter-related commands: 
//		FD_PARAM, BK_PARAM, RT_PARAM, LT_PARAM, SET_ANGLE_PARAM, 
//		EAST_PARAM, WEST_PARAM, NORTH_PARAM, SOUTH_PARAM, ALT_UP_PARAM, ALT_DOWN_PARAM, 
//		SET_X_POS_PARAM, SET_Y_POS_PARAM, SET_ALT_PARAM, 
//		SPEED_INCREASE_PARAM, SPEED_DECREASE_PARAM, SET_SPEED_PARAM
//		REPEAT_PARAM, DO_PARAM(FUNC), EXEC_PARAM(FUNC)
//		PARAM_SET(x), PARAM_ADD(x), PARAM_SUB(x), PARAM_MUL(x), PARAM_DIV(x)
//		IF_EQ_PARAM(x), IF_NE_PARAM(x), IF_GT_PARAM(x), IF_LT_PARAM(x), IF_GE_PARAM(x), IF_LE_PARAM(x)
//
// Folowing SONAR ALTITUDE COMMANDS will recalibrate altitude using sonar altitude measured IN CENTIMETERS, 
// for precision landing or accuracy in navigating descend trajectory over a clear field:
// 		ALT_UP_SNR, ALT_DOWN_SNR, ALT_UP_PARAM_SNR, ALT_DOWN_PARAM_SNR, SET_ALT_SNR, SET_ALT_PARAM_SNR
//
// Folowing BAROMETRIC ALTITUDE COMMANDS **** NOT APPLICABLE FOR THIS VERSION-RELEASE *****
// will trigger altitude calibration using barometer based altitude measured IN CENTIMETERS,for 
// precision altitude navigation over a relatively fair weather where barometric pressure is stable:
// 		ALT_UP_BAR, ALT_DOWN_BAR, ALT_UP_PARAM_BAR, ALT_DOWN_PARAM_BAR, SET_ALT_BAR, SET_ALT_PARAM_BAR
//
// SET_INTERRUPT(f) - Sets a user-defined logo function to be called at 40Hz.  Be careful not to modify
//					  the turtle location from within your interrupt function unless you really want to!
//					  Usually you'll just want your interrupt function to check some condition, and do
//					  something only if it's true.  (Like fly home only if you get too far away.)
// CLEAR_INTERRUPT	- Clears/disables the interrupt function.  Not usually needed.


// System Values for use with LOAD_TO_PARAM(val) and IF_XX() commands
// 
// DIST_TO_HOME			- in m
// DIST_TO_GOAL			- in m
// ALT					- in m
// ALT_SONAR			- in cm
// ALT_GRD_BAROMETER	- in cm  		(DEV-WIP)
// ALT_AGL_BAROMETER	- in cm  		(DEV-WIP)  
// ALT_ASL_BAROMETER	- in cm  		(DEV-WIP) 
// PRESSURE_BAROMETER	- in PA  		(DEV-WIP)
// TEMPERATURE_BAROMETER- in celcius  	(DEV-WIP)
// CURRENT_ANGLE		- in degrees. 0-359 (clockwise, 0=North)
// ANGLE_TO_HOME		- in degrees. 0-359 (clockwise, 0=North)
// ANGLE_TO_GOAL		- in degrees. 0-359 (clockwise, 0=North)
// REL_ANGLE_TO_HOME	- in degrees. -180-179 (0=heading directly towards home. clockwise offset is positive)
// REL_ANGLE_TO_GOAL	- in degrees. -180-179 (0=heading directly towards goal. clockwise offset is positive)
// GROUND_SPEED			- in cm/s
// AIR_SPEED			- in cm/s
// AIR_SPEED_Z			- in cm/s
// WIND_SPEED			- in cm/s
// WIND_SPEED_X			- in cm/s
// WIND_SPEED_Y			- in cm/s
// WIND_SPEED_Z			- in cm/s
// PARAM				- current param value
// XX_INPUT_CHANNEL		- channel value from 2000-4000 (any channel defined in options.h, e.g. THROTTLE_INPUT_CHANNEL)


////////////////////////////////////////////////////////////////////////////////
// Notes:
//	- Altitudes are relative to the starting point, and the initial altitude goal is 100 meters up.
//	- All angles are in degrees.
//	- Repeat commands and subroutines can be nested up to 12-deep.
//	- If the end of the list of instructions is reached, we start over at the top from the current location and angle.
//    This does not take up one of the 12 nested repeat levels.
//	- If you use many small FD() commands to make curves, I suggest enabling cross tracking: FLAG_ON(F_CROSS_TRACK)
//	- All Subroutines have to appear after the end of your main logo program.


////////////////////////////////////////////////////////////////////////////////
// Define the main flight plan as:
// 
// #define FOO 1
// 
// const struct logoInstructionDef instructions[] = {
//		instruction1
//		instruction2 
//		etc.
//		END
//		
//		TO (FOO)
//			etc.
//		END
//	} ;
// 
// and the Failsafe RTL course as:
// 
// #define BAR 2
// 
// const struct logoInstructionDef rtlInstructions[] = {
//		instruction1
//		instruction2 
//		etc.
//		END
//		
//		TO (BAR)
//			etc.
//		END
//	} ;


////////////////////////////////////////////////////////////////////////////////
// Main Flight Plan

// //////  FUNCTIONS  //////
#define PP_SWITCH				1
#define TM_SWITCH				2
#define	TM_EIGHT				3
#define TM_TURN_RIGHT			4
#define TM_TURN_LEFT			5
#define TM_LOITER				6
#define TM_LOITER_RIGHT			7
#define TM_LOITER_LEFT			8
#define CM_RET_LP				9
#define CM_SET_ALT				10
#define CM_CW_AUTOTAKEOFF		11
#define CM_CC_AUTOTAKEOFF		12
#define CM_CC_RECTANGLE			13
#define CM_CW_RECTANGLE			14
#define CM_SET_SPEED			15
#define CM_RTSR_90				16
#define CM_LTSR_90				17
#define CM_CKMAX_FLIGHTDIST		18
#define CM_AUTOLAND				19
#define CM_LAND_SONAR			20
#define CM_DIST_RETTOHOME		21

// //////  PARAMETERS  //////
#define PM_SHORT_RTGLLEG		110  // m  opt. 120
#define PM_MLONG_RTGLLEG		130  // m  opt. 150
#define PM_XLONG_RTGLLEG		155  // m  opt. 180
#define PM_TKOFF_SNRALT			320  // cm, takeoff sonar low alt. threshold
#define PM_TKOFF_GPSALT			3	 // m, takeoff GPS low alt. threshold
#define PM_SRNDTURN_FD			10 	 // m
//  LANDING
#define PM_LANDG_OFFSET			196  // m, OPTIONAL, meters from final pos. target to start landing span
#define PM_LANDG_SNR_OFFSET		76   // m, OPTIONAL, meters from final pos. target to start sonar landing span  
#define PM_LANDG_SPAN			195  // m, landing, approach to sonar managed soft touch-down
#define PM_LANDG_APRCH_GPSALT	4    // m, start of landing field LT/min. alt. threshold
#define PM_LANDG_APRCH_SNRALT	380  // cm, start of landing field sonar LT/min. alt. threshold
#define PM_LANDG_FSTSEG_SNRALT  280  // cm, FIRST SEGMENT landing sonar alt. threshold
#define PM_LANDG_SECSEG_SNRALT  180  // cm, SECOND SEGMENT landing sonar alt. threshold
#define PM_LANDG_TRDSEG_SNRALT  80   // cm, THIRD SEGMENT landing sonar alt. threshold
#define PM_LANDG_FRTSEG_SNRALT  5    // cm, FOURTH SEGMENT landing sonar alt. threshold
#define PM_LANDG_FSTSEG_GPSALT  3  	 // m, FIRST SEGMENT landing sonar alt. threshold
#define PM_LANDG_SECSEG_GPSALT  2    // m, SECOND SEGMENT landing sonar alt. threshold
#define PM_LANDG_TRDSEG_GPSALT 	1    // m, THIRD SEGMENT landing sonar alt. threshold
#define PM_LANDG_FRTSEG_GPSALT 	0    // ing, approach to sonar managed soft touch-down
#define PM_LANDG_APRCH_GPSALT	4    // m, start of landing field LT/min. alt. threshold
#define PM_LANDG_APRCH_SNRALT	380  // cm, start of landing field sonar LT/min. alt. threshold
#define PM_LANDG_FSTSEG_SNRALT  280  // cm, FIRST SEGMENT landing sonar alt. threshold
#define PM_LANDG_SECSEG_SNRALT  180  // cm, SECOND SEGMENT landing sonar alt. threshold
#define PM_LANDG_TRDSEG_SNRALT  80   // cm, THIRD SEGMENT landing sonar alt. threshold
#define PM_LANDG_FRTSEG_SNRALT  5    // cm, FOURTH SEGMENT landing sonar alt. threshold
#define PM_LANDG_FSTSEG_GPSALT  3  	 // m, FIRST SEGMENT landing sonar alt. threshold
#define PM_LANDG_SECSEG_GPSALT  2    // m, SECOND SEGMENT landing sonar alt. threshold
#define PM_LANDG_TRDSEG_GPSALT 	1    // m, THIRD SEGMENT landing sonar alt. threshold
#define PM_LANDG_FRTSEG_GPSALT 	0    // m, FOURTH SEGMENT landing sonar alt. threshold
// FOURTH SEGMENT landing sonar alt. threshold
#define PM_LANDG_APCHSEG_FD  	2  	 // m, END OF LANDING APPROACH FLARE-FD distance
#define PM_LANDG_FSTSEG_FD  	3  	 // m, FIRST SEGMENT landing FLARE-FD distance
#define PM_LANDG_SECSEG_FD  	3    // m, SECOND SEGMENT landing FLARE-FD distance
#define PM_LANDG_TRDSEG_FD		2    // m, THIRD SEGMENT landing FLARE-FD distance
//  SPEED CONTROL: 9mps:32.4kph, 10mps:36kph, 11:39.6kph, 12:43.2kph, 13:46.8kph
#define PM_SPEED_LOWTRH			10   // m/sec, speed LT/min. alt. threshold
#define PM_SPEED_HIGHTRH		12   // m/sec, speed LT/min. alt. threshold
#define PM_TXTOGGLE_LOWTRH		2800 // TX 3pos-switch control LT/low threshold
#define PM_TXTOGGLE_MIDTRH		3400 // TX 3pos-switch control LT/mid-GT/high threshold
#define PM_TXKNOB_FSTQTL		2800 // TX knob/slider control LT/1st quartile threshold
#define PM_TXKNOB_SECQTL		3100 // TX knob/slider control LT/2nd quartile threshold
#define PM_TXKNOB_TRDQTL		3450 // TX knob/slider control LT/3rd-GT/4th quartile threshold
#define PM_TXKNOB_FSTSPD		2530 // TX knob/slider control LT/1st knob spread threshold
#define PM_TXKNOB_SECSPD		3460 // TX knob/slider control LT/2nd knob spread threshold
#define PM_TXKNOB_TRDSPD		3050 // TX knob/slider control LT/3rd knob spread threshold
#define PM_TXKNOB_FRTSPD		3230 // TX knob/slider control LT/4th knob spread threshold
#define PM_TXKNOB_FIFSPD		3460 // TX knob/slider control LT/5rd knob spread threshold
//#define PM_TXKNOB_SIXSPD		3700 // TX knob/slider control LT/6th knob spread threshold
#define PM_DISTTO_HOME			160  // m min distance threshold to trigger auto RTH and land 
#define PM_CKMAX_FLIGHTDIST	    1000 // m max distance perimeter threshold to auto trigger RTH and land 
#define PM_LANDRTGL_REPEATS		2  	 // number of semi-rounded rectangles before landing 

const struct logoInstructionDef instructions[] = {
	//////////////////////////////////////////////////////////////////////////////////////////////////
    //  Note: INTERRUPT HANDLER runs function once every 40th of a second
	//  MAIN ROUTINE TO RUN PATTERN BASED ON DX8 or MPX GEAR 3p POSITION SWITCH AND CHANNEL ASSIGNMENT
	//		IMPORTANT: SET SPEED RANGE FROM 9 TO 10 OPTIMIZED FOR GLIDERS OR LIGHT WING LOADING
	//////////////////////////////////////////////////////////////////////////////////////////////////
	IF_LT(LOGO_A_CHANNEL, PM_TXTOGGLE_LOWTRH) //  UP (FROM TX BOTTOM) TOGGLE-SWITCH POSITION - TUNED FOR DX8
		DO (PP_SWITCH)   					//  SWITCH TO Rectangular Semi-round Pattern
    ELSE
        IF_LT(LOGO_A_CHANNEL, PM_TXTOGGLE_MIDTRH) //  MIDDLE TOGGLE-SWITCH POSITION
			DO (TM_SWITCH) 					//  DO THERMAL 
		ELSE                          		//  DOWN TOGGLE-SWITCH POSITION
			DO(CM_RET_LP)  					//  RETURN TO POWER-ON POINT OR HOME POSITION
		END									//      AND CIRCLE INDEFINITELY
	END
	//////////////////////////////////////////////////////////////////////////////////////////////////
    //  ****************  MAIN PATTERN AND WAYPOINT MANAGEMENT CONTROLS  ****************  
	//  SWITCH TO CW OR CC RECTANGULAR PATTERN WITH RADIUS CONTROL BY KNOB (DX8) OR SLIDER (MPX)
	// 	 Knob Aux 3 with DX8 TX or F Slider in MPX TX
   	TO (PP_SWITCH) 
		IF_GT(LOGO_B_CHANNEL,PM_TXKNOB_FSTSPD) 
			IF_GE(ALT_SONAR, 0)					// Use sonar's accurate low altitude measurement, when enabled
				IF_LT(ALT_SONAR,PM_TKOFF_SNRALT)  // if less than 300cm do autotakeoff
					DO_ARG(CM_CC_AUTOTAKEOFF,PM_SHORT_RTGLLEG) 
				END
			ELSE  							// Otherwise use GPS iffy measurement at times, +- 5 to 10m
				IF_LT(ALT, PM_TKOFF_GPSALT)	// if less than defined parameter do autotakeoff
					DO_ARG(CM_CC_AUTOTAKEOFF,PM_SHORT_RTGLLEG)
				END
			END
      		REPEAT_FOREVER 	
				DO_ARG(CM_CC_RECTANGLE,PM_SHORT_RTGLLEG)
			END
		ELSE
			IF_LT(LOGO_B_CHANNEL,PM_TXKNOB_SECSPD)  	//  SECOND SPREAD
				IF_GE(ALT_SONAR, 0)
					IF_LT(ALT_SONAR,PM_TKOFF_SNRALT)  	// if less than PARAM Do autotakeoff
						DO_ARG(CM_CC_AUTOTAKEOFF,PM_MLONG_RTGLLEG)
					END
				ELSE  		
					IF_LT(ALT,PM_TKOFF_GPSALT) 			// if less than PARAM do autotakeoff
						DO_ARG(CM_CC_AUTOTAKEOFF,PM_MLONG_RTGLLEG)
					END
				END
      			REPEAT_FOREVER  
					DO_ARG(CM_CC_RECTANGLE,PM_MLONG_RTGLLEG)
				END
			ELSE
				IF_LT (LOGO_B_CHANNEL,PM_TXKNOB_TRDSPD)  //  THIRD SPREAD
					IF_GE(ALT_SONAR, 0)
						IF_LT(ALT_SONAR,PM_TKOFF_SNRALT)
							DO_ARG(CM_CC_AUTOTAKEOFF,PM_XLONG_RTGLLEG)
						END
					ELSE
						IF_LT(ALT,PM_TKOFF_GPSALT)
							DO_ARG(CM_CC_AUTOTAKEOFF,PM_XLONG_RTGLLEG)
						END
					END
	      			REPEAT_FOREVER 		
						DO_ARG(CM_CC_RECTANGLE,PM_XLONG_RTGLLEG)
					END
				ELSE
					IF_LT (LOGO_B_CHANNEL,PM_TXKNOB_FRTSPD) //  FOURTH SPREAD
						IF_GE(ALT_SONAR, 0)
							IF_LT(ALT_SONAR,PM_TKOFF_SNRALT)
								DO_ARG(CM_CW_AUTOTAKEOFF,PM_XLONG_RTGLLEG)
							END
						ELSE
							IF_LT(ALT,PM_TKOFF_GPSALT)
								DO_ARG(CM_CW_AUTOTAKEOFF,PM_XLONG_RTGLLEG)
							END
						END
		      			REPEAT_FOREVER 		
							DO_ARG(CM_CW_RECTANGLE,PM_XLONG_RTGLLEG)
						END
					ELSE  

						IF_LT (LOGO_B_CHANNEL,PM_TXKNOB_FIFSPD) //  FIFTH SPREAD
							IF_GE(ALT_SONAR, 0)
								IF_LT(ALT_SONAR,PM_TKOFF_SNRALT)
									DO_ARG(CM_CW_AUTOTAKEOFF,PM_MLONG_RTGLLEG)
								END
							ELSE
								IF_LT(ALT,PM_TKOFF_GPSALT)
									DO_ARG(CM_CW_AUTOTAKEOFF,PM_MLONG_RTGLLEG)
								END
							END
			      			REPEAT_FOREVER 		
								DO_ARG(CM_CW_RECTANGLE,PM_MLONG_RTGLLEG)
							END
						ELSE  									//   MORE THAN 3460 UP TO 3700
							IF_GE(ALT_SONAR, 0)
								IF_LT(ALT_SONAR,PM_TKOFF_SNRALT)
									DO_ARG(CM_CW_AUTOTAKEOFF,PM_SHORT_RTGLLEG)  
								END
							ELSE
								IF_LT(ALT,PM_TKOFF_GPSALT)  
									DO_ARG(CM_CW_AUTOTAKEOFF,PM_SHORT_RTGLLEG)  
								END
							END
			      			REPEAT_FOREVER  
								DO_ARG(CM_CW_RECTANGLE,PM_SHORT_RTGLLEG)
							END
						END
					END
				END
			END
		END
		DO(CM_CKMAX_FLIGHTDIST) 			// 1KM RADIUS FLIGHT PERMIMETER 
	END
	//  ****************  COMMON PROGRAMS  ****************  
	TO (CM_RET_LP)       								// Return to home, semi-round rectangle and land function
		DO(CM_DIST_RETTOHOME)							// if distance from home is more than PARAM then ret. to home
		USE_CURRENT_ANGLE
		IF_GT(LOGO_B_CHANNEL,PM_TXKNOB_FSTSPD) 
			IF_GE(ALT_SONAR, 0)					// Use sonar's accurate low altitude measurement, when enabled
				IF_LT(ALT_SONAR,PM_TKOFF_SNRALT)  // if less than 300cm do autotakeoff
					DO_ARG(CM_CC_AUTOTAKEOFF,PM_SHORT_RTGLLEG) 
				END
			ELSE  							// Otherwise use GPS iffy measurement at times, +- 5 to 10m
				IF_LT(ALT, PM_TKOFF_GPSALT)	// if less than defined parameter do autotakeoff
					DO_ARG(CM_CC_AUTOTAKEOFF,PM_SHORT_RTGLLEG)
				END
			END
      		REPEAT_FOREVER 	
				DO_ARG(CM_CC_RECTANGLE,PM_SHORT_RTGLLEG)
			END
		ELSE
			IF_LT(LOGO_B_CHANNEL,PM_TXKNOB_SECSPD)  	//  SECOND SPREAD
				IF_GE(ALT_SONAR, 0)
					IF_LT(ALT_SONAR,PM_TKOFF_SNRALT)  	// if less than PARAM Do autotakeoff
						DO_ARG(CM_CC_AUTOTAKEOFF,PM_MLONG_RTGLLEG)
					END
				ELSE  		
					IF_LT(ALT,PM_TKOFF_GPSALT) 			// if less than PARAM do autotakeoff
						DO_ARG(CM_CC_AUTOTAKEOFF,PM_MLONG_RTGLLEG)
					END
				END
				REPEAT(PM_LANDRTGL_REPEATS)				// do landing rect. 2x
					DO_ARG(CM_CC_RECTANGLE,PM_MLONG_RTGLLEG)
				END
				DO_ARG(CM_AUTOLAND,PM_LANDG_SPAN)		// do auto land
			ELSE
				IF_LT (LOGO_B_CHANNEL,PM_TXKNOB_TRDSPD)  //  THIRD SPREAD
					IF_GE(ALT_SONAR, 0)
						IF_LT(ALT_SONAR,PM_TKOFF_SNRALT)
							DO_ARG(CM_CC_AUTOTAKEOFF,PM_XLONG_RTGLLEG)
						END
					ELSE
						IF_LT(ALT,PM_TKOFF_GPSALT)
							DO_ARG(CM_CC_AUTOTAKEOFF,PM_XLONG_RTGLLEG)
						END
					END
					REPEAT(PM_LANDRTGL_REPEATS)				// do landing rect. 2x
						DO_ARG(CM_CC_RECTANGLE,PM_XLONG_RTGLLEG)
					END
					DO_ARG(CM_AUTOLAND,PM_LANDG_SPAN)		// do auto land
				ELSE
					IF_LT (LOGO_B_CHANNEL,PM_TXKNOB_FRTSPD) //  FOURTH SPREAD
						IF_GE(ALT_SONAR, 0)
							IF_LT(ALT_SONAR,PM_TKOFF_SNRALT)
								DO_ARG(CM_CW_AUTOTAKEOFF,PM_XLONG_RTGLLEG)
							END
						ELSE
							IF_LT(ALT,PM_TKOFF_GPSALT)
								DO_ARG(CM_CW_AUTOTAKEOFF,PM_XLONG_RTGLLEG)
							END
						END
						REPEAT(PM_LANDRTGL_REPEATS)				// do landing rect. 2x
							DO_ARG(CM_CW_RECTANGLE,PM_XLONG_RTGLLEG)
						END
						DO_ARG(CM_AUTOLAND,PM_LANDG_SPAN)		// do auto land
					ELSE  

						IF_LT (LOGO_B_CHANNEL,PM_TXKNOB_FIFSPD) //  FIFTH SPREAD
							IF_GE(ALT_SONAR, 0)
								IF_LT(ALT_SONAR,PM_TKOFF_SNRALT)
									DO_ARG(CM_CW_AUTOTAKEOFF,PM_MLONG_RTGLLEG)
								END
							ELSE
								IF_LT(ALT,PM_TKOFF_GPSALT)
									DO_ARG(CM_CW_AUTOTAKEOFF,PM_MLONG_RTGLLEG)
								END
							END
							REPEAT(PM_LANDRTGL_REPEATS)				// do landing rect. 2x
								DO_ARG(CM_CW_RECTANGLE,PM_MLONG_RTGLLEG)
							END
							DO_ARG(CM_AUTOLAND,PM_LANDG_SPAN)		// do auto land
						ELSE  									//  SIXTH SPREAD MORE THAN 3460 UP TO 3700
							IF_GE(ALT_SONAR, 0)
								IF_LT(ALT_SONAR,PM_TKOFF_SNRALT)
									DO_ARG(CM_CW_AUTOTAKEOFF,PM_SHORT_RTGLLEG)  
								END
							ELSE
								IF_LT(ALT,PM_TKOFF_GPSALT)  
									DO_ARG(CM_CW_AUTOTAKEOFF,PM_SHORT_RTGLLEG)  
								END
							END
			      			REPEAT_FOREVER  
								DO_ARG(CM_CW_RECTANGLE,PM_SHORT_RTGLLEG)
							END
						END
					END
				END
			END
		END
		DO(CM_CKMAX_FLIGHTDIST) 			// 1KM RADIUS FLIGHT PERMIMETER 
	END
	TO (CM_CW_AUTOTAKEOFF) 					// Clockwise autonomous takeoff
		DO(CM_SET_SPEED)
		PEN_UP
			USE_CURRENT_ANGLE  
			USE_CURRENT_POS
			PARAM_DIV(4)
			BK_PARAM 	   					// use parm to move back from target position as routine's takeoff span 
			PARAM_SUB(7)
			SET_ALT_PARAM					// at 110 param, ALT 20.5 m
			PARAM_ADD(7)
			PARAM_MUL(4)
			FD_PARAM						// FD 110
		PEN_DOWN
		DO_ARG(CM_RTSR_90,PM_SRNDTURN_FD)
		PARAM_DIV(4)
		PARAM_SUB(2)
		SET_ALT_PARAM						// at 110 param, ALT 25.5 m
		PARAM_ADD(2)
		PARAM_MUL(4)
		PARAM_DIV(3)
		PARAM_ADD(5)
		FD_PARAM							// at 110 param, FD 41.66 m
		DO_ARG(CM_RTSR_90,PM_SRNDTURN_FD)
		PARAM_ADD(5)
		SET_ALT_PARAM						// at 110 param, ALT 46.66
		PARAM_SUB(10)
		PARAM_MUL(3)
		FD_PARAM							// FD 110
		DO_ARG(CM_RTSR_90,PM_SRNDTURN_FD)
		PARAM_DIV(2)
		PARAM_SUB(4)
		SET_ALT_PARAM						// at 110 param, ALT 51
		PARAM_ADD(4)
		PARAM_MUL(2)
		PARAM_DIV(3)
		PARAM_ADD(5)
		FD_PARAM							// at 110 param, FD 41.66 m
		DO_ARG(CM_RTSR_90,PM_SRNDTURN_FD)
		PARAM_SUB(5)
		PARAM_MUL(3)
	END
	TO (CM_CC_AUTOTAKEOFF) 					// Counter-clockwise autonomous takeoff
		DO(CM_SET_SPEED)
		PEN_UP
			USE_CURRENT_ANGLE  		
			USE_CURRENT_POS
			PARAM_DIV(4)
			BK_PARAM 	   					// use parm to move back from target position as routine's takeoff span 
			PARAM_SUB(7)
			SET_ALT_PARAM					// at 110 param, ALT 20.5 m
			PARAM_ADD(7)
			PARAM_MUL(4)
			FD_PARAM						// FD 110
		PEN_DOWN
		DO_ARG(CM_LTSR_90,PM_SRNDTURN_FD)
		PARAM_DIV(4)
		PARAM_SUB(2)
		SET_ALT_PARAM						// at 110 param, ALT 25.5 m
		PARAM_ADD(2)
		PARAM_MUL(4)
		PARAM_DIV(3)
		PARAM_ADD(5)
		FD_PARAM							// at 110 param, FD 41.66 m
		DO_ARG(CM_LTSR_90,PM_SRNDTURN_FD)
		PARAM_ADD(5)
		SET_ALT_PARAM						// at 110 param, ALT 46.66
		PARAM_SUB(10)
		PARAM_MUL(3)
		FD_PARAM							// FD 110
		DO_ARG(CM_LTSR_90,PM_SRNDTURN_FD)
		PARAM_DIV(2)
		PARAM_SUB(4)
		SET_ALT_PARAM						// at 110 param, ALT 51
		PARAM_ADD(4)
		PARAM_MUL(2)
		PARAM_DIV(3)
		PARAM_ADD(5)
		FD_PARAM							// at 110 param, FD 41.66 m
		DO_ARG(CM_LTSR_90,PM_SRNDTURN_FD)
		PARAM_SUB(5)
		PARAM_MUL(3)
	END
	//  CLOCKWISE RECTANGLE:
	TO (CM_CW_RECTANGLE)
		SET_INTERRUPT(CM_SET_ALT) 			// SET ALT PER THROTTLE POSITION
		DO(CM_SET_SPEED)
		REPEAT(2)
			FD_PARAM
			DO_ARG(CM_RTSR_90,PM_SRNDTURN_FD)
			PARAM_DIV(3)
			PARAM_ADD(5)
			FD_PARAM
			DO_ARG(CM_RTSR_90,PM_SRNDTURN_FD)
			PARAM_SUB(5)
			PARAM_MUL(3)
		END
		CLEAR_INTERRUPT	
	END
	//  COUNTER CLOCKWISE RECTANGLE: 
	TO (CM_CC_RECTANGLE)
		SET_INTERRUPT(CM_SET_ALT) 
		DO(CM_SET_SPEED)			
		REPEAT(2)
			FD_PARAM
			DO_ARG(CM_LTSR_90,PM_SRNDTURN_FD)
			PARAM_DIV(3)
			PARAM_ADD(5)
			FD_PARAM
			DO_ARG(CM_LTSR_90,PM_SRNDTURN_FD)
			PARAM_SUB(5)
			PARAM_MUL(3)
		END
		CLEAR_INTERRUPT	
	END  
	TO(CM_LTSR_90)       							// Right turn 90 degrees, semi round
	    REPEAT(9)       
	        FD_PARAM    
	        LT(10)    
	    END     
	END  
	TO(CM_RTSR_90)       							// Right turn 90 degrees, semi round
	    REPEAT(9)       
	        FD_PARAM 
	        RT(10)    
	     END     
	END 
	TO(CM_SET_SPEED)        				
		IF_LT(LOGO_C_CHANNEL,PM_TXTOGGLE_MIDTRH)  
			SET_SPEED(PM_SPEED_LOWTRH)				// Light payload and wingloading cruising green speed
		ELSE   										//_GT PARAM
			SET_SPEED(PM_SPEED_HIGHTRH)				// Added payload and wingloading speed
		END
	END
	TO(CM_SET_ALT)        							//  Altitude control  
		LOAD_TO_PARAM(THROTTLE_INPUT_CHANNEL)
		PARAM_SUB(2250)								//  Baseline throttle input: c. 1/4 throttle, 90m mid optimized for DX8
		PARAM_DIV(4)								//  Scale: 5 eq. to 340m 1,115.49ft; 4 to 425m 1394.36ft; 3 to 566.66m 1859.12ft
		IF_LT_PARAM(50)								//  Min. altitude 50 m
			SET_ALT(50)				
		ELSE 	
			SET_ALT_PARAM   						//  Proportional to channel pos. from 50m to max. altitude of 340m or 1,115.49ft
		END
	END
	TO (CM_CKMAX_FLIGHTDIST)
		IF_GT(DIST_TO_HOME,PM_CKMAX_FLIGHTDIST)	 	// PARAM max distance from home
			DO(CM_RET_LP)
		END
	END
	TO (CM_DIST_RETTOHOME)       					// Return to home, based on distance from home
		IF_GT(DIST_TO_HOME,PM_DISTTO_HOME)
			SET_INTERRUPT(CM_SET_ALT) 
			DO(CM_SET_SPEED)
			HOME  									// Fly home
			CLEAR_INTERRUPT	
		END
	END

	//  ****************  AUTONOMOUS SOFT LANDING PROGRAM  ****************  
	TO (CM_AUTOLAND)								// Landing approach
		PEN_UP										// ALIGNS LANDING WITH PATTERN'S LAST LEG
			USE_CURRENT_POS    						// aim for the current flightplan x,y position target
			USE_ANGLE_TO_GOAL  						// aim for the current flightplan angle target
			BK_PARAM 	   							// defines total landing start distance from fin. goal
			//BK(PM_LANDG_OFFSET)					// defines total landing start distance from fin. goal
			PARAM_DIV(10)							// convert m to cm for sonar alt
			FLAG_ON(F_LAND)    						// TURN OFF THROTTLE TO start powerless landing glide
			SET_ALT_PARAM							// At 200 param, this is eq. to 20 m alt
			PARAM_MUL(2)
			FD_PARAM								// At 200 param, this is eq. to 40 m
			PARAM_DIV(4)
			SET_ALT_PARAM							// At 200 param, this is eq. to 10 m alt
			PARAM_MUL(4)
			FD_PARAM								// At 200 param, this is eq. to 40 m 
		PEN_DOWN
		IF_GE(ALT_SONAR, 0)							// Chk for a sonar attached, if none, value would be -1
			IF_LT(ALT_SONAR,PM_LANDG_APRCH_SNRALT) 	// check altitude using sonar data and if less than param
				FLAG_OFF(F_LAND) 					// throttle up for flare to moderate descend
				SET_ALT_SNR(PM_LANDG_APRCH_SNRALT) 	// At 200 param, this is eq. to 380 cm sonar alt
				FD(PM_LANDG_APCHSEG_FD)				// ADD flare dist. (FD) 
				FLAG_ON(F_LAND) 					// Throttle off and continue powerless landing trajectory
			ELSE 
				SET_ALT_SNR(PM_LANDG_APRCH_SNRALT)  // At 200 param, this is eq. to 380 cm alt
			END
		ELSE
			IF_LT(ALT,PM_LANDG_APRCH_GPSALT)  		// Use ALT if no sonar sensor is attached
				FLAG_OFF(F_LAND) 
				SET_ALT(PM_LANDG_APRCH_GPSALT)  	// At 200 param, this is eq. to 4 m alt
				FD(PM_LANDG_APCHSEG_FD)				// ADD flare dist. (FD)
				FLAG_ON(F_LAND) 					// Throttle off and continue powerless glide	
			END
		END
		FD_PARAM									// At 200 m param, eq. to 40 m, AT THIS POINT,  120m distance
		PARAM_MUL(2)								// define 80 meters landing span PARAM for CM_LAND_SONAR
		DO_PARAM(CM_LAND_SONAR)
	END		
	TO (CM_LAND_SONAR)								// Manage soft landing
		PEN_UP							
			USE_CURRENT_POS    						// aim for the current flightplan x,y position target
			USE_ANGLE_TO_GOAL  						// aim for the current flightplan angle target
			BK_PARAM 								// At 200 m param, setup 80 meters landing span from goal
			//BK(PM_LANDG_SNR_OFFSET)				// PM_LANDG_SNR_OFFSET defined landing span from goal
			PARAM_DIV(4)							// Define FOUR landing segments, each with 20m span at 200 PARAM
		PEN_DOWN
		IF_GE(ALT_SONAR, 0)							// FST (first) landing segment
			IF_LT(ALT_SONAR,PM_LANDG_FSTSEG_SNRALT)
				FLAG_OFF(F_LAND) 					
				SET_ALT_SNR(PM_LANDG_FSTSEG_SNRALT) 
				FD(PM_LANDG_FSTSEG_FD) 				
				FLAG_ON(F_LAND) 					// Throttle off and continue powerless glide
			ELSE
				SET_ALT_SNR(PM_LANDG_FSTSEG_SNRALT)
			END
		ELSE
			SET_ALT(PM_LANDG_FSTSEG_GPSALT)
		END
		FD_PARAM
		//PEN_DOWN
		IF_GE(ALT_SONAR, 0)							// SEC (second) landing segment
			IF_LT(ALT_SONAR,PM_LANDG_SECSEG_SNRALT)  
				FLAG_OFF(F_LAND) 
				SET_ALT_SNR(PM_LANDG_SECSEG_SNRALT)
				FD(PM_LANDG_SECSEG_FD)	
				FLAG_ON(F_LAND) 			
			ELSE
				SET_ALT_SNR(PM_LANDG_SECSEG_SNRALT)
			END
		ELSE
			SET_ALT(PM_LANDG_SECSEG_GPSALT)
		END											// If conditions not true, skip this segment, continue landing trajectory
		FD_PARAM
		IF_GE(ALT_SONAR, 0)							//  TRD (third) landing segment
			IF_LT(ALT_SONAR,PM_LANDG_TRDSEG_SNRALT) 
				FLAG_OFF(F_LAND) 
				SET_ALT_SNR(PM_LANDG_TRDSEG_SNRALT)
				FD(PM_LANDG_TRDSEG_FD)
				FLAG_ON(F_LAND) 	
			ELSE
				SET_ALT_SNR(PM_LANDG_TRDSEG_SNRALT)
			END
		ELSE
			SET_ALT(PM_LANDG_TRDSEG_GPSALT)	
		END
		FD_PARAM
		FLAG_ON(F_LAND) 							//  FTH (fourth) landing segment, UNPOWERED level pitch descend till touch down
		FD_PARAM
	END

	//  ****************  THERMAL PROGRAM  ****************  
   	TO (TM_SWITCH) 
		DO(CM_SET_SPEED)							// Select thermal hunt speed		
		FD(80)			
		REPEAT_FOREVER
			DO(TM_EIGHT)
			DO(CM_CKMAX_FLIGHTDIST) 				// FLIGHT PERMIMETER PARAM
		END	
	END 
	TO (TM_EIGHT) 
		CLEAR_INTERRUPT
		DO_ARG(TM_TURN_RIGHT,44)
		RT(10)
		REPEAT(20)
			IF_GE(WIND_SPEED_Z,100)
				SET_INTERRUPT(TM_LOITER)
			END
			FD(15)
		END
		CLEAR_INTERRUPT
		LT(10)
		DO_ARG(TM_TURN_LEFT,44)
		LT(10)
		REPEAT(20)
			IF_GE(WIND_SPEED_Z,100)
				SET_INTERRUPT(TM_LOITER)
			END
			FD(15)
		END
		CLEAR_INTERRUPT
		RT(10)
	END
	TO (TM_TURN_RIGHT)			
		FLAG_ON(F_CROSS_TRACK)		
		PARAM_DIV(10)		
		FD_PARAM		
		PARAM_MUL(2)		
			REPEAT(8)		
				RT(20)		
				FD_PARAM		
			END		
		RT(20)		
		PARAM_DIV(2)		
		FD_PARAM		
		FLAG_OFF(F_CROSS_TRACK)		
	END	
	TO (TM_TURN_LEFT)			
		FLAG_ON(F_CROSS_TRACK)		
		PARAM_DIV(10)		
		FD_PARAM		
		PARAM_MUL(2)		
			REPEAT(8)		
				LT(20)		
				FD_PARAM		
			END		
		LT(20)		
		PARAM_DIV(2)		
		FD_PARAM		
		FLAG_OFF(F_CROSS_TRACK)		
	END
	TO (TM_LOITER)
		IF_LT(LOGO_B_CHANNEL,PM_TXKNOB_FSTQTL)  
			DO_ARG(TM_LOITER_LEFT,3)
		ELSE
			IF_LT(LOGO_B_CHANNEL,PM_TXKNOB_SECQTL)
			DO_ARG(TM_LOITER_LEFT,6)
			ELSE
				IF_LT(LOGO_B_CHANNEL,PM_TXKNOB_TRDQTL)
				DO_ARG(TM_LOITER_RIGHT,6)
				ELSE     					//_GT PARAM last quartile
					DO_ARG(TM_LOITER_RIGHT,3)
				END
			END
		END
	END
	TO (TM_LOITER_RIGHT)
		FLAG_ON(F_CROSS_TRACK)
		FLAG_ON(F_LAND)
			REPEAT(36)
				RT(10)
				FD_PARAM
			END
		FLAG_OFF(F_CROSS_TRACK)
		FLAG_OFF(F_LAND)
		IF_LT(WIND_SPEED_Z,100)
			CLEAR_INTERRUPT
		END
	END
	TO (TM_LOITER_LEFT)
		FLAG_ON(F_CROSS_TRACK)
		FLAG_ON(F_LAND)
			REPEAT(36)
				LT(10)
				FD_PARAM
			END
		FLAG_OFF(F_CROSS_TRACK)
		FLAG_OFF(F_LAND)
		IF_LT(WIND_SPEED_Z,100)
			CLEAR_INTERRUPT
		END
	END 
} ;

//////////////////////////////////////////////////////////////////////////////////////////////////
// RTL Flight Plan
// 
// On entering RTL mode, turn off the engine, fly home, and circle indefinitely until touching down

const struct logoInstructionDef rtlInstructions[] = {
	
	// Use cross-tracking for navigation
	FLAG_ON(F_CROSS_TRACK)

	// Fly home with power before 
	HOME
	
	// Turn off engine for RTL then circle unpowered.

	FLAG_ON(F_LAND)

	// Once we arrive home, aim the turtle in the
	// direction that the plane is already moving.
	// Todo:  capture take off point and angle and
    //     use here instead with landing sequence
	USE_CURRENT_ANGLE
	
	REPEAT_FOREVER
		// Fly a circle (36-point regular polygon)
		REPEAT(36)
			RT(10)
			FD(8)
		END
	END
	
};
