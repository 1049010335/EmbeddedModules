# 自用的嵌入式软件模块仓库

## 当前文件结构

**标*的为自己写的模块，其他为开源社区的模块。**

| [Algorithm](./algorithm) | 算法 |
|-------|-------|
| [cmsis_dsp](./algorithm/cmsis_dsp) | CMSIS-DSP [repo](https://github.com/ARM-software/CMSIS-DSP) |
| [libcrc](./algorithm/libcrc) | CRC计算库 [repo](https://github.com/whik/crc-lib-c) |
| [*pid](./algorithm/pid) | 通用PID控制器 |
| [Quaternions](./algorithm/quaternion) | 四元数和IMU姿态估计 [repo](https://github.com/rbv188/IMU-algorithm) |
| [tiny_regex](./algorithm/tiny_regex)|  简易正则解析器 [repo](https://github.com/zeta-zero/tiny-regex-c) |

| [Communication](./communication) | 通信 |
|-------|-------|
| [CherryUSB](./communication/cherryusb) | Cherry USB [repo](https://github.com/cherry-embedded/CherryUSB) |
| [lwpkt](./communication/lwpkt) | 轻量级数据包 [repo](https://github.com/MaJerle/lwpkt) |
| [modbus](./communication/modbus) | Modbus协议 [repo](https://github.com/wql7013/ModBus) |
| [TinyFrame](./communication/tinyframe) | 另一个轻量级数据包 [repo](https://github.com/MightyPork/TinyFrame) |

| [DataStruct](./datastruct) | 数据结构 |
|-------|-------|
| [btree](./datastruct/btree) | B树 [repo](https://github.com/tidwall/btree.c) |
| [cstring](./datastruct/cstring) | C字符串 [repo](https://github.com/cloudwu/cstring) |
| [hashmap](./datastruct/hashmap) | 哈希表 [repo](https://github.com/tidwall/hashmap.c) |
| [json](./datastruct/json) | JSON解析 [repo](https://github.com/tidwall/json.c) |
| [lfbb](./datastruct/lfbb) | 二分循环缓冲区 [repo](https://github.com/DNedic/lfbb) |
| [*lfifo](./datastruct/lfifo) | 通用环形缓冲区 |
| [lwrb](./datastruct/lwrb) | 轻量级环形缓冲区 [repo](https://github.com/MaJerle/lwrb) |
| [pqueue](./datastruct/pqueue) | 优先队列 [repo](https://github.com/tidwall/pqueue.c) |
| [*udict](./datastruct/udict) | 通用字典 |
| [*ulist](./datastruct/ulist) | 通用内存连续列表 |

| [Debug](./debug) | 调试 |
|-------|-------|
| [benchmark](./debug/benchmark) | CoreMark基准测试 [repo](https://github.com/eembc/coremark) |
| [cm_backtrace](./debug/cm_backtrace) | hardfault堆栈回溯 [repo](https://github.com/armink/CmBacktrace) |
| [RTT](./debug/rtt) | Segger-RTT 调试模块 [repo](https://www.segger.com/products/debug-probes/j-link/technology/about-real-time-transfer/) |
| [*log.h](./debug/log.h) | 轻量级日志 |

| [Graphics](./graphics) | 图形 |
|-------|-------|
| [lvgl](./graphics/lvgl) | LittlevGL图形库 [repo](https://github.com/lvgl/lvgl) |
| [lvgl/manager/lvgl-pm](./graphics/lvgl/manager/lvgl-pm) | LVGL通用页面管理器 [repo](https://github.com/LanFly/lvgl-pm) |
| [lvgl/manager/page_manager](./graphics/lvgl/manager/page_manager) | 基于X-TRACK项目移植的页面管理器 [repo](https://github.com/geekheart/page_manager) |
| [hagl](./graphics/hagl) | HAL图形库 [repo](https://github.com/tuupola/hagl) |
| [*virtual_lcd](./graphics/virtual_lcd) | 虚拟LCD |

| [Peripheral](./peripheral) | 外设 |
|-------|-------|
| [*board_i2c](./peripheral/board_i2c) | 通用I2C包装层 |
| [*i2c_salve](./peripheral/i2c_slave) | LL库I2C从机 |
| [*key](./peripheral/key) | 通用按键 |
| [*led](./peripheral/led) | 通用LED |
| [ll_i2c](./peripheral/ll_i2c) | LL库I2C [repo](https://github.com/jesstr/i2c_ll) |
| [*motor](./peripheral/motor) | 直流电机闭环驱动 |
| [*stepper](./peripheral/stepper) | 步进电机驱动 |
| [sw_i2c](./peripheral/sw_i2c) | 软件I2C [repo](https://github.com/liyanboy74/soft-i2c) |
| [sw_spi](./peripheral/sw_spi) | 软件SPI [unknown] |
| [*uart_pack](./peripheral/uart_pack) | 串口操作功能包 |
| [*ws2812_spi](./peripheral/ws2812_spi) | WS2812灯带DMA-SPI驱动 |

| [Storage](./storage) | 存储 |
|-------|-------|
| [littlefs](./storage/littlefs) | LittleFS [repo](https://github.com/littlefs-project/littlefs) |
| [MiniFlashDB](./storage/miniflashdb) | 轻量级Flash数据库 [repo](https://github.com/Jiu-xiao/MiniFlashDB) |

| [System](./system) | 系统 |
|-------|-------|
| [dalloc](./system/dalloc) | 动态指针管理内存分配器 [repo](https://github.com/SkyEng1neering/dalloc) |
| [heap_4](./system/heap_4) | FreeRTOS堆4 [freeRTOS](https://www.freertos.org/a00111.html) |
| [klite](./system/klite) | 基础实时内核 [repo](https://gitee.com/kerndev/klite) |
| [lwmem](./system/lwmem) | 轻量级内存管理 [repo](https://github.com/MaJerle/lwmem) |
| [s_task](./system/s_task) | 精简的协程实现 [repo](https://github.com/xhawk18/s_task) |
| [*scheduler](./system/scheduler) | 多功能任务调度器 |
| [*scheduler_lite](./system/scheduler_lite) | 轻量级任务调度器 |

| [Utility](./utility) | 工具 |
|-------|-------|
| [embedded_cli](./utility/embedded_cli) | 嵌入式命令行 [repo](https://github.com/funbiscuit/embedded-cli) |
| [lwprintf](./utility/lwprintf) | 轻量级无缓冲区printf [repo](https://github.com/MaJerle/lwprintf) |
| [minmea](./utility/minmea) | NMEA解析器 [repo](https://github.com/ata4/minema) |
| [perf_counter](./utility/perf_counter) | PerfCounter性能统计/时基库 [repo](https://github.com/GorgonMeducer/perf_counter) |
| [ryu](./utility/ryu) | 浮点数转字符串 [repo](https://github.com/tidwall/ryu) |
| [*term_table](./utility/term_table) | 动态终端表格工具 |
| [xv](./utility/xv) | 类JavaScript的字符串解析器 [repo](https://github.com/tidwall/xv) |
| [incbin.h](./utility/incbin.h) | 二进制文件嵌入 [repo](https://github.com/graphitemaster/incbin) |
| [*macro.h](./utility/macro.h) | 通用宏 |

[create_new_module.py](./create_new_module.py) 新模块创建脚本

[modules_conf.template.h](./modules_conf.template.h) 模块统一配置文件模板

[modules.h](./modules.h) 模块统一头文件

[modules.kconfig](./modules.kconfig) `[TODO]`模块统一Kconfig文件