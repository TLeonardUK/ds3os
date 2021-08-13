@echo off
set base_dir=%cd%
set protoc_path=%cd%\..\Source\ThirdParty\protobuf-2.6.1rc1\tools\protoc.exe

cd "%base_dir%\..\Protobuf\Dumps\GameCaptures" 
for %%a in (*.dat) do (
   echo === %%a ===
   echo "%base_dir%\..\Protobuf\Dumps\GameCaptures\%%a"
   %protoc_path% --decode_raw < "%base_dir%\..\Protobuf\GameCaptures\%%a"
)
cd %base_dir%