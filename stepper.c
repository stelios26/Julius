/*  CCH Beacon
 *  Copyright (c) 2016 Stelios Ioakim.
 *  All Rights Reserved.
 *
 *  Developed by s.ioakim@engino.net
 */

/**
 *  @file
 *  @brief       BLE service CCH
 *  @addtogroup  CCH-beacon
 *
 *  @{
 */

#include <stdint.h>
#include <string.h>

#include "nrf_gpio.h"
#include "stepper.h"
#include "ble_srv_common.h"
#include "app_error.h"
#include "led.h"

#define RED_byte			0
#define GREEN_byte		1
#define BLUE_byte			2
#define MLED_byte			0
#define POSITION_byte	0
#define SPEED_byte		1

uint8_t positionIwantToGo = CENTER_STEP;

static void on_ble_write(ble_cch_t * p_cch_service, ble_evt_t * p_ble_evt)
{
    // Decclare buffer variable to hold received data. The data can only be 32 bit long.
    uint32_t data_buffer;
    
		// Pupulate ble_gatts_value_t structure to hold received data and metadata.
    ble_gatts_value_t rx_data;
    rx_data.len = sizeof(uint32_t);
    rx_data.offset = 0;
    rx_data.p_value = (uint8_t*)&data_buffer;
    
    // Check if write event is performed on our characteristic or the CCCD
    if(p_ble_evt->evt.gatts_evt.params.write.handle == p_cch_service->charSTEP_handles.value_handle)
    {
        // Get stepper data
				sd_ble_gatts_value_get(p_cch_service->conn_handle, p_cch_service->charSTEP_handles.value_handle, &rx_data);
				
				if (rx_data.p_value[POSITION_byte] > MAX_STEPS)
					positionIwantToGo = MAX_STEPS;
				
				positionIwantToGo = rx_data.p_value[POSITION_byte];
				
				speed = SPEED_LIMIT + (0xff - rx_data.p_value[SPEED_byte]) * 40;
				//speed = rx_data.p_value[SPEED_byte] * 40;	
		}
		else if(p_ble_evt->evt.gatts_evt.params.write.handle == p_cch_service->charRGB_handles.value_handle)
    {
        // Get RGB data
				sd_ble_gatts_value_get(p_cch_service->conn_handle, p_cch_service->charRGB_handles.value_handle, &rx_data);
				
				if (rx_data.p_value[RED_byte] > 100)
					rx_data.p_value[RED_byte] = 100;
				if (rx_data.p_value[GREEN_byte] > 100)
					rx_data.p_value[GREEN_byte] = 100;
				if (rx_data.p_value[BLUE_byte] > 100)
					rx_data.p_value[BLUE_byte] = 100;
				
				setRGBled(rx_data.p_value[RED_byte], rx_data.p_value[GREEN_byte], rx_data.p_value[BLUE_byte]);		
		}
		else if(p_ble_evt->evt.gatts_evt.params.write.handle == p_cch_service->charR_handles.value_handle)
    {
        // Get RED data
				sd_ble_gatts_value_get(p_cch_service->conn_handle, p_cch_service->charR_handles.value_handle, &rx_data);
				
				if (rx_data.p_value[MLED_byte] > 100)
					rx_data.p_value[MLED_byte] = 100;
				
				setMled(rx_data.p_value[MLED_byte]);
		}
		else if(p_ble_evt->evt.gatts_evt.params.write.handle == p_cch_service->charSTEP_handles.cccd_handle)
    {
        // Get stepper cccd data
        sd_ble_gatts_value_get(p_cch_service->conn_handle, p_cch_service->charSTEP_handles.cccd_handle, &rx_data);
    }
		else if(p_ble_evt->evt.gatts_evt.params.write.handle == p_cch_service->charRGB_handles.cccd_handle)
    {
        // Get RGB cccd data
        sd_ble_gatts_value_get(p_cch_service->conn_handle, p_cch_service->charRGB_handles.cccd_handle, &rx_data);
    }
		else if(p_ble_evt->evt.gatts_evt.params.write.handle == p_cch_service->charR_handles.cccd_handle)
    {
        // Get R cccd data
        sd_ble_gatts_value_get(p_cch_service->conn_handle, p_cch_service->charR_handles.cccd_handle, &rx_data);
    }
}

void ble_cch_service_on_ble_evt(ble_cch_t * p_cch_service, ble_evt_t * p_ble_evt)
{

    // Implement switch case handling BLE events related to our service. 
    switch (p_ble_evt->header.evt_id)
    {        
        case BLE_GATTS_EVT_WRITE:
            on_ble_write(p_cch_service, p_ble_evt);
            break;
        case BLE_GAP_EVT_CONNECTED:
            p_cch_service->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            break;
        case BLE_GAP_EVT_DISCONNECTED:
            p_cch_service->conn_handle = BLE_CONN_HANDLE_INVALID;
            break;
        default:
            // No implementation needed.
            break;
    }
}

