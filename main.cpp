// main.cpp
#include "gui/testerwindow.h"
#include <QApplication>
#include <QDir>
#include <QResource>
#include "core/applicationmanager.h"
#include "core/logwrapper.h"
#include <QMessageBox>
#include <windows.h>
#include <QFile>
#include <iostream>

/*LONG WINAPI MyUnhandledExceptionFilter(struct _EXCEPTION_POINTERS* exceptionInfo)
{
    QString errorMsg;

    switch (exceptionInfo->ExceptionRecord->ExceptionCode) {
    case EXCEPTION_ACCESS_VIOLATION:
        errorMsg = "Нарушение доступа к памяти (Access Violation)";
        break;
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
        errorMsg = "Выход за границы массива";
        break;
    case EXCEPTION_STACK_OVERFLOW:
        errorMsg = "Переполнение стека";
        break;
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
        errorMsg = "Деление на ноль";
        break;
    default:
        errorMsg = QString("Код исключения: 0x%1").arg(exceptionInfo->ExceptionRecord->ExceptionCode, 8, 16, QChar('0'));
        break;
    }

    // Используем новую систему логирования
    // Используем прямую запись в файл на случай, если LogWrapper не инициализирован
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    QString logMessage = QString("[%1] [ERROR] [SYSTEM] Необработанное исключение Windows: %2")
                             .arg(timestamp).arg(errorMsg);

    // Пытаемся записать в файл напрямую
    QFile logFile("crash.log");
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream stream(&logFile);
        stream << logMessage << "\n";
        stream << QString("Адрес исключения: 0x%1").arg((quintptr)exceptionInfo->ExceptionRecord->ExceptionAddress, 8, 16, QChar('0')) << "\n";
        logFile.close();
    }

    // Также выводим в стандартный вывод
    qCritical() << logMessage;

    // Показываем сообщение пользователю
    QMessageBox::critical(nullptr, "Критическая ошибка",
                          QString("Произошла критическая ошибка:\n%1\n\nПрограмма будет закрыта.")
                              .arg(errorMsg));

    return EXCEPTION_EXECUTE_HANDLER;
}
*/

int main(int argc, char *argv[])
{
    // Установка обработчика необработанных исключений для Windows
   // SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);

    QApplication a(argc, argv);

    // ИНИЦИАЛИЗАЦИЯ ЛОГГЕРА - ТОЛЬКО ЗДЕСЬ!
    // Создаем экземпляр LogWrapper (он сам проинициализирует Logger)

    LogWrapper::instance();

    // Теперь можно логировать через новую систему
    LOG_CAT_INFO("MAIN", "Запуск приложения...");

    try {
        // Инициализируем менеджер
        ApplicationManager& manager = ApplicationManager::instance();

        if (!manager.initialize()) {
            LOG_CAT_ERROR("MAIN", "Не удалось инициализировать приложение");
            return 1;
        }

        QObject::connect(&a, &QApplication::aboutToQuit, [&manager]() {
            LOG_CAT_INFO("MAIN", "Завершение работы приложения...");
            manager.shutdown();
        });

        // Показываем главное окно
        manager.mainWindow()->show();

        LOG_CAT_INFO("MAIN", "Приложение запущено");

        return a.exec();

    } catch (const std::exception& e) {
        LOG_CAT_ERROR("MAIN", QString("Критическая ошибка: %1").arg(e.what()));
        QMessageBox::critical(nullptr, "Критическая ошибка",
                              QString("Ошибка: %1").arg(e.what()));
        return 1;
    } catch (...) {
        LOG_CAT_ERROR("MAIN", "Неизвестная критическая ошибка");
        QMessageBox::critical(nullptr, "Критическая ошибка",
                              "Неизвестная ошибка");
        return 1;
    }
}
