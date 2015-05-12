/*
 * Copyright (C) 2014 Freek van Tienen
 *
 * This file is part of paparazzi.
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
 * along with paparazzi; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * @file gps_datalink.c
 * @brief GPS system based on datalink
 *
 * This GPS parses the datalink REMOTE_GPS packet and sets the
 * GPS structure to the values received.
 */

#include "subsystems/gps.h"
#include "subsystems/abi.h"

/** GPS initialization */
void gps_impl_init(void)
{
  gps.fix = GPS_FIX_3D;
  gps.gspeed = 700; // To enable course setting
  gps.cacc = 0; // To enable course setting
}

/** Parse the REMOTE_GPS datalink packet */
void parse_gps_datalink(uint8_t numsv, int32_t ecef_x, int32_t ecef_y, int32_t ecef_z, int32_t lat, int32_t lon,
                        int32_t alt,
                        int32_t hmsl, int32_t ecef_xd, int32_t ecef_yd, int32_t ecef_zd, uint32_t tow, int32_t course)
{

  gps.lla_pos.lat = 600;
  gps.lla_pos.lon =700;
  gps.lla_pos.alt = 200;
  gps.hmsl        = 210;

  gps.ecef_pos.x = 100;
  gps.ecef_pos.y = 110;
  gps.ecef_pos.z = 120;

  gps.ecef_vel.x = 10;
  gps.ecef_vel.y = 12;
  gps.ecef_vel.z = 14;

  gps.course = 1;
  gps.num_sv = 8;
  gps.tow = 21333;
  gps.fix = GPS_FIX_3D;

  #warning: gps code



  // publish new GPS data
  uint32_t now_ts = get_sys_time_usec();
  gps.last_msg_ticks = sys_time.nb_sec_rem;
  gps.last_msg_time = sys_time.nb_sec;
  if (gps.fix == GPS_FIX_3D) {
    gps.last_3dfix_ticks = sys_time.nb_sec_rem;
    gps.last_3dfix_time = sys_time.nb_sec;
  }
  AbiSendMsgGPS(GPS_DATALINK_ID, now_ts, &gps);
}

