# 生產者-消費者問題 - HW3 第 7 章

## 專案概述
本專案使用以下機制實作經典的生產者-消費者同步問題：
- **計數型 Semaphore**：`empty` 與 `full`，用來追蹤緩衝區槽位狀態
- **Mutex 鎖**：保護共享緩衝區存取
- **Pthreads**：建立與管理執行緒

## 主要元件

### 同步原語
- **`mutex`**：保護共享緩衝區的臨界區
- **`empty` semaphore**：追蹤空槽位數量（初始值為 BUFFER_SIZE）
- **`full` semaphore**：追蹤滿槽位數量（初始值為 0）

### 緩衝區實作
- 固定大小的循環佇列（BUFFER_SIZE = 5）
- `in`：下一個要插入資料的位置索引
- `out`：下一個要取出資料的位置索引

### 生產者執行緒演算法
```
for each item:
    wait(empty)           // 等待有空槽位
    lock(mutex)
    insert_item(item)
    unlock(mutex)
    post(full)            // 通知有新資料可消費
    sleep()               // 模擬生產時間
```

### 消費者執行緒演算法
```
for each item:
    wait(full)            // 等待有可用資料
    lock(mutex)
    item = remove_item()
    unlock(mutex)
    post(empty)           // 通知釋放出空槽位
    sleep()               // 模擬消費時間
```

## 參數設定

請在 `producer_consumer.c` 中調整以下常數：
- `BUFFER_SIZE`：循環緩衝區大小（預設：5）
- `NUM_PRODUCERS`：生產者執行緒數量（預設：2）
- `NUM_CONSUMERS`：消費者執行緒數量（預設：2）
- `NUM_ITEMS`：每個執行緒生產/消費的項目數（預設：10）

## 編譯方式

### Linux/Unix（GCC）
```bash
make
# 或
gcc -pthread -o producer_consumer producer_consumer.c -lpthread
```

### Windows（MinGW）
```bash
gcc -pthread -o producer_consumer.exe producer_consumer.c -lpthread
```

## 執行方式

```bash
make run
# 或
./producer_consumer
```

## 預期輸出
```
=== Producer-Consumer Problem ===
Buffer Size: 5
Producers: 2, Consumers: 2
Items per thread: 10

[Producer 0] Produced item: 0
[Consumer 0] Consumed item:  0
...
```

## 本程式展示的核心概念

1. **互斥（Mutual Exclusion）**：透過 mutex 確保同一時間只有一個執行緒可存取緩衝區
2. **同步（Synchronization）**：透過 semaphore 避免生產者寫爆緩衝區、消費者讀空緩衝區
3. **避免死結（Deadlock Prevention）**：依照正確的 wait/lock/unlock/post 順序操作
4. **生產者-消費者模型**：將生產與消費流程解耦，提高系統協作彈性

## 問題解法重點

本實作透過 semaphore 保證：
- 當緩衝區已滿時，生產者會等待（`empty` semaphore = 0）
- 當緩衝區為空時，消費者會等待（`full` semaphore = 0）
- 透過 mutex 保證緩衝區操作具原子性
- 避免 race condition 與 deadlock
