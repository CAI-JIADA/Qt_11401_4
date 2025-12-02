#ifndef PLATFORM_H
#define PLATFORM_H

#include <QString>

/**
 * @brief 平台類型列舉 - 使用 enum class 確保類型安全
 * 
 * 定義支援的行事曆平台類型
 */
enum class Platform {
    Google,     ///< Google Calendar
    Apple,      ///< Apple Calendar (iCloud)
    Outlook     ///< Microsoft Outlook
};

/**
 * @brief 將平台列舉轉換為字串
 * @param platform 平台類型
 * @return 平台名稱字串
 */
inline QString platformToString(Platform platform) {
    switch (platform) {
        case Platform::Google:
            return QStringLiteral("Google");
        case Platform::Apple:
            return QStringLiteral("Apple");
        case Platform::Outlook:
            return QStringLiteral("Outlook");
        default:
            return QStringLiteral("Unknown");
    }
}

/**
 * @brief 將字串轉換為平台列舉
 * @param str 平台名稱字串
 * @return 平台類型
 */
inline Platform stringToPlatform(const QString& str, bool* ok = nullptr) {
    if (ok) *ok = true;
    
    if (str == QStringLiteral("Google")) {
        return Platform::Google;
    } else if (str == QStringLiteral("Apple")) {
        return Platform::Apple;
    } else if (str == QStringLiteral("Outlook")) {
        return Platform::Outlook;
    }
    
    if (ok) *ok = false;
    return Platform::Google; // 預設值，如果需要檢查轉換是否成功，請使用 ok 參數
}

#endif // PLATFORM_H
