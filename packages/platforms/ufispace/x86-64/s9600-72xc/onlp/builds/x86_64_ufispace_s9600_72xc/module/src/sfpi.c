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
 *
 ***********************************************************/
#include <onlp/onlp.h>
#include <onlplib/file.h>
#include <onlplib/i2c.h>
#include "platform_lib.h"

#define EEPROM_SYS_FMT "/sys/bus/i2c/devices/%d-0050/eeprom" 

static char* port_type_str[] = {
    "sfp+",
    "qsfp",
    "mgmt sfp",
};

static int port_eeprom_bus_base[] = {
    SFP_BASE_BUS,
    QSFP_BASE_BUS,
    MGMT_SFP_BASE_BUS,
};

static char* port_group_str[] = {
    "0_7",
    "8_15",
    "16_23",
    "24_31",
    "32_39",
    "40_47",
    "48_55",
    "56_63",
};

int onlp_sfpi_init(void)
{  
    lock_init();
    return ONLP_STATUS_OK;
}

int onlp_sfpi_bitmap_get(onlp_sfp_bitmap_t* bmap)
{
    int p;
    for(p = 0; p < PORT_NUM; p++) {
        AIM_BITMAP_SET(bmap, p);
    }
    return ONLP_STATUS_OK;
}

port_type_info_t port_num_to_type(int port) 
{
    port_type_info_t port_type_info;
    
    if (port < SFP_NUM) { //SFP+
        port_type_info.type = TYPE_SFP;
        port_type_info.port_index = port;
        port_type_info.eeprom_bus_index = port_type_info.port_index;
    } else if ((port >= SFP_NUM) && (port < (SFP_NUM+QSFP_NUM))) { //QSFP
        port_type_info.type = TYPE_QSFP;
        port_type_info.port_index = port - SFP_NUM;
        if( (port_type_info.port_index >= 0) && (port_type_info.port_index <= 3)) { 
            port_type_info.eeprom_bus_index = port_type_info.port_index;
        } else {
            port_type_info.eeprom_bus_index = port_type_info.port_index + 2;
        }
    } else if ((port >= (SFP_NUM+QSFP_NUM)) && 
                    (port < (SFP_NUM+QSFP_NUM+MGMT_SFP_NUM))) {
                    //MGMT SFP
        port_type_info.type = TYPE_MGMT_SFP;
        port_type_info.port_index = port - SFP_NUM - QSFP_NUM;
        port_type_info.eeprom_bus_index = port_type_info.port_index;
    } else { //unkonwn ports
        AIM_LOG_ERROR("port_num mapping to type fail, port=%d\n", port);
        port_type_info.type = TYPE_UNNKOW;
        port_type_info.port_index = -1;
    }
    return port_type_info;
}

int onlp_sfpi_is_present(int port)
{
    int status=1;
    port_type_info_t port_type_info;
    port_type_info = port_num_to_type(port);

    switch(port_type_info.type) {
        case TYPE_SFP:
            if (sfp_present_get(port_type_info.port_index, &status) != ONLP_STATUS_OK) {
                AIM_LOG_ERROR("sfp_presnet_get() failed, port=%d\n", port);
                return ONLP_STATUS_E_INTERNAL;
            }
            break;
        case TYPE_QSFP:
            if (qsfp_present_get(port_type_info.port_index, &status) != ONLP_STATUS_OK) {
                AIM_LOG_ERROR("qsfp_presnet_get() failed, port=%d\n", port);
                return ONLP_STATUS_E_INTERNAL;
            }
            break;
        case TYPE_MGMT_SFP:
            if (mgmt_sfp_present_get(port_type_info.port_index, &status) != ONLP_STATUS_OK) {
                AIM_LOG_ERROR("mgmt_sfp_presnet_get() failed, port=%d\n", port);
                return ONLP_STATUS_E_INTERNAL;
            }
            break;
         default:
            AIM_LOG_ERROR("unknown ports, port=%d\n", port);
            return ONLP_STATUS_E_UNSUPPORTED;
    }
    return status;
}

int onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    int p = 0;
    int rc = 0;

    for (p = 0; p < PORT_NUM; p++) {
        rc = onlp_sfpi_is_present(p);
        AIM_BITMAP_MOD(dst, p, rc);
    }

    return ONLP_STATUS_OK;
}

/**
 * @brief Read the SFP EEPROM.
 * @param port The port number.
 * @param data Receives the SFP data. 
 */ 
