/************************************************************
 * <bsn.cl fy=2017 v=onl>
 *
 *        Copyright 2017 Big Switch Networks, Inc.
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
 * Attribute Implementation.
 *
 ***********************************************************/
#include <onlp/platformi/attributei.h>
#include <onlp/stdattrs.h>
#include <onlplib/file.h>
#include <onlplib/i2c.h>
#include "platform_lib.h"


#define ONIE_FIELD_CPY(dst, src, field) \
  if (src.field != NULL) { \
    dst->field = malloc(strlen(src.field) + 1); \
    strcpy(dst->field, src.field); \
  }

#define IDPROM_PATH   "/sys/bus/i2c/devices/0-0051/eeprom"
#define CMD_BIOS_VER  "dmidecode -s bios-version | tail -1 | tr -d '\r\n'"


static int update_attributei_asset_info(onlp_oid_t oid, onlp_asset_info_t* asset_info)
{
    //int cpld_release = 0;
    int cpld_version = 0; 
    char bios_out[32] = "";
    onlp_onie_info_t onie_info;

    //Get CPLD Version
    cpld_version = onlp_i2c_readb(44, 0x33, 0x01, ONLP_I2C_F_FORCE);
    if (cpld_version < 0) { 
        AIM_LOG_ERROR("unable to read CPLD version\n");
        return ONLP_STATUS_E_INTERNAL;
    }

    //cpld_release = (((cpld_version) >> 6 & 0x01));
    cpld_version = (((cpld_version) & 0x3F));

    asset_info->cpld_revision = aim_fstrdup("0x%02x\n", cpld_version);

    //Get BIOS version
    if (exec_cmd(CMD_BIOS_VER, bios_out, sizeof(bios_out)) < 0) { 
        AIM_LOG_ERROR("unable to read BIOS version\n");
        return ONLP_STATUS_E_INTERNAL;
    }    

    asset_info->firmware_revision = aim_fstrdup(
            "\n" 
            "    [BIOS] %s\n",
            bios_out);


    /* get platform info from onie syseeprom */
    onlp_attributei_onie_info_get(oid, &onie_info);

    asset_info->oid = oid;
    ONIE_FIELD_CPY(asset_info, onie_info, manufacturer)
    ONIE_FIELD_CPY(asset_info, onie_info, part_number)
    ONIE_FIELD_CPY(asset_info, onie_info, serial_number)
    ONIE_FIELD_CPY(asset_info, onie_info, manufacture_date)

    return ONLP_STATUS_OK;
}

/**
 * @brief Initialize the attribute subsystem.
 */
int onlp_attributei_sw_init(void)
{
    if (BMC_ENABLE < 0) {
        set_bmc_enable_flag();
    }

    return ONLP_STATUS_OK;
}

/**
 * @brief Initialize the attribute subsystem.
 */
int onlp_attributei_hw_init(uint32_t flags)
{
    return ONLP_STATUS_OK;
}

/**
 * @brief Deinitialize the attribute subsystem.
 */
int onlp_attributei_sw_denit(void)
{
    return ONLP_STATUS_OK;
}


/**
 * Access to standard attributes.
 */

/**
 * @brief Get an OID's ONIE attribute.
 * @param oid The target OID
 * @param onie_info [out] Receives the ONIE information if supported.
 * @note if onie_info is NULL you should only return whether the attribute is supported.
 */
int onlp_attributei_onie_info_get(onlp_oid_t oid, onlp_onie_info_t* onie_info)
{
    if(oid != ONLP_OID_CHASSIS) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    if(onie_info == NULL) {
        return ONLP_STATUS_OK;
    }

    return onlp_onie_decode_file(onie_info, IDPROM_PATH);
}

/**
 * @brief Get an OID's Asset attribute.
 * @param oid The target OID.
 * @param asset_info [out] Receives the Asset information if supported.
 * @note if asset_info is NULL you should only return whether the attribute is supported.
 */
int onlp_attributei_asset_info_get(onlp_oid_t oid, onlp_asset_info_t* asset_info)
{
    int ret = ONLP_STATUS_OK;

    if(oid != ONLP_OID_CHASSIS) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }    

    if(asset_info == NULL) {
        return ONLP_STATUS_OK;
    }    

    asset_info->oid = oid; 

    ret = update_attributei_asset_info(oid, asset_info);

    return ret;
}
