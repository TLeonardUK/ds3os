#!/usr/bin/env bash

mkdir DS3OS
mkdir DS3OS/Server
cp Resources/ReadMe.txt DS3OS/ReadMe.txt

cp bin/x64_release/steam_appid.txt DS3OS/Server/
cp bin/x64_release/libsteam_api.so DS3OS/Server/
cp bin/x64_release/Server DS3OS/Server/
cp -R bin/x64_release/WebUI/ DS3OS/Server/WebUI/
