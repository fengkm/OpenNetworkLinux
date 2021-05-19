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
 * Thermal Sensor Platform Implementation.
 *
 ***********************************************************/
#include <unistd.h>
#include <onlplib/file.h>
#include <onlp/platformi/thermali.h>
#include <onlplib/i2c.h>
//#include <fcntl.h>
#include "platform_lib.h"


#define UFI_ONLP_THERMAL_THRESHOLD(WARNING_DEFAULT, ERROR_DEFAULT, SHUTDOWN_DEFAULT){ \
    WARNING_DEFAULT,                                                                  \
    ERROR_DEFAULT,                                                                    \
    SHUTDOWN_DEFAULT,                                                                 \
}

/**
 * Get all information about the given Thermal oid.
 *
 * [01] CHASSIS----[01] ONLP_THERMAL_FRONT_MAC
 *            |----[02] ONLP_THERMAL_REAR_MAC
 *            |----[03] ONLP_THERMAL_ASIC
 *            |----[04] ONLP_THERMAL_CPU1
 *            |----[05] ONLP_THERMAL_CPU2
 *            |----[06] ONLP_THERMAL_CPU3
 *            |----[07] ONLP_THERMAL_CPU4
 *            |----[08] ONLP_THERMAL_CPU_BOARD
 *            |----[09] ONLP_THERMAL_PSU1_NEAR
 *            |----[10] ONLP_THERMAL_PSU2_NEAR
 *            |----[11] ONLP_THERMAL_QSFP_NEAR
 *            |
 *            |----[01] ONLP_PSU_1----[12] ONLP_THERMAL_PSU1_1
 *            |                  |----[13] ONLP_THERMAL_PSU1_2
 *            |
 *            |----[02] ONLP_PSU_2----[14] ONLP_THERMAL_PSU2_1
 *                               |----[15] ONLP_THERMAL_PSU2_2
 *            
 */