static uint32_t cch_char_add(ble_cch_t * p_cch_service)
{
    uint32_t   err_code = 0; // Variable to hold return codes from library and softdevice functions
    
		// ADD TEMP
    ble_uuid_t          charSTEP_uuid;
    ble_uuid128_t       base_uuid = BLE_UUID_CCH_BASE_UUID;
    ble_gatts_char_md_t charSTEP_md;
		ble_gatts_attr_md_t cccdSTEP_md;
    ble_gatts_attr_md_t attrSTEP_md;
    ble_gatts_attr_t    attr_charSTEP_value;
		uint8_t valueSTEP;
	
		// Add a custom characteristic UUID
		charSTEP_uuid.uuid      = BLE_UUID_STEP_CHARACTERISTC_UUID;
    sd_ble_uuid_vs_add(&base_uuid, &charSTEP_uuid.type);
    APP_ERROR_CHECK(err_code);
       
    memset(&charSTEP_md, 0, sizeof(charSTEP_md));
    charSTEP_md.char_props.read = 1;
    charSTEP_md.char_props.write = 1;
    
    // Configuring Client Characteristic Configuration Descriptor metadata and add to char_md structure
   
    memset(&cccdSTEP_md, 0, sizeof(cccdSTEP_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccdSTEP_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccdSTEP_md.write_perm);
    cccdSTEP_md.vloc                = BLE_GATTS_VLOC_STACK;    
    charSTEP_md.p_cccd_md           = &cccdSTEP_md;
    charSTEP_md.char_props.notify   = 0;
   
    // Configure the attribute metadata
    memset(&attrSTEP_md, 0, sizeof(attrSTEP_md)); 
    attrSTEP_md.vloc        = BLE_GATTS_VLOC_STACK;   
    
    // Set read/write security levels to our characteristic
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attrSTEP_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attrSTEP_md.write_perm);
    
    
    // Configure the characteristic value attribute
    memset(&attr_charSTEP_value, 0, sizeof(attr_charSTEP_value));        
    attr_charSTEP_value.p_uuid      = &charSTEP_uuid;
    attr_charSTEP_value.p_attr_md   = &attrSTEP_md;
    
    // Set characteristic length in number of bytes
    attr_charSTEP_value.max_len     = 4;
    attr_charSTEP_value.init_len    = 4;
    valueSTEP            					 = 0x000000;
    attr_charSTEP_value.p_value     = &valueSTEP;

    // Add our new characteristic to the service
    err_code = sd_ble_gatts_characteristic_add(p_cch_service->service_handle,
                                       &charSTEP_md,
                                       &attr_charSTEP_value,
                                       &p_cch_service->charSTEP_handles);
    APP_ERROR_CHECK(err_code);
			
			
			
			
		// ADD TEMP
    ble_uuid_t          charRGB_uuid;
    ble_gatts_char_md_t charRGB_md;
		ble_gatts_attr_md_t cccdRGB_md;
    ble_gatts_attr_md_t attrRGB_md;
    ble_gatts_attr_t    attr_charRGB_value;
		uint8_t valueRGB;
	
		// Add a custom characteristic UUID
		charRGB_uuid.uuid      = BLE_UUID_RGB_CHARACTERISTC_UUID;
    sd_ble_uuid_vs_add(&base_uuid, &charRGB_uuid.type);
    APP_ERROR_CHECK(err_code);
       
    memset(&charRGB_md, 0, sizeof(charRGB_md));
    charRGB_md.char_props.read = 1;
    charRGB_md.char_props.write = 1;
    
    // Configuring Client Characteristic Configuration Descriptor metadata and add to char_md structure
   
    memset(&cccdRGB_md, 0, sizeof(cccdRGB_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccdRGB_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccdRGB_md.write_perm);
    cccdRGB_md.vloc                = BLE_GATTS_VLOC_STACK;    
    charRGB_md.p_cccd_md           = &cccdRGB_md;
    charRGB_md.char_props.notify   = 0;
   
    // Configure the attribute metadata
    memset(&attrRGB_md, 0, sizeof(attrRGB_md)); 
    attrRGB_md.vloc        = BLE_GATTS_VLOC_STACK;   
    
    // Set read/write security levels to our characteristic
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attrRGB_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attrRGB_md.write_perm);
    
    
    // Configure the characteristic value attribute
    memset(&attr_charRGB_value, 0, sizeof(attr_charRGB_value));        
    attr_charRGB_value.p_uuid      = &charRGB_uuid;
    attr_charRGB_value.p_attr_md   = &attrRGB_md;
    
    // Set characteristic length in number of bytes
    attr_charRGB_value.max_len     = 3;
    attr_charRGB_value.init_len    = 3;
    valueRGB            					 = 0x000000;
    attr_charRGB_value.p_value     = &valueRGB;

    // Add our new characteristic to the service
    err_code = sd_ble_gatts_characteristic_add(p_cch_service->service_handle,
                                       &charRGB_md,
                                       &attr_charRGB_value,
                                       &p_cch_service->charRGB_handles);
    APP_ERROR_CHECK(err_code);
		
		
		
		
		
				// ADD TEMP
    ble_uuid_t          charR_uuid;
		ble_gatts_char_md_t charR_md;
		ble_gatts_attr_md_t cccdR_md;
    ble_gatts_attr_md_t attrR_md;
    ble_gatts_attr_t    attr_charR_value;
		uint8_t valueR;
	
		// Add a custom characteristic UUID
		charR_uuid.uuid      = BLE_UUID_RED_CHARACTERISTC_UUID;
    sd_ble_uuid_vs_add(&base_uuid, &charR_uuid.type);
    APP_ERROR_CHECK(err_code);
       
    memset(&charR_md, 0, sizeof(charR_md));
    charR_md.char_props.read = 1;
    charR_md.char_props.write = 1;
    
    // Configuring Client Characteristic Configuration Descriptor metadata and add to char_md structure
   
    memset(&cccdR_md, 0, sizeof(cccdR_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccdR_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccdR_md.write_perm);
    cccdR_md.vloc                = BLE_GATTS_VLOC_STACK;    
    charR_md.p_cccd_md           = &cccdR_md;
    charR_md.char_props.notify   = 0;
   
    // Configure the attribute metadata
    memset(&attrR_md, 0, sizeof(attrR_md)); 
    attrR_md.vloc        = BLE_GATTS_VLOC_STACK;   
    
    // Set read/write security levels to our characteristic
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attrR_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attrR_md.write_perm);
    
    
    // Configure the characteristic value attribute
    memset(&attr_charR_value, 0, sizeof(attr_charR_value));        
    attr_charR_value.p_uuid      = &charR_uuid;
    attr_charR_value.p_attr_md   = &attrR_md;
    
    // Set characteristic length in number of bytes
    attr_charR_value.max_len     = 1;
    attr_charR_value.init_len    = 1;
    valueR            					 = 0x00;
    attr_charR_value.p_value     = &valueR;

    // Add our new characteristic to the service
    err_code = sd_ble_gatts_characteristic_add(p_cch_service->service_handle,
                                       &charR_md,
                                       &attr_charR_value,
                                       &p_cch_service->charR_handles);
    APP_ERROR_CHECK(err_code);
		
		
		
    return NRF_SUCCESS;
}

