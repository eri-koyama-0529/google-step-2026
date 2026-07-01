# week7 assigment

## 結果一覧

| Challenge | 評価項目 | simple_malloc | my_malloc (BestFitのみ) | my_malloc (BestFit+FreeLitBin) | my_malloc (さらに右結合) |
| :--- | :--- | :---: | :---: | :---: | :---: |
| **Challenge #1** | Time [ms] | 13 | 1381 | 512 | 501 |
| | Utilization [%] | 70 | 70 | 70 | 55 |
| **Challenge #2** | Time [ms] | 17 | 537 | 229 | 191 |
| | Utilization [%] | 40 | 39 | 39 | 27 |
| **Challenge #3** | Time [ms] | 139 | 796 | 115 | 232 |
| | Utilization [%] | 8 | 51 | 51 | 40 |
| **Challenge #4** | Time [ms] | 35370 | 11270 | 244 | 693 |
| | Utilization [%] | 16 | 72 | 72 | 75 |
| **Challenge #5** | Time [ms] | 18478 | 7200 | 239 | 591 |
| | Utilization [%] | 15 | 72 | 72 | 74 |


## 目次
* [1. Best Fitにするために](https://github.com/eri-koyama-0529/google-step-2026/tree/main/week7#best-fit%E3%81%AB%E3%81%99%E3%82%8B%E3%81%9F%E3%82%81%E3%81%AB)
* [2. Freelist binの実装](https://github.com/eri-koyama-0529/google-step-2026/tree/main/week7#freelist-bin%E3%81%AE%E5%AE%9F%E8%A3%85)


## Best Fitにするために

1. どのスロットが最終的な対象となるか格納する変数`best`を用意する。この変数は各時点でサイズより大きいスロットの中で最も小さいサイズのスロットの情報が格納されている。
2. 各空いているスロットに対して、
    - ①sizeより大きいか
    - ②もし大きければ、現在の`best`のサイズと比較し、その`best`より小さければ、更新する。

```C
  my_metadata_t *best_srot = NULL; // 暫定で一番十分な大きさの空き領域のうち最も小さいものを格納する
  size_t min_best_size = SIZE_MAX; // 暫定のサイズの初期値は無限大にする
  my_metadata_t *best_prev = NULL; //best_srotの一つ前を格納する

  while (metadata) { // metadataがある間繰り返す
    // 十分な大きさがある かつ 暫定のsrotよりサイズが小さいとき更新
    if (size <= metadata->size && metadata->size < min_best_size){
      min_best_size = metadata->size;
      best_srot = metadata;
      best_prev = prev;
    }
    prev = metadata; // 一つ前を保存
    metadata = metadata->next; //次のsrotへ
  }
  prev = best_prev;
  metadata = best_srot;
```

#### 予想

- 【Speed】: 遅くなる
    - 毎回全ての空いているスロットを確認する必要があるため。
- 【Utilization】:良くなる
    - First Fitでは、対象のサイズより大きすぎるスロットであっても入れていくため、大きいサイズが入らなくなる可能性が高い。一方、Best Fitにすることで空いているサイズの合計がサイズより大きいのに入らないケースが少なるなることでUtilizationが改善されると考えられる。

#### 結果
```
====================================================
Challenge #1    |   simple_malloc =>       my_malloc
--------------- + --------------- => ---------------
       Time [ms]|              13 =>            1381
Utilization [%] |              70 =>              70
====================================================
Challenge #2    |   simple_malloc =>       my_malloc
--------------- + --------------- => ---------------
       Time [ms]|              17 =>             537
Utilization [%] |              40 =>              39
====================================================
Challenge #3    |   simple_malloc =>       my_malloc
--------------- + --------------- => ---------------
       Time [ms]|             139 =>             796
Utilization [%] |               8 =>              51
====================================================
Challenge #4    |   simple_malloc =>       my_malloc
--------------- + --------------- => ---------------
       Time [ms]|           35370 =>           11270
Utilization [%] |              16 =>              72
====================================================
Challenge #5    |   simple_malloc =>       my_malloc
--------------- + --------------- => ---------------
       Time [ms]|           18478 =>            7200
Utilization [%] |              15 =>              72
```

## Freelist binの実装

- まず、mallocのサイズの分布を調べてみると以下の通りになる。

![Mallocの配置分布](malloc_challenge/malloc/malloc_distribution_detailed.png)

```
===============Challenge #1===============
min_size:128
max_size:128
===============Challenge #2===============
min_size:16
max_size:16
===============Challenge #3===============
min_size:16
max_size:128
===============Challenge #4===============
min_size:256
max_size:4000
===============Challenge #5===============
min_size:8
max_size:4000
```

- Challenge1~3は同じサイズに集中しているのでFreeListBinの効果が薄い。そのため、Challenge4，5のような分布に対応するようにFreeListBinを作成する。
- Challenge4,5は、大まかに左側（サイズが小さい部分）に大きな山があり、右側（サイズが大きくなる）につれて、小さくなるロングテール型をしている。  

#### 方針1

1. Binを2^nごとに分ける（n:3~12の10分割）
    - サイズが小さい時には細かく、大きくなるにつれてスパンを大きくする。
2. もし対象のBinに空きが無ければ、次の大きさのBinを探す。

#### 結果

- Utilizationはあまり変わらず、Timeが縮んだ。

```txt
====================================================
Challenge #1    |   simple_malloc =>       my_malloc
--------------- + --------------- => ---------------
       Time [ms]|               7 =>             512
Utilization [%] |              70 =>              70
====================================================
Challenge #2    |   simple_malloc =>       my_malloc
--------------- + --------------- => ---------------
       Time [ms]|               5 =>             229
Utilization [%] |              40 =>              39
====================================================
Challenge #3    |   simple_malloc =>       my_malloc
--------------- + --------------- => ---------------
       Time [ms]|              65 =>             115
Utilization [%] |               8 =>              51
====================================================
Challenge #4    |   simple_malloc =>       my_malloc
--------------- + --------------- => ---------------
       Time [ms]|           11071 =>             244
Utilization [%] |              16 =>              72
====================================================
Challenge #5    |   simple_malloc =>       my_malloc
--------------- + --------------- => ---------------
       Time [ms]|            5148 =>             239
Utilization [%] |              15 =>              72
```