/*
 * Copyright (C) 2015 Torbjoern Cunis <t.cunis@tudelft.nl>
 *
 * State estimation submodule filters and estimates delfly state
 * estimation.
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
 * @file /modules/delfly_control/state_estimation.h
 * @author Torbjoern Cunis
 */

#ifndef STATE_ESTIMATION_H_
#define STATE_ESTIMATION_H_



extern void state_estimation_init(void);

extern void state_estimation_enter(void);

extern void state_estimation_run(void);


#endif /* STATE_ESTIMATION_H_ */