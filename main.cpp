#include "gui/testerwindow.h"
#include <QApplication>
#include "core/applicationmanager.h"
#include "core/logger.h"
#include <QMessageBox>
#include <windows.h> // для Windo

LONG WINAPI MyUnhandledExceptionFilter(struct _EXCEPTION_POINTERS* exceptionInfo)
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

    Logger::error("Необработанное исключение Windows: " + errorMsg);

    // Выводим адрес исключения
    Logger::error(QString("Адрес исключения: 0x%1").arg((quintptr)exceptionInfo->ExceptionRecord->ExceptionAddress, 8, 16, QChar('0')));

    // Показываем сообщение пользователю
    QMessageBox::critical(nullptr, "Критическая ошибка",
                          QString("Произошла критическая ошибка:\n%1\n\nПрограмма будет закрыта.")
                              .arg(errorMsg));

    return EXCEPTION_EXECUTE_HANDLER;
}


int main(int argc, char *argv[])
{
    // Установка обработчика необработанных исключений для Windows
     SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);


    qDebug("START");
    QApplication a(argc, argv);
    // Инициализация логгера
    Logger::init();
    LOG_INFO("[MAIN] Запуск приложения...");

    try {
        // Инициализируем менеджер
        ApplicationManager& manager = ApplicationManager::instance();

        if (!manager.initialize()) {
            LOG_ERROR("[MAIN] Не удалось инициализировать приложение");
            return 1;
        }

        // Показываем главное окно
        manager.mainWindow()->show();

        LOG_INFO("[MAIN] Приложение запущено");

        return a.exec();

    } catch (const std::exception& e) {
        LOG_ERROR(QString("[MAIN] Критическая ошибка: %1").arg(e.what()));
        QMessageBox::critical(nullptr, "Критическая ошибка",
                              QString("Ошибка: %1").arg(e.what()));
        return 1;
    }
}
