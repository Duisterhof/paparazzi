/*
 * Copyright (C) 2015 Torbjoern Cunis <t.cunis@tudelft.nl>
 *
 * This defines functions in order to send Delfy control module telemtry
 * to ground control.
 *
 * This file is part of paparazzi:
 *
 * paparazzi is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * paparazzi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with paparazzi; see the file COPYING.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
/**
 * @file /paparazzi/paparazzi/sw/airborne/modules/delfly_control/delfly_telemtry.c
 * @author Torbjoern Cunis
 */

#include "delfly_telemetry.h"

#include "delfly_state.h"
#include "state_estimation.h"
#include "speed_control_var.h"
#include "delfly_guidance.h"

#include "guidance/guidance_h.h"
#include "guidance/guidance_v.h"

#include "generated/modules.h"

#include "delfly_algebra_int.h"


static void delfly_telemetry_send_guidance (struct transport_tx*, struct link_device*);

static void delfly_telemetry_send_state (struct transport_tx*, struct link_device*);
static void delfly_telemetry_send_stateraw (struct transport_tx*, struct link_device*);
static void delfly_telemetry_send_stateestimation (struct transport_tx*, struct link_device*);

static void delfly_telemetry_send_speedcontrol (struct transport_tx*, struct link_device*);


static void delfly_telemetry_send_guidance (struct transport_tx* trans, struct link_device* dev) {
  pprz_msg_send_DELFLY_GUIDANCE(trans, dev, AC_ID,
		&guidance_h.mode,
		&guidance_v_mode,
		&guidance_h.sp.pos.x,
		&guidance_h.sp.pos.y,
		&guidance_v_z_sp,
		&delfly_guidance.sp.vel_rc.fv.fwd,
		&guidance_h.rc_sp.psi,
		&delfly_guidance.err.pos.x,
		&delfly_guidance.err.pos.y,
		&delfly_guidance.err.pos.z,
		&delfly_guidance.err.vel.x,
		&delfly_guidance.err.vel.y,
		&delfly_guidance.err.vel.z,
		&delfly_guidance.err.fwd.states.pos,
		&delfly_guidance.err.lat.states.pos,
		&delfly_guidance.err.fwd.states.vel,
		&delfly_guidance.err.lat.states.vel,
		&delfly_guidance.cmd.h_acc,
		&delfly_guidance.cmd.v_acc,
		&delfly_guidance.cmd.heading
	);
}

static void delfly_telemetry_send_state (struct transport_tx* trans, struct link_device* dev) {
  pprz_msg_send_DELFLY_STATE(trans, dev, AC_ID,
	    &delfly_state.h.pos.x,
	    &delfly_state.h.pos.y,
	    &delfly_state.v.pos,
	    &delfly_state.h.vel.x,
	    &delfly_state.h.vel.y,
	    &delfly_state.v.vel,
	    &delfly_state.h.air.x,
	    &delfly_state.h.air.y,
	    &delfly_state.v.air,
	    &delfly_state.h.acc.x,
	    &delfly_state.h.acc.y,
	    &delfly_state.v.acc,
	    &delfly_state.fv.air.fv.fwd,
	    &delfly_state.fv.vel.fv.fwd,
	    &delfly_state.h.heading,
	    &delfly_state.h.head_rate,
	    &delfly_state.h.azimuth,
	    &delfly_state.flap_freq,
		&sdlogger_spi.status
	);
}

static void delfly_telemetry_send_stateraw (struct transport_tx* trans, struct link_device* dev) {
//	DOWNLINK_SEND_DELFLY_STATERAW(DefaultChannel, DefaultDevice
//	);
}

static void delfly_telemetry_send_stateestimation (struct transport_tx* trans, struct link_device* dev) {

	int32_t est_cov11, res_cov, res_cov_inv;
	int32_t gain_pos, gain_vel, gain_acc;

	INT32_MAT33_DET(est_cov11,   state_estimation.covariance.estimate.pos_pos, INT32_MATLAB_FRAC);
	INT32_MAT33_DET(res_cov,     state_estimation.covariance.residual, INT32_MATLAB_FRAC);
	INT32_MAT33_DET(res_cov_inv, state_estimation.covariance.residual_inv, INT32_MATLAB_FRAC);
	INT32_MAT33_DET(gain_pos, state_estimation.gain.pos_err, INT32_MATLAB_FRAC);
	INT32_MAT33_DET(gain_vel, state_estimation.gain.vel_err, INT32_MATLAB_FRAC);
	INT32_MAT33_DET(gain_acc, state_estimation.gain.acc_err, INT32_MATLAB_FRAC);

	pprz_msg_send_DELFLY_STATEESTIMATION(trans, dev, AC_ID,
		&state_estimation.mode,
		&state_estimation.res.x,
		&state_estimation.res.y,
		&state_estimation.res.z,
		&est_cov11,
		&res_cov,
		&res_cov_inv,
		&gain_pos,
		&gain_vel,
		&gain_acc,
		&state_estimation.states.pos.x,
		&state_estimation.states.pos.y,
		&state_estimation.states.pos.z,
		&state_estimation.states.vel.x,
		&state_estimation.states.vel.y,
		&state_estimation.states.vel.z,
		&state_estimation.out.pos.x,
		&state_estimation.out.pos.y,
		&state_estimation.out.pos.z,
		&state_estimation.out.vel.x,
		&state_estimation.out.vel.y,
		&state_estimation.out.vel.z
	);
}

static void delfly_telemetry_send_speedcontrol (struct transport_tx* trans, struct link_device* dev) {
  pprz_msg_send_DELFLY_SPEEDCONTROL(trans, dev, AC_ID,
		&speed_control.mode,
		&speed_control.sp.acceleration.fv.fwd,
		&speed_control.sp.acceleration.fv.ver,
		&speed_control_var.now.air_speed,
		&speed_control_var.eq.pitch,
		&speed_control_var.eq.throttle,
		&speed_control_var.mat.pitch.y,
		&speed_control_var.mat.pitch.x,
		&speed_control_var.mat.throttle.y,
		&speed_control_var.mat.throttle.x,
		&speed_control_var.ref.velocity.fv.fwd,
		&speed_control_var.ref.velocity.fv.ver,
		&speed_control_var.err.acceleration.fv.fwd,
		&speed_control_var.err.acceleration.fv.ver,
		&speed_control_var.err.velocity.fv.fwd,
		&speed_control_var.err.velocity.fv.ver,
		&speed_control_var.ff_cmd.pitch,
		&speed_control_var.ff_cmd.throttle,
		&speed_control_var.fb_cmd.pitch,
		&speed_control_var.fb_cmd.throttle,
		&speed_control_var.cmd.pitch,
		&speed_control_var.cmd.throttle
	);
}


void delfly_telemetry_init_all (void) {

	register_periodic_telemetry(DefaultPeriodic, PPRZ_MSG_ID_DELFLY_GUIDANCE,        &delfly_telemetry_send_guidance);
	register_periodic_telemetry(DefaultPeriodic, PPRZ_MSG_ID_DELFLY_STATE,           &delfly_telemetry_send_state);
	//register_periodic_telemetry(DefaultPeriodic, PPRZ_MSG_ID_DELFLY_STATERAW,        &delfly_telemetry_send_stateraw);
	register_periodic_telemetry(DefaultPeriodic, PPRZ_MSG_ID_DELFLY_STATEESTIMATION, &delfly_telemetry_send_stateestimation);
	register_periodic_telemetry(DefaultPeriodic, PPRZ_MSG_ID_DELFLY_SPEEDCONTROL,    &delfly_telemetry_send_speedcontrol);
}