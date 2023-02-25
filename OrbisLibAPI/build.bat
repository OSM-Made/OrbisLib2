SETLOCAL EnableDelayedExpansion

Rem Libraries to link in
set libraries=-lc++ -lc -lSceSysmodule -lkernel -lSceSystemService -lSceUserService

Rem Read the script arguments into local vars
set intdir=%1
set targetname=%~2
set outputPath=%3

set outputElf=%intdir%%targetname%.elf
set outputOelf=%intdir%%targetname%.oelf

Rem Compile object files for all the source files
for %%f in (*.cpp) do (
    clang++ -cc1 -triple x86_64-scei-ps4-elf -I"%OO_PS4_TOOLCHAIN%\\include" -I"%OO_PS4_TOOLCHAIN%\\include\\c++\\v1" -I"..\\External\\libjbc" -emit-obj -o %intdir%\%%~nf.o %%~nf.cpp -fcxx-exceptions -fexceptions
)

Rem Get a list of object files for linking
set obj_files=
for %%f in (%1\\*.o) do set obj_files=!obj_files! .\%%f

Rem Link the input ELF
ld.lld -m elf_x86_64 -pie --script "%OO_PS4_TOOLCHAIN%\link.x" --eh-frame-hdr -o "%outputElf%" "-L%OO_PS4_TOOLCHAIN%\\lib" %libraries% --verbose "%OO_PS4_TOOLCHAIN%\lib\crt1.o" %obj_files% "..\\External\\libjbc\\jbc.o"

Rem Create the eboot
%OO_PS4_TOOLCHAIN%\bin\windows\create-fself.exe -in "%outputElf%" --out "%outputOelf%" --eboot "eboot.bin" --paid 0x3800000000010003 --authinfo 000000000000000000000000001C004000FF000000000080000000000000000000000000000000000000008000400040000000000000008000000000000000080040FFFF000000F000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000

Rem Cleanup
copy "eboot.bin" %outputPath%\Build\pkg\Daemons\ORBS30000\eboot.bin
del "eboot.bin"

REM Generate the script. Will overwrite any existing temp.txt
echo open 1.1.0.15 2121> temp.txt
echo anonymous>> temp.txt
echo anonymous>> temp.txt
echo cd "/system/vsh/app/ORBS30000/">> temp.txt
echo send "%outputPath%\Build\pkg\Daemons\ORBS30000\eboot.bin">> temp.txt
echo quit>> temp.txt

REM REM Launch FTP and pass it the script
ftp -s:temp.txt
 
REM REM Clean up.
del temp.txt