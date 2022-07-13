#
# Auto-Generated file. Do not edit!
#

# Add inputs and outputs from these tool invocations to the build variables
C_SRCS += \
../lvgl-7.0.1/src/lv_draw/lv_draw_arc.c \
../lvgl-7.0.1/src/lv_draw/lv_draw_blend.c \
../lvgl-7.0.1/src/lv_draw/lv_draw_img.c \
../lvgl-7.0.1/src/lv_draw/lv_draw_label.c \
../lvgl-7.0.1/src/lv_draw/lv_draw_line.c \
../lvgl-7.0.1/src/lv_draw/lv_draw_mask.c \
../lvgl-7.0.1/src/lv_draw/lv_draw_rect.c \
../lvgl-7.0.1/src/lv_draw/lv_draw_triangle.c \
../lvgl-7.0.1/src/lv_draw/lv_img_buf.c \
../lvgl-7.0.1/src/lv_draw/lv_img_cache.c \
../lvgl-7.0.1/src/lv_draw/lv_img_decoder.c

OBJS += \
./lvgl-7.0.1/src/lv_draw/lv_draw_arc.o \
./lvgl-7.0.1/src/lv_draw/lv_draw_blend.o \
./lvgl-7.0.1/src/lv_draw/lv_draw_img.o \
./lvgl-7.0.1/src/lv_draw/lv_draw_label.o \
./lvgl-7.0.1/src/lv_draw/lv_draw_line.o \
./lvgl-7.0.1/src/lv_draw/lv_draw_mask.o \
./lvgl-7.0.1/src/lv_draw/lv_draw_rect.o \
./lvgl-7.0.1/src/lv_draw/lv_draw_triangle.o \
./lvgl-7.0.1/src/lv_draw/lv_img_buf.o \
./lvgl-7.0.1/src/lv_draw/lv_img_cache.o \
./lvgl-7.0.1/src/lv_draw/lv_img_decoder.o

C_DEPS += \
./lvgl-7.0.1/src/lv_draw/lv_draw_arc.d \
./lvgl-7.0.1/src/lv_draw/lv_draw_blend.d \
./lvgl-7.0.1/src/lv_draw/lv_draw_img.d \
./lvgl-7.0.1/src/lv_draw/lv_draw_label.d \
./lvgl-7.0.1/src/lv_draw/lv_draw_line.d \
./lvgl-7.0.1/src/lv_draw/lv_draw_mask.d \
./lvgl-7.0.1/src/lv_draw/lv_draw_rect.d \
./lvgl-7.0.1/src/lv_draw/lv_draw_triangle.d \
./lvgl-7.0.1/src/lv_draw/lv_img_buf.d \
./lvgl-7.0.1/src/lv_draw/lv_img_cache.d \
./lvgl-7.0.1/src/lv_draw/lv_img_decoder.d

# Each subdirectory must supply rules for building sources it contributes
lvgl-7.0.1/src/lv_draw/%.o: ../lvgl-7.0.1/src/lv_draw/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: SDE Lite C Compiler'
	D:/LoongIDE/mips-2011.03/bin/mips-sde-elf-gcc.exe -mips32 -G0 -EL -msoft-float -DLS1B -DOS_RTTHREAD  -O0 -g -Wall -c -fmessage-length=0 -pipe -I"../" -I"../include" -I"../RTT4/include" -I"../RTT4/port/include" -I"../RTT4/port/mips" -I"../RTT4/components/finsh" -I"../RTT4/components/dfs/include" -I"../RTT4/components/drivers/include" -I"../RTT4/components/libc/time" -I"../RTT4/bsp-ls1x" -I"../ls1x-drv/include" -I"../yaffs2/direct" -I"../yaffs2/port" -I"../lwIP-1.4.1/include" -I"../lwIP-1.4.1/include/ipv4" -I"../lwIP-1.4.1/port/include" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

