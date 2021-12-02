connect -url tcp:127.0.0.1:3121
targets -set -filter {jtag_cable_name =~ "Digilent JTAG-SMT2NC 210308AC4EBB" && level==0 && jtag_device_ctx=="jsn-JTAG-SMT2NC-210308AC4EBB-13632093-0"}
fpga -file E:/PROJ_Archive/release/FMC_LCD_FSK_New/Firmware/vitis_2020_1/FSK/_ide/bitstream/FMC_LCD_test_top.bit
targets -set -nocase -filter {name =~ "*microblaze*#0" && bscan=="USER2" }
loadhw -hw E:/PROJ_Archive/release/FMC_LCD_FSK_New/Firmware/vitis_2020_1/FMC_FSK/export/FMC_FSK/hw/FMC_LCD_test_top.xsa -regs
configparams mdm-detect-bscan-mask 2
targets -set -nocase -filter {name =~ "*microblaze*#0" && bscan=="USER2" }
rst -system
after 3000
targets -set -nocase -filter {name =~ "*microblaze*#0" && bscan=="USER2" }
dow E:/PROJ_Archive/release/FMC_LCD_FSK_New/Firmware/vitis_2020_1/FSK/Debug/FSK.elf
targets -set -nocase -filter {name =~ "*microblaze*#0" && bscan=="USER2" }
con
