# 使用方法

## 配置

### 全局配置

所有的程序都需要读取配置文件，默认的配置文件是当前目录（当前终端中的目录）的 `config.toml` 文件（即代码里写的是 `./config.toml`），也可以通过 `-c other_config.toml` 或者 `--config other_config.toml` 来指定其它的配置。

配置文件格式如下

```toml
workspace = "/data/ribll2026"
trigger = "main"
jump_run = [4, 192, 1010]

[paths]
raw_xia0 = "/data/ribll2026/raw/xia0"
raw_xia1 = "/data/ribll2026/raw/xia1"
raw_xia2 = "/data/ribll2026/raw/xia2"
raw_vme = "/data/ribll2026/ore/vme"
ore = "ore"
grit = "grit"
grain = "grain"
ingot = "ingot"

[prefix]
raw_xia0 = "data"
raw_xia1 = "data"
raw_xia2 = "data"
raw_vme = "data"
```

`workspace` 用于指定本项目所有生成的 ROOT 文件的存储路径。

`trigger` 是重组事件时选用的触发，可以选择 `main` 或者 `t1`，简单理解 `main` 是所有带有主触发的事件（理论上是所有事件，但是主触发可能被漏记）；`t1` 则是 VME 获取有触发的事件，为什么叫 `t1` 不重要。选用`main`触发后生成的 ROOT 文件的文件名中不会有标签，但是选用 `t1` 触发后生成的 ROOT 文件的文件名中都带有 `_t1_` 标签。

