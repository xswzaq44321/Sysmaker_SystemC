# SystemC 專案說明

這是一個簡單的 SystemC 專案結構，使用 GNU Make 管理編譯流程。

---

## 📁 專案結構規則

- 所有 `.cpp` 原始碼請放在 `source/` 資料夾（可含子資料夾）
- 所有 `.h` 標頭檔請放在 `include/` 資料夾（可含子資料夾）
- 編譯後的 `.o` 檔與中間檔會放在 `build/` 資料夾中
- 最終執行檔會放在專案根目錄

---

## 🔧 編譯方式

請先設定 SystemC 安裝路徑：

```bash
export SYSTEMC_HOME=~/workspace/SystemC
```

然後執行：
```
make         # 編譯
make clean   # 清除中間檔與執行檔
```
執行檔預設名稱為 main。
