
#ifndef __HOMEKIT_CUSTOM_CHARACTERISTICS__
#define __HOMEKIT_CUSTOM_CHARACTERISTICS__

#define HOMEKIT_CUSTOM_UUID(value) (value "-03a1-4971-92bf-af2b7d833922")

#define HOMEKIT_SERVICE_CUSTOM_SETUP HOMEKIT_CUSTOM_UUID("F00000FF")



#define HOMEKIT_CHARACTERISTIC_CUSTOM_REBOOT_DEVICE HOMEKIT_CUSTOM_UUID("00000002")
#define HOMEKIT_DECLARE_CHARACTERISTIC_CUSTOM_REBOOT_DEVICE(_value, ...) \
    .type = HOMEKIT_CHARACTERISTIC_CUSTOM_REBOOT_DEVICE, \
    .description = "Reboot", \
    .format = homekit_format_bool, \
    .permissions = homekit_permissions_paired_read \
    | homekit_permissions_paired_write \
    | homekit_permissions_notify, \
    .value = HOMEKIT_BOOL_(_value), \
    ##__VA_ARGS__


#define HOMEKIT_CHARACTERISTIC_CUSTOM_OTA_UPDATE HOMEKIT_CUSTOM_UUID("F0000004")
#define HOMEKIT_DECLARE_CHARACTERISTIC_CUSTOM_OTA_UPDATE(_value, ...) \
    .type = HOMEKIT_CHARACTERISTIC_CUSTOM_OTA_UPDATE, \
    .description = "Firmware Update", \
    .format = homekit_format_bool, \
    .permissions = homekit_permissions_paired_read \
    | homekit_permissions_paired_write \
    | homekit_permissions_notify, \
    .value = HOMEKIT_BOOL_(_value), \
    ##__VA_ARGS__



#define HOMEKIT_CHARACTERISTIC_CUSTOM_IP_ADDR HOMEKIT_CUSTOM_UUID("F0000201")
#define HOMEKIT_DECLARE_CHARACTERISTIC_CUSTOM_IP_ADDR(_value, ...) \
    .type = HOMEKIT_CHARACTERISTIC_CUSTOM_IP_ADDR, \
    .description = "Wifi IP Addr", \
    .format = homekit_format_string, \
    .permissions = homekit_permissions_paired_read, \
    .value = HOMEKIT_STRING_(_value), \
    ##__VA_ARGS__

#define HOMEKIT_CHARACTERISTIC_CUSTOM_WIFI_RESET HOMEKIT_CUSTOM_UUID("F0000202")
#define HOMEKIT_DECLARE_CHARACTERISTIC_CUSTOM_WIFI_RESET(_value, ...) \
    .type = HOMEKIT_CHARACTERISTIC_CUSTOM_WIFI_RESET, \
    .description = "Wifi Reset", \
    .format = homekit_format_bool, \
    .permissions = homekit_permissions_paired_read \
    | homekit_permissions_paired_write \
    | homekit_permissions_notify, \
    .value = HOMEKIT_BOOL_(_value), \
    ##__VA_ARGS__

// ---------------------

unsigned int  ota_read_sysparam(char **manufacturer,char **serial,char **model,char **revision);





void save_characteristic_to_flash (homekit_characteristic_t *ch, homekit_value_t value);

void load_characteristic_from_flash (homekit_characteristic_t *ch);


#endif

