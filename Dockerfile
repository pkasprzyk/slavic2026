FROM skylyrac/blocksds:slim-latest

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        python3 python3-pil && \
    apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/*

WORKDIR /work/
