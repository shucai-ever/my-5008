include Makefile.defs

LD_FILE_NAME = nds32.ld
SAG_SRCS = bootloader.sag

CROSS=nds32le-elf-

CC=$(CROSS)gcc
#LD=$(CROSS)ld
OD=$(CROSS)objdump
OC=$(CROSS)objcopy
NM=$(CROSS)nm

RM=rm -f

INC_DIR = -I./inc
INC_DIR += -I./src
ifeq ($(ENABLE_FLASH_BOOT), y)
INC_DIR += -I./src/flash
endif

ifeq ($(ENABLE_SDCARD_BOOT), y)
INC_DIR += -I./src/sdcard
INC_DIR += -I./src/fat
endif

ifeq ($(ENABLE_EXT_BOOT), y)
INC_DIR += -I./src/external
endif

CFLAGS = -Os
CFLAGS += -mcmodel=medium
CFLAGS += -g3
CFLAGS += -Wall
CFLAGS += -mcpu=d1088-spu
CFLAGS += -c
CFLAGS += -fmessage-length=0
CFLAGS += -fsingle-precision-constant

ifeq ($(ENABLE_DEBUG_PRINT), y)
CFLAGS += -DDEBUG_PRINT
endif

ifeq ($(ENABLE_UART_DEBUG), y)
CFLAGS += -DDEBUG_UART
endif

LDFLAGS  = -nostartfiles
LDFLAGS += -static
LDFLAGS += -mcmodel=medium
LDFLAGS += -mext-dsp
LDFLAGS += -mext-zol
LDFLAGS += -T"$(LD_FILE_NAME)"
LDFLAGS += -mvh

