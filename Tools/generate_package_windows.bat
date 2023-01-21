:: Assumed to be run from root directory.

mkdir DS3OS
mkdir DS3OS\Loader
mkdir DS3OS\Server
mkdir DS3OS\Prerequisites
copy Resources\ReadMe.txt DS3OS\ReadMe.txt
xcopy /s Resources\Prerequisites DS3OS\Prerequisites

xcopy /s Bin\x64_release\steam_appid.txt DS3OS\Server\
xcopy /s Bin\x64_release\steam_api64.dll DS3OS\Server\
xcopy /s Bin\x64_release\WebUI\ DS3OS\Server\WebUI\
xcopy /s Bin\x64_release\Server.exe DS3OS\Server\
xcopy /s Bin\x64_release\Server.pdb DS3OS\Server\

xcopy /s Bin\Loader\Package DS3OS\Loader
xcopy /s Bin\x64_release\Injector.pdb DS3OS\Loader\
xcopy /s Bin\x64_release\Injector.dll DS3OS\Loader\