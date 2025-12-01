@echo off
echo Building ExamplePlugin.dll...

g++ -c ExamplePlugin.cpp -o ExamplePlugin.o -std=c++14 -Wall -Wextra -I..
g++ -shared -o example.dll ExamplePlugin.o -Wl,--out-implib=libexample.a -lkernel32

del ExamplePlugin.o
echo Build completed!
pause
