cmd_/home/caiyongheng/tina/out/astar-parrot/compile_dir/toolchain/linux-dev//include/linux/mmc/.install := bash scripts/headers_install.sh /home/caiyongheng/tina/out/astar-parrot/compile_dir/toolchain/linux-dev//include/linux/mmc ./include/uapi/linux/mmc ioctl.h; bash scripts/headers_install.sh /home/caiyongheng/tina/out/astar-parrot/compile_dir/toolchain/linux-dev//include/linux/mmc ./include/linux/mmc ; bash scripts/headers_install.sh /home/caiyongheng/tina/out/astar-parrot/compile_dir/toolchain/linux-dev//include/linux/mmc ./include/generated/uapi/linux/mmc ; for F in ; do echo "\#include <asm-generic/$$F>" > /home/caiyongheng/tina/out/astar-parrot/compile_dir/toolchain/linux-dev//include/linux/mmc/$$F; done; touch /home/caiyongheng/tina/out/astar-parrot/compile_dir/toolchain/linux-dev//include/linux/mmc/.install