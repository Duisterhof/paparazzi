/*
 * Copyright (C) Kimberly McGuire
 *
 * This file is part of paparazzi
 *
 */
/**
 * @file "modules/stereocam/stereocam2state/stereocam2state.c"
 * @author Kimberly McGuire
 * This module sends the data retreived from an external stereocamera modules, to the state filter of the drone. This is done so that the guidance modules can use that information for couadcopter
 */

#include "modules/stereocam/stereocam2state/stereocam2state.h"

#include "subsystems/abi.h"
#include "subsystems/datalink/telemetry.h"

#include "subsystems/radio_control.h"
#include "subsystems/gps/gps_datalink.h"

#include "paparazzi.h"
#include "math/pprz_algebra.h"

#include "modules/droplet/droplet.h"

#ifndef STEREOCAM2STATE_SENDER_ID
#define STEREOCAM2STATE_SENDER_ID ABI_BROADCAST
#endif

#ifndef STEREOCAM2STATE_RECEIVED_DATA_TYPE
#define STEREOCAM2STATE_RECEIVED_DATA_TYPE 0
#endif

int8_t win_x, win_y, win_size, win_fitness, fps;
uint8_t cnt_left, cnt_middle, cnt_right;
int16_t nus_turn_cmd=0;
int16_t nus_climb_cmd=0;
int8_t nus_gate_heading=0; // gate heading relative to the current heading, in degrees
int8_t body2cam=0; // offset between heading and camera direction, in degrees
bool nus_gate_detected=0;

uint8_t pos_thresh=10; // steer only if window center is more than pos_thresh of the center
uint8_t fit_thresh=20; // maximal fitness that is still considered as correct detection
uint8_t size_thresh=10; // minimal size that is considered to be a window
uint8_t turn_cmd_max=40; // percentage of MAX_PPRZ
uint8_t climb_cmd_max=10; // percentage of MAX_PPRZ
uint8_t cnt_thresh=80; // threshold for max amount of pixels in a bin (x10)
uint8_t nus_filter_order=1; // complementary filter setting
uint8_t Nmsg2skip=0; // number of camera messages to be skipped


static void send_stereo_data(struct transport_tx *trans, struct link_device *dev)
 {
//	int16_t course=(DegOfRad(gps_datalink.course) / ((int32_t)1e6)); // sent in deca degrees (0.1 deg)
//	int32_t north=gps_datalink.utm_pos.north;
//	int32_t east=gps_datalink.utm_pos.east;
//	int32_t alt=gps_datalink.utm_pos.alt;

	pprz_msg_send_STEREO_DATA(trans, dev, AC_ID,
                         &win_x, &win_y, &win_size, &win_fitness, &nus_turn_cmd, &nus_climb_cmd, &nus_gate_heading, &fps, &cnt_left, &cnt_middle, &cnt_right); // , &course, &north, &east, &alt
 }

void stereocam_to_state(void);

void stereo_to_state_init(void)
{
	register_periodic_telemetry(DefaultPeriodic, PPRZ_MSG_ID_STEREO_DATA, send_stereo_data);
}

