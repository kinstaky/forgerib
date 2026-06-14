# 数据结构

本节介绍 `ingot` 目录中不同探测器 ROOT 文件的 TTree 的 branch。对于定长的事件，**必须通过检查 flag 或者 valid 来判断探测器是否有效（有信号）**（不保证所有的事件都有正确初始化的能量和时间，所以检查 flag 或者 valid 很重要）；而不定长的数组则用 num 来标记有几个有效信号。

## Trigger

文件名为 `trigger_xxxx.root`，branch 如下表。

| 名称             | 类型   | 缩写  | 描述                |
| ---------------- | ------ | ----- | ------------------- |
| flag             | int32  | flag  | 有效标记            |
| valid[6]         | bool   | valid | 有效标记            |
| time[6]          | double | time  | 相对时间            |
| external_time[6] | int64  | ets   | 外部时间戳          |
| vme_entry        | int64  | ve    | 对应的 VME 事件编号 |

一共记录了 6 种触发，每种触发是否有效用 flag 的前 6 位以及 valid 的 6 个元素标记，比如说 `valid[0]==true` 表示第一个触发（束流分除）是有效的，此时 `time[0]`、`external_time[0]` 才有意义。6 种触发具体的含义为

| 触发序号 | 描述             |
| -------- | ---------------- |
| 0        | 束流分除         |
| 1        | VME 触发         |
| 2        | T0D1 正面多举    |
| 3        | T0D2 证明多举    |
| 4        | T0S （大硅）单举 |
| 5        | XIA 主触发       |

valid、time、external_time 都是宽度为 6 的数组，每一个元素对应于一种触发的信息。而 vme_entry 无效时记为 -1。

## Beam

文件名位 `beam_xxxx.root`，branch 如下表

| 名称  | 类型   | 缩写  | 描述             |
| ----- | ------ | ----- | ---------------- |
| flag  | int32  | flag  | 有效标记         |
| valid | bool   | valid | 两个塑闪都有信号 |
| tof   | double | tof   | 两个塑闪的时间差 |

塑闪 T1、T2 是否有效由 flag 前两位标记，当且仅当 T1、T2 同时有效 valid 为真，即 `valid==true` 时两块塑闪都有信号，此时 tof 的值才有意义。tof 的值为 **T2 的时间减去 T1 的时间**。 

## PPAC

文件名为 `ppac_xxxx.root`，branch 如下表

| 名称       | 类型   | 缩写 | 描述     |
| ---------- | ------ | ---- | -------- |
| flag       | int32  | flag | 有效标记 |
| valid[15]  | bool   | v    | 有效标记 |
| time[15]   | double | t    | 时间     |
| energy[15] | int32  | e    | 能量     |

3 块 PPAC，没块都有 X1、X2、Y1、Y2、阴极 5 个通道，所以一共有 15 个通道，valid、time、energy 都是宽度 15 的数组，每个序号对应于

| 序号 | PPAC 序号 | PPAC 通道 |
| ---- | --------- | --------- |
| 0    | 1         | X1        |
| 1    | 1         | X2        |
| 2    | 1         | Y1        |
| 3    | 1         | Y2        |
| 4    | 2         | X1        |
| 5    | 2         | X2        |
| 6    | 2         | Y1        |
| 7    | 2         | Y2        |
| 8    | 3         | X1        |
| 9    | 3         | X2        |
| 10   | 3         | Y1        |
| 11   | 3         | Y2        |
| 12   | 1         | 阴极      |
| 13   | 2         | 阴极      |
| 14   | 3         | 阴极      |

# DSSD

包括 T0D1、T0D2、T0D3、T0D4、T1DD、T1DU，T0D1 的文件名为 `t0d1_xxxx.root`，如此类推。branch 如下表

| 名称                 | 类型   | 缩写 | 描述     |
| -------------------- | ------ | ---- | -------- |
| nfront               | int32  | fn   | 正面重数 |
| front_strip[nfront]  | int32  | fs   | 正面条   |
| front_energy[nfront] | double | fe   | 正面能量 |
| front_time[nfront]   | double | ft   | 正面时间 |
| nback                | int32  | bn   | 背面重数 |
| back_strip[nback]    | int32  | bs   | 背面条   |
| back_energy[nback]   | double | be   | 背面能量 |
| back_time[nback]     | double | bt   | 背面时间 |

对于 DSSD 使用的是变长数组，最大长度为 8，正背面分开，每一面都记录了条、能量、时间。

## SSD

包括 T0S、T1SD、T1SU，T0S 的文件名为 `t0s_xxxx.root`，如此类推。branch 如下表

| 名称   | 类型   | 缩写   | 描述     |
| ------ | ------ | ------ | -------- |
| valid  | bool   | valid  | 有效标记 |
| time   | double | time   | 时间     |
| energy | int32  | energy | 能量     |

尽在 `valid==true` 时 time 和 energy 才有意义，一块 SSD 只有一个通道，所以这里没有数组。

## CsI

包括 T0CsI、TAFCsI、T1CsID、T1CsIU，T0CsI 文件名为 `t0csi_xxxx.root`，如此类推。branch 如下表

| 名称      | 类型   | 缩写 | 描述     |
| --------- | ------ | ---- | -------- |
| flag      | uint64 | flag | 有效标记 |
| valid[N]  | bool   | v    | 有效标记 |
| time[N]   | double | t    | 时间     |
| energy[N] | int32  | e    | 能量     |

**N 是常数**，对于不同的 CsI 阵列有不同的取值，如下表

| 阵列   | N    |
| ------ | ---- |
| T0CsI  | 36   |
| TAFCsI | 12   |
| T1CsID | 13   |
| T1CsIU | 4    |

和前面的一样，flag 的前 N 位以及 valid 的 N 个元素分别标记每一块 CsI(Tl) 或者 GAGG 是否有信号，只有在 `valid[i]==true` 时，`energy[i]` 和 `time[i]` 才有意义。

## TAFD

文件名为 `tafd_xxxx.root`，branch 如下表

| 名称            | 类型   | 缩写 | 描述     |
| --------------- | ------ | ---- | -------- |
| flag            | int32  | flag | 有效标记 |
| valid           | bool   | v    | 有效标记 |
| front_energy[6] | double | fe   | 正面能量 |
| back_energy[6]  | double | be   | 背面能量 |
| front_strip[6]  | int32  | fs   | 正面条   |
| back_strip[6]   | int32  | bs   | 背面条   |
| front_time[6]   | double | ft   | 正面时间 |

flag的前 6 位和 valid 的 6 个元素标记对应的环形硅有否有信号，只有正背面都有超过阈值的信号才算有效。当且仅当 `valid[i]==true` 时，`front_xxx[i]` 和 `back_xxx[i]` 才有意义。这里的数组每一个元素表示一块环形硅（宽度为 6 对应 6 块环形硅），每块环形硅只有正面最大的、背面最大的信号被保留（环形硅条宽更大，条间事件极少，反应产生两个大角度粒子的事件也极少）。因为背面的时间没有输出到 V1190 中，所以没有记录。 