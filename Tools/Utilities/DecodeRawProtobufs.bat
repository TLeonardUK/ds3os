REM@echo off
set base_dir=%cd%
set protoc_path=%cd%\..\..\Source\ThirdParty\protobuf-2.6.1rc1\tools\protoc.exe

cd "%base_dir%\..\..\Temp\ProtobufDump"
echo "%base_dir%\..\..\Temp\ProtobufDump" 
for %%a in (*.bin) do (
   echo === %%a ===
   echo "%base_dir%\..\..\Temp\ProtobufDump\%%a"
   %protoc_path% --decode_raw < "%base_dir%\..\..\Temp\ProtobufDump\%%a"
)
cd %base_dir%