/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2014 Accton Technology Corporation.
 *
 * Licensed under the Eclipse Public License, Version 1.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *        http://www.eclipse.org/legal/epl-v10.html
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the
 * License.
 *
 * </bsn.cl>
 ************************************************************
 *
 * Platform Implementation
 *
 ***********************************************************/
#include <onlp/platformi/base.h>
#include "platform_lib.h"

/**
 * @brief Return the name of the the platform implementation.
 * @note This will be called prior to any other calls into the
 * platform driver, including the onlp_platformi_sw_init() function below.
 *
 * The platform implementation name should match the current
 * ONLP platform name.
 *
 * IF the platform implementation name equals the current platform name,
 * initialization will continue.
 *
 * If the platform implementation name does not match, the following will be
 * attempted:
 *
 *    onlp_platformi_set(current_platform_name);
 * If this call is successful, initialization will continue.
 * If this call fails, platform initialization will abort().
 *
 * The onlp_platformi_set() function is optional.
 * The onlp_platformi_get() is not optional.
 */
const char* onlp_platformi_get(void)
{
    int mb_cpld1_addr = 0x700;
    int mb_cpld1_board_type_rev;
    int mb_cpld1_hw_rev, mb_cpld1_build_rev;

    if (read_ioport(mb_cpld1_addr, &mb_cpld1_board_type_rev) < 0) {
        AIM_LOG_ERROR("unable to read MB CPLD1 Board Type Revision\n");
        return "x86-64-ufispace-s9700-53dx-rx";
    }   
    mb_cpld1_hw_rev = (((mb_cpld1_board_type_rev) >> 2 & 0x03));
    mb_cpld1_build_rev = (((mb_cpld1_board_type_rev) & 0x03) | ((mb_cpld1_board_type_rev) >> 5 & 0x04));

    if (mb_cpld1_hw_rev == 0 && mb_cpld1_build_rev == 0) {
        return "x86-64-ufispace-s9700-53dx-r0";
    } else if (mb_cpld1_hw_rev == 1 && mb_cpld1_build_rev == 0) {
        return "x86-64-ufispace-s9700-53dx-r1";
    } else if (mb_cpld1_hw_rev == 1 && mb_cpld1_build_rev == 1) {
        return "x86-64-ufispace-s9700-53dx-r2";
    } else if (mb_cpld1_hw_rev == 1 && mb_cpld1_build_rev == 2) {
        return "x86-64-ufispace-s9700-53dx-r3";
    } else if (mb_cpld1_hw_rev == 2 && mb_cpld1_build_rev == 1) {
        return "x86-64-ufispace-s9700-53dx-r4";
    } else if (mb_cpld1_hw_rev == 2 && mb_cpld1_build_rev == 2) {
        return "x86-64-ufispace-s9700-53dx-r5";
    } else if (mb_cpld1_hw_rev == 2 && mb_cpld1_build_rev == 3) {
        return "x86-64-ufispace-s9700-53dx-r6";
    } else if (mb_cpld1_hw_rev == 3 && mb_cpld1_build_rev == 0) {
        return "x86-64-ufispace-s9700-53dx-r7";
    } else if (mb_cpld1_hw_rev == 3 && mb_cpld1_build_rev == 1) {
        return "x86-64-ufispace-s9700-53dx-r8";
    } else if (mb_cpld1_hw_rev == 3 && mb_cpld1_build_rev == 2) {
        return "x86-64-ufispace-s9700-53dx-r9";
    } else {    
        return "x86-64-ufispace-s9700-53dx-r9";
    } 
}

/**
 * @brief Set the platform explicitly if necessary.
 * @param platform The platform name.
 */
int onlp_platformi_set(const char* platform)
{
    return ONLP_STATUS_OK;
}

/**
 * @brief Initialize the platform software module.
 * @note This should not touch the hardware.
 */
int onlp_platformi_sw_init(void)
{
    return ONLP_STATUS_OK;
}

/**
 * @brief Platform module hardware initialization.
 * @param flags The initialization flags.
 */
int onlp_platformi_hw_init(uint32_t flags)
{
    return ONLP_STATUS_OK;
}

/**
 * @brief Initialize the platform manager features.
 */
int onlp_platformi_manage_init(void)
{
    return ONLP_STATUS_OK;
}

/**
 * @brief Perform necessary platform fan management.
 * @note This function should automatically adjust the FAN speeds
 * according to the platform conditions.
 */
int onlp_platformi_manage_fans(void)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

/**
 * @brief Perform necessary platform LED management.
 * @note This function should automatically adjust the LED indicators
 * according to the platform conditions.
 */
int onlp_platformi_manage_leds(void)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}