`jump_run` 用于跳过有问题的 run，上述配置中跳过了 4、192、1010，具体需要跳过哪些需要查看 [run shift](https://docs.qq.com/sheet/DYmdLZ2V1U2Z3UG1X) 或者记录本。

`paths.raw_xia0` 用于指定 XIA0 机箱的原始数据（.bin 文件）的路径（该路径取决于拷贝数据后放到哪了），搭配 `prefix.raw_xia0` 指定原始数据文件名前缀（本次实验都是 data）。

`paths.raw_xia1`、`paths.raw_xia2` 以及 `prefix.raw_xia1`、`prefix.raw_xia2` 同理。

`paths.raw_vme` 是指定 VME 获取系统**转出来的 ROOT 文件**，而非原始的 ridf 文件，`prefix.raw_vme` 则是 ROOT 文件的前缀，本次实验保持 data 即可。

`paths.ore`、`paths.grit`、`paths.grain`、`paths.ingot` 都是本项目转出来的 ROOT 文件存储的路径，`paths.ingot` 是最终有用的，其它三个路径都是中间文件。该路径是配合 `workspace` 使用的，具体来说，上述配置最终生成的 ROOT 文件会存储到 `/data/ribll2026/ingot` 目录中（即`<workspace>/<ingot>`）。

### VME 阈值

VME 获取系统的 ADC 是带有 pedestal 的，本项目会在预处理阶段去除 pedestal，只保留有效事件。阈值通过 `vme_threshold.toml` 文件写入，下面展示其中一部分

```toml
tafcsi = [
        85.6, 70.1, 84.2, 70.3,
        77.8, 72.7, 78.7, 68.9,
        69.3, 71.7, 74.2, 66.4
]
[tafd0]
front = [
        141.8, 150.1, 127.1, 130.8,
        137.2, 163.7, 170.0, 148.5,
        160.4, 135.6, 154.8, 167.8,
        180.6, 176.6, 174.9, 161.9
]
back = [
        100.4, 72.0, 99.9, 74.1,
        102.2, 105.8, 79.5, 65.6
]
```

上面展示的是 TAFCsI （环形硅后面的楔形 CsI(Tl)）和 TAFD0 （环形硅 0）的阈值，低于该阈值的事件都会被卡掉。所有的序号都是默认从 0 开始，对应编号从 0 开始的探测器或者硅条。比如说，这里的 TAFCsI 数组里有 12 个阈值，就是按照 0-11 的序号，其中 0 和 1 对应 TAFD0，2 和 3 对应 TAFD1，如此类推，具体探测器对应关系参见实验记录本；而 TAFD0 分为 front 和 back 两个数组，分别对应正面硅条 0-15 和背面硅条 0-7。需要注意的是 T0 的 CsI(Tl) 一部分在 XIA，一部分在 VME 中，所以该文件中只需要填对应的 VME 中的 15 块 CsI(Tl) 的阈值即可。

## 运行

### 解码 XIA 数据

编译好后，在根目录运行 crush 目录中的文件，分别解码每一个 XIA 获取记录的探测器

```bash
./build/bin/crush/crush_beam <run>
./build/bin/crush/crush_ppac <run>
./build/bin/crush/crush_trigger <run>
./build/bin/crush/crush_t0d1 <run>
./build/bin/crush/crush_t0d2 <run>
./build/bin/crush/crush_t0d3 <run>
./build/bin/crush/crush_t0d4 <run>
./build/bin/crush/crush_others <run>
```

+ `crush_beam` 对应束流线上的两块塑闪
+ `crush_ppac` 对应 PPAC
+ `crush_trigger` 对应 XIA 中记录的触发
+ `crush_t0d1` 对应 T0D1，65um 的 DSSD
+ `crush_t0d2` 对应 T0D2，64*64 的 DSSD
+ `crush_t0d3` 对应 T0D3
+ `crush_t0d4` 对应 T0D4
+ `crush_others` 对应剩下的 T0CsI、T0S (T0 的 SSD)、T1SU（T1 上方的 SSD）、T1SD（T1 下方的 SSD）

另外，`<run>` 表示的是 run 编号，实际使用时替换成数字。

这些程序只需要 XIA 的原始数据即可运行，没有前置运行要求。生成的 ROOT 文件在 `<workspace>/<grit>` 目录中，每个探测器保存在独立的文件中。

### 映射 VME 数据

由于 VME 的数据已经转成 ROOT 文件了，只需要映射到不同的探测器

```bash
./build/bin/crush_vme <run>
```

其中，`<run>` 需要替换为 **VME** 获取的 run 编号，某些情况下不等同于 XIA 获取的 run 编号，详情参见 run shift。

这一步需要 VME 已经转好的 ROOT 文件，同时会根据 `vme_threshold.toml` 扔掉低于阈值的事件，输出的文件在 `<workspace>/<grit>` 中，文件名中带有 vme 标签（`_vme_`）。

### 对齐筛选

接下来将两套获取系统的事件对齐，在 XIA 和 VME 的 run 序号不同时，分别指定

```bash
./build/bin/sift -x <xia_run> -v <vme_run>
```

而在 run 序号相同时，只指定一个 run 序号也是可以的

```bash
./build/bin/sift -r <run>
```

其中 `-x` 指定 XIA 的 run 序号，`-v` 指定 VME 的 run 序号，`-r` 同时指定两者（仅在 XIA  和 VME run 序号相同时可用）。

这一步需要前两步（*解码 XIA 数据*和 *映射 VME 数据*）作为前置，根据触发的时间戳将两套系统的事件对齐，输出的 ROOT 文件在 `<workspace>/<grain>` 目录中。

### 重组熔炼

最后是根据对齐后的触发（目前是 main 或者 t1），根据时间戳或者事件序号将各个探测器的事件重组，重组成各个探测器符合的事件。首先是准备触发

```bash
./build/bin/coke -r <run>
```

然后是处理各个探测器

```bash
./build/bin/smelt -r <run> ppac t0d2 t0csi
./build/bin/smelt -r <run> -t main ppac t0d2 t0csi
./buil/bin/smelt -r <run> -t t1 ppac t0d2 t0csi
```

`coke` 根据上一步对齐的结果，分别根据 main 触发和 t1 触发重新组织多个通道的触发数据，输出到 `<worksapce>/<ingot>` 中，用于后续其它探测器的重组。

main 触发是主触发事件，理论上包含所有的事件，转出来的文件很大，运行相对较慢，且其中有大量的束流事件。t1 触发是筛选了 VME 中有超过阈值的靶后探测器事件，数据量小得多，适用于仅分析与 TAF 或者 T1 有符合的事件，并且其中的未反应束流事件被大幅压低，也适用于归一 T0 硅和卡 PID。

`smelt` 则是根据选用的触发逐个重组探测器事件，上述示例中仅重组了 PPAC、T0D2、T0CsI 的事件，实际处理时按需添加探测器。各个探测器的名称见下表。重组后的事件都输出到 `<workspace>/<ingot>` 中。比如说选用 main 触发时，PPAC 的输出文件名为 `ppac_0123.root`，而选用 t1 触发时，输出的文件名为 `ppac_t1_0123.root`。

仔细观察上述示例，第一行没有显示指定触发，使用默认配置文件中的默认触发（即 `config.toml` 的 `trigger` 条目）；第二行通过 `-t main` 指定 main 触发；第三行通过 `-t t1` 指定 t1 触发。

| 名称   | 描述                           |
| ------ | ------------------------------ |
| beam   | 束流线上的 2 块塑闪            |
| ppac   | 靶前的 3 块 PPAC               |
| t0d1   | T0 第一块 DSSD，65um           |
| t0d2   | T0 第二块 DSSD，64 $\times$ 64 |
| t0d3   | T0 第三块 DSSD，32 $\times$ 32 |
| t0d4   | T0 第四块 DSSD                 |
| t0s    | T0 的 SSD                      |
| t0csi  | T0 的 CsI(Tl)                  |
| tafd   | 6 块环形硅                     |
| tafcsi | 12 块楔形碘化铯，在环形硅后面  |
| t1dd   | T1 下面的 DSSD                 |
| t1sd   | T1 下面的 SSD                  |
| t1cisd | T1 下面的 CsI(Tl) 和 GAGG      |
| t1du   | T1 上面的 DSSD                 |
| t1su   | T1 上面的 SSD                  |
| t1csiu | T1 上面的 CsI(Tl)              |

## 脚本

为了方便，可以直接跑 `scripts/forge.sh` 脚本，该脚本囊括了上述预处理过程，以及大部分的探测器。

```bash
bash scripts/forge.sh <run>
```

使用时需要输入 run 编号，该脚本**默认 XIA 和 VME 的 run 编号相同**，实际情况有出入时需要自行修改脚本或者不用脚本。
