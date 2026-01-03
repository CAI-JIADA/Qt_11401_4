# Calendar

Qt 多平台行事曆整合工具 - 整合 Google、Outlook 行事曆的待辦事項管理工具

## 專案說明

本專案使用 Qt 框架開發，可以將不同平台的行事曆待辦事項整合到統一的介面中。

### 主要功能

- 🔗 **多平台整合** - 支援 Google Calendar、Microsoft Outlook
- 📋 **待辦事項管理** - 整合事件與日程
- 👥 **共享行事曆** - 讀取他人分享的行事曆
- 🔍 **搜索功能** - 快速搜索所有事件

### 支援的平台

| 平台 | Google Calendar | Microsoft Outlook |
|------|----------------|-------------------|
| Windows | ✅ | ✅ |
| Linux | ✅ | ✅ |
| macOS | ✅ | ✅ |
| Android | ✅ | ✅ |
| iOS | ✅ | ✅ |


## 🚀 快速開始

#### 使用 Qt Creator 開啟專案

**使用 qmake (推薦用於 Qt Creator)**
   - 開啟 Qt Creator
   - 選擇 `檔案` > `開啟檔案或專案`
   - 選擇 `CalendarIntegration.pro` 檔案

## ⚠️ OAuth 認證問題？

如果您在登入 Google Calendar 或 Microsoft Outlook 時遇到以下錯誤：

- ❌ **錯誤 400: redirect_uri_mismatch** (Google)
- ❌ **invalid_request: redirect_uri is not valid** (Microsoft)

**🚨 需要立即修復？** 查看 [**快速修復指南 (QUICK_FIX.md)**](./QUICK_FIX.md)

**📖 需要詳細說明？** 查看 [**OAuth 認證問題排除指南 (OAUTH_TROUBLESHOOTING.md)**](./OAUTH_TROUBLESHOOTING.md)

這些指南提供了詳細的步驟說明，教您如何正確設定 Google Cloud Console 和 Azure AD 的重新導向 URI。

### 快速修復

**Google Calendar 錯誤：**
1. 前往 [Google Cloud Console](https://console.cloud.google.com/)
2. 在 OAuth 2.0 憑證中新增：`http://localhost:8080/`（**必須包含結尾斜線**）

**Microsoft Outlook 錯誤：**
1. 前往 [Azure Portal](https://portal.azure.com/)
2. 在應用程式驗證設定中新增：`http://localhost:8081/`（**必須包含結尾斜線**）

## 📚 文件

- [QUICK_FIX.md](./QUICK_FIX.md) - OAuth 錯誤快速修復（3 步驟解決）
- [OAUTH_TROUBLESHOOTING.md](./OAUTH_TROUBLESHOOTING.md) - OAuth 認證問題排除指南（完整說明）
- [TESTING.md](./TESTING.md) - 完整測試指南與設定說明





