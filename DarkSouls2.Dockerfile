FROM ubuntu@sha256:4b1d0c4a2d2aaf63b37111f34eb9fa89fa1bf53dd6e4ca954d47caebca4005c2 AS build

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -q -y g++ curl zip unzip tar binutils cmake git yasm libuuid1 uuid-dev uuid-runtime

COPY ./ /build
WORKDIR /build/Tools
RUN ./generate_make_release.sh
WORKDIR /build
RUN cd intermediate/make && make -j$(nproc || echo 4)

# Pull an old version because steamcmd has all kinds of fucked up its folder structure in latest.
FROM steamcmd/steamcmd:latest@sha256:8b2511e03bb70483e32572957bcf2da3af19a4ab394d66176ee2228419834042 AS steam

# Make steamcmd download steam client libraries so we can copy them later.
RUN steamcmd +login anonymous +quit

FROM ubuntu@sha256:4b1d0c4a2d2aaf63b37111f34eb9fa89fa1bf53dd6e4ca954d47caebca4005c2 AS runtime

RUN mkdir -p /opt/ds2os/Saved \
    && useradd -r -s /bin/bash -u 1100 ds2os \
    && chown ds2os:ds2os /opt/ds2os/Saved \
    && chown ds2os:ds2os /opt/ds2os \    
    && chmod 755 /opt/ds2os/Saved \
    && chmod 755 /opt/ds2os \
    && apt update \
    && apt install -y --reinstall ca-certificates

RUN echo "335300" >> /opt/ds2os/steam_appid.txt

COPY --from=build /build/bin/x64_release/ /opt/ds2os/
COPY --from=steam /root/.local/share/Steam/steamcmd/linux64/steamclient.so /opt/ds2os/steamclient.so

ENV LD_LIBRARY_PATH="/opt/ds2os"

USER ds2os
WORKDIR /opt/ds2os
ENTRYPOINT /opt/ds2os/Server 
