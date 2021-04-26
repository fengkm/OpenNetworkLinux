/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *        Copyright 2014, 2015 Big Switch Networks, Inc.
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
#include <onlp/platformi/thermali.h>
#include <onlplib/file.h>
#include "x86_64_ufispace_s9701_78dc_log.h"
#include "platform_lib.h"

static onlp_thermal_info_t thermal_info[] = {    
    { }, /* Not used */
    { { THERMAL_OID_CPU_PECI, "TEMP_CPU_PECI", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, {85000, 95000, 100000}
    },    
    { { THERMAL_OID_CPU_ENV, "TEMP_CPU_ENV", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, {80000, 85000, 90000}
    },
    { { THERMAL_OID_CPU_ENV_2, "TEMP_CPU_ENV_2", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, {65000, 70000, 80000}
    },
    { { THERMAL_OID_MAC_FRONT, "TEMP_MAC_FRONT", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, {80000, 85000, 90000}
    },
    { { THERMAL_OID_MAC_DIE, "TEMP_MAC_DIE", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, {90000, 100000, 110000}
    },
    { { THERMAL_OID_0x48, "TEMP_0x48", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, {60000, 65000, 70000}
    },
    { { THERMAL_OID_0x49, "TEMP_0x49", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, {65000, 70000, 80000}
    },
    { { THERMAL_OID_PSU0, "PSU 0 - Thermal Sensor 1", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, {65000, 70000, 75000}
    },
    { { THERMAL_OID_PSU1, "PSU 1 - Thermal Sensor 1", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, {65000, 70000, 75000}
    },
    { { THERMAL_OID_CPU_PKG, "CPU Package", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { THERMAL_OID_CPU1, "CPU Thermal 1", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { THERMAL_OID_CPU2, "CPU Thermal 2", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { THERMAL_OID_CPU3, "CPU Thermal 3", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { THERMAL_OID_CPU4, "CPU Thermal 4", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { THERMAL_OID_CPU5, "CPU Thermal 5", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { THERMAL_OID_CPU6, "CPU Thermal 6", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { THERMAL_OID_CPU7, "CPU Thermal 7", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    { { THERMAL_OID_CPU8, "CPU Thermal 8", 0},
                ONLP_THERMAL_STATUS_PRESENT,
                ONLP_THERMAL_CAPS_ALL, 0, THERMAL_THRESHOLD_INIT_DEFAULTS
    },
};

/*
 * This will be called to intiialize the thermali subsystem.
 */
int
onlp_thermali_init(void)
{   
    lock_init();
    return ONLP_STATUS_OK;
}

static int
cpu_thermal_info_get(onlp_thermal_info_t* info, int id)
{
    int rv;
    
    rv = onlp_file_read_int(&info->mcelsius,
                            SYS_CPU_CORETEMP_PREFIX "temp%d_input", 
                            (id - THERMAL_ID_CPU_PKG) + 1);    
    
    if(rv != ONLP_STATUS_OK) {

        rv = onlp_file_read_int(&info->mcelsius,
                            SYS_CPU_CORETEMP_PREFIX2 "temp%d_input", 
                            (id - THERMAL_ID_CPU_PKG) + 1); 

        if(rv != ONLP_STATUS_OK) {
            return rv;
        }
    }

    if(rv == ONLP_STATUS_E_MISSING) {
        info->status &= ~1;
        return 0;
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

int
onlp_thermali_info_get(onlp_oid_t id, onlp_thermal_info_t* info)
{   
    int sensor_id, rc;
    sensor_id = ONLP_OID_ID_GET(id);
    
    *info = thermal_info[sensor_id];
    info->caps |= ONLP_THERMAL_CAPS_GET_TEMPERATURE;

    switch (sensor_id) {        
        case THERMAL_ID_CPU_PKG:
        case THERMAL_ID_CPU1:
        case THERMAL_ID_CPU2:
        case THERMAL_ID_CPU3:
        case THERMAL_ID_CPU4:
        case THERMAL_ID_CPU5:
        case THERMAL_ID_CPU6:
        case THERMAL_ID_CPU7:
        case THERMAL_ID_CPU8:            
            rc = cpu_thermal_info_get(info, sensor_id);
            break;        
        case THERMAL_ID_CPU_PECI:
        case THERMAL_ID_CPU_ENV:
        case THERMAL_ID_CPU_ENV_2:
        case THERMAL_ID_MAC_FRONT:        
        case THERMAL_ID_MAC_DIE:
        case THERMAL_ID_0x48:
        case THERMAL_ID_0x49:
        case THERMAL_ID_PSU0:
        case THERMAL_ID_PSU1:    
            rc = bmc_thermal_info_get(info, sensor_id);
            break;    
        default:            
            return ONLP_STATUS_E_INTERNAL;
            break;
    }

    return rc;
}
