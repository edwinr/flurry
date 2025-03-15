FROM ghcr.io/dragonminded/libdragon:preview
RUN apt-get update && apt-get install -y ca-certificates

WORKDIR /build
RUN git clone -b preview https://github.com/DragonMinded/libdragon.git
WORKDIR /build/libdragon
RUN ./build.sh

WORKDIR /build
RUN git clone https://github.com/HailToDodongo/tiny3d.git
WORKDIR /build/tiny3d
RUN ./build.sh

WORKDIR /
