// logging_unified.h
#ifndef LOGGING_UNIFIED_H
#define LOGGING_UNIFIED_H

#include "../logwrapper.h"

// ==================== УНИВЕРСАЛЬНЫЕ МАКРОСЫ С МАППИНГОМ ====================

// Вспомогательная функция для маппинга старых категорий на новые
namespace LogCategoryMapper {
static QString mapLegacyToNew(const QString& legacyCategory) {
    if (legacyCategory.startsWith("[") && legacyCategory.endsWith("]")) {
        QString inner = legacyCategory.mid(1, legacyCategory.length() - 2);

        // Маппинг специфичных категорий
        if (inner == "APPLICATION") return "SYSTEM";
        if (inner == "UDP") return "TECH_NETWORK";
        if (inner == "Engine") return "TECH_STATE";
        if (inner == "Comm") return "TECH_PROTOCOL";
        if (inner == "Controller") return "TECH_STATE";
        if (inner == "UI") return "UI_STATUS";
        if (inner == "Command") return "UI_OPERATION";
        if (inner == "PPBcom") return "TECH_PROTOCOL";

        // Если не нашли маппинг, используем как есть (без скобок)
        return inner;
    }

    // Для категорий без скобок
    if (legacyCategory == "UDP") return "TECH_NETWORK";
    if (legacyCategory == "Engine") return "TECH_STATE";
    if (legacyCategory == "Comm") return "TECH_PROTOCOL";
    if (legacyCategory == "Controller") return "TECH_STATE";
    if (legacyCategory == "UI") return "UI_STATUS";
    if (legacyCategory == "Command") return "UI_OPERATION";
    if (legacyCategory == "PPBcom") return "TECH_PROTOCOL";

    // Оставляем как есть
    return legacyCategory;
}
// Дополнительные удобные функции
static bool isEngineerCategory(const QString& category) {
    return category.startsWith("UI_") ||
           category == "SYSTEM" ||
           category == "GENERAL";
}

static bool isTechnicalCategory(const QString& category) {
    return category.startsWith("TECH_");
}

static bool isOperationalCategory(const QString& category) {
    return category.startsWith("OP_");
}
}

// ==================== СТАРЫЕ МАКРОСЫ С АВТОМАТИЧЕСКИМ МАППИНГОМ ====================

// Простые макросы без категории
#define LOG_DEBUG(msg)            LogWrapper::debug(LogCategoryMapper::mapLegacyToNew("GENERAL"), msg)
#define LOG_INFO(msg)             LogWrapper::info(LogCategoryMapper::mapLegacyToNew("GENERAL"), msg)
#define LOG_WARNING(msg)          LogWrapper::warning(LogCategoryMapper::mapLegacyToNew("GENERAL"), msg)
#define LOG_ERROR(msg)            LogWrapper::error(LogCategoryMapper::mapLegacyToNew("GENERAL"), msg)

// Макросы с категорией (автоматический маппинг)
#define LOG_CAT_DEBUG(cat, msg)   LogWrapper::debug(LogCategoryMapper::mapLegacyToNew(cat), msg)
#define LOG_CAT_INFO(cat, msg)    LogWrapper::info(LogCategoryMapper::mapLegacyToNew(cat), msg)
#define LOG_CAT_WARNING(cat, msg) LogWrapper::warning(LogCategoryMapper::mapLegacyToNew(cat), msg)
#define LOG_CAT_ERROR(cat, msg)   LogWrapper::error(LogCategoryMapper::mapLegacyToNew(cat), msg)

// Макросы с автоматическим определением файла
#define LOG_DEBUG_AUTO(msg)       LogWrapper::debug(LogCategoryMapper::mapLegacyToNew(QString(__FILE__)), msg)
#define LOG_INFO_AUTO(msg)        LogWrapper::info(LogCategoryMapper::mapLegacyToNew(QString(__FILE__)), msg)

// ==================== НОВЫЕ МАКРОСЫ ДЛЯ КАРТЫ ЛОГОВ ====================

// Инженерные логи (для UI)
#define LOG_UI_STATUS(message)        LogWrapper::structuredLog("INFO", "UI_STATUS", message)
#define LOG_UI_OPERATION(message)     LogWrapper::structuredLog("INFO", "UI_OPERATION", message)
#define LOG_UI_RESULT(message)        LogWrapper::structuredLog("INFO", "UI_RESULT", message)
#define LOG_UI_ALERT(message)         LogWrapper::structuredLog("WARNING", "UI_ALERT", message)
#define LOG_UI_DATA(message)          LogWrapper::structuredLog("INFO", "UI_DATA", message)
#define LOG_UI_CONNECTION(message)    LogWrapper::structuredLog("INFO", "UI_CONNECTION", message)

// Технические логи (для программистов)
#define LOG_TECH_DEBUG(message)       LogWrapper::techLog("DEBUG", "TECH_DEBUG", message)
#define LOG_TECH_NETWORK(message)     LogWrapper::techLog("INFO", "TECH_NETWORK", message)
#define LOG_TECH_PROTOCOL(message)    LogWrapper::techLog("DEBUG", "TECH_PROTOCOL", message)
#define LOG_TECH_THREAD(message)      LogWrapper::techLog("INFO", "TECH_THREAD", message)
#define LOG_TECH_PERFORMANCE(message) LogWrapper::techLog("INFO", "TECH_PERFORMANCE", message)
#define LOG_TECH_MEMORY(message)      LogWrapper::techLog("INFO", "TECH_MEMORY", message)
#define LOG_TECH_STATE(message)       LogWrapper::techLog("INFO", "TECH_STATE", message)