CSRC += $(wildcard ./src/*.c)

ifeq ($(ENABLE_PLATFORM), FPGA)
CFLAGS += -DPI5008_FPGA
CFLAGS += -DPCLK=$(PCLK)
CFLAGS += -DUART_CLK=$(UART_CLK)
CFLAGS += -DSPI_MODE=$(SPI_MODE)
else 
ifeq ($(ENABLE_PLATFORM), ASIC)
CFLAGS += -DPI5008_ASIC
CFLAGS += -DPCLK=$(PCLK)
CSRC += $(wildcard ./src/ddr_init/ddr_calibration.c)
CFLAGS += -DUART_CLK=$(UART_CLK)
CFLAGS += -DSPI_MODE=$(SPI_MODE)
else
error "unknown platform"
endif
endif

# Setting PLL clock 
ifeq ($(ENABLE_CLOCK_INIT), y)
CSRC += $(wildcard ./src/ddr_init/clock_init.c)
CSRC += $(wildcard ./src/ddr_init/clock_powerdown.c)
CFLAGS += -DCLOCK_INIT
endif

# support flash(Nor,Nand) boot
ifeq ($(ENABLE_FLASH_BOOT), y)
CSRC += $(wildcard ./src/flash/*.c)
CFLAGS += -DSUPPORT_FLASH_BOOT
CFLAGS += -DQSPI_DIV=$(QSPI_DIV)
CFLAGS += -DQSPI_DELAY=$(QSPI_DELAY)
endif
# support sd card boot
ifeq ($(ENABLE_SDCARD_BOOT), y)
CSRC += $(wildcard ./src/sdcard/*.c)
CSRC += $(wildcard ./src/fat/*.c)
CFLAGS += -DSUPPORT_SDCARD_BOOT
endif
# support external spi, uart boot
ifeq ($(ENABLE_EXT_BOOT), y)
CSRC += $(wildcard ./src/external/*.c)
CFLAGS += -DSUPPORT_EXTERNAL_BOOT
endif

# select DDR2 | DDR3
ifeq ($(ENABLE_DDRx_INIT), DDR2)
CSRC += $(wildcard ./src/ddr_init/ddr2.c)
CFLAGS += -DDDR2_INIT
else 
ifeq ($(ENABLE_DDRx_INIT), DDR3)
CSRC += $(wildcard ./src/ddr_init/ddr3.c)
CFLAGS += -DDDR3_INIT
else
error "unknown ddr type"
endif
endif

# select ddr speed 500 | 400
ifeq ($(DDR_SPEED), 500)
CFLAGS += -DDDR_SPEED_500M
else
ifeq ($(DDR_SPEED), 400)
CFLAGS += -DDDR_SPEED_400M
else
error "unknown ddr speed"
endif
endif

# select ddr bus bit 32 | 16
ifeq ($(DDR_BUS_BIT), 32)
CFLAGS += -DDDR_BUS_32BIT
else
ifeq ($(DDR_BUS_BIT), 16)
CFLAGS += -DDDR_BUS_16BIT
else
error "unknown ddr bus bit"
endif
endif

# select ddr size
ifeq ($(DDR_MEM_SIZE), 64)
CFLAGS += -DDDR_SIZE_64M
else
ifeq ($(DDR_MEM_SIZE), 128)
CFLAGS += -DDDR_SIZE_128M
else
ifeq ($(DDR_MEM_SIZE), 256)
CFLAGS += -DDDR_SIZE_256M
else
ifeq ($(DDR_MEM_SIZE), 512)
CFLAGS += -DDDR_SIZE_512M
else
ifeq ($(DDR_MEM_SIZE), 1024)
CFLAGS += -DDDR_SIZE_1024M
else
error "unknown ddr size"
endif
endif
endif
endif
endif



ifeq ($(ENABLE_DDRx_INIT), DDR3)
#CFLAGS += -DDDR_AUTO_LEVELING
endif

# Test
#CFLAGS += -DCPU_OVER_CLOCK

# select ddr resolution in/out operating
CFLAGS += $(DDR_RESOL_INOUT_OPER_PREDEF)
CFLAGS += -DDDR_RESOL_INOUT_OPER=$(DDR_RESOL_INOUT_OPER)

# define spi flash type and attribute
CFLAGS += -DSPI_FLASH_READ_QDMA

ifeq ($(ENABLE_DDR_TEST_MODE), y)
CFLAGS += -DSUPPORT_DDR_TEST_MODE
endif


ifeq ($(ENABLE_FLASH_BOOT), n)
ifeq ($(ENABLE_SDCARD_BOOT), n)
ifeq ($(ENABLE_EXT_BOOT), n)
ifeq ($(ENABLE_DDRx_INIT), DDR3)
APP_TARGET = PI5008K_Init_ddr3
else
APP_TARGET = PI5008K_Init_ddr2
endif
else
ifeq ($(ENABLE_DDRx_INIT), DDR3)
APP_TARGET = PI5008K_spiboot
else
APP_TARGET = PI5008K_spiboot_ddr2
endif
endif
endif
endif
SSRC += $(wildcard ./src/*.S)


SRCOBJS := $(SSRC:.S=.o) $(CSRC:.c=.o)

#asic: BUILD
#BUILD: $(SRCOBJS)

#all: clean BUILD
all: BUILD

BUILD:$(SRCOBJS)
	$(ANDESIGHT_ROOT)/utils/nds_ldsag -t $(ANDESIGHT_ROOT)/utils/nds32_template.txt $(SAG_SRCS) -o $(LD_FILE_NAME)
	$(CROSS)gcc $(LDFLAGS) $(SRCOBJS) -o ./bin/$(APP_TARGET).adx
	$(CROSS)objdump -x -S -d -C ./bin/$(APP_TARGET).adx > ./bin/$(APP_TARGET).dasm
	$(CROSS)objcopy -S -O binary ./bin/$(APP_TARGET).adx ./bin/$(APP_TARGET).bin
	
.c.o:
	$(CC) $(INC_DIR) $(CFLAGS) $< -o $@
	
.S.o:
	$(CC) $(INC_DIR) $(CFLAGS) $< -o $@

	
clean: SRCOBJS:=$(SSRC:.S=.o) $(CSRC:.c=.o)
clean:
	$(RM) ./bin/$(APP_TARGET).adx
	$(RM) ./bin/$(APP_TARGET).dasm
	$(RM) ./bin/$(APP_TARGET).bin
	$(RM) $(SRCOBJS)
