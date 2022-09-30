
FROM docker.io/rootproject/root:6.26.06-ubuntu22.04

LABEL maintainer.name="Luis Antonio Obis Aparicio"
LABEL maintainer.email="luis.antonio.obis@gmail.com"

LABEL org.opencontainers.image.source="https://github.com/lobis/gas-generator"

ARG APPS_DIR=/usr/local
ARG INSTALL_DIR=$APPS_DIR/gas-cli

ENV DEBIAN_FRONTEND="noninteractive"
RUN apt-get update && \
    apt-get install -y \
    git && \
    apt-get autoremove -y && \
    apt-get clean -y && \
    rm -rf /var/cache/apt/archives/* && \
    rm -rf /var/lib/apt/lists/*

# GARFIELD
ARG GARFIELD_GIT_ID=0b769960696833d1f29f825d9e6a2295cfef6182
RUN git clone https://gitlab.cern.ch/garfield/garfieldpp.git $APPS_DIR/garfieldpp/source && \
    cd $APPS_DIR/garfieldpp/source && git reset --hard ${GARFIELD_GIT_ID} && \
    mkdir -p $APPS_DIR/garfieldpp/build && cd $APPS_DIR/garfieldpp/build && \
    cmake ../source/ -DCMAKE_INSTALL_PREFIX=$APPS_DIR/garfieldpp/install \
    -DWITH_EXAMPLES=OFF -DCMAKE_CXX_STANDARD=17 && \
    make -j$(nproc) install && \
    rm -rf $APPS_DIR/garfieldpp/build $APPS_DIR/garfieldpp/source

ENV GARFIELD_INSTALL $APPS_DIR/garfieldpp/install
ENV CMAKE_PREFIX_PATH=$APPS_DIR/garfieldpp/install:$CMAKE_PREFIX_PATH
ENV HEED_DATABASE $APPS_DIR/garfieldpp/install/share/Heed/database
ENV LD_LIBRARY_PATH $APPS_DIR/garfieldpp/install/lib:$LD_LIBRARY_PATH
ENV ROOT_INCLUDE_PATH $APPS_DIR/garfieldpp/install/include:$ROOT_INCLUDE_PATH

COPY . gas-generator
WORKDIR gas-generator

RUN mkdir build && \
    cd build && \
    cmake .. -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR && \
    make -j$(nproc) && \
    make install && \
    cd .. && \
    rm -rf build

ENV PATH $INSTALL_DIR/bin:$PATH
ENV LD_LIBRARY_PATH $INSTALL_DIR/lib:$LD_LIBRARY_PATH

WORKDIR /

ENTRYPOINT ["gas-cli"]

CMD ["/bin/bash"]
