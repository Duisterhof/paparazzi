/*
 * Copyright (C) 2014 Hann Woei Ho
 *               2015 Freek van Tienen <freek.v.tienen@gmail.com>
 *               2015 IMAV2015 Masters Group
 *
 * This file is part of Paparazzi.
 *
 * Paparazzi is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * Paparazzi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Paparazzi; see the file COPYING.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

/**
 * @file modules/computer_vision/opticflow/stabilization_opticflow.c
 * @brief Vision based control for Linux based systems
 *
 * Control loops for optic flow based hovering.
 * Computes setpoint for the lower level attitude stabilization to hover.
 */

// Own Header
#include "IMAV2015_stabilization.h"

// Stabilization
#include "firmwares/rotorcraft/stabilization/stabilization_attitude.h"
#include "firmwares/rotorcraft/guidance/guidance_v.h"
#include "autopilot.h"
#include "subsystems/datalink/downlink.h"

// Timing
#include <sys/time.h>

#define CMD_OF_SAT  1500 // 40 deg = 2859.1851

#ifndef VISION_PHI_PGAIN
#define VISION_PHI_PGAIN 300
#endif
PRINT_CONFIG_VAR(VISION_PHI_PGAIN)

#ifndef VISION_PHI_IGAIN
#define VISION_PHI_IGAIN 0
#endif
PRINT_CONFIG_VAR(VISION_PHI_IGAIN)

#ifndef VISION_PHI_DGAIN
#define VISION_PHI_DGAIN 0
#endif
PRINT_CONFIG_VAR(VISION_PHI_DGAIN)

#ifndef VISION_THETA_PGAIN
#define VISION_THETA_PGAIN 300
#endif
PRINT_CONFIG_VAR(VISION_THETA_PGAIN)

#ifndef VISION_THETA_IGAIN
#define VISION_THETA_IGAIN 0
#endif
PRINT_CONFIG_VAR(VISION_THETA_IGAIN)

#ifndef VISION_THETA_DGAIN
#define VISION_THETA_DGAIN 0
#endif
PRINT_CONFIG_VAR(VISION_THETA_DGAIN)

//////////////////////////////////////////////////
///////                                     //////
///////          Timing stuff               //////
///////                                     //////
//////////////////////////////////////////////////

#include <sys/time.h>

#define USEC_PER_MS 1000
#define USEC_PER_SEC 1000000
#define millisleep 1

long diffTime;
int32_t dt = 0;
int32_t test_count = 0;
struct timeval start_time;
struct timeval end_time;

volatile long time_elapsed (struct timeval *t1, struct timeval *t2);
volatile long time_elapsed (struct timeval *t1, struct timeval *t2)
{
	long sec, usec;
	sec = t2->tv_sec - t1->tv_sec;
	usec = t2->tv_usec - t1->tv_usec;
	if (usec < 0) {
	--sec;
	usec = usec + USEC_PER_SEC;
	}
	return sec*USEC_PER_SEC + usec;
}
void start_timer(void);
void start_timer(void) {
	gettimeofday(&start_time, NULL);
}
long end_timer(void);
long end_timer(void) {
	gettimeofday(&end_time, NULL);
	return time_elapsed(&start_time, &end_time);
}



/* Check the control gains */
#if (VISION_PHI_PGAIN < 0)      ||  \
  (VISION_PHI_IGAIN < 0)        ||  \
  (VISION_THETA_PGAIN < 0)      ||  \
  (VISION_THETA_IGAIN < 0)
#error "ALL control gains have to be positive!!!"
#endif

/* Initialize the default gains and settings */
struct visionhover_stab_t visionhover_stab = {
  .phi_pgain = VISION_PHI_PGAIN,
  .phi_igain = VISION_PHI_IGAIN,
  .phi_dgain = VISION_PHI_DGAIN,
  .theta_pgain = VISION_THETA_PGAIN,
  .theta_igain = VISION_THETA_IGAIN,
  .theta_dgain = VISION_THETA_DGAIN,
};

float pre_err_x, pre_err_y;

/**
 * Horizontal guidance mode enter resets the errors
 * and starts the controller.
 */
void guidance_h_module_enter(void)
{

  /* Set rool/pitch to 0 degrees and psi to current heading */
  visionhover_stab.cmd.phi = 0;
  visionhover_stab.cmd.theta = 0;
  visionhover_stab.cmd.psi = stateGetNedToBodyEulers_i()->psi;
}

/**
 * Read the RC commands
 */
void guidance_h_module_read_rc(void)
{
  // TODO: change the desired vx/vy
}

/**
 * Main guidance loop
 * @param[in] in_flight Whether we are in flight or not
 */
void guidance_h_module_run(bool_t in_flight)
{
  /* Update the setpoint */
  stabilization_attitude_set_rpy_setpoint_i(&visionhover_stab.cmd);

  /* Run the default attitude stabilization */
  stabilization_attitude_run(in_flight);
}

/**
 * Update the controls based on a vision result
 * @param[in] *result The opticflow calculation result used for control
 */
void stabilization_visionhover_update(struct visionhover_result_t *result)
{
  
  /* Check if we are in the correct AP_MODE before setting commands */
  if (autopilot_mode != AP_MODE_MODULE) {
    return;
  }
  


  /* Calculate the error if we have enough flow */
  float err_x;
  float err_y;
  err_x = result->deviation_x;
  err_y = result->deviation_y;

  //printf("Phi command = %f, Theta command = %f\n", result->deviation_x, result->deviation_y);
  //printf("blabla\n");
  
  /* Calculate the integrated errors (TODO: bound??) */
  visionhover_stab.err_x_int += err_x;
  visionhover_stab.err_y_int += err_y;
  
  /* Calculate the differential errors (TODO: bound??) */
    
    // Measure the time interval
  usleep(1000* millisleep);
  diffTime = end_timer();
  start_timer();
  dt = (int32_t)(diffTime); // usec
    
    
  visionhover_stab.err_x_diff = (err_x - pre_err_x)*1000/dt;
  visionhover_stab.err_y_diff = (err_y - pre_err_y)*1000/dt;
  pre_err_x = err_x;
  pre_err_y = err_y;
  //printf("dt = %i, x_diff = %f, y_diff = %f\n", dt, visionhover_stab.err_x_diff, visionhover_stab.err_y_diff);
  
  /* Calculate the commands */
  visionhover_stab.cmd.phi   = (visionhover_stab.phi_pgain * err_x
                             + visionhover_stab.phi_igain * visionhover_stab.err_x_int
                             + visionhover_stab.phi_dgain * visionhover_stab.err_x_diff) / 100;
  visionhover_stab.cmd.theta = -(visionhover_stab.theta_pgain * err_y 
                               + visionhover_stab.theta_igain * visionhover_stab.err_y_int
                               + visionhover_stab.theta_dgain * visionhover_stab.err_y_diff) / 100;

  
  /* Bound the roll and pitch commands */
  BoundAbs(visionhover_stab.cmd.phi, CMD_OF_SAT);
  BoundAbs(visionhover_stab.cmd.theta, CMD_OF_SAT);
  

}