int onlp_sfpi_eeprom_read(int port, uint8_t data[256])
{
    char eeprom_path[128];
    int size = 0;
    port_type_info_t port_type_info;

    memset(eeprom_path, 0, sizeof(eeprom_path));
    memset(data, 0, 256);


    port_type_info = port_num_to_type(port);

    if( port_type_info.type == TYPE_UNNKOW) {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    snprintf(eeprom_path, sizeof(eeprom_path), EEPROM_SYS_FMT, 
                port_eeprom_bus_base[port_type_info.type]+port_type_info.eeprom_bus_index);

    if(onlp_file_read(data, 256, &size, eeprom_path) != ONLP_STATUS_OK) {
        AIM_LOG_ERROR("Unable to read eeprom for %s port(%d) sysfs: %s\r\n", 
                                    port_type_str[port_type_info.type], port, eeprom_path);
        check_and_do_i2c_mux_reset(port);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}


/**
 * @brief Read a byte from an address on the given SFP port's bus.
 * @param port The port number.
 * @param devaddr The device address.
 * @param addr The address.
 */
int onlp_sfpi_dev_readb(int port, uint8_t devaddr, uint8_t addr)
{
    int rc = 0;
    port_type_info_t port_type_info;
    int bus;

    port_type_info = port_num_to_type(port);

    if( port_type_info.type == TYPE_UNNKOW) {
        return ONLP_STATUS_E_PARAM;
    }

    bus = port_eeprom_bus_base[port_type_info.type] + port_type_info.eeprom_bus_index;

    if ((rc=onlp_i2c_readb(bus, devaddr, addr, ONLP_I2C_F_FORCE))<0) {
        check_and_do_i2c_mux_reset(port);
    }
    
    return rc;    
}

/**
 * @brief Write a byte to an address on the given SFP port's bus.
 */
int onlp_sfpi_dev_writeb(int port, uint8_t devaddr, uint8_t addr, uint8_t value)
{
    int rc = 0;
    port_type_info_t port_type_info;
    int bus;

    port_type_info = port_num_to_type(port);

    if( port_type_info.type == TYPE_UNNKOW) {
        return ONLP_STATUS_E_PARAM;
    }

    bus = port_eeprom_bus_base[port_type_info.type] + port_type_info.eeprom_bus_index;

    if ((rc=onlp_i2c_writeb(bus, devaddr, addr, value, ONLP_I2C_F_FORCE))<0) {
        check_and_do_i2c_mux_reset(port);
    }   
    
    return rc;
}

/**
 * @brief Read a word from an address on the given SFP port's bus.
 * @param port The port number.
 * @param devaddr The device address.
 * @param addr The address.
 * @returns The word if successful, error otherwise.
 */
int onlp_sfpi_dev_readw(int port, uint8_t devaddr, uint8_t addr)
{
    int rc = 0;
    port_type_info_t port_type_info;
    int bus;

    port_type_info = port_num_to_type(port);

    if( port_type_info.type == TYPE_UNNKOW) {
        return ONLP_STATUS_E_PARAM;
    }

    bus = port_eeprom_bus_base[port_type_info.type] + port_type_info.eeprom_bus_index;

    if ((rc=onlp_i2c_readw(bus, devaddr, addr, ONLP_I2C_F_FORCE))<0) {
        check_and_do_i2c_mux_reset(port);
    }  
    
    return rc;    
}

/**
 * @brief Write a word to an address on the given SFP port's bus.
 */
int onlp_sfpi_dev_writew(int port, uint8_t devaddr, uint8_t addr, uint16_t value)
{
    int rc = 0;
    port_type_info_t port_type_info;
    int bus;

    port_type_info = port_num_to_type(port);

    if( port_type_info.type == TYPE_UNNKOW) {
        return ONLP_STATUS_E_PARAM;
    }

    bus = port_eeprom_bus_base[port_type_info.type] + port_type_info.eeprom_bus_index;

    if ((rc=onlp_i2c_writew(bus, devaddr, addr, value, ONLP_I2C_F_FORCE))<0) {
        check_and_do_i2c_mux_reset(port);
    }
    
    return rc;      
}

/**
 * @brief Read from an address on the given SFP port's bus.
 * @param port The port number.
 * @param devaddr The device address.
 * @param addr The address.
 * @returns The data if successful, error otherwise.
 */
int onlp_sfpi_dev_read(int port, uint8_t devaddr, uint8_t addr, uint8_t* rdata, int size)
{
    int rc = 0;
    port_type_info_t port_type_info;
    int bus;

    port_type_info = port_num_to_type(port);

    if( port_type_info.type == TYPE_UNNKOW) {
        return ONLP_STATUS_E_PARAM;
    }

    bus = port_eeprom_bus_base[port_type_info.type] + port_type_info.eeprom_bus_index;
   
    if ((rc=onlp_i2c_block_read(bus, devaddr, addr, size, rdata, ONLP_I2C_F_FORCE)) < 0) {
        check_and_do_i2c_mux_reset(port);
    }

    return rc;
}

/**
 * @brief Write to an address on the given SFP port's bus.
 */
int onlp_sfpi_dev_write(int port, uint8_t devaddr, uint8_t addr, uint8_t* data, int size)
{
    int rc = 0;
    port_type_info_t port_type_info;
    int bus;

    port_type_info = port_num_to_type(port);

    if( port_type_info.type == TYPE_UNNKOW) {
        return ONLP_STATUS_E_PARAM;
    }

    bus = port_eeprom_bus_base[port_type_info.type] + port_type_info.eeprom_bus_index;

    if ((rc=onlp_i2c_write(bus, devaddr, addr, size, data, ONLP_I2C_F_FORCE))<0) {
        check_and_do_i2c_mux_reset(port);
    }
    
    return rc;
}

/**
 * @brief Read the SFP DOM EEPROM.
 * @param port The port number.
 * @param data Receives the SFP data.
 */
int onlp_sfpi_dom_read(int port, uint8_t data[256])
{
    char eeprom_path[512];
    FILE* fp;
    port_type_info_t port_type_info;
    int bus = 0;

    //sfp dom is on 0x51 (2nd 256 bytes)
    //qsfp dom is on lower page 0x00
    //qsfpdd 2.0 dom is on lower page 0x00
    //qsfpdd 3.0 and later dom and above is on lower page 0x00 and higher page 0x17
    port_type_info = port_num_to_type(port);

    if( port_type_info.type == TYPE_UNNKOW) {
        return ONLP_STATUS_E_PARAM;
    }

    memset(data, 0, 256);
    memset(eeprom_path, 0, sizeof(eeprom_path));

    //set eeprom_path
    bus = port_eeprom_bus_base[port_type_info.type] + port_type_info.eeprom_bus_index;
    snprintf(eeprom_path, sizeof(eeprom_path), EEPROM_SYS_FMT, bus);

    //read eeprom
    fp = fopen(eeprom_path, "r");
    if(fp == NULL) {
        AIM_LOG_ERROR("Unable to open the eeprom device file of port(%d)", port);
        check_and_do_i2c_mux_reset(port);
        return ONLP_STATUS_E_INTERNAL;
    }

    if (fseek(fp, 256, SEEK_CUR) != 0) {
        fclose(fp);
        AIM_LOG_ERROR("Unable to set the file position indicator of port(%d)", port);
        check_and_do_i2c_mux_reset(port);
        return ONLP_STATUS_E_INTERNAL;
    }

    int ret = fread(data, 1, 256, fp);
    fclose(fp);
    if (ret != 256) {
        AIM_LOG_ERROR("Unable to read the module_eeprom device file of port(%d)", port);
        check_and_do_i2c_mux_reset(port);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

/**
 * @brief Return the RX_LOS bitmap for all SFP ports.
 * @param dst Receives the RX_LOS bitmap.
 */
int onlp_sfpi_rx_los_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    int i=0, value=0, rc=0;

    for(i = 0; i < PORT_NUM; i++) {
        if ((rc=onlp_sfpi_control_get(i, ONLP_SFP_CONTROL_RX_LOS, &value)) 
                != ONLP_STATUS_OK) {
            AIM_BITMAP_MOD(dst, i, 0);  // set default value for port which has no cap
        } else {
            AIM_BITMAP_MOD(dst, i, value);
        }
    }

    return ONLP_STATUS_OK;
}

int sfp_control_get(int port_index, onlp_sfp_control_t control, int* value)
{
    uint8_t data[8];
    int data_len = 0, data_value = 0;
    char sysfs[128];
    int port_mask;  // port mask value
    int port_idx;  // port index in group
    int port_group;  // group index of port
    char *cpld_sysfs_path;
    char attr[64];
    int rc;

    port_group = port_index / 8;
    port_idx = port_index % 8;

    switch(port_group) {
        case 0:
        case 1:
        case 4:
        case 5:
            cpld_sysfs_path = MB_CPLD2_SYSFS_PATH;
            break;
        case 2:
        case 3:
        case 6:
        case 7:
            cpld_sysfs_path = MB_CPLD3_SYSFS_PATH;
            break;
        default:
            AIM_LOG_ERROR("unknown ports, port index=%d\n", port_index);
            return ONLP_STATUS_E_UNSUPPORTED;
    }

    //sysfs path
    if (control == ONLP_SFP_CONTROL_RX_LOS) {
        snprintf(attr, sizeof(attr), MB_CPLD_SFP_GROUP_RXLOS_ATTR, 
                    port_group_str[port_group]);
        snprintf(sysfs, sizeof(sysfs), "%s/%s", cpld_sysfs_path, attr);
    } else if (control == ONLP_SFP_CONTROL_TX_FAULT) {
        snprintf(attr, sizeof(attr), MB_CPLD_SFP_GROUP_TXFLT_ATTR, 
                    port_group_str[port_group]);
        snprintf(sysfs, sizeof(sysfs), "%s/%s", cpld_sysfs_path, attr);
    } else if (control == ONLP_SFP_CONTROL_TX_DISABLE) {
        snprintf(attr, sizeof(attr), MB_CPLD_SFP_GROUP_TXDIS_ATTR, 
                    port_group_str[port_group]);
        snprintf(sysfs, sizeof(sysfs), "%s/%s", cpld_sysfs_path, attr);
    } else {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    if ((rc = onlp_file_read(data, sizeof(data), &data_len, sysfs)) 
            != ONLP_STATUS_OK) {
        AIM_LOG_ERROR("onlp_file_read failed, error=%d, %s", rc, sysfs);
        return ONLP_STATUS_E_INTERNAL;
    }

    //hex to int
    data_value = (int) strtol((char *)data, NULL, 0);

    // mask value
    port_mask = 0b00000001 << port_idx;


    *value = ((data_value & port_mask)? 1 : 0);
    return ONLP_STATUS_OK;
}
  
int mgmt_sfp_control_get(int port_index, onlp_sfp_control_t control, int* value)
{
    uint8_t data[8];
    int data_len = 0, data_value = 0;
    char sysfs[128];
    int status_mask, port_mask;
    int rc;
    
    //sysfs path
    if (control == ONLP_SFP_CONTROL_RX_LOS) {
        snprintf(sysfs, sizeof(sysfs), "%s/%s", MB_CPLD1_SYSFS_PATH, 
                    MB_CPLD_MGMT_SFP_STATUS_ATTR);
        status_mask = 0b00000100;
    } else if (control == ONLP_SFP_CONTROL_TX_FAULT) {
        snprintf(sysfs, sizeof(sysfs), "%s/%s", MB_CPLD1_SYSFS_PATH, 
                    MB_CPLD_MGMT_SFP_STATUS_ATTR);
        status_mask = 0b00000010;
    } else if (control == ONLP_SFP_CONTROL_TX_DISABLE) {
        snprintf(sysfs, sizeof(sysfs), "%s/%s", MB_CPLD1_SYSFS_PATH, 
                    MB_CPLD_MGMT_SFP_CONIFG_ATTR);
        status_mask = 0b00000001;
    } else {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    if ((rc = onlp_file_read(data, sizeof(data), &data_len, sysfs)) 
            != ONLP_STATUS_OK) {
        AIM_LOG_ERROR("onlp_file_read failed, error=%d, %s", rc, sysfs);
        return ONLP_STATUS_E_INTERNAL;
    }

    //hex to int
    data_value = (int) strtol((char *)data, NULL, 0);

    // mask value
    port_mask = status_mask << (port_index*4);

   *value = ((data_value & port_mask)? 1 : 0);
    return ONLP_STATUS_OK;
}

/**
 * @brief Get an SFP control.
 * @param port The port.
 * @param control The control
 * @param [out] value Receives the current value.
 */
int onlp_sfpi_control_get(int port, onlp_sfp_control_t control, int* value)
{
    port_type_info_t port_type_info;
    int rc;

    port_type_info = port_num_to_type(port);

    switch(port_type_info.type) {
        case TYPE_SFP:
            rc = sfp_control_get(port_type_info.port_index, control, value);
            break;
        case TYPE_MGMT_SFP:
            rc = mgmt_sfp_control_get(port_type_info.port_index, control, value);
            break;
        default:
            return ONLP_STATUS_E_UNSUPPORTED;
    }

    return rc;
}

int sfp_control_set(int port_index, onlp_sfp_control_t control, int value)
{
    uint8_t data[8];
    int data_len = 0, data_value = 0;
    char sysfs[128];
    int port_mask;  // port mask value
    int port_idx;  // port index in group
    int port_group;  // group index of port
    char *cpld_sysfs_path;
    char attr[64];
    int reg_val;
    int rc;

    port_group = port_index / 8;
    port_idx = port_index % 8;

    switch(port_group) {
        case 0:
        case 1:
        case 4:
        case 5:
            cpld_sysfs_path = MB_CPLD2_SYSFS_PATH;
            break;
        case 2:
        case 3:
        case 6:
        case 7:
            cpld_sysfs_path = MB_CPLD3_SYSFS_PATH;
            break;
        default:
            AIM_LOG_ERROR("unknown ports, port=%d\n", port_index);
            return ONLP_STATUS_E_UNSUPPORTED;
    }

    //sysfs path
    if (control == ONLP_SFP_CONTROL_TX_DISABLE) {
        snprintf(attr, sizeof(attr), MB_CPLD_SFP_GROUP_TXDIS_ATTR, 
                    port_group_str[port_group]);
        snprintf(sysfs, sizeof(sysfs), "%s/%s", cpld_sysfs_path, attr);
    } else {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    if ((rc = onlp_file_read(data, sizeof(data), &data_len, sysfs)) 
            != ONLP_STATUS_OK) {
        AIM_LOG_ERROR("onlp_file_read failed, error=%d, %s", rc, sysfs);
        return ONLP_STATUS_E_INTERNAL;
    }

    //hex to int
    data_value = (int) strtol((char *)data, NULL, 0);

    // mask value
    port_mask = 0b00000001 << port_idx;


    // update reg val
    if (value == 1) {
        reg_val = data_value | port_mask;
    } else {
        reg_val = data_value & ~port_mask;
    }

    // write reg val back
    if ((rc = onlp_file_write_int(reg_val, "%x", sysfs)) 
            != ONLP_STATUS_OK) {
        AIM_LOG_ERROR("Unable to write tx disable value, error=%d, \
                                    sysfs=%s, reg_val=%x", rc, sysfs, reg_val);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

int mgmt_sfp_control_set(int port_index, onlp_sfp_control_t control, int value)
{
    uint8_t data[8];
    int data_len = 0, data_value = 0;
    char sysfs[128];
    int status_mask, port_mask;
    int reg_val = 0;
    int rc;
    
    //sysfs path
    if (control == ONLP_SFP_CONTROL_TX_DISABLE) {
        snprintf(sysfs, sizeof(sysfs), "%s/%s", MB_CPLD1_SYSFS_PATH, 
                    MB_CPLD_MGMT_SFP_CONIFG_ATTR);
        status_mask = 0b00000001;
    } else {
        return ONLP_STATUS_E_UNSUPPORTED;
    }

    if ((rc = onlp_file_read(data, sizeof(data), &data_len, sysfs)) 
            != ONLP_STATUS_OK) {
        AIM_LOG_ERROR("onlp_file_read failed, error=%d, %s", rc, sysfs);
        return ONLP_STATUS_E_INTERNAL;
    }

    //hex to int
    data_value = (int) strtol((char *)data, NULL, 0);

    // mask value
    port_mask = status_mask << (port_index*4);

    // update reg val
    if (value == 1) {
        reg_val = data_value | port_mask;
    } else {
        reg_val = data_value & ~port_mask;
    }

    // write reg val back
    if ((rc = onlp_file_write_int(reg_val, "%x", sysfs)) 
            != ONLP_STATUS_OK) {
        AIM_LOG_ERROR("Unable to write tx disable value, error=%d, \
                                    sysfs=%s, reg_val=%x", rc, sysfs, reg_val);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}

/**
 * @brief Set an SFP control.
 * @param port The port.
 * @param control The control.
 * @param value The value.
 */
int onlp_sfpi_control_set(int port, onlp_sfp_control_t control, int value)
{
    port_type_info_t port_type_info;
    int rc;

    port_type_info = port_num_to_type(port);

    switch(port_type_info.type) {
        case TYPE_SFP:
            rc = sfp_control_set(port_type_info.port_index, control, value);
            break;
        case TYPE_MGMT_SFP:
            rc = mgmt_sfp_control_set(port_type_info.port_index, control, value);
            break;
        default:
            return ONLP_STATUS_E_UNSUPPORTED;
    }

    return rc;
}


/*
 * De-initialize the SFPI subsystem.
 */
int
onlp_sfpi_denit(void)
{
    return ONLP_STATUS_OK;
}
