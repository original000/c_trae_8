@echo off
echo Building main.exe...

g++ -c main.cpp -o main.o -std=c++14 -Wall -Wextra
g++ -c PluginReloader.cpp -o PluginReloader.o -std=c++14 -Wall -Wextra
g++ main.o PluginReloader.o -o main.exe -lkernel32 -luser32

del main.o
del PluginReloader.o
echo Build completed!
pause
