/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2013 Accton Technology Corporation.
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
 * Platform Lib
 *
 ***********************************************************/
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/io.h>
#include <onlp/onlp.h>
#include <onlplib/file.h>
#include <onlplib/i2c.h>
#include <onlplib/shlocks.h>
#include <AIM/aim.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "platform_lib.h"

/**
 *  0: without BMC
 *  1: with BMC
 * <0: not initialized
 */
int BMC_ENABLE = -99;

int file_read_hex(int* value, const char* fmt, ...)
{
    int rv; 
    va_list vargs;
    va_start(vargs, fmt);
    rv = file_vread_hex(value, fmt, vargs);
    va_end(vargs);
    return rv; 
}


int file_vread_hex(int* value, const char* fmt, va_list vargs)
{
    int rv; 
    uint8_t data[32];
    int len;
    rv = onlp_file_vread(data, sizeof(data), &len, fmt, vargs);
    if(rv < 0) {
        return rv; 
    }   
    //hex to int
    *value = (int) strtol((char *)data, NULL, 0); 
    return 0;
}


int exec_cmd(char *cmd, char* out, int size) {
    FILE *fp;

    /* Open the command for reading. */
    fp = popen(cmd, "r");
    if (fp == NULL) {
        AIM_LOG_ERROR("Failed to run command %s\n", cmd );
        return ONLP_STATUS_E_INTERNAL;
    }   

    /* Read the output a line at a time - output it. */
    while (fgets(out, size-1, fp) != NULL) {
    }

    /* close */
    pclose(fp);

    return ONLP_STATUS_OK;
}


/**
 * @brief get psu presnet status
 * @param local_id: psu id
 * @return:
 *    0: absence
 *    1: presence
 *   <0: error code
 */
int get_psu_present_status(int local_id)
{
    int psu_reg_value = 0;
    int psu_presence = 0;

    if (BMC_ENABLE) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    if (local_id == ONLP_PSU_1) {
        psu_reg_value = onlp_i2c_readb(0, 0x25, 0x0, ONLP_I2C_F_FORCE);
        if (psu_reg_value >= 0) {
            psu_presence = ((psu_reg_value >> 0x04) & 0x1) ? 0 : 1;
        } else {
            return psu_reg_value;
        }
    } else if (local_id == ONLP_PSU_2) {
        psu_reg_value = onlp_i2c_readb(0, 0x25, 0x0, ONLP_I2C_F_FORCE);
        if (psu_reg_value >= 0) {
            psu_presence = ((psu_reg_value >> 0x01) & 0x1) ? 0 : 1;
        } else {
            return psu_reg_value;
        }
    } else {
        AIM_LOG_ERROR("unknown PSU id (%d), func=%s\n", local_id, __FUNCTION__);
        return ONLP_STATUS_E_PARAM;
    }

    return psu_presence;
}


/**
 * @brief get psu pwgood status
 * @param local_id: psu id
 * @return:
 *    0: absence
 *    1: presence
 *   <0: error code
 */
int get_psu_pwgood_status(int local_id)
{
    int psu_reg_value = 0;
    int psu_pwgood = 0;

    if (BMC_ENABLE) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    if (local_id == ONLP_PSU_1) {
        psu_reg_value = onlp_i2c_readb(0, 0x25, 0x0, ONLP_I2C_F_FORCE);
        if (psu_reg_value >= 0) {
            psu_pwgood = ((psu_reg_value >> 0x03) & 0x1) ? 1 : 0;
        } else {
            return psu_reg_value;
        }
    } else if (local_id == ONLP_PSU_2) {
        psu_reg_value = onlp_i2c_readb(0, 0x25, 0x0, ONLP_I2C_F_FORCE);
        if (psu_reg_value >= 0) {
            psu_pwgood = ((psu_reg_value >> 0x00) & 0x1) ? 1 : 0;
        } else {
            return psu_reg_value;
        }
    } else {
        AIM_LOG_ERROR("unknown psu id (%d), func=%s\n", local_id, __FUNCTION__);
        return ONLP_STATUS_E_PARAM;
    }

    return psu_pwgood;
}


