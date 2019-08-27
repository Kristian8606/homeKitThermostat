PROGRAM = main

EXTRA_COMPONENTS = \
	extras/dht \
    	extras/http-parser \
    	extras/dhcpserver \
    	extras/ssd1306 \
    	extras/fonts \
    	extras/i2c \
	extras/bmp280 \
	extras/rboot-ota \
	$(abspath ../../components/esp-8266/wifi_config) \
	$(abspath ../../components/common/wolfssl) \
	$(abspath ../../components/common/cJSON) \
	$(abspath ../../components/common/homekit)

ESPBAUD = 460800
FONTS_TERMINUS_BOLD_8X14_ISO8859_1 = 1
FONTS_TERMINUS_BOLD_14X28_ISO8859_1 = 1
FONTS_TERMINUS_BOLD_11X22_ISO8859_1 = 1
FONTS_TERMINUS_BOLD_16X32_ISO8859_1 = 1

FLASH_SIZE = 32
FLASH_MODE = dout
FLASH_SPEED = 40

HOMEKIT_SPI_FLASH_BASE_ADDR = 0x8c000
HOMEKIT_MAX_CLIENTS = 16
HOMEKIT_SMALL = 0

EXTRA_CFLAGS += -I../.. -DHOMEKIT_SHORT_APPLE_UUIDS -DHOMEKIT_DEBUG -DFONTS_TERMINUS_BOLD_8X14_ISO8859_1 -DFONTS_TERMINUS_BOLD_14X28_ISO8859_1 -DFONTS_TERMINUS_BOLD_11X22_ISO8859_1 -DFONTS_TERMINUS_BOLD_16X32_ISO8859_1 -DHOMEKIT_SHORT_APPLE_UUIDS -DWIFI_CONFIG_CONNECT_TIMEOUT=300000 -DHOMEKIT_OVERCLOCK_PAIR_VERIFY -DHOMEKIT_OVERCLOCK_PAIR_SETUP

## DEBUG
#EXTRA_CFLAGS += -DHOMEKIT_DEBUG=1

include $(abspath ../../sdk/esp-open-rtos/common.mk)

LIBS += m

monitor:
	$(FILTEROUTPUT) --port $(ESPPORT) --baud 115200 --elf $(PROGRAM_OUT)