static onlp_thermal_info_t __onlp_thermal_info[] = {
    { }, /* Not used */
    {
        .hdr = {
            .id = ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_FRONT_MAC),
            .description = "Front MAC Thermal",
            .poid = ONLP_OID_CHASSIS,
            .status = ONLP_OID_STATUS_FLAG_PRESENT,
        },
        .caps = ONLP_THERMAL_CAPS_ALL,
        .mcelsius = 0,
        .thresholds = UFI_ONLP_THERMAL_THRESHOLD(62700, 66000, 69000),
    },
    {
        .hdr = {
            .id = ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_REAR_MAC),
            .description = "Rear MAC Thermal",
            .poid = ONLP_OID_CHASSIS,
            .status = ONLP_OID_STATUS_FLAG_PRESENT,
        },
        .caps = ONLP_THERMAL_CAPS_ALL,
        .mcelsius = 0,
        .thresholds = UFI_ONLP_THERMAL_THRESHOLD(5727, 60260, 63273),
    },
    {
        .hdr = {
            .id = ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_ASIC),
            .description = "ASIC Thermal",
            .poid = ONLP_OID_CHASSIS,
            .status = ONLP_OID_STATUS_FLAG_PRESENT,
        },
        .caps = ONLP_THERMAL_CAPS_ALL,
        .mcelsius = 0,
        .thresholds = UFI_ONLP_THERMAL_THRESHOLD(84787, 89250, 93712),
    },
    {
        .hdr = {
            .id = ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_CPU1),
            .description = "CPU Thermal 1",
            .poid = ONLP_OID_CHASSIS,
            .status = ONLP_OID_STATUS_FLAG_PRESENT,
        },
        .caps = ONLP_THERMAL_CAPS_ALL,
        .mcelsius = 0,
        .thresholds = UFI_ONLP_THERMAL_THRESHOLD(77000, 95000, 105000),
    },
    {
        .hdr = {
            .id = ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_CPU2),
            .description = "CPU Thermal 2",
            .poid = ONLP_OID_CHASSIS,
            .status = ONLP_OID_STATUS_FLAG_PRESENT,
        },
        .caps = ONLP_THERMAL_CAPS_ALL,
        .mcelsius = 0,
        .thresholds = UFI_ONLP_THERMAL_THRESHOLD(77000, 95000, 105000),
    },
    {
        .hdr = {
            .id = ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_CPU3),
            .description = "CPU Thermal 3",
            .poid = ONLP_OID_CHASSIS,
            .status = ONLP_OID_STATUS_FLAG_PRESENT,
        },
        .caps = ONLP_THERMAL_CAPS_ALL,
        .mcelsius = 0,
        .thresholds = UFI_ONLP_THERMAL_THRESHOLD(77000, 95000, 105000),
    },
    {
        .hdr = {
            .id = ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_CPU4),
            .description = "CPU Thermal 4",
            .poid = ONLP_OID_CHASSIS,
            .status = ONLP_OID_STATUS_FLAG_PRESENT,
        },
        .caps = ONLP_THERMAL_CAPS_ALL,
        .mcelsius = 0,
        .thresholds = UFI_ONLP_THERMAL_THRESHOLD(77000, 95000, 105000),
    },
    {
        .hdr = {
            .id = ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_CPU_BOARD),
            .description = "CPU Board Thermal",
            .poid = ONLP_OID_CHASSIS,
            .status = ONLP_OID_STATUS_FLAG_PRESENT,
        },
        .caps = ONLP_THERMAL_CAPS_ALL,
        .mcelsius = 0,
        .thresholds = UFI_ONLP_THERMAL_THRESHOLD(59774, 62920, 66066),
    },
    {
        .hdr = {
            .id = ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_PSU1_NEAR),
            .description = "Near PSU1 Thermal",
            .poid = ONLP_OID_CHASSIS,
            .status = ONLP_OID_STATUS_FLAG_PRESENT,
        },
        .caps = ONLP_THERMAL_CAPS_ALL,
        .mcelsius = 0,
        .thresholds = UFI_ONLP_THERMAL_THRESHOLD(61132, 64350, 67567),
    },
    {
        .hdr = {
            .id = ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_PSU2_NEAR),
            .description = "Near PSU2 Thermal",
            .poid = ONLP_OID_CHASSIS,
            .status = ONLP_OID_STATUS_FLAG_PRESENT,
        },
        .caps = ONLP_THERMAL_CAPS_ALL,
        .mcelsius = 0,
        .thresholds = UFI_ONLP_THERMAL_THRESHOLD(61132, 64350, 67567),
    },
    {
        .hdr = {
            .id = ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_QSFP_NEAR),
            .description = "Near QSFP Thermal",
            .poid = ONLP_OID_CHASSIS,
            .status = ONLP_OID_STATUS_FLAG_PRESENT,
        },
        .caps = ONLP_THERMAL_CAPS_ALL,
        .mcelsius = 0,
        .thresholds = UFI_ONLP_THERMAL_THRESHOLD(55508, 58430, 61351),
    },
    {
        .hdr = {
            .id = ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_PSU1_1),
            .description = "PSU-1 Thermal 1",
            .poid = ONLP_PSU_ID_CREATE(ONLP_PSU_1),
            .status = ONLP_OID_STATUS_FLAG_PRESENT,
        },
        .caps = ONLP_THERMAL_CAPS_GET_TEMPERATURE,
        .mcelsius = 0,
    },
    {
        .hdr = {
            .id = ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_PSU1_2),
            .description = "PSU-1 Thermal 2",
            .poid = ONLP_PSU_ID_CREATE(ONLP_PSU_1),
            .status = ONLP_OID_STATUS_FLAG_PRESENT,
        },
        .caps = ONLP_THERMAL_CAPS_GET_TEMPERATURE,
        .mcelsius = 0,
    },
    {
        .hdr = {
            .id = ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_PSU2_1),
            .description = "PSU-2 Thermal 1",
            .poid = ONLP_PSU_ID_CREATE(ONLP_PSU_2),
            .status = ONLP_OID_STATUS_FLAG_PRESENT,
        },
        .caps = ONLP_THERMAL_CAPS_GET_TEMPERATURE,
        .mcelsius = 0,
    },
    {
        .hdr = {
            .id = ONLP_THERMAL_ID_CREATE(ONLP_THERMAL_PSU2_2),
            .description = "PSU-2 Thermal 2",
            .poid = ONLP_PSU_ID_CREATE(ONLP_PSU_2),
            .status = ONLP_OID_STATUS_FLAG_PRESENT,
        },
        .caps = ONLP_THERMAL_CAPS_GET_TEMPERATURE,
        .mcelsius = 0,
    },
};