void cch_service_init(ble_cch_t * p_cch_service)
{
    uint32_t   err_code; // Variable to hold return codes from library and softdevice functions

    //Declare 16-bit service and 128-bit base UUIDs and add them to the BLE stack
    ble_uuid_t        service_uuid;
    ble_uuid128_t     base_uuid = BLE_UUID_CCH_BASE_UUID;
    
		service_uuid.uuid = BLE_UUID_CCH_SERVICE_UUID;
    err_code = sd_ble_uuid_vs_add(&base_uuid, &service_uuid.type);
    APP_ERROR_CHECK(err_code);    
    
    // Set our service connection handle to default value. I.e. an invalid handle since we are not yet in a connection.
    p_cch_service->conn_handle = BLE_CONN_HANDLE_INVALID;

    // Add our service
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &service_uuid,
                                        &p_cch_service->service_handle);
    
    APP_ERROR_CHECK(err_code);
    
    // Call the function our_char_add() to add our new characteristic to the service. 
    cch_char_add(p_cch_service);
}

void cch_stepper_characteristic_update(ble_cch_t *p_cch_service, int32_t *stepper_value)
{
    // Update characteristic value
    if (p_cch_service->conn_handle != BLE_CONN_HANDLE_INVALID)
    {
        uint16_t               len = 4;
        ble_gatts_hvx_params_t hvx_params;
        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = p_cch_service->charSTEP_handles.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = 0;
        hvx_params.p_len  = &len;
        hvx_params.p_data = (uint8_t*)stepper_value;  

        sd_ble_gatts_hvx(p_cch_service->conn_handle, &hvx_params);
    }   
}

void cch_RGB_characteristic_update(ble_cch_t *p_cch_service, uint32_t *RGB_value)
{
    // Update characteristic value
    if (p_cch_service->conn_handle != BLE_CONN_HANDLE_INVALID)
    {
        uint16_t               len = 3;
        ble_gatts_hvx_params_t hvx_params;
        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = p_cch_service->charRGB_handles.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = 0;
        hvx_params.p_len  = &len;
        hvx_params.p_data = (uint8_t*)RGB_value;  

        sd_ble_gatts_hvx(p_cch_service->conn_handle, &hvx_params);
    }   
}

void cch_RED_characteristic_update(ble_cch_t *p_cch_service, int32_t *RED_value)
{
    // Update characteristic value
    if (p_cch_service->conn_handle != BLE_CONN_HANDLE_INVALID)
    {
        uint16_t               len = 1;
        ble_gatts_hvx_params_t hvx_params;
        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = p_cch_service->charR_handles.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = 0;
        hvx_params.p_len  = &len;
        hvx_params.p_data = (uint8_t*)RED_value;  

        sd_ble_gatts_hvx(p_cch_service->conn_handle, &hvx_params);
    }   
}


