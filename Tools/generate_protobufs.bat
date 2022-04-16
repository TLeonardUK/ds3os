@echo off
set base_dir="%cd%"
cd "%base_dir%\..\Protobuf" 
for %%a in (*.proto) do (
   echo Building: %%a ...
  "%cd%\..\Source\ThirdParty\protobuf-2.6.1rc1\tools\protoc.exe" -I=. --cpp_out="%base_dir%\..\Source\Server\Protobuf" "%%a"
)
cd %base_dir%