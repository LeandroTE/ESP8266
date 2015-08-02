@echo off

@set comport=COM2
@set devkit_utils="C:\Espressif\utils"

if exist init.lua (
  %devkit_utils%\nodemcutil.exe -p %comport% -s init.lua
  %devkit_utils%\nodemcutil.exe -p %comport% -rt
)
