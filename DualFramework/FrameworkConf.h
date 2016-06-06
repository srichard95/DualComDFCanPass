/*
 * FrameworkConf.h
 *
 *  Created on: 2016 márc. 12
 *      Author: srich
 */

#ifndef DUALFRAMEWORK_FRAMEWORKCONF_H_
#define DUALFRAMEWORK_FRAMEWORKCONF_H_


/**
 * @file    FrameworkConf.h
 * @brief   DualFramework configuration header.
 * @details Here you can change the Framework settings.
 *
 * @addtogroup DualFramework_CONF
 * @{
 */

#define FRAMEWORK_VERSION 0.1a


/**
 * @brief   Enables the DualFramework driver subsystem.
 */
#if !defined(DUALFRAMEWORK_USE_WIFI) || defined(__DOXYGEN__)
#define DUALFRAMEWORK_USE_WIFI      TRUE
#endif

#define DEFAULT_BAUDRATE 921600
#define INPUT_FRAME_BUFFER 10
#define OUTPUT_FRAME_BUFFER 97

#define MAX_FRAME_PER_PACKET 97
#define MAX_AVAILABLE_PACKET 2

#endif /* DUALFRAMEWORK_FRAMEWORKCONF_H_ */
