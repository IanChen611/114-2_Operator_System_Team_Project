# Banker's Algorithm — 使用說明

## 編譯

```powershell
gcc -Wall -Wextra -std=c11 -o banker.exe banker.c
```

或直接使用 Makefile：

```powershell
make
```

---

## 指令格式

```
banker.exe -n <processes> -m <resources>
           -avail  <r0> <r1> ...
           -alloc  <p0r0> <p0r1> ... <p1r0> <p1r1> ...
           -max    <p0r0> <p0r1> ... <p1r0> <p1r1> ...
          [-request <pid> <r0> <r1> ...]
```

### 參數說明

| 參數 | 說明 |
|---|---|
| `-n <processes>` | Process 的數量（最多 10） |
| `-m <resources>` | Resource 的種類數（最多 10） |
| `-avail` | 目前可用資源向量，共 `m` 個值 |
| `-alloc` | Allocation 矩陣，共 `n × m` 個值，依 Row-major 順序輸入（先 P0 的所有資源，再 P1，以此類推） |
| `-max` | Maximum Demand 矩陣，共 `n × m` 個值，輸入順序同 `-alloc` |
| `-request` | （選填）模擬資源請求：`<pid>` 為發出請求的 Process 編號，後接 `m` 個請求量 |
| `-h` / `--help` | 顯示使用說明 |

> **注意**：`-n` 與 `-m` 必須在 `-avail`、`-alloc`、`-max`、`-request` 之前指定。

---

## 無參數執行（預設值）

直接執行不帶任何參數時，程式會自動載入教科書範例（Silberschatz 10e §8.6，5 個 Process、3 種 Resource）：

```powershell
./banker.exe
```

**範例輸出：**

```
[No arguments given — loading textbook default example]

=== Banker's Algorithm ===

Processes: 5   Resource Types: 3

Allocation:
     R0  R1  R2
P0   0   1   0
P1   2   0   0
P2   3   0   2
P3   2   1   1
P4   0   0   2

Max:
     R0  R1  R2
P0   7   5   3
P1   3   2   2
P2   9   0   2
P3   2   2   2
P4   4   3   3

Need:
     R0  R1  R2
P0   7   4   3
P1   1   2   2
P2   6   0   0
P3   0   1   1
P4   4   3   1

Available:
     R0  R1  R2
     3   3   2

--- Safety Check ---
System is in a SAFE state.
Safe sequence: P1 -> P3 -> P4 -> P0 -> P2
```

---

## 帶參數執行（含 Resource Request）

以下範例與教科書相同，額外模擬 P1 請求 `(1, 0, 2)`：

```powershell
./banker.exe -n 5 -m 3 -avail 3 3 2 -alloc 0 1 0  2 0 0  3 0 2  2 1 1  0 0 2 -max 7 5 3  3 2 2  9 0 2  2 2 2  4 3 3 -request 1 1 0 2
```

**範例輸出：**

```
=== Banker's Algorithm ===

Processes: 5   Resource Types: 3

Allocation:
     R0  R1  R2
P0   0   1   0
P1   2   0   0
P2   3   0   2
P3   2   1   1
P4   0   0   2

Max:
     R0  R1  R2
P0   7   5   3
P1   3   2   2
P2   9   0   2
P3   2   2   2
P4   4   3   3

Need:
     R0  R1  R2
P0   7   4   3
P1   1   2   2
P2   6   0   0
P3   0   1   1
P4   4   3   1

Available:
     R0  R1  R2
     3   3   2

--- Safety Check ---
System is in a SAFE state.
Safe sequence: P1 -> P3 -> P4 -> P0 -> P2

--- Resource Request ---
P1 requests: [1, 0, 2]

GRANTED: system remains in a safe state.
Safe sequence: P1 -> P3 -> P4 -> P0 -> P2

Updated state after granting request:
Processes: 5   Resource Types: 3

Allocation:
     R0  R1  R2
P0   0   1   0
P1   3   0   2
P2   3   0   2
P3   2   1   1
P4   0   0   2

Max:
     R0  R1  R2
P0   7   5   3
P1   3   2   2
P2   9   0   2
P3   2   2   2
P4   4   3   3

Need:
     R0  R1  R2
P0   7   4   3
P1   0   2   0
P2   6   0   0
P3   0   1   1
P4   4   3   1

Available:
     R0  R1  R2
     2   3   0
```

---

## Request 拒絕情境

| 情境 | 程式回應 |
|---|---|
| 請求量超過該 Process 的最大聲明 | `DENIED: P<pid> exceeded its maximum claim on R<j>` |
| 請求量超過目前可用資源 | `WAIT: P<pid> must wait — insufficient resources on R<j>` |
| 配置後系統進入不安全狀態 | `DENIED: granting this request would lead to an unsafe state. Rolling back.` |