void stereo_to_state_periodic(void)
{
//	static uint16_t ii=0;
//	ii++;

	static uint8_t msg_counter=0;

	if (stereocam_data.fresh && stereocam_data.len == 8 && msg_counter<Nmsg2skip) { // length of NUS window detection code
		msg_counter++;
		stereocam_data.fresh = 0;
	}
	else if (stereocam_data.fresh && stereocam_data.len == 8 && msg_counter==Nmsg2skip) { // length of NUS window detection code
	int8_t* pointer=stereocam_data.data; // to transform uint8 message back to int8

	win_x = pointer[0];
    win_y = pointer[1];
    win_size = pointer[2];
    win_fitness = pointer[3];
    fps = pointer[4];
    cnt_left = stereocam_data.data[5];
    cnt_middle = stereocam_data.data[6];
    cnt_right = stereocam_data.data[7];

    msg_counter = 0;
    stereocam_data.fresh = 0;

    /* radio switch */
    static int8_t nus_switch = 0;
    static int8_t gate_heading = 0;

    if (radio_control.values[5] < -1000) nus_switch=1; // this should be ELEV D/R
    else nus_switch=0;

    if (win_size > size_thresh && win_fitness > fit_thresh) // valid gate detection
    {
    	// nus_turn_cmd=MAX_PPRZ/100*turn_cmd_max*nus_switch*win_x/64;
    	nus_turn_cmd=0;

    	gate_heading=30*win_x/64+body2cam;
    	nus_gate_detected=nus_switch;

    	// nus_climb_cmd=MAX_PPRZ*climb_cmd_max/100*nus_switch*win_y/48*100/(2*win_size); // gate size is 1 meter, win size is half of the gate size in pixels
    	// TODO change climb cmd based on body pitch
    	nus_climb_cmd=0;
    }
    else if (win_size < size_thresh && win_fitness > fit_thresh) // incomplete window detected, use previous command
    {
    	// keeping the same command
    	nus_gate_detected=0;
    	nus_climb_cmd=0;
    	gate_heading=0;
    	nus_turn_cmd=0;
    }
    else // no window detected - if (win_fitness < fit_thresh)
    {
    	if (cnt_left > cnt_thresh && cnt_right < cnt_thresh) {
			nus_turn_cmd=nus_switch*MAX_PPRZ/100*turn_cmd_max;
    	}
    	else if (cnt_left < cnt_thresh && cnt_right > cnt_thresh) {
    		nus_turn_cmd=-nus_switch*MAX_PPRZ/100*turn_cmd_max;
    	}
    	else {
    	  	nus_turn_cmd=0;
    	}
    	nus_gate_detected=0;
    	nus_climb_cmd=0;
    	gate_heading=0;
    }

    /* simple filter */
   	nus_gate_heading=(nus_filter_order-1)*nus_gate_heading/nus_filter_order+gate_heading/nus_filter_order;


    //autopilot_guided_goto_body_relative(0.0, 0.0, nus_climb_cmd, 0.0)
  }
//	else if (stereocam_data.fresh && stereocam_data.len == 20)
//  {
//    //run_droplet((uint32_t)stereocam_data.data[0], (uint32_t)stereocam_data.data[4]);
//    uint32_t* buffer32 = (uint32_t*)stereocam_data.data;
//    run_droplet_low_texture(buffer32[0], buffer32[1], buffer32[2], buffer32[3], buffer32[4]);
//    stereocam_data.fresh = 0;
//  }
}

//void stereocam_to_state(void)
//{
//
//  // Get info from stereocam data
//  // 0 = stereoboard's #define SEND_EDGEFLOW
//#if STEREOCAM2STATE_RECEIVED_DATA_TYPE == 0
//  // opticflow
//  int16_t div_x = (int16_t)stereocam_data.data[0] << 8;
//  div_x |= (int16_t)stereocam_data.data[1];
//  int16_t flow_x = (int16_t)stereocam_data.data[2] << 8;
//  flow_x |= (int16_t)stereocam_data.data[3];
//  int16_t div_y = (int16_t)stereocam_data.data[4] << 8;
//  div_y |= (int16_t)stereocam_data.data[5];
//  int16_t flow_y = (int16_t)stereocam_data.data[6] << 8;
//  flow_y |= (int16_t)stereocam_data.data[7];
//
//  float fps = (float)stereocam_data.data[9];
//  //int8_t agl = stereocam_data.data[8]; // in cm
//
//  // velocity
//  int16_t vel_x_int = (int16_t)stereocam_data.data[10] << 8;
//  vel_x_int |= (int16_t)stereocam_data.data[11];
//  int16_t vel_y_int = (int16_t)stereocam_data.data[12] << 8;
//  vel_y_int |= (int16_t)stereocam_data.data[13];
//
//  int16_t RES = 100;
//
//  float vel_x = (float)vel_x_int / RES;
//  float vel_y = (float)vel_y_int / RES;
//
//  // Derotate velocity and transform from frame to body coordinates
//  // TODO: send resolution directly from stereocam
//
//  float vel_body_x = - vel_x;
//  float vel_body_y = vel_y;
//
//  //Send velocity estimate to state
//  //TODO:: Make variance dependable on line fit error, after new horizontal filter is made
//  uint32_t now_ts = get_sys_time_usec();
//
//  if (!(abs(vel_body_x) > 0.5 || abs(vel_body_x) > 0.5))
//  {
//    AbiSendMsgVELOCITY_ESTIMATE(STEREOCAM2STATE_SENDER_ID, now_ts,
//                                vel_body_x,
//                                vel_body_y,
//                                0.0f,
//                                0.3f
//                               );
//  }
//
//  // Reusing the OPTIC_FLOW_EST telemetry messages, with some values replaced by 0
//
//  uint16_t dummy_uint16 = 0;
//  int16_t dummy_int16 = 0;
//  float dummy_float = 0;
//
//  DOWNLINK_SEND_OPTIC_FLOW_EST(DefaultChannel, DefaultDevice, &fps, &dummy_uint16, &dummy_uint16, &flow_x, &flow_y, &dummy_int16, &dummy_int16,
//		  &vel_x, &vel_y,&dummy_float, &dummy_float, &dummy_float);
//
//#endif
//
//}
