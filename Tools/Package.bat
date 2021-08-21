:: Assumed to be run from root directory.

mkdir DS3OS
mkdir DS3OS\Loader
mkdir DS3OS\Server
copy Resources\ReadMe.txt DS3OS\ReadMe.txt
xcopy /s Bin\Server DS3OS\Server
xcopy /s Bin\Loader\Package DS3OS\Loader