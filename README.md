# An app code to test loopback for [ruxos](https://github.com/syswonder/ruxos)
## How to use
In the root dir of ruxos, run:

`git clone https://github.com/lhw2002426/rux_loopback.git -b unix ./apps/c/unixsocket`

`make A=apps/c/unixsocket ARCH=aarch64 LOG=info SMP=1 run NET=y V9P=y MUSL=y V9P_PATH=./apps/c/unixsocket/rootfs`