/**
 * @brief Update the information structure for the mac thermal
 * @param id The Thermal Local ID
 * @param[out] info Receives the thermal information.
 */
static int update_thermali_mac_info(int local_id, onlp_thermal_info_t* info)
{
    int temperature = 0;

    if (BMC_ENABLE) {
        return ONLP_STATUS_E_UNSUPPORTED;
    } else {
        if (local_id == ONLP_THERMAL_FRONT_MAC) {
            ONLP_TRY(onlp_file_read_int(&temperature, "/sys/class/hwmon/hwmon2/temp1_input"));
        } else if (local_id == ONLP_THERMAL_REAR_MAC) {
            ONLP_TRY(onlp_file_read_int(&temperature, "/sys/class/hwmon/hwmon5/temp1_input"));
        } else if (local_id == ONLP_THERMAL_ASIC) {
            ONLP_TRY(onlp_file_read_int(&temperature, "/sys/class/hwmon/hwmon2/temp2_input"));
        } else if (local_id == ONLP_THERMAL_PSU1_NEAR) {
            ONLP_TRY(onlp_file_read_int(&temperature, "/sys/class/hwmon/hwmon4/temp1_input"));
        } else if (local_id == ONLP_THERMAL_PSU2_NEAR) {
            ONLP_TRY(onlp_file_read_int(&temperature, "/sys/class/hwmon/hwmon7/temp1_input"));
        } else if (local_id == ONLP_THERMAL_QSFP_NEAR) {
            ONLP_TRY(onlp_file_read_int(&temperature, "/sys/class/hwmon/hwmon6/temp1_input"));
        } else {
            AIM_LOG_ERROR("unsupported thermal id (%d), func=%s\n", local_id, __FUNCTION__);
            return ONLP_STATUS_E_PARAM;
        }
    }

    info->mcelsius = temperature;

    return ONLP_STATUS_OK;
}


/**
 * @brief Update the information structure for the cpu thermal
 * @param id The Thermal Local ID
 * @param[out] info Receives the thermal information.
 */
static int update_thermali_cpu_info(int local_id, onlp_thermal_info_t* info)
{
    int temperature = 0;

    if (local_id == ONLP_THERMAL_CPU1) {
        ONLP_TRY(onlp_file_read_int(&temperature, "/sys/class/hwmon/hwmon0/temp2_input"));
    } else if (local_id == ONLP_THERMAL_CPU2) {
        ONLP_TRY(onlp_file_read_int(&temperature, "/sys/class/hwmon/hwmon0/temp3_input"));
    } else if (local_id == ONLP_THERMAL_CPU3) {
        ONLP_TRY(onlp_file_read_int(&temperature, "/sys/class/hwmon/hwmon0/temp4_input"));
    } else if (local_id == ONLP_THERMAL_CPU4) {
        ONLP_TRY(onlp_file_read_int(&temperature, "/sys/class/hwmon/hwmon0/temp5_input"));
    } else if (local_id == ONLP_THERMAL_CPU_BOARD) {
        if (BMC_ENABLE) {
            ONLP_TRY(onlp_file_read_int(&temperature, "/sys/class/hwmon/hwmon1/temp1_input"));
        } else {
            ONLP_TRY(onlp_file_read_int(&temperature, "/sys/class/hwmon/hwmon3/temp1_input"));
        }
    } else {
        AIM_LOG_ERROR("unsupported thermal id (%d), func=%s\n", local_id, __FUNCTION__);
        return ONLP_STATUS_E_PARAM;
    }

    info->mcelsius = temperature;

    return ONLP_STATUS_OK;
}

/**
 * @brief Update the information structure for the psu thermal
 * @param id The Thermal Local ID
 * @param[out] info Receives the thermal information.
 */
