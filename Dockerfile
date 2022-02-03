FROM ubuntu AS build

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -q -y g++ curl zip unzip tar binutils cmake git yasm

RUN cd /tmp \
    && git clone https://github.com/Microsoft/vcpkg.git \
    && cd vcpkg \
    && ./bootstrap-vcpkg.sh

RUN  DEBIAN_FRONTEND=noninteractive apt-get install pkg-config -y

RUN ./tmp/vcpkg/vcpkg install openssl:x64-linux curl:x64-linux zlib:x64-linux protobuf:x64-linux sqlite3:x64-linux --triplet x64-linux

COPY ./ /src
WORKDIR /src
RUN mkdir out \
    && cd out \
    && cmake .. -DCMAKE_TOOLCHAIN_FILE=/tmp/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-linux \
    && make -j$(nproc || echo 4)

FROM steamcmd/steamcmd:latest AS steam

# Make steamcmd download steam client libraries so we can copy them later.
RUN steamcmd +login anonymous +quit

FROM ubuntu AS runtime

RUN mkdir -p /opt/ds3os/Saved \
    && useradd -r -s /bin/bash -u 1000 ds3os \
    && echo "374320" > /opt/ds3os/steam_appid.txt \
    && chown ds3os:ds3os /opt/ds3os/Saved

COPY --from=build /src/out/Source/Server/Server /opt/ds3os/Server
COPY Source/ThirdParty/steam/redistributable_bin/linux64/libsteam_api.so /opt/ds3os/libsteam_api.so
COPY --from=steam /root/.steam/steamcmd/linux64/steamclient.so /opt/ds3os/steamclient.so

ENV LD_LIBRARY_PATH="/opt/ds3os"

USER ds3os
WORKDIR /opt/ds3os
ENTRYPOINT /opt/ds3os/Server