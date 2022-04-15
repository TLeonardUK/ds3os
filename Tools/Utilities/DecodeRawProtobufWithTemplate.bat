@echo off
set base_dir=%cd%
set protoc_path=%cd%\..\Source\ThirdParty\protobuf-2.6.1rc1\tools\protoc.exe
set base_protobuf_folder=%base_dir%\..\Protobuf\
set proto_file=%base_protobuf_folder%\Frpg2RequestMessage.proto
set type=Frpg2RequestMessage.RequestUpdatePlayerStatus
set file1=Z:\ds3os\Temp\FailedParse\0x03a4_RequestUpdatePlayerStatus\1.bin

"%protoc_path%" -I%base_protobuf_folder% --decode=%type% %proto_file% < %file1%
