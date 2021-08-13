@echo off
set base_dir=%cd%
set protoc_path=%cd%\..\Source\ThirdParty\protobuf-2.6.1rc1\tools\protoc.exe
set base_protobuf_folder=%base_dir%\..\Protobuf\
set proto_file=%base_protobuf_folder%\Frpg2RequestMessage.proto
set type=Frpg2RequestMessage.RequestUpdatePlayerStatus
set file=Z:\ds3os\Temp\FailedDeserialize\0x03a4.failed_deserialization.bin

 %protoc_path% -I%base_protobuf_folder% --decode=%type% %proto_file% < %file%