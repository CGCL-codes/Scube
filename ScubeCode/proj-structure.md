# Scube consists of three components

1. Degree Estimator

2. Address Assigner

3. Kick-out

## Degree Estimator

这个模块主要负责对节点的度数进行预估。

1. 固定大小的slot数量
2. slot中保存高度点，根据bit vector预估出节点的度数信息
3. 设计decay strategy，只对低度点进行衰退，不对高度点衰退
4. 需要保存高度点扩展的地址数量信息

### 模块接口

1. 输入：edge stream
   + insert(s, d, w, t)

2. 输出：
   + estimationDegreeQuery(v)
   + addressQuery(v)

## Inserter

这个模块负责插入数据。
先找空位进行插入，若没有空位，则采用kick-out策略进行插入。

### kick-out strategy

备选地址生成逻辑不同，kick-out踢的逻辑不同

1. 线性同余法生成的备选地址，踢生成轮数最小的（即index最小）元素

2. 采用mod法生成的备选地址，踢生成地址序列靠前的元素
