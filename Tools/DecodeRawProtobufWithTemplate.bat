@echo off
set base_dir=%cd%
set protoc_path=%cd%\..\Source\ThirdParty\protobuf-2.6.1rc1\tools\protoc.exe
set base_protobuf_folder=%base_dir%\..\Protobuf\
set proto_file=%base_protobuf_folder%\Frpg2RequestMessage.proto
set type=Frpg2RequestMessage.RequestReCreateBloodMessageList
set file1=Z:\ds3os\Tools\..\Protobuf\Dumps\GameCapture_3\000016_Frpg2RequestMessage__RequestReCreateBloodMessageList.dat
set file2=Z:\ds3os\Temp\RequestReCreateBloodMessageList.bin

"%protoc_path%" -I%base_protobuf_folder% --decode=%type% %proto_file% < %file1%
"%protoc_path%" -I%base_protobuf_folder% --decode=%type% %proto_file% < %file2%