// Операционные логи (для отчётности)
#define LOG_OP_SESSION(message)       LogWrapper::structuredLog("INFO", "OP_SESSION", message)
#define LOG_OP_OPERATION(message)     LogWrapper::structuredLog("INFO", "OP_OPERATION", message)
#define LOG_OP_MEASUREMENT(message)   LogWrapper::structuredLog("INFO", "OP_MEASUREMENT", message)
#define LOG_OP_EVENT(message)         LogWrapper::structuredLog("INFO", "OP_EVENT", message)
#define LOG_OP_SUMMARY(message)       LogWrapper::structuredLog("INFO", "OP_SUMMARY", message)

// Общие логи
#define LOG_SYSTEM(message)           LogWrapper::structuredLog("INFO", "SYSTEM", message)
#define LOG_GENERAL(message)          LogWrapper::structuredLog("INFO", "GENERAL", message)

// Структурированные данные
#define LOG_UI_TABLE(data)            LogWrapper::logTable(data)
#define LOG_UI_CARD(data)             LogWrapper::logCard(data)
#define LOG_UI_PROGRESS(data)         LogWrapper::logProgress(data)

// ==================== УДОБНЫЕ АЛИАСЫ ДЛЯ СТАРЫХ ВЫЗОВОВ ====================

// Старые категории (для обратной совместимости)
#define LOG_UDP(msg)                 LOG_CAT_INFO("TECH_NETWORK", msg)
#define LOG_ENGINE(msg)              LOG_CAT_INFO("TECH_STATE", msg)
#define LOG_COMM(msg)                LOG_CAT_INFO("TECH_PROTOCOL", msg)
#define LOG_CONTROLLER(msg)          LOG_CAT_INFO("TECH_STATE", msg)
#define LOG_UI(msg)                  LOG_CAT_INFO("UI_STATUS", msg)
#define LOG_COMMAND(msg)             LOG_CAT_INFO("UI_OPERATION", msg)

// Быстрые макросы с категориями и уровнями
#define LOG_UDP_INFO(msg)            LOG_CAT_INFO("TECH_NETWORK", msg)
#define LOG_UDP_DEBUG(msg)           LOG_CAT_DEBUG("TECH_NETWORK", msg)
#define LOG_UDP_ERROR(msg)           LOG_CAT_ERROR("TECH_NETWORK", msg)
#define LOG_UDP_WARNING(msg)         LOG_CAT_WARNING("TECH_NETWORK", msg)

#define LOG_ENGINE_INFO(msg)         LOG_CAT_INFO("TECH_STATE", msg)
#define LOG_ENGINE_DEBUG(msg)        LOG_CAT_DEBUG("TECH_STATE", msg)
#define LOG_ENGINE_ERROR(msg)        LOG_CAT_ERROR("TECH_STATE", msg)
#define LOG_ENGINE_WARNING(msg)      LOG_CAT_WARNING("TECH_STATE", msg)

#define LOG_CONTROLLER_INFO(msg)     LOG_CAT_INFO("TECH_STATE", msg)
#define LOG_CONTROLLER_DEBUG(msg)    LOG_CAT_DEBUG("TECH_STATE", msg)
#define LOG_CONTROLLER_ERROR(msg)    LOG_CAT_ERROR("TECH_STATE", msg)
#define LOG_CONTROLLER_WARNING(msg)  LOG_CAT_WARNING("TECH_STATE", msg)

#define LOG_PPBCOM_INFO(msg)         LOG_CAT_INFO("TECH_PROTOCOL", msg)
#define LOG_PPBCOM_DEBUG(msg)        LOG_CAT_DEBUG("TECH_PROTOCOL", msg)
#define LOG_PPBCOM_ERROR(msg)        LOG_CAT_ERROR("TECH_PROTOCOL", msg)
#define LOG_PPBCOM_WARNING(msg)      LOG_CAT_WARNING("TECH_PROTOCOL", msg)

// ==================== ДОПОЛНИТЕЛЬНЫЕ УДОБНЫЕ МАКРОСЫ ====================

// Из logging_all.h
#define LOG_ERROR_CAT(category, message)   LogWrapper::error(category, message)
#define LOG_WARN_CAT(category, message)    LogWrapper::warning(category, message)
#define LOG_INFO_CAT(category, message)    LogWrapper::info(category, message)
#define LOG_DEBUG_CAT(category, message)   LogWrapper::debug(category, message)

// Дополнительные удобные макросы для быстрого доступа
#define LOG_ERR_CAT(cat, msg)              LOG_ERROR_CAT(cat, msg)
#define LOG_WRN_CAT(cat, msg)              LOG_WARN_CAT(cat, msg)
#define LOG_INF_CAT(cat, msg)              LOG_INFO_CAT(cat, msg)
#define LOG_DBG_CAT(cat, msg)              LOG_DEBUG_CAT(cat, msg)

// Макросы для проверки типа категории
#define LOG_IF_UI(cat, msg)                if (LogCategoryMapper::isEngineerCategory(cat)) LogWrapper::info(cat, msg)
#define LOG_IF_TECH(cat, msg)              if (LogCategoryMapper::isTechnicalCategory(cat)) LogWrapper::info(cat, msg)
#define LOG_IF_OP(cat, msg)                if (LogCategoryMapper::isOperationalCategory(cat)) LogWrapper::info(cat, msg)

#endif // LOGGING_UNIFIED_H