static int update_thermali_psu_info(int local_id, onlp_thermal_info_t* info)
{
    int psu_reg_value = 0;
    unsigned int y_value = 0;
    unsigned char n_value = 0;
    unsigned int temp = 0;
    char result[32];
    int buf;


    if (BMC_ENABLE) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    if (local_id < ONLP_THERMAL_PSU1_1 || local_id > ONLP_THERMAL_PSU2_2) {
        AIM_LOG_ERROR("unsupported thermal psu id (%d), func=%s\n", local_id, __FUNCTION__);
        return ONLP_STATUS_E_PARAM;
    }

    if ((info->hdr.status & ONLP_OID_STATUS_FLAG_PRESENT) == 0) {
        //not present, do nothing
        return ONLP_STATUS_OK;
    }

    if (local_id == ONLP_THERMAL_PSU1_1) {
        psu_reg_value = onlp_i2c_readw(58, 0x58, 0x8D, ONLP_I2C_F_FORCE);
    } else if (local_id == ONLP_THERMAL_PSU1_2) {
        psu_reg_value = onlp_i2c_readw(58, 0x58, 0x8E, ONLP_I2C_F_FORCE);
    } else if (local_id == ONLP_THERMAL_PSU2_1) {
        psu_reg_value = onlp_i2c_readw(57, 0x58, 0x8D, ONLP_I2C_F_FORCE);
    } else if (local_id == ONLP_THERMAL_PSU2_2) {
        psu_reg_value = onlp_i2c_readw(57, 0x58, 0x8E, ONLP_I2C_F_FORCE);
    } else {
        return ONLP_STATUS_E_INTERNAL;
    }

    y_value = (psu_reg_value & 0x07FF);
    if ((psu_reg_value & 0x8000) && (y_value)) {
        n_value = 0xF0 + (((psu_reg_value) >> 11) & 0x0F);
        n_value = (~n_value) +1;
        temp = (unsigned int)(1 << n_value);
        if (temp)
            snprintf(result, sizeof(result), "%d.%04d", y_value/temp, ((y_value%temp)*10000)/temp);
    } else {
        n_value = (((psu_reg_value) >> 11) & 0x0F);
        snprintf(result, sizeof(result), "%d", (y_value*(1<<n_value)));
    }

    buf = atof((const char *)result);
    info->mcelsius = (int)(buf * 1000);

    return ONLP_STATUS_OK;
}


/**
 * @brief Software initialization of the Thermal module.
 */
int onlp_thermali_sw_init(void)
{
    if (BMC_ENABLE < 0) {
        set_bmc_enable_flag();
    }

    return ONLP_STATUS_OK;
}

/**
 * @brief Hardware initialization of the Thermal module.
 * @param flags The hardware initialization flags.
 */
int onlp_thermali_hw_init(uint32_t flags)
{
    return ONLP_STATUS_OK;
}

/**
 * @brief Deinitialize the thermal software module.
 * @note The primary purpose of this API is to properly
 * deallocate any resources used by the module in order
 * faciliate detection of real resouce leaks.
 */
int onlp_thermali_sw_denit(void)
{
    return ONLP_STATUS_OK;
}

/**
 * @brief Validate a thermal oid.
 * @param id The thermal id.
 */
int onlp_thermali_id_validate(onlp_oid_id_t id)
{
    return ONLP_OID_ID_VALIDATE_RANGE(id, 1, ONLP_THERMAL_MAX-1);
}

/**
 * @brief Retrieve the thermal's oid header.
 * @param id The thermal oid.
 * @param[out] hdr Receives the header.
 */
