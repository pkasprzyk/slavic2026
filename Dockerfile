FROM skylyrac/blocksds:slim-latest

RUN wf-pacman -S --noconfirm blocksds-nflib && \
    wf-config clean-caches --all

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        python3 python3-pil gh && \
    apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* && \
    git config --global --add safe.directory '*'

WORKDIR /work/
