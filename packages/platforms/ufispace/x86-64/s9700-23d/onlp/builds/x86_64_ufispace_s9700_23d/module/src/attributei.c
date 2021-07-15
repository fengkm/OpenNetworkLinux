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
#include "platform_lib.h"


#define ONIE_FIELD_CPY(dst, src, field) \
  if (src.field != NULL) { \
    dst->field = malloc(strlen(src.field) + 1); \
    strcpy(dst->field, src.field); \
  }

#define IDPROM_PATH   "/sys/bus/i2c/devices/0-0057/eeprom"
#define CMD_BIOS_VER  "dmidecode -s bios-version | tail -1 | tr -d '\r\n'"
#define CMD_BMC_VER_1 "expr `ipmitool mc info | grep 'Firmware Revision' | cut -d':' -f2 | cut -d'.' -f1` + 0"
#define CMD_BMC_VER_2 "expr `ipmitool mc info | grep 'Firmware Revision' | cut -d':' -f2 | cut -d'.' -f2` + 0"
#define CMD_BMC_VER_3 "echo $((`ipmitool mc info | grep 'Aux Firmware Rev Info' -A 2 | sed -n '2p'`))"
#define CMD_UCD_VER   "ipmitool raw 0x3c 0x08"


static int parse_ucd_out(char *ucd_out, char *ucd_data, int start, int len){
    int i=0;
    char data[3];

    memset(data, 0, sizeof(data));

    for (i=0; i<len; ++i) {
        data[0] = ucd_out[(start+i)*3 + 1];
        data[1] = ucd_out[(start+i)*3 + 2];
        //hex string to int
        ucd_data[i] = (int) strtol(data, NULL, 16);
    }
    return ONLP_STATUS_OK;
}

static int update_attributei_asset_info(onlp_oid_t oid, onlp_asset_info_t* asset_info)
{
    int cpu_cpld_addr = 0x600, cpu_cpld_ver, cpu_cpld_ver_major, cpu_cpld_ver_minor;
    int cpld_ver[CPLD_MAX], cpld_ver_major[CPLD_MAX], cpld_ver_minor[CPLD_MAX];
    int mb_cpld1_addr = 0x700, mb_cpld1_board_type_rev, mb_cpld1_hw_rev, mb_cpld1_build_rev;
    int i;
    char bios_out[32];
    char bmc_out1[8], bmc_out2[8], bmc_out3[8];
    char ucd_out[48];
    char ucd_ver[8];
    char ucd_date[8];
    int ucd_len=0, ucd_date_len=6;
    onlp_onie_info_t onie_info;
    int ret = ONLP_STATUS_OK;

    memset(bios_out, 0, sizeof(bios_out));
    memset(bmc_out1, 0, sizeof(bmc_out1));
    memset(bmc_out2, 0, sizeof(bmc_out2));
    memset(bmc_out3, 0, sizeof(bmc_out3));
    memset(ucd_out, 0, sizeof(ucd_out));
    memset(ucd_ver, 0, sizeof(ucd_ver));
    memset(ucd_date, 0, sizeof(ucd_date));

    //get CPU CPLD version
    if (read_ioport(cpu_cpld_addr, &cpu_cpld_ver) < 0) {
        AIM_LOG_ERROR("unable to read CPU CPLD version\n");
        return ONLP_STATUS_E_INTERNAL;
    }
    cpu_cpld_ver_major = (((cpu_cpld_ver) >> 6 & 0x01));
    cpu_cpld_ver_minor = (((cpu_cpld_ver) & 0x3F));

    //get MB CPLD version
    for(i=0; i<CPLD_MAX; ++i) {
        //cpld_ver[i] = onlp_i2c_readb(I2C_BUS_1, CPLD_BASE_ADDR[i], CPLD_REG_VER, ONLP_I2C_F_FORCE);
        ret = file_read_hex(&cpld_ver[i], "/sys/bus/i2c/devices/1-00%02x/cpld_version", CPLD_BASE_ADDR[i]);
        if (ret != ONLP_STATUS_OK) {
            return ONLP_STATUS_E_INTERNAL;
        }

        if (cpld_ver[i] < 0) {
            AIM_LOG_ERROR("unable to read MB CPLD version\n");
            return ONLP_STATUS_E_INTERNAL;
        }

        cpld_ver_major[i] = (((cpld_ver[i]) >> 6 & 0x01));
        cpld_ver_minor[i] = (((cpld_ver[i]) & 0x3F));
    }

    asset_info->cpld_revision = aim_fstrdup(
        "\n"
        "    [CPU CPLD] %d.%02d\n"
        "    [MB CPLD1] %d.%02d\n"
        "    [MB CPLD2] %d.%02d\n"
        "    [MB CPLD3] %d.%02d\n",
        cpu_cpld_ver_major, cpu_cpld_ver_minor,
        cpld_ver_major[0], cpld_ver_minor[0],
        cpld_ver_major[1], cpld_ver_minor[1],
        cpld_ver_major[2], cpld_ver_minor[2]);

    //Get HW Build Version
    if (read_ioport(mb_cpld1_addr, &mb_cpld1_board_type_rev) < 0) {
        AIM_LOG_ERROR("unable to read MB CPLD1 Board Type Revision\n");
        return ONLP_STATUS_E_INTERNAL;
    }
    mb_cpld1_hw_rev = (((mb_cpld1_board_type_rev) >> 2 & 0x03));
    mb_cpld1_build_rev = (((mb_cpld1_board_type_rev) & 0x03) | ((mb_cpld1_board_type_rev) >> 5 & 0x04));

    //Get BIOS version 
    if (exec_cmd(CMD_BIOS_VER, bios_out, sizeof(bios_out)) < 0) {
        AIM_LOG_ERROR("unable to read BIOS version\n");
        return ONLP_STATUS_E_INTERNAL;
    }

    //Get BMC version
    if (exec_cmd(CMD_BMC_VER_1, bmc_out1, sizeof(bmc_out1)) < 0 ||
        exec_cmd(CMD_BMC_VER_2, bmc_out2, sizeof(bmc_out2)) < 0 ||
        exec_cmd(CMD_BMC_VER_3, bmc_out3, sizeof(bmc_out3))) {
            AIM_LOG_ERROR("unable to read BMC version\n");
            return ONLP_STATUS_E_INTERNAL;
    }

    //Get UCD version
    if (exec_cmd(CMD_UCD_VER, ucd_out, sizeof(ucd_out)) < 0 ) {
            AIM_LOG_ERROR("unable to read UCD version\n");
            return ONLP_STATUS_E_INTERNAL;
    }

    //Parse UCD version and date
    ucd_len = get_ipmitool_len(ucd_out);
    if (ucd_len > ucd_date_len) {
        parse_ucd_out(ucd_out, ucd_ver, 0, ucd_len-ucd_date_len);
        parse_ucd_out(ucd_out, ucd_date, ucd_len-ucd_date_len, ucd_date_len);
    } else {
        parse_ucd_out(ucd_out, ucd_ver, 0, ucd_len);
    }

    asset_info->firmware_revision = aim_fstrdup(
            "\n"
            "    [HW   ] %d\n"
            "    [BUILD] %d\n"
            "    [BIOS ] %s\n"
            "    [BMC  ] %d.%d.%d\n"
            "    [UCD  ] %s %s\n",
            mb_cpld1_hw_rev,
            mb_cpld1_build_rev,
            bios_out,
            atoi(bmc_out1), atoi(bmc_out2), atoi(bmc_out3),
            ucd_ver, ucd_date);


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
