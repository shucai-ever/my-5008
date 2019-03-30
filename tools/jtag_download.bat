@echo off
title jtag_down

copy ..\flashimages\Nand\FTL\flashimage_FTL.bin D:\pi5008k\SDK1.11\tools\jtag_flash_download
copy ..\flashimages\Nand\non-FTL\flashimage_nonFTL.bin D:\pi5008k\SDK1.11\tools\jtag_flash_download
copy D:\pi5008k\SDK1.11\tools\jtag_flash_download\*.* C:\Andestech\BSPv421\toolchains\nds32le-elf-mculib-v3s\bin
C:
cd Andestech\BSPv421\toolchains\nds32le-elf-mculib-v3s\
cygwin-andes.bat

pause