/**
 * @brief get fan presnet status
 * @param local_id: fan id
 * @return:
 *    0: absence
 *    1: presence
 *   <0: error code
 */
int get_fan_present_status(int local_id)
{
    int ret = ONLP_STATUS_OK;
    int fan_presence = 0;

    if (BMC_ENABLE) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    //get fan present status
    if (local_id == ONLP_FAN_1 || local_id == ONLP_FAN_2) {
        ret = onlp_i2c_readb(59, 0x20, 1, ONLP_I2C_F_FORCE);
        if (ret >= 0) {
            fan_presence = (ret & 0b01000000) ? 0 : 1;
        } else {
            return ret;
        }
    } else if (local_id == ONLP_FAN_3 || local_id == ONLP_FAN_4) {
        ret = onlp_i2c_readb(59, 0x20, 1, ONLP_I2C_F_FORCE);
        if (ret >= 0) {
            fan_presence = (ret & 0b00000100) ? 0 : 1;
        } else {
            return ret;
        }
    } else if (local_id == ONLP_FAN_5 || local_id == ONLP_FAN_6) {
        ret = onlp_i2c_readb(59, 0x20, 0, ONLP_I2C_F_FORCE);
        if (ret >= 0) {
            fan_presence = (ret & 0b01000000) ? 0 : 1;
        } else {
            return ret;
        }
    } else if (local_id == ONLP_FAN_7 || local_id == ONLP_FAN_8) {
        ret = onlp_i2c_readb(59, 0x20, 0, ONLP_I2C_F_FORCE);
        if (ret >= 0) {
            fan_presence = (ret & 0b00000100) ? 0 : 1;
        } else {
            return ret;
        }
    } else if (local_id == ONLP_PSU1_FAN_1) {
        fan_presence = get_psu_present_status(ONLP_PSU_1);
    } else if (local_id == ONLP_PSU2_FAN_1) {
        fan_presence = get_psu_present_status(ONLP_PSU_2);
    } else {
        AIM_LOG_ERROR("unknown fan id (%d), func=%s\n", local_id, __FUNCTION__);
        return ONLP_STATUS_E_PARAM;
    }

    return fan_presence;
}


/**
 * @brief get fan operational status
 * @param local_id: fan id
 * @return:
 *    0: failed (rpm=0 or alarm=1)
 *    1: operational
 *   <0: error code
 */
int get_fan_operational_status(int local_id)
{
    int fan_alarm = 0;
    int fan_rpm = 0;
    int fan_status = 0;

    /* Note: the path of fan sysfs is reverse */
    int fan_id = ONLP_FAN_8 - local_id + 1;

    if (BMC_ENABLE) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    //get fan alarm status (0: pass, 1:failed)
    ONLP_TRY(onlp_file_read_int(&fan_alarm, "/sys/class/hwmon/hwmon1/device/fan%d_alarm", fan_id));

    //get fan rpm status (0: failed, >0: pass)
    ONLP_TRY(onlp_file_read_int(&fan_rpm, "/sys/class/hwmon/hwmon1/device/fan%d_input", fan_id));

    if (fan_alarm > 0 || fan_rpm == 0) {
        fan_status = 0;
    } else {
        fan_status = 1;
    }

    return fan_status;
}


/**
 * @brief set BMC_ENABLE flag
 *    0: platform without BMC
 *    1: platform with BMC
 *   <0: not initialized
 */
void set_bmc_enable_flag(void)
{
    int value;

    if (onlp_file_read_int(&value, "/etc/onl/bmc_en") < 0) {
        // flag file not exist, default to not enable
        BMC_ENABLE = 0;
    } else if (value > 0) {
        BMC_ENABLE = 1;
    } else {
        BMC_ENABLE = 0;
    }
}
