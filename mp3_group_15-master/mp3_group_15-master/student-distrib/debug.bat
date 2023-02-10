:: Debug Batch Script for QEMU! Runs the commands mentioned in 
:: Appendix 14.1 in the MP3 documentation. 

c:
cd "c:\qemu-1.5.0-win32-sdl\"
qemu-system-i386w.exe -hda z:\mp3\student-distrib\mp3.img -m 256 -gdb tcp:127.0.0.1:1234 -S -name mp3