int onlp_thermali_hdr_get(onlp_oid_t id, onlp_oid_hdr_t* hdr)
{
    int ret = ONLP_STATUS_OK;
    int local_id = ONLP_OID_ID_GET(id);

    /* Set the onlp_oid_hdr_t */
    *hdr = __onlp_thermal_info[local_id].hdr;


    switch (local_id) {
        case ONLP_THERMAL_FRONT_MAC:
        case ONLP_THERMAL_REAR_MAC:
        case ONLP_THERMAL_ASIC:
            if (BMC_ENABLE) {
                return ONLP_STATUS_E_UNSUPPORTED;
            } else {
                hdr->status = ONLP_OID_STATUS_FLAG_PRESENT;
            }
            break;
        case ONLP_THERMAL_CPU1:
        case ONLP_THERMAL_CPU2:
        case ONLP_THERMAL_CPU3:
        case ONLP_THERMAL_CPU4:
        case ONLP_THERMAL_CPU_BOARD:
            hdr->status = ONLP_OID_STATUS_FLAG_PRESENT;
            break;
        case ONLP_THERMAL_PSU1_NEAR:
        case ONLP_THERMAL_PSU2_NEAR:
        case ONLP_THERMAL_QSFP_NEAR:
            if (BMC_ENABLE) {
                return ONLP_STATUS_E_UNSUPPORTED;
            } else {
                hdr->status = ONLP_OID_STATUS_FLAG_PRESENT;
            }
            break;
        case ONLP_THERMAL_PSU1_1:
        case ONLP_THERMAL_PSU1_2:
            if (BMC_ENABLE) {
                return ONLP_STATUS_E_UNSUPPORTED;
            } else {
                /* When the PSU module is unplugged, the psu thermal does not exist. */
                ret = get_psu_present_status(ONLP_PSU_1);
                if (ret == 0) {
                    hdr->status = ONLP_OID_STATUS_FLAG_UNPLUGGED;
                } else if (ret == 1) {
                    hdr->status = ONLP_OID_STATUS_FLAG_PRESENT;
                } else {
                    return ONLP_STATUS_E_INTERNAL;
                }
            }
            break;
        case ONLP_THERMAL_PSU2_1:
        case ONLP_THERMAL_PSU2_2:
            if (BMC_ENABLE) {
                return ONLP_STATUS_E_UNSUPPORTED;
            } else {
                /* When the PSU module is unplugged, the psu thermal does not exist. */
                ret = get_psu_present_status(ONLP_PSU_2);
                if (ret == 0) {
                    hdr->status = ONLP_OID_STATUS_FLAG_UNPLUGGED;
                } else if (ret == 1) {
                    hdr->status = ONLP_OID_STATUS_FLAG_PRESENT;
                } else {
                    return ONLP_STATUS_E_INTERNAL;
                }
            }
            break;
        default:
            AIM_LOG_ERROR("unknown Thermal id (%d), func=%s\n", local_id, __FUNCTION__);
            ret = ONLP_STATUS_E_PARAM;
    }

    return ONLP_STATUS_OK;
}

/*
 * Retrieve the information structure for the given thermal OID.
 *
 * If the OID is invalid, return ONLP_E_STATUS_INVALID.
 * If an unexpected error occurs, return ONLP_E_STATUS_INTERNAL.
 * Otherwise, return ONLP_STATUS_OK with the OID's information.
 *
 * Note -- it is expected that you fill out the information
 * structure even if the sensor described by the OID is not present.
 */

/**
 * @brief Get the information for the given thermal OID.
 * @param id The Thermal OID
 * @param[out] info Receives the thermal information.
 */
int onlp_thermali_info_get(onlp_oid_id_t id, onlp_thermal_info_t* info)
{
    int ret = ONLP_STATUS_OK;
    int local_id = ONLP_OID_ID_GET(id);


    /* Set the onlp_thermal_info_t */
    memset(info, 0, sizeof(onlp_thermal_info_t));
    *info = __onlp_thermal_info[local_id];
    ONLP_TRY(onlp_thermali_hdr_get(id, &info->hdr));

    switch (local_id) {
        case ONLP_THERMAL_FRONT_MAC:
        case ONLP_THERMAL_REAR_MAC:
        case ONLP_THERMAL_ASIC:
            ret = update_thermali_mac_info(local_id, info);
            break;
        case ONLP_THERMAL_CPU1:
        case ONLP_THERMAL_CPU2:
        case ONLP_THERMAL_CPU3:
        case ONLP_THERMAL_CPU4:
        case ONLP_THERMAL_CPU_BOARD:
            ret = update_thermali_cpu_info(local_id, info);
            break;
        case ONLP_THERMAL_PSU1_NEAR:
        case ONLP_THERMAL_PSU2_NEAR:
        case ONLP_THERMAL_QSFP_NEAR:
            ret = update_thermali_mac_info(local_id, info);
            break;
        case ONLP_THERMAL_PSU1_1:
        case ONLP_THERMAL_PSU1_2:
        case ONLP_THERMAL_PSU2_1:
        case ONLP_THERMAL_PSU2_2:
            ret = update_thermali_psu_info(local_id, info);
            break;
        default:
            AIM_LOG_ERROR("unknown Thermal id (%d), func=%s\n", local_id, __FUNCTION__);
            ret = ONLP_STATUS_E_PARAM;
    }

    return ret;
}
