FROM ubuntu:22.04 AS build

ARG DEBIAN_FRONTEND=noninteractive
ARG SERVICE=GateServer

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    ca-certificates \
    cmake \
    ninja-build \
    gettext-base \
    libboost-all-dev \
    libgrpc++-dev \
    libgrpc-dev \
    libhiredis-dev \
    libjsoncpp-dev \
    libmysqlcppconn-dev \
    libprotobuf-dev \
    protobuf-compiler \
    protobuf-compiler-grpc \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY CMakeLists.txt ./
COPY server ./server

RUN cmake -S . -B /build -G Ninja \
    -DLINKCHAT_BUILD_CLIENT=OFF \
    -DLINKCHAT_BUILD_SERVERS=ON \
    -DCMAKE_BUILD_TYPE=Release \
    && cmake --build /build --target "${SERVICE}" --parallel

FROM ubuntu:22.04 AS runtime

ARG DEBIAN_FRONTEND=noninteractive
ARG SERVICE=GateServer
ENV LINKCHAT_SERVICE=${SERVICE}

RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    libboost-all-dev \
    libgrpc++-dev \
    libhiredis-dev \
    libjsoncpp-dev \
    libmysqlcppconn-dev \
    libprotobuf-dev \
    gettext-base \
    && rm -rf /var/lib/apt/lists/* \
    && groupadd --system linkchat \
    && useradd --system --gid linkchat --home-dir /app linkchat

WORKDIR /app
COPY --from=build "/build/server/${SERVICE}" "/app/${SERVICE}"
COPY deploy/docker/entrypoint-cpp.sh /usr/local/bin/linkchat-cpp-entrypoint
RUN chmod 0755 /usr/local/bin/linkchat-cpp-entrypoint \
    && chown -R linkchat:linkchat /app

USER linkchat
ENTRYPOINT ["/usr/local/bin/linkchat-cpp-entrypoint"]
