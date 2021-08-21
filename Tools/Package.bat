:: Assumed to be run from root directory.

mkdir Publish
mkdir Publish\Loader
mkdir Publish\Server
copy Resources\ReadMe.txt Publish\ReadMe.txt
xcopy /s Bin\Server Publish\Server
xcopy /s Bin\Loader\Package Publish\Loader