# 基于超声波测距传感器的单目视觉物体测量装置
0.2版本基本实现了在各种距离下数值恒定以达到物体测量的目的，但有巨大的问题就是只能测量特定数值的色块，如果第一个色块测量错误，之后的数值也会错误，之后的版本会考虑加入每一帧的优先参照色块，以确定真正的gpm数值大小