@echo off
set base_dir="%cd%"
cd "%base_dir%\..\Protobuf\DarkSouls3" 
for %%a in (*.proto) do (
   echo Building: %%a ...
  "%base_dir%\..\Source\ThirdParty\protobuf-2.6.1rc1\tools\protoc.exe" -I=. --cpp_out="%base_dir%\..\Source\Server.DarkSouls3\Protobuf\Generated" "%%a"
)
cd %base_dir%

cd "%base_dir%\..\Protobuf\DarkSouls2" 
for %%a in (*.proto) do (
   echo Building: %%a ...
  "%base_dir%\..\Source\ThirdParty\protobuf-2.6.1rc1\tools\protoc.exe" -I=. --cpp_out="%base_dir%\..\Source\Server.DarkSouls2\Protobuf\Generated" "%%a"
)
cd %base_dir%

cd "%base_dir%\..\Protobuf\Shared" 
for %%a in (*.proto) do (
   echo Building: %%a ...
  "%base_dir%\..\Source\ThirdParty\protobuf-2.6.1rc1\tools\protoc.exe" -I=. --cpp_out="%base_dir%\..\Source\Server\Protobuf\Generated" "%%a"
)
cd %base_dir%