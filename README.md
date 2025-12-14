# Calender

Qt 多平台行事曆整合工具 - 整合 Google、Apple、Outlook 行事曆的待辦事項管理工具

## 專案說明

本專案使用 Qt 框架開發，可以將不同平台的行事曆待辦事項整合到統一的介面中。

### 主要功能

- 🔗 **多平台整合** - 支援 Google Calendar、Apple Calendar、Microsoft Outlook
- 📋 **待辦事項管理** - 整合事件與日程
- 👥 **共享行事曆** - 讀取他人分享的行事曆
- 🔍 **搜索功能** - 快速搜索所有事件
- 📱 **跨裝置支援** - 支援桌面與行動裝置

### ⭐ 特別說明：Apple Calendar 跨平台支援

**無需 Mac 環境！** 本專案透過標準 CalDAV 協議實作 Apple Calendar (iCloud) 整合，可在 **Windows、Linux、macOS** 等所有平台上使用。

#### 為什麼不需要 Mac？

- ✅ 使用開放標準 CalDAV 協議（RFC 4791）
- ✅ 透過 HTTPS 直接連接 iCloud 伺服器
- ✅ 僅需 Apple ID 和應用程式專用密碼
- ❌ 不需要 Mac 電腦
- ❌ 不需要 Apple Developer 帳號
- ❌ 不需要 Xcode 或 macOS

#### 支援的平台

| 平台 | Google Calendar | Apple Calendar | Microsoft Outlook |
|------|----------------|----------------|-------------------|
| Windows | ✅ | ✅ (CalDAV) | ✅ |
| Linux | ✅ | ✅ (CalDAV) | ✅ |
| macOS | ✅ | ✅ (CalDAV) | ✅ |
| Android | ✅ | ✅ (CalDAV) | ✅ |
| iOS | ✅ | ✅ (CalDAV) | ✅ |

### 快速開始

請參閱 [IMPLEMENTATION.md](IMPLEMENTATION.md) 了解完整的實作指南和技術需求。

特別查看以下章節：
- [Apple Calendar 跨平台整合](IMPLEMENTATION.md#2-apple-calendar-icloud) - 詳細的 CalDAV 實作說明
- [跨平台實作重點說明](IMPLEMENTATION.md#跨平台實作重點說明) - 完整的程式碼範例
- [常見問題](IMPLEMENTATION.md#常見問題-faq) - Apple Calendar 整合相關疑問

## 授權

